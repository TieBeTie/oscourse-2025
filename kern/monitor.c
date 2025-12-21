/* Simple command-line kernel monitor useful for
 * controlling the kernel and exploring the system interactively. */

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/env.h>
#include <inc/x86.h>
#include <inc/dwarf.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kclock.h>
#include <kern/kdebug.h>
#include <kern/tsc.h>
#include <kern/timer.h>
#include <kern/env.h>
#include <kern/trap.h>
#include <kern/pmap.h>

#define WHITESPACE "\t\r\n "
#define MAXARGS    16

/* Functions implementing monitor commands */
int mon_help(int argc, char **argv, struct Trapframe *tf);
int mon_kerninfo(int argc, char **argv, struct Trapframe *tf);
int mon_backtrace(int argc, char **argv, struct Trapframe *tf);
int mon_dumpcmos(int argc, char **argv, struct Trapframe *tf);
int mon_start(int argc, char **argv, struct Trapframe *tf);
int mon_stop(int argc, char **argv, struct Trapframe *tf);
int mon_frequency(int argc, char **argv, struct Trapframe *tf);
int mon_memory(int argc, char **argv, struct Trapframe *tf);
int mon_hello(int argc, char **argv, struct Trapframe *tf);
int mon_test_signed(int argc, char **argv, struct Trapframe *tf);
int mon_test_ptr(int argc, char **argv, struct Trapframe *tf);
int mon_test_string(int argc, char **argv, struct Trapframe *tf);
int mon_test_all(int argc, char **argv, struct Trapframe *tf);
int mon_quit(int argc, char **argv, struct Trapframe *tf);

struct Command {
    const char *name;
    const char *desc;
    /* return -1 to force monitor to exit */
    int (*func)(int argc, char **argv, struct Trapframe *tf);
};

static struct Command commands[] = {
        {"help", "Display this list of commands", mon_help},
        {"kerninfo", "Display information about the kernel", mon_kerninfo},
        {"backtrace", "Print stack backtrace", mon_backtrace},
        {"dumpcmos", "Display CMOS contents", mon_dumpcmos},
        {"timer_start", "Start timer", mon_start},
        {"timer_stop", "Stop timer", mon_stop},
        {"timer_freq", "Get timer frequency", mon_frequency},
        {"memory", "Display free memory pages", mon_memory},
        {"hello", "Print a greeting message", mon_hello},
        {"test_signed", "Test signed int parameter", mon_test_signed},
        {"test_ptr", "Test pointer parameter", mon_test_ptr},
        {"test_string", "Test string parameter", mon_test_string},
        {"test_all", "Run all parameter tests", mon_test_all},
        {"quit", "Exit monitor", mon_quit},
};
#define NCOMMANDS (sizeof(commands) / sizeof(commands[0]))

/* Implementations of basic kernel monitor commands */

int
mon_help(int argc, char **argv, struct Trapframe *tf) {
    for (size_t i = 0; i < NCOMMANDS; i++)
        cprintf("%s - %s\n", commands[i].name, commands[i].desc);
    return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf) {
    extern char _head64[], entry[], etext[], edata[], end[];

    cprintf("Special kernel symbols:\n");
    cprintf("  _head64 %16lx (virt)  %16lx (phys)\n", (unsigned long)_head64, (unsigned long)_head64);
    cprintf("  entry   %16lx (virt)  %16lx (phys)\n", (unsigned long)entry, (unsigned long)entry - KERN_BASE_ADDR);
    cprintf("  etext   %16lx (virt)  %16lx (phys)\n", (unsigned long)etext, (unsigned long)etext - KERN_BASE_ADDR);
    cprintf("  edata   %16lx (virt)  %16lx (phys)\n", (unsigned long)edata, (unsigned long)edata - KERN_BASE_ADDR);
    cprintf("  end     %16lx (virt)  %16lx (phys)\n", (unsigned long)end, (unsigned long)end - KERN_BASE_ADDR);
    cprintf("Kernel executable memory footprint: %luKB\n", (unsigned long)ROUNDUP(end - entry, 1024) / 1024);
    return 0;
}

static void print_param_value(const struct Dwarf_VarInfo *param, uintptr_t rbp_addr, int64_t offset);

/* From DWARF4 specification, Section 2.6 "Location Descriptions":
 * "Location descriptions describe how to access objects and compute values."
 * 
 * From System V x86-64 ABI, Section 3.2.2 "The Stack Frame":
 * "The stack grows downwards from high addresses. The end of the input argument area
 * shall be aligned on a 16 byte boundary."
 * 
 * Вычисляет адрес параметра на основе информации из DWARF location description.
 * Для параметров в регистрах (address < 0) возвращает 0, так как регистры вызывающей
 * функции недоступны в backtrace. Для параметров на стеке вычисляет адрес относительно
 * соответствующего frame pointer (rbp или next_rbp).
 */

static uintptr_t
calculate_param_address(int64_t dwarf_address, uintptr_t rbp, uintptr_t next_rbp) {
    /* From DWARF4 specification, Section 2.6.1.1.2 "Register Location Descriptions":
     * Используем диапазон < -1000 для обозначения регистров, чтобы отличить от смещений.
     */
    if (dwarf_address <= -1000) {
        /* Параметр в регистре: dwarf_address = -(reg_num + 1000) */
        return 0; /* Недоступно в backtrace */
    } else if (dwarf_address < 0) {
        /* From System V x86-64 ABI, Section 3.2.2 "The Stack Frame":
         * Отрицательное смещение от caller's RBP.
         * Параметры могут быть сохранены в caller's frame по отрицательным смещениям.
         */
        return next_rbp + dwarf_address; /* dwarf_address уже отрицательное */
    } else if (dwarf_address > 0) {
        /* Положительное смещение от caller's RBP (стандартные параметры на стеке) */
        return next_rbp + dwarf_address;
    } else {
        /* dwarf_address == 0 - нет информации */
        return 0;
    }
}

/* Updated print_single_parameter using new infrastructure */
static void
print_single_parameter(const struct Dwarf_VarInfo *param, uintptr_t rbp, uintptr_t next_rbp, int param_index) {
    if (strlen(param->name) == 0) {
        cprintf("%s=", param->type_name);
    } else {
        cprintf("%s %s=", param->type_name, param->name);
    }
    
    cprintf("\n[PARAM %d] name='%s', type='%s', kind=%d, address=%ld\n",
            param_index, param->name, param->type_name, param->kind, param->address);
    
    if (param->address == 0) {
        cprintf("[PARAM %d] No location information\n", param_index);
        cprintf("?");
        return;
    }
    
    uintptr_t param_addr = calculate_param_address(param->address, rbp, next_rbp);
    
    if (param_addr == 0) {
        cprintf("[PARAM %d] Parameter unavailable (in register or optimized out)\n", param_index);
        cprintf("?");
        return;
    }
    
    cprintf("[PARAM %d] Reading value from address 0x%lx\n", param_index, param_addr);
    print_param_value(param, param_addr, 0);
}


static void
print_function_parameters(const struct Ripdebuginfo *info, uintptr_t rbp, uintptr_t next_rbp) {
    if (info->rip_fn_narg == 0) {
        return;
    }
    
    for (int i = 0; i < info->rip_fn_narg; ++i) {
        const struct Dwarf_VarInfo *param = &info->rip_fn_params[i];
        print_single_parameter(param, rbp, next_rbp, i);
        
        if (i != info->rip_fn_narg - 1) {
            cprintf(", ");
        }
    }
}

static void
print_function_info(uintptr_t rip, const struct Ripdebuginfo *info, uintptr_t rbp, uintptr_t next_rbp) {
    cprintf("    0x%016lx in %.*s (", 
            rip - info->rip_fn_addr,
            info->rip_fn_namelen, info->rip_fn_name);
    
    print_function_parameters(info, rbp, next_rbp);
    
    cprintf(") at %s:%d\n", info->rip_file, info->rip_line);
}
/* From DWARF4 specification, Section 5.1 "Base Type Entries":
 * "A base type entry has a DW_AT_byte_size attribute whose value is a constant
 * containing the size in bytes of the storage unit used to represent an object of the given type."
 */
static void
print_param_value(const struct Dwarf_VarInfo *param, uintptr_t param_addr, int64_t offset) {
    (void)offset; /* Unused, kept for interface compatibility */
    
    if (param->kind == KIND_SIGNED_INT) {
        switch (param->byte_size) {
        case 1:
            cprintf("%d", *(int8_t *)param_addr);
            break;
        case 2:
            cprintf("%d", *(int16_t *)param_addr);
            break;
        case 4:
            cprintf("%d", *(int32_t *)param_addr);
            break;
        case 8:
            cprintf("%ld", *(int64_t *)param_addr);
            break;
        default:
            cprintf("?");
            break;
        }
    } else if (param->kind == KIND_STRING) {
        uintptr_t str_ptr = *(uintptr_t *)param_addr;
        if (str_ptr == 0) {
            cprintf("NULL");
        } else {
            const char *str = (const char *)str_ptr;
            cprintf("\"");
            for (int i = 0; i < 64; i++) {
                char c = str[i];
                if (c == '\0') break;
                if (c == '"' || c == '\\' || c == '\n' || c == '\t') {
                    cprintf("\\");
                    if (c == '\n') cprintf("n");
                    else if (c == '\t') cprintf("t");
                    else cprintf("%c", c);
                } else if (c >= 32 && c < 127) {
                    cprintf("%c", c);
                } else {
                    cprintf("\\x%02x", (uint8_t)c);
                }
            }
            cprintf("\"");
        }
    } else if (param->kind == KIND_POINTER) {
        uintptr_t ptr_val = *(uintptr_t *)param_addr;
        cprintf("0x%08lx", ptr_val);
    } else {
        cprintf("?");
    }
}


int
mon_backtrace(int argc, char **argv, struct Trapframe *tf) {
    cprintf("Stack backtrace:\n");

    struct Ripdebuginfo info = { 0 };
    uint64_t rbp = read_rbp();

    while (rbp != 0) {
        uint64_t *rbp_ptr = (uint64_t *)rbp;
        uint64_t next_rbp = rbp_ptr[0];
        uint64_t rip = rbp_ptr[1];

        // Сначала выводим адреса
        cprintf("  rbp %016lx  rip %016lx\n", rbp, rip);

        // Затем получаем debug информацию для RIP
        if (debuginfo_rip(rip, &info) == 0) {
            // ВАЖНО: параметры функции находятся в ТЕКУЩЕМ фрейме (rbp),
            // а не в следующем! Но для функций с параметрами через регистры
            // нужно искать их сохраненные значения.
            print_function_info(rip, &info, rbp, next_rbp);
        } else {
            cprintf("    %s:%d: %.*s+%ld\n",
                    info.rip_file, info.rip_line,
                    info.rip_fn_namelen, info.rip_fn_name,
                    rip - info.rip_fn_addr);
        }

        rbp = next_rbp;
    }

    return 0;
}

int
mon_hello(int argc, char **argv, struct Trapframe *tf) {
    cprintf("Hello from JOS kernel monitor!\n");
    cprintf("Lab 2 implementation by kozhagulov.r\n");
    return 0;
}

/* Implement timer_start (mon_start), timer_stop (mon_stop), timer_freq (mon_frequency) commands. */
int
mon_start(int argc, char **argv, struct Trapframe *tf) {
    (void)tf;
    if (argc < 2) {
        print_timer_error();
        return 0;
    }
    timer_start(argv[1]);
    return 0;
}

int
mon_stop(int argc, char **argv, struct Trapframe *tf) {
    (void)argc;
    (void)argv;
    (void)tf;
    timer_stop();
    return 0;
}

int
mon_frequency(int argc, char **argv, struct Trapframe *tf) {
    (void)tf;
    if (argc < 2) {
        print_timer_error();
        return 0;
    }
    timer_cpu_frequency(argv[1]);
    return 0;
}

// LAB 4: Your code here
int
mon_dumpcmos(int argc, char **argv, struct Trapframe *tf) {
    for (int i = 0; i < CMOS_START + CMOS_SIZE; i += 0x10) {
        cprintf("%02x:", i);
        for (int j = i; j < i + 0x10; j++)
            cprintf(" %02x", cmos_read8(j));
        cprintf("\n");
    }

    return 0;
}

/*
 * Monitor command to display free memory pages.
 * 
 * Calls dump_memory_lists() to show current state of free page allocation
 * grouped by class size. This helps debug and monitor memory allocator state.
 */
int
mon_memory(int argc, char **argv, struct Trapframe *tf) {
    dump_memory_lists();
    return 0;
}

/* Test functions for backtrace parameter display */

/* Test signed integer */
void test_signed_int(int32_t a) {
    (void)a;
    cprintf("pointer: %p\n", &a);
    cprintf("to int: %lld\n", (long long)&a);
    mon_backtrace(0, NULL, NULL);
}

/* Test pointer */
void test_pointer(void *ptr) {
    (void)ptr;
    cprintf("pointer: %p\n", ptr);
    mon_backtrace(0, NULL, NULL);
}

/* Test string */
void test_string(const char *str) {
    (void)str;
    cprintf("str: %s\n", str);
    mon_backtrace(0, NULL, NULL);
}

/* Test functions called from monitor */
int
mon_test_signed(int argc, char **argv, struct Trapframe *tf) {
    (void)argc; (void)argv; (void)tf;
    test_signed_int(777);
    return 0;
}

int
mon_test_ptr(int argc, char **argv, struct Trapframe *tf) {
    (void)argc; (void)argv; (void)tf;
    test_pointer((void *)0x12345678);
    return 0;
}

int
mon_test_string(int argc, char **argv, struct Trapframe *tf) {
    (void)argc; (void)argv; (void)tf;
    /* Используем статическую строку, чтобы она была доступна в памяти */
    static const char test_str[] = "Hello, JOS!";
    test_string(test_str);
    return 0;
}

int
mon_test_all(int argc, char **argv, struct Trapframe *tf) {
    (void)argc; (void)argv; (void)tf;
    cprintf("=== Running all backtrace parameter tests ===\n\n");
    
    cprintf("--- Test signed int ---\n");
    test_signed_int(-123456);
    cprintf("\n");
    
    cprintf("--- Test pointer ---\n");
    test_pointer((void *)0x12345678);
    cprintf("\n");
    
    cprintf("--- Test string ---\n");
    {
        static const char test_str[] = "Test string for backtrace";
        test_string(test_str);
    }
    cprintf("\n");
    
    cprintf("=== All tests completed ===\n");
    return 0;
}

int
mon_quit(int argc, char **argv, struct Trapframe *tf) {
    (void)argc; (void)argv; (void)tf;
    return -1;  /* Force monitor to exit */
}

/* Kernel monitor command interpreter */

static int
runcmd(char *buf, struct Trapframe *tf) {
    int argc = 0;
    char *argv[MAXARGS];

    argv[0] = NULL;

    /* Parse the command buffer into whitespace-separated arguments */
    for (;;) {
        /* gobble whitespace */
        while (*buf && strchr(WHITESPACE, *buf)) *buf++ = 0;
        if (!*buf) break;

        /* save and scan past next arg */
        if (argc == MAXARGS - 1) {
            cprintf("Too many arguments (max %d)\n", MAXARGS);
            return 0;
        }
        argv[argc++] = buf;
        while (*buf && !strchr(WHITESPACE, *buf)) buf++;
    }
    argv[argc] = NULL;

    /* Lookup and invoke the command */
    if (!argc) return 0;
    for (size_t i = 0; i < NCOMMANDS; i++) {
        if (strcmp(argv[0], commands[i].name) == 0)
            return commands[i].func(argc, argv, tf);
    }

    cprintf("Unknown command '%s'\n", argv[0]);
    return 0;
}

void
monitor(struct Trapframe *tf) {

    cprintf("Welcome to the JOS kernel monitor!\n");
    cprintf("Type 'help' for a list of commands.\n");

    if (tf) print_trapframe(tf);

    char *buf;
    do buf = readline("K> ");
    while (!buf || runcmd(buf, tf) >= 0);
}
