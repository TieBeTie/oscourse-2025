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
int mon_test_unsigned(int argc, char **argv, struct Trapframe *tf);
int mon_test_ptr(int argc, char **argv, struct Trapframe *tf);
int mon_test_multi(int argc, char **argv, struct Trapframe *tf);
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
        {"test_unsigned", "Test unsigned int parameter", mon_test_unsigned},
        {"test_ptr", "Test pointer parameter", mon_test_ptr},
        {"test_multi", "Test multiple parameters", mon_test_multi},
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

static void
print_single_parameter(const struct Dwarf_VarInfo *param, uintptr_t rbp, uintptr_t next_rbp, int param_index) {
    /* From DWARF4 specification, Section 3.3.4 "Declarations Owned by Subroutines and Entry Points":
     * "The unspecified parameters of a variable parameter list are represented by a debugging
     * information entry with the tag DW_TAG_unspecified_parameters."
     * 
     * Для variadic-параметров выводим "...", так как их значения не могут быть прочитаны
     * из стека (количество и типы неизвестны).
     */
    if (param->is_variadic) {
        cprintf("...");
        return;
    }
    
    if (strlen(param->name) == 0) {
        cprintf("%s=", param->type_name);
    } else {
        cprintf("%s %s=", param->type_name, param->name);
    }
    
    /* From System V x86-64 ABI, Section 3.2.3 "Parameter Passing":
     * Первые 6 целочисленных/указательных параметров передаются через регистры:
     * %rdi, %rsi, %rdx, %rcx, %r8, %r9. Остальные - через стек.
     * 
     * From DWARF4 specification, Section 2.6.1.1.2 "Register Location Descriptions":
     * Если location expression содержит DW_OP_reg*, параметр находится в регистре.
     * 
     * From DWARF4 specification, Section 2.5.1.2 "Register Based Addressing":
     * "The DW_OP_fbreg operation provides a signed LEB128 offset from the address specified by
     * the location description in the DW_AT_frame_base attribute of the current function."
     * 
     * Если location list указывает на регистр (param->address < 0), параметр находится в регистре
     * вызывающей функции, и мы не можем его прочитать из стека напрямую. В этом случае пытаемся
     * прочитать из стандартного места, где функция могла сохранить регистр (хотя это не гарантировано).
     * 
     * Если location list недоступна (param->address == 0), используем fallback на стандартные
     * смещения из System V x86-64 ABI для параметров, передаваемых через стек.
     */
    int64_t offset;
    uintptr_t base_rbp;
    if (param->address == 0) {
        /* Fallback на стандартные смещения из ABI - параметры в стеке вызывающей функции.
         * Но это работает только для параметров, передаваемых через стек (7-й и далее). */
        offset = 16 + param_index * 8;
        base_rbp = next_rbp;  /* Параметры находятся в стеке вызывающей функции */
    } else {
        /* Используем смещение из DWARF location list */
        offset = param->address;
        /* DW_OP_breg6 и DW_OP_fbreg дают смещение от текущего RBP функции */
        /* Для параметров функции они обычно указывают на локальные переменные/параметры
         * относительно текущего фрейма, но в x86-64 ABI параметры находятся в стеке вызывающей функции.
         * Однако, если DWARF указывает offset от текущего RBP, используем текущий RBP. */
        base_rbp = rbp;  /* Смещение от текущего RBP */
    }
    
    print_param_value(param, base_rbp, offset);
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

static void
print_param_value(const struct Dwarf_VarInfo *param, uintptr_t rbp_addr, int64_t offset) {
    /* From DWARF4 specification, Section 3.3.4 "Declarations Owned by Subroutines and Entry Points":
     * "The unspecified parameters of a variable parameter list are represented by a debugging
     * information entry with the tag DW_TAG_unspecified_parameters."
     * 
     * Для variadic-параметров выводим "...", так как их значения не могут быть прочитаны
     * из стека (количество и типы неизвестны).
     */
    if (param->is_variadic) {
        cprintf("...");
        return;
    }

    /* From System V x86-64 ABI, Section 3.2.2 "The Stack Frame":
     * Parameters are passed on the stack in the caller's frame.
     * 
     * Вычисляем адрес параметра в стеке: базовый адрес (rbp_addr) + смещение (offset).
     * rbp_addr - это RBP вызывающей функции (next_rbp), где находятся параметры текущей функции.
     * Это соответствует тому, что параметры функции находятся в стековом фрейме вызывающей функции,
     * а не в текущем фрейме.
     */
    uintptr_t param_addr = rbp_addr + offset;
    
    /* From DWARF4 specification, Section 5.1 "Base Type Entries":
     * "A base type is represented by a debugging information entry with the tag DW_TAG_base_type.
     * A base type entry has a DW_AT_encoding attribute describing how the base type is encoded
     * and is to be interpreted."
     * 
     * From DWARF4 specification, Figure 13 "Encoding attribute values":
     * "DW_ATE_signed signed binary integer"
     * "DW_ATE_unsigned unsigned binary integer"
     * 
     * Для корректного вывода значений параметров необходимо различать виды типов.
     * Для минимальной версии поддерживаем только базовые типы и указатели:
     * KIND_SIGNED_INT, KIND_UNSIGNED_INT, KIND_POINTER.
     */
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
    } else if (param->kind == KIND_UNSIGNED_INT) {
        switch (param->byte_size) {
        case 1:
            cprintf("%u", *(uint8_t *)param_addr);
            break;
        case 2:
            cprintf("%u", *(uint16_t *)param_addr);
            break;
        case 4:
            cprintf("%u", *(uint32_t *)param_addr);
            break;
        case 8:
            cprintf("%lu", *(uint64_t *)param_addr);
            break;
        default:
            cprintf("?");
            break;
        }
    /* From DWARF4 specification, Section 5.2 "Modified Type Entries":
     * "A modified type entry describing a pointer or reference type (using DW_TAG_pointer_type,
     * DW_TAG_reference_type or DW_TAG_rvalue_reference_type) may have a
     * DW_AT_address_class attribute to describe how objects having the given pointer or reference
     * type ought to be dereferenced."
     * 
     * Указатели выводятся в шестнадцатеричном формате как адреса памяти.
     */
    } else if (param->kind == KIND_STRING) {
        /* From DWARF4 specification, Section 5.2 "Modified Type Entries":
         * "A modified type entry describing a pointer or reference type..."
         * 
         * Строки (char*) читаются из памяти по указателю и выводятся как null-terminated строки.
         * Ограничиваем длину для безопасности.
         */
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
        /* Неизвестный тип - выводим как hex dump.
         * Для минимальной версии не поддерживаем структуры, массивы и другие сложные типы.
         */
        cprintf("0x");
        for (int i = param->byte_size - 1; i >= 0; i--) {
            cprintf("%02x", ((uint8_t *)param_addr)[i]);
        }
    }
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf) {
    // LAB 2: Your code here
    /* From docs/lab2_description.txt:
     * "Функция трассировки должна отображать данные в следующем формате:
     * Stack backtrace:
     *   rbp 0000008041616f00  rip 00000080416041ef"
   */
    cprintf("Stack backtrace:\n");

    struct Ripdebuginfo info = { 0 };
    /* From docs/lab2_description.txt:
    * "Читаем текущее значение регистра RBP с помощью функции read_rbp(). RBP указывает
     * на адрес в стеке, где сохранён предыдущий RBP. Это позволяет нам обойти стек, следуя цепочке
     * сохранённых указателей RBP согласно соглашению о вызовах x86-64.
     */
    uint64_t rbp = read_rbp();

    /* From docs/lab2_description.txt, lines 81-82:
     * "Первая строка соответствует выполняемой в данный момент функции (mon_backtrace), вторая —
     * функции, которая вызвала mon_backtrace и так далее. Изучив файл kern/entry.S, вы найдете
     * простой способ определить момент, когда нужно остановиться."
     * 
     * From kern/entry.S, line 16:
     * "xor %ebp, %ebp"
     * 
     * В entry.S начальный RBP устанавливается в 0 (xor %ebp, %ebp). Поэтому цикл
     * продолжается, пока rbp != 0. Когда мы достигаем начального фрейма (rbp == 0), это означает,
     * что мы дошли до точки входа в ядро, и нужно остановиться. Это стандартный способ определения
     * конца цепочки стековых фреймов в x86-64.
     */
    while (rbp != 0) {
        /* From System V x86-64 ABI, Section 3.2.2 "The Stack Frame", Figure 3.3:
         * "PositionContentsFrame
         * 0(%rbp)previous%rbpvalue
         * 8(%rbp)return address"
         * 
         * From docs/lab2_description.txt, lines 63-64:
         * "| saved %rbp  | ниже черты стек вызываемой функции
         * %rbp -> +-------------+"
         * 
         * В x86-64 стековый фрейм имеет следующую структуру: по адресу [rbp] находится
         * сохранённое значение предыдущего RBP (указатель на фрейм вызывающей функции), а по адресу
         * [rbp+8] находится адрес возврата (RIP). Преобразуем rbp в указатель на uint64_t, чтобы
         * прочитать эти значения из стека согласно структуре фрейма из System V ABI.
         */
        uint64_t *rbp_ptr = (uint64_t *)rbp;
        uint64_t next_rbp = rbp_ptr[0];
        uint64_t rip = rbp_ptr[1];

        cprintf("  rbp %016lx  rip %016lx\n", rbp, rip);

        if (debuginfo_rip(rip, &info) == 0) {
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
    mon_backtrace(0, NULL, NULL);
}

/* Test unsigned integer */
void test_unsigned_int(uint32_t a) {
    (void)a;
    mon_backtrace(0, NULL, NULL);
}

/* Test pointer */
void test_pointer(void *ptr) {
    (void)ptr;
    mon_backtrace(0, NULL, NULL);
}

/* Test multiple parameters */
void test_multiple_params(int32_t a, uint32_t b, void *c) {
    (void)a; (void)b; (void)c;
    mon_backtrace(0, NULL, NULL);
}

/* Test string */
void test_string(const char *str) {
    (void)str;
    mon_backtrace(0, NULL, NULL);
}

/* Test functions called from monitor */
int
mon_test_signed(int argc, char **argv, struct Trapframe *tf) {
    (void)argc; (void)argv; (void)tf;
    test_signed_int(-123456);
    return 0;
}

int
mon_test_unsigned(int argc, char **argv, struct Trapframe *tf) {
    (void)argc; (void)argv; (void)tf;
    test_unsigned_int(123456);
    return 0;
}

int
mon_test_ptr(int argc, char **argv, struct Trapframe *tf) {
    (void)argc; (void)argv; (void)tf;
    test_pointer((void *)0x12345678);
    return 0;
}

int
mon_test_multi(int argc, char **argv, struct Trapframe *tf) {
    (void)argc; (void)argv; (void)tf;
    test_multiple_params(-100, 200, (void *)0xABCDEF00);
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
    
    cprintf("--- Test unsigned int ---\n");
    test_unsigned_int(123456);
    cprintf("\n");
    
    cprintf("--- Test pointer ---\n");
    test_pointer((void *)0x12345678);
    cprintf("\n");
    
    cprintf("--- Test multiple parameters ---\n");
    test_multiple_params(-100, 200, (void *)0xABCDEF00);
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
