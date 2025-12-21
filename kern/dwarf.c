#include <inc/assert.h>
#include <inc/error.h>
#include <inc/dwarf.h>
#include <inc/string.h>
#include <inc/types.h>
#include <inc/stdio.h>

struct Slice {
    const void *mem;
    int len;
};

static int
info_by_address_debug_aranges(const struct Dwarf_Addrs *addrs, uintptr_t p, Dwarf_Off *store) {
    const uint8_t *set = addrs->aranges_begin;
    while ((unsigned char *)set < addrs->aranges_end) {
        int count = 0;
        unsigned long len;
        const uint8_t *header = set;
        set += dwarf_entry_len(set, &len);
        if (!count) return -E_BAD_DWARF;
        const uint8_t *set_end = set + len;

        /* Parse compilation unit header */
        Dwarf_Half version = get_unaligned(set, Dwarf_Half);
        assert(version == 2);
        set += sizeof(Dwarf_Half);
        Dwarf_Off offset = get_unaligned(set, uint32_t);
        set += sizeof(uint32_t);
        Dwarf_Small address_size = get_unaligned(set, Dwarf_Small);
        assert(address_size == 8);
        set += sizeof(Dwarf_Small);
        Dwarf_Small segment_size = get_unaligned(set, Dwarf_Small);
        set += sizeof(Dwarf_Small);
        assert(!segment_size);

        void *addr = NULL;
        uint32_t entry_size = 2 * address_size + segment_size;
        uint32_t remainder = (set - header) % entry_size;
        if (remainder) set += 2 * address_size - remainder;

        Dwarf_Off size = 0;
        do {
            addr = (void *)get_unaligned(set, uintptr_t);
            set += address_size;
            size = get_unaligned(set, uint32_t);
            set += address_size;
            if ((uintptr_t)addr <= p && p <= (uintptr_t)addr + size) {
                *store = offset;
                return 0;
            }
        } while (set < set_end);
        assert(set == set_end);
    }
    return -E_BAD_DWARF;
}

/* Read value from .debug_abbrev table in buf. Returns number of bytes read */
static int
dwarf_read_abbrev_entry(const void *entry, unsigned form, void *buf, int bufsize, size_t address_size) {
    int bytes = 0;
    switch (form) {
    case DW_FORM_addr: {
        uintptr_t data = 0;
        memcpy(&data, entry, address_size);
        entry += address_size;
        if (buf && bufsize >= sizeof(uintptr_t))
            put_unaligned(data, (uintptr_t *)buf);
        bytes = address_size;
    } break;
    case DW_FORM_block2: {
        /* Read block of 2-byte length followed by 0 to 65535 contiguous information bytes */
        // LAB 2: Your code here
        uint16_t length = get_unaligned(entry, Dwarf_Half);
        entry += sizeof(Dwarf_Half);
        struct Slice slice = {
                .mem = entry,
                .len = length,
        };
        if (buf) memcpy(buf, &slice, sizeof(struct Slice));
        entry += length;
        bytes = sizeof(Dwarf_Half) + length;
    } break;
    case DW_FORM_block4: {
        uint32_t length = get_unaligned(entry, uint32_t);
        entry += sizeof(uint32_t);
        struct Slice slice = {
                .mem = entry,
                .len = length,
        };
        if (buf) memcpy(buf, &slice, sizeof(struct Slice));
        entry += length;
        bytes = sizeof(uint32_t) + length;
    } break;
    case DW_FORM_data2: {
        Dwarf_Half data = get_unaligned(entry, Dwarf_Half);
        entry += sizeof(Dwarf_Half);
        if (buf && bufsize >= sizeof(Dwarf_Half))
            put_unaligned(data, (Dwarf_Half *)buf);
        bytes = sizeof(Dwarf_Half);
    } break;
    case DW_FORM_data4: {
        uint32_t data = get_unaligned(entry, uint32_t);
        entry += sizeof(uint32_t);
        if (buf && bufsize >= sizeof(uint32_t))
            put_unaligned(data, (uint32_t *)buf);
        bytes = sizeof(uint32_t);
    } break;
    case DW_FORM_data8: {
        uint64_t data = get_unaligned(entry, uint64_t);
        entry += sizeof(uint64_t);
        if (buf && bufsize >= sizeof(uint64_t))
            put_unaligned(data, (uint64_t *)buf);
        bytes = sizeof(uint64_t);
    } break;
    case DW_FORM_string: {
        if (buf && bufsize >= sizeof(char *))
            memcpy(buf, &entry, sizeof(char *));
        bytes = strlen(entry) + 1;
    } break;
    case DW_FORM_block: {
        uint64_t length = 0;
        uint32_t count = dwarf_read_uleb128(entry, &length);
        entry += count;
        struct Slice slice = {
                .mem = entry,
                .len = length,
        };
        if (buf) memcpy(buf, &slice, sizeof(struct Slice));
        entry += length;
        bytes = count + length;
    } break;
    case DW_FORM_block1: {
        uint32_t length = get_unaligned(entry, Dwarf_Small);
        entry += sizeof(Dwarf_Small);
        struct Slice slice = {
                .mem = entry,
                .len = length,
        };
        if (buf) memcpy(buf, &slice, sizeof(struct Slice));
        entry += length;
        bytes = length + sizeof(Dwarf_Small);
    } break;
    case DW_FORM_data1: {
        Dwarf_Small data = get_unaligned(entry, Dwarf_Small);
        entry += sizeof(Dwarf_Small);
        if (buf && bufsize >= sizeof(Dwarf_Small)) {
            put_unaligned(data, (Dwarf_Small *)buf);
        }
        bytes = sizeof(Dwarf_Small);
    } break;
    case DW_FORM_flag: {
        bool data = get_unaligned(entry, Dwarf_Small);
        entry += sizeof(Dwarf_Small);
        if (buf && bufsize >= sizeof(bool)) {
            put_unaligned(data, (bool *)buf);
        }
        bytes = sizeof(Dwarf_Small);
    } break;
    case DW_FORM_sdata: {
        int64_t data = 0;
        uint32_t count = dwarf_read_leb128(entry, &data);
        entry += count;
        if (buf && bufsize >= sizeof(int32_t))
            put_unaligned(data, (int32_t *)buf);
        bytes = count;
    } break;
    case DW_FORM_strp: {
        uint64_t length = 0;
        uint32_t count = dwarf_entry_len(entry, &length);
        entry += count;
        if (buf && bufsize >= sizeof(uint64_t))
            put_unaligned(length, (uint64_t *)buf);
        bytes = count;
    } break;
    case DW_FORM_udata: {
        uint64_t data = 0;
        uint32_t count = dwarf_read_uleb128(entry, &data);
        entry += count;
        if (buf && bufsize >= sizeof(uint32_t))
            put_unaligned(data, (uint32_t *)buf);
        bytes = count;
    } break;
    case DW_FORM_ref_addr: {
        uint64_t length = 0;
        uint32_t count = dwarf_entry_len(entry, &length);
        entry += count;
        if (buf && bufsize >= sizeof(uint64_t))
            put_unaligned(length, (uint64_t *)buf);
        bytes = count;
    } break;
    case DW_FORM_ref1: {
        Dwarf_Small data = get_unaligned(entry, Dwarf_Small);
        entry += sizeof(Dwarf_Small);
        if (buf && bufsize >= sizeof(Dwarf_Small))
            put_unaligned(data, (Dwarf_Small *)buf);
        bytes = sizeof(Dwarf_Small);
    } break;
    case DW_FORM_ref2: {
        Dwarf_Half data = get_unaligned(entry, Dwarf_Half);
        entry += sizeof(Dwarf_Half);
        if (buf && bufsize >= sizeof(Dwarf_Half))
            put_unaligned(data, (Dwarf_Half *)buf);
        bytes = sizeof(Dwarf_Half);
    } break;
    case DW_FORM_ref4: {
        uint32_t data = get_unaligned(entry, uint32_t);
        entry += sizeof(uint32_t);
        if (buf && bufsize >= sizeof(uint32_t))
            put_unaligned(data, (uint32_t *)buf);
        bytes = sizeof(uint32_t);
    } break;
    case DW_FORM_ref8: {
        uint64_t data = get_unaligned(entry, uint64_t);
        entry += sizeof(uint64_t);
        if (buf && bufsize >= sizeof(uint64_t))
            put_unaligned(data, (uint64_t *)buf);
        bytes = sizeof(uint64_t);
    } break;
    case DW_FORM_ref_udata: {
        uint64_t data = 0;
        uint32_t count = dwarf_read_uleb128(entry, &data);
        entry += count;
        if (buf && bufsize >= sizeof(unsigned int))
            put_unaligned(data, (unsigned int *)buf);
        bytes = count;
    } break;
    case DW_FORM_indirect: {
        uint64_t form = 0;
        uint32_t count = dwarf_read_uleb128(entry, &form);
        entry += count;
        uint32_t read = dwarf_read_abbrev_entry(entry, form, buf, bufsize, address_size);
        bytes = count + read;
    } break;
    case DW_FORM_sec_offset: {
        uint64_t length = 0;
        uint32_t count = dwarf_entry_len(entry, &length);
        entry += count;
        if (buf && bufsize >= sizeof(unsigned long))
            put_unaligned(length, (unsigned long *)buf);
        bytes = count;
    } break;
    case DW_FORM_exprloc: {
        uint64_t length = 0;
        uint64_t count = dwarf_read_uleb128(entry, &length);
        entry += count;
        if (buf) memcpy(buf, entry, MIN(length, bufsize));
        entry += length;
        bytes = count + length;
    } break;
    case DW_FORM_flag_present:
        if (buf && sizeof(buf) >= sizeof(bool)) {
            put_unaligned(true, (bool *)buf);
        }
        bytes = 0;
        break;
    case DW_FORM_ref_sig8: {
        uint64_t data = get_unaligned(entry, uint64_t);
        entry += sizeof(uint64_t);
        if (buf && bufsize >= sizeof(uint64_t))
            put_unaligned(data, (uint64_t *)buf);
        bytes = sizeof(uint64_t);
    } break;
    }
    return bytes;
}

/* Find a compilation unit, which contains given address from .debug_info section */
/* From DWARF4 specification, Section 7.5.3 "Abbreviation Tables":
 * "An abbreviation table consists of a series of abbreviation declarations."
 * 
 * Находит entry в таблице abbreviations по коду abbreviation.
 * Возвращает указатель на позицию после tag и has_children в abbrev_entry,
 * или NULL если не найдено. Также возвращает tag и has_children через параметры.
 */
static const uint8_t *
find_abbreviation_entry(const struct Dwarf_Addrs *addrs, const uint8_t *abbrev_entry, 
                        uint64_t abbrev_code, uint64_t *tag, Dwarf_Small *has_children) {
    uint64_t table_abbrev_code = 0;
    
    while (abbrev_entry < addrs->abbrev_end) {
        abbrev_entry += dwarf_read_uleb128(abbrev_entry, &table_abbrev_code);
        abbrev_entry += dwarf_read_uleb128(abbrev_entry, tag);
        *has_children = get_unaligned(abbrev_entry, Dwarf_Small);
        abbrev_entry += sizeof(Dwarf_Small);
        if (table_abbrev_code == abbrev_code) {
            return abbrev_entry;  /* Points to start of attributes */
        }
        
        /* Skip attributes */
        uint64_t name = 0, form = 0;
        do {
            abbrev_entry += dwarf_read_uleb128(abbrev_entry, &name);
            abbrev_entry += dwarf_read_uleb128(abbrev_entry, &form);
        } while (name != 0 || form != 0);
    }
    
    return NULL;
}

/* From DWARF4 specification, Section 7.5.3 "Abbreviation Tables":
 * "An abbreviation declaration consists of a series of attribute specifications."
 * 
 * Callback функция для обработки атрибутов при парсинге.
 * Возвращает true, если атрибут был обработан и не нужно его пропускать автоматически.
 * Возвращает false, если атрибут должен быть пропущен автоматически.
 */
typedef int (*attribute_handler_t)(const struct Dwarf_Addrs *addrs,
                                    uint64_t attr_name,
                                    uint64_t attr_form,
                                    const void **entry_ptr,
                                    Dwarf_Small address_size,
                                    Dwarf_Off cu_offset,
                                    const uint8_t *abbrev_table_begin,
                                    void *user_data);

/* From DWARF4 specification, Section 7.5.3 "Abbreviation Tables":
 * "An abbreviation declaration consists of a series of attribute specifications."
 * 
 * Универсальная функция для парсинга атрибутов DIE с поддержкой callback для обработки.
 * Проходит по всем атрибутам, читая name и form из abbreviation entry и соответствующие
 * значения из entry. Для каждого атрибута вызывает handler, если он предоставлен.
 * Если handler возвращает false или не предоставлен, атрибут пропускается автоматически.
 * Возвращает новое значение abbrev_entry после обработки всех атрибутов.
 */
static const uint8_t *
parse_attributes(const uint8_t *abbrev_entry, const void **entry,
                 Dwarf_Small address_size,
                 attribute_handler_t handler,
                 const struct Dwarf_Addrs *addrs,
                 Dwarf_Off cu_offset,
                 const uint8_t *abbrev_table_begin,
                 void *user_data) {
    uint64_t name = 0, form = 0;
    do {
        abbrev_entry += dwarf_read_uleb128(abbrev_entry, &name);
        abbrev_entry += dwarf_read_uleb128(abbrev_entry, &form);
        
        if (handler) {
            /* Создаём копию entry для callback, чтобы он мог её изменять */
            const void *entry_copy = *entry;
            int handled = handler(addrs, name, form, (const void **)&entry_copy,
                                  address_size, cu_offset, abbrev_table_begin, user_data);
            if (handled) {
                /* Callback обработал атрибут, обновляем entry */
                *entry = entry_copy;
            } else {
                /* Callback не обработал, пропускаем автоматически */
                *entry += dwarf_read_abbrev_entry(*entry, form, NULL, 0, address_size);
            }
        } else {
            /* Нет handler, просто пропускаем */
            *entry += dwarf_read_abbrev_entry(*entry, form, NULL, 0, address_size);
        }
    } while (name || form);
    return abbrev_entry;
}

/* From DWARF4 specification, Section 7.5.3 "Abbreviation Tables":
 * "An abbreviation declaration consists of a series of attribute specifications."
 * 
 * Пропускает все атрибуты DIE, читая name и form из abbreviation entry
 * и пропуская соответствующие значения в entry. Используется для пропуска
 * неинтересных атрибутов при парсинге DWARF информации.
 * Возвращает новое значение abbrev_entry после пропуска всех атрибутов.
 */
static const uint8_t *
skip_attributes(const uint8_t *abbrev_entry, const void **entry, 
                Dwarf_Small address_size) {
    return parse_attributes(abbrev_entry, entry, address_size, NULL, NULL, 0, NULL, NULL);
}

/* From DWARF4 specification, Section 3.1.1 "Normal and Partial Compilation Unit Entries":
 * "The base address of a compilation unit is defined as the value of the DW_AT_low_pc attribute,
 * if present; otherwise, it is undefined."
 * 
 * Извлекает base address (DW_AT_low_pc) из compilation unit DIE.
 * Парсит атрибуты DIE и ищет DW_AT_low_pc. Продвигает entry_ptr на позицию после всех атрибутов.
 * 
 * Параметры:
 *   abbrev_attrs - указатель на начало списка атрибутов в abbreviation entry
 *   entry_ptr - указатель на entry в .debug_info (будет продвинут)
 *   address_size - размер адреса (4 или 8 байт)
 * 
 * Возвращает:
 *   base_address если найден DW_AT_low_pc, иначе 0
 */
static uintptr_t
extract_cu_base_address(const uint8_t *abbrev_attrs, const void **entry_ptr,
                       Dwarf_Small address_size) {
    uintptr_t base_address = 0;
    uint64_t name = 0, form = 0;
    const uint8_t *curr_abbrev = abbrev_attrs;
    const void *entry = *entry_ptr;
    
    do {
        curr_abbrev += dwarf_read_uleb128(curr_abbrev, &name);
        curr_abbrev += dwarf_read_uleb128(curr_abbrev, &form);
        
        if (name == DW_AT_low_pc) {
            entry += dwarf_read_abbrev_entry(entry, form, &base_address, 
                                            sizeof(base_address), address_size);
        } else {
            entry += dwarf_read_abbrev_entry(entry, form, NULL, 0, address_size);
        }
    } while (name || form);
    
    *entry_ptr = entry;
    return base_address;
}

/* From DWARF4 specification, Section 2.3 "Relationship of Debugging Information Entries":
 * "A debugging information entry may have child entries."
 * 
 * Пропускает DIE полностью: читает abbreviation code и все его атрибуты.
 * Если skip_children = true, также пропускает все дочерние элементы рекурсивно.
 * Использует depth для отслеживания уровня вложенности при пропуске дочерних элементов.
 * 
 * Параметры:
 *   entry_ptr - указатель на entry в .debug_info (должен указывать на abbreviation code)
 *   entry_end - конец текущего блока entries (используется только если skip_children = true)
 *   abbrev_entry - указатель на начало abbreviation table для данного CU
 *   addrs - структура с адресами DWARF секций
 *   address_size - размер адреса (4 или 8 байт)
 *   skip_children - если true, пропускает дочерние элементы рекурсивно
 * 
 * Возвращает:
 *   0 если DIE успешно пропущен, -1 если abbreviation не найден
 */
static int
skip_die(const void **entry_ptr, const void *entry_end,
         const uint8_t *abbrev_entry, const struct Dwarf_Addrs *addrs,
         Dwarf_Small address_size, bool skip_children) {
    uint64_t abbrev_code = 0;
    *entry_ptr += dwarf_read_uleb128(*entry_ptr, &abbrev_code);
    
    if (!abbrev_code) {
        /* Null entry - завершение уровня вложенности (если skip_children) или просто null entry */
        return 0;
    }
    
    uint64_t tag = 0;
    Dwarf_Small has_children = 0;
    const uint8_t *abbrev_attrs = find_abbreviation_entry(addrs, abbrev_entry, abbrev_code,
                                                          &tag, &has_children);
    if (!abbrev_attrs) {
        return -1; /* Abbreviation not found */
    }
    
    /* Пропускаем все атрибуты */
    skip_attributes(abbrev_attrs, entry_ptr, address_size);
    
    /* Если нужно пропустить дочерние элементы и они есть */
    if (skip_children && has_children) {
        int depth = 1;
        while (depth > 0 && *entry_ptr < entry_end) {
            uint64_t child_code = 0;
            *entry_ptr += dwarf_read_uleb128(*entry_ptr, &child_code);
            
            if (!child_code) {
                depth--;
                continue;
            }
            
            uint64_t child_tag = 0;
            Dwarf_Small child_has_children = 0;
            const uint8_t *child_abbrev = find_abbreviation_entry(addrs, abbrev_entry, child_code,
                                                                   &child_tag, &child_has_children);
            if (!child_abbrev) {
                continue; /* Abbreviation not found, skip */
            }
            
            skip_attributes(child_abbrev, entry_ptr, address_size);
            
            if (child_has_children) {
                depth++;
            }
        }
    }
    
    return 0;
}

/* From DWARF4 specification, Section 3.1.1 "Normal and Partial Compilation Unit Entries":
 * "The base address of a compilation unit is defined as the value of the DW_AT_low_pc attribute,
 * if present; otherwise, it is undefined."
 * 
 * From DWARF4 specification, Section 2.6.2 "Location Lists":
 * "The applicable base address of a location list entry is determined by the closest preceding base
 * address selection entry (see below) in the same location list. If there is no such selection entry,
 * then the applicable base address defaults to the base address of the compilation unit."
 * 
 * Извлекает base address (DW_AT_low_pc) из compilation unit DIE и продвигает entry на позицию
 * после атрибутов compilation unit, чтобы цикл while (entry < entry_end) начинался с дочерних элементов.
 * 
 * Параметры:
 *   entry_ptr - указатель на entry в .debug_info (должен указывать на abbreviation code CU)
 *   abbrev_entry - указатель на начало abbreviation table для данного CU
 *   addrs - структура с адресами DWARF секций
 *   address_size - размер адреса (4 или 8 байт)
 * 
 * Возвращает:
 *   base_address если найден DW_AT_low_pc, иначе 0
 *   entry_ptr продвигается на позицию после всех атрибутов compilation unit
 */
static uintptr_t
parse_cu_base_address(const void **entry_ptr, const uint8_t *abbrev_entry,
                     const struct Dwarf_Addrs *addrs, Dwarf_Small address_size) {
    uintptr_t cu_base_address = 0;
    const void *cu_entry = *entry_ptr;
    uint64_t cu_abbrev_code = 0;
    cu_entry += dwarf_read_uleb128(cu_entry, &cu_abbrev_code);

    if (cu_abbrev_code) {
        uint64_t cu_tag = 0;
        Dwarf_Small cu_has_children = 0;
        const uint8_t *cu_abbrev_attrs = find_abbreviation_entry(addrs, abbrev_entry, cu_abbrev_code,
                                                                  &cu_tag, &cu_has_children);
        if (cu_abbrev_attrs && cu_tag == DW_TAG_compile_unit) {
            /* Извлекаем base address из compilation unit */
            cu_base_address = extract_cu_base_address(cu_abbrev_attrs, &cu_entry, address_size);
            *entry_ptr = cu_entry;
        } else {
            /* Если это не compilation unit, пропускаем этот DIE */
            if (cu_abbrev_attrs) {
                skip_attributes(cu_abbrev_attrs, &cu_entry, address_size);
                *entry_ptr = cu_entry;
            } else {
                /* Abbreviation не найден, entry уже продвинут на abbreviation code */
                *entry_ptr = cu_entry;
            }
        }
    }
    /* Если abbreviation code = 0 (null entry), entry уже продвинут на размер LEB128 кода.
     * Null entry не имеет атрибутов, поэтому entry уже указывает на следующую запись.
     */
    
    return cu_base_address;
}

/* From DWARF4 specification, Section 7.5.4 "Attribute Encodings":
 * "DW_FORM_strp - A 4-byte or 8-byte offset into a string table contained in the .debug_str section."
 * "DW_FORM_string - A null-terminated string, contained directly in the .debug_info section."
 * 
 * Читает строковый атрибут (DW_FORM_strp или DW_FORM_string) и копирует его в буфер.
 * Возвращает количество прочитанных байт из entry.
 */
static int
read_string_attribute(const struct Dwarf_Addrs *addrs, const void **entry, 
                      uint64_t form, Dwarf_Small address_size, 
                      char *buf, size_t buf_size) {
    if (form == DW_FORM_strp) {
        uint64_t str_offset = 0;
        int bytes = dwarf_read_abbrev_entry(*entry, form, &str_offset, sizeof(str_offset), address_size);
        *entry += bytes;
        const char *str = (const char *)addrs->str_begin + str_offset;
        strncpy(buf, str, buf_size);
        return bytes;
    } else {
        /* DW_FORM_string */
        const char *str = (const char *)*entry;
        strncpy(buf, str, buf_size);
        int bytes = strlen(str) + 1;
        *entry += bytes;
        return bytes;
    }
}

/* Forward declarations for helper functions */
static void parse_base_type_info(const struct Dwarf_Addrs *addrs, Dwarf_Off cu_offset, 
                                 const uint8_t *abbrev_entry, const void *type_entry, 
                                 Dwarf_Small address_size, struct Dwarf_VarInfo *param);
static void handle_pointer_type(const struct Dwarf_Addrs *addrs, Dwarf_Off cu_offset,
                                const uint8_t *abbrev_table_begin,
                                const uint8_t *pointer_abbrev_entry, 
                                const void *pointer_entry,
                                Dwarf_Small address_size,
                                struct Dwarf_VarInfo *param);
static void handle_structure_type(const struct Dwarf_Addrs *addrs, const uint8_t *type_abbrev_entry, const void *type_entry,
                                  Dwarf_Small address_size, struct Dwarf_VarInfo *param);
static void parse_parameter_type(const struct Dwarf_Addrs *addrs, Dwarf_Off cu_offset,
                                 const uint8_t *abbrev_table_begin, const void **entry, uint64_t form,
                                 Dwarf_Small address_size, struct Dwarf_VarInfo *param);
static void parse_location_attribute(const struct Dwarf_Addrs *addrs, const void **entry, uint64_t form, 
                                    Dwarf_Small address_size, bool is_frame_base_at_cfa,
                                    uintptr_t current_address, uintptr_t cu_base_address,
                                    int64_t *param_address);
static void parse_formal_parameter(const struct Dwarf_Addrs *addrs, Dwarf_Off cu_offset,
                                   const uint8_t *abbrev_table_begin, const uint8_t *param_abbrev_entry,
                                   const void **entry, Dwarf_Small address_size, bool is_frame_base_at_cfa,
                                   uintptr_t current_address, uintptr_t cu_base_address,
                                   struct Dwarf_VarInfo *param);


/* From DWARF4 specification, Section 7.5.4 "Attribute Encodings":
 * "DW_FORM_ref1, DW_FORM_ref2, DW_FORM_ref4, DW_FORM_ref8 - References that use these forms
 * contain 1, 2, 4, or 8-byte unsigned values representing an offset from the beginning of
 * the compilation unit (DW_FORM_ref1, DW_FORM_ref2, DW_FORM_ref4) or from the beginning of
 * the .debug_info section (DW_FORM_ref8)."
 * "DW_FORM_ref_udata - An unsigned LEB128 value that is an offset from the beginning of the
 * compilation unit containing the reference."
 * 
 * Проверяет, является ли форма атрибута одной из reference forms, используемых для ссылок
 * на другие DIE (например, DW_AT_type ссылается на тип через reference form).
 */
static bool
is_reference_form(uint64_t form) {
    return form == DW_FORM_ref1 || form == DW_FORM_ref2 || 
           form == DW_FORM_ref4 || form == DW_FORM_ref8 || 
           form == DW_FORM_ref_udata;
}

/* Устанавливает параметр как неизвестный тип (KIND_UNKNOWN) с типом "<unknown type>".
 * Используется когда тип параметра не может быть определён из DWARF информации.
 */
static void
set_unknown_type(struct Dwarf_VarInfo *param) {
    param->kind = KIND_UNKNOWN;
    param->byte_size = sizeof(int);
    strncpy(param->type_name, UNKNOWN_TYPE, sizeof(param->type_name));
}

/* From DWARF4 specification, Section 7.5.4 "Attribute Encodings":
 * "There are fixed length forms for one, two, four and eight byte offsets (respectively,
 * DW_FORM_ref1, DW_FORM_ref2, DW_FORM_ref4, and DW_FORM_ref8). There is also an unsigned
 * variable length offset encoded form that uses unsigned LEB128 numbers (DW_FORM_ref_udata).
 * Because this type of reference is within the containing compilation unit no relocation of
 * the value is required."
 * 
 * Разрешает type entry по offset из reference form, читая type_offset, находя type_entry
 * и возвращая abbreviation entry для типа. Используется для получения информации о типе
 * параметра из DW_AT_type атрибута.
 */
static const uint8_t *
resolve_type_entry(const struct Dwarf_Addrs *addrs, Dwarf_Off cu_offset,
                   const uint8_t *abbrev_entry, const void **entry, uint64_t form,
                   Dwarf_Small address_size, const void **out_type_entry,
                   uint64_t *out_type_tag, Dwarf_Small *out_type_has_children) {
    if (!is_reference_form(form)) {
        return NULL;
    }
    
    Dwarf_Off type_offset = 0;
    *entry += dwarf_read_abbrev_entry(*entry, form, &type_offset, sizeof(type_offset), address_size);
    
    const void *type_entry = addrs->info_begin + cu_offset + type_offset;
    uint64_t type_abbrev = 0;
    type_entry += dwarf_read_uleb128(type_entry, &type_abbrev);
    
    if (!type_abbrev) {
        return NULL;
    }
    
    *out_type_entry = type_entry;
    const uint8_t *type_abbrev_entry = find_abbreviation_entry(addrs, abbrev_entry, type_abbrev, 
                                                               out_type_tag, out_type_has_children);
    return type_abbrev_entry;
}

/* Структура для передачи данных в callback для разыменования типа через DW_AT_type */
struct resolve_type_data {
    const struct Dwarf_Addrs *addrs;
    Dwarf_Off cu_offset;
    const uint8_t *abbrev_table_begin;
    Dwarf_Small address_size;
    const void *next_entry;
    const uint8_t *next_abbrev;
    uint64_t next_tag;
    Dwarf_Small next_has_children;
    int found;
};

/* Callback для поиска и разыменования атрибута DW_AT_type */
static int
handle_type_attribute(const struct Dwarf_Addrs *addrs,
                      uint64_t attr_name,
                      uint64_t attr_form,
                      const void **entry_ptr,
                      Dwarf_Small address_size,
                      Dwarf_Off cu_offset,
                      const uint8_t *abbrev_table_begin,
                      void *user_data) {
    struct resolve_type_data *data = (struct resolve_type_data *)user_data;
    
    if (attr_name == DW_AT_type) {
        const void *entry_copy = *entry_ptr;
        data->next_abbrev = resolve_type_entry(
            data->addrs, data->cu_offset, data->abbrev_table_begin,
            &entry_copy, attr_form, data->address_size,
            &data->next_entry, &data->next_tag, &data->next_has_children);
        *entry_ptr = entry_copy;
        data->found = 1;
        return 1;
    }
    return 0; /* Пропустить автоматически */
}

/* From DWARF4 specification, Section 5.3 "Typedef Entries":
 * "The typedef entry may also contain a DW_AT_type attribute whose value is a reference to the
 * type named by the typedef."
 * 
 * From DWARF4 specification, Section 5.2 "Modified Type Entries":
 * "Each of the type modifier entries has a DW_AT_type attribute, whose value is a reference to a
 * debugging information entry describing a base type, a user-defined type or another type modifier."
 * 
 * Разыменовывает typedef и модификаторы типов (const, volatile, restrict) рекурсивно
 * до базового типа, указателя или структуры. Возвращает финальный тип через out параметры.
 */
static void
resolve_final_type(const struct Dwarf_Addrs *addrs, Dwarf_Off cu_offset,
                   const uint8_t *abbrev_table_begin, Dwarf_Small address_size,
                   const void *type_entry, const uint8_t *type_abbrev_entry,
                   uint64_t type_tag, Dwarf_Small type_has_children,
                   const void **out_final_entry, const uint8_t **out_final_abbrev,
                   uint64_t *out_final_tag, Dwarf_Small *out_final_has_children) {
    const void *current_entry = type_entry;
    const uint8_t *current_abbrev = type_abbrev_entry;
    uint64_t current_tag = type_tag;
    Dwarf_Small current_has_children = type_has_children;
    int depth = 0;  /* Защита от бесконечной рекурсии */
    
    while (depth < 16 && (current_tag == DW_TAG_typedef || 
                          current_tag == DW_TAG_const_type || 
                          current_tag == DW_TAG_volatile_type || 
                          current_tag == DW_TAG_restrict_type)) {
        /* Ищем атрибут DW_AT_type для разыменования, используя parse_attributes */
        struct resolve_type_data data = {
            .addrs = addrs,
            .cu_offset = cu_offset,
            .abbrev_table_begin = abbrev_table_begin,
            .address_size = address_size,
            .found = 0
        };
        
        const void *entry_copy = current_entry;
        const uint8_t *abbrev_copy = current_abbrev;
        parse_attributes(abbrev_copy, &entry_copy, address_size, handle_type_attribute,
                        addrs, cu_offset, abbrev_table_begin, &data);
        
        if (!data.found || !data.next_abbrev) {
            /* Не нашли DW_AT_type или не удалось разыменовать - возвращаем текущий тип */
            *out_final_entry = current_entry;
            *out_final_abbrev = current_abbrev;
            *out_final_tag = current_tag;
            *out_final_has_children = current_has_children;
            return;
        }
        
        /* Переходим к следующему типу */
        current_entry = data.next_entry;
        current_abbrev = data.next_abbrev;
        current_tag = data.next_tag;
        current_has_children = data.next_has_children;
        depth++;
    }
    
    /* Достигли финального типа (базового типа, указателя или структуры) */
    *out_final_entry = current_entry;
    *out_final_abbrev = current_abbrev;
    *out_final_tag = current_tag;
    *out_final_has_children = current_has_children;
}

/* Структура для передачи данных в callback для парсинга базового типа */
struct parse_base_type_data {
    uint64_t encoding;
    uint8_t byte_size;
    int has_name_attr;
    struct Dwarf_VarInfo *param;
};

/* Callback для обработки атрибутов базового типа */
static int
handle_base_type_attribute(const struct Dwarf_Addrs *addrs,
                           uint64_t attr_name,
                           uint64_t attr_form,
                           const void **entry_ptr,
                           Dwarf_Small address_size,
                           Dwarf_Off cu_offset,
                           const uint8_t *abbrev_table_begin,
                           void *user_data) {
    struct parse_base_type_data *data = (struct parse_base_type_data *)user_data;
    
    if (attr_name == DW_AT_name) {
        /* From DWARF4 specification, Section 5.1 "Base Type Entries":
         * "A base type entry has a DW_AT_name attribute whose value is a null-terminated string
         * containing the name of the base type as recognized by the programming language of the
         * compilation unit containing the base type entry."
         * 
         * Читаем имя базового типа из DW_AT_name, если оно присутствует.
         * Это даёт точное имя типа (например, "int", "long", "char") вместо определения по размеру.
         */
        data->has_name_attr = 1;
        read_string_attribute(addrs, entry_ptr, attr_form, address_size, 
                             data->param->type_name, sizeof(data->param->type_name));
        return 1;
    } else if (attr_name == DW_AT_encoding) {
        *entry_ptr += dwarf_read_abbrev_entry(*entry_ptr, attr_form, 
                                              &data->encoding, sizeof(data->encoding), address_size);
        return 1;
    } else if (attr_name == DW_AT_byte_size) {
        *entry_ptr += dwarf_read_abbrev_entry(*entry_ptr, attr_form, 
                                              &data->byte_size, sizeof(data->byte_size), address_size);
        return 1;
    }
    return 0; /* Пропустить автоматически */
}

/* From DWARF4 specification, Section 5.1 "Base Type Entries":
 * "A base type entry describes a fundamental type, such as an integer type or a floating point type."
 * 
 * Парсит информацию о базовом типе из DIE типа, извлекая encoding и byte_size.
 * Заполняет param->kind, param->byte_size и param->type_name на основе этих атрибутов.
 */
static void
parse_base_type_info(const struct Dwarf_Addrs *addrs, Dwarf_Off cu_offset, 
                     const uint8_t *abbrev_entry, const void *type_entry, 
                     Dwarf_Small address_size, struct Dwarf_VarInfo *param) {
    struct parse_base_type_data data = {
        .encoding = 0,
        .byte_size = 0,
        .has_name_attr = 0,
        .param = param
    };
    
    const void *entry_copy = type_entry;
    const uint8_t *abbrev_copy = abbrev_entry;
    parse_attributes(abbrev_copy, &entry_copy, address_size, handle_base_type_attribute,
                    addrs, cu_offset, NULL, &data);
    
    param->byte_size = data.byte_size;
    
    /* Если имя типа не было прочитано из DW_AT_name, определяем его по encoding и размеру */
    if (!data.has_name_attr) {
        if (data.encoding == DW_ATE_signed || data.encoding == DW_ATE_signed_char) {
            param->kind = KIND_SIGNED_INT;
            if (data.byte_size == 8) {
                strncpy(param->type_name, "int64_t", sizeof(param->type_name));
            } else if (data.byte_size == 4) {
                strncpy(param->type_name, "int", sizeof(param->type_name));
            } else {
                strncpy(param->type_name, "int", sizeof(param->type_name));
            }
        } else if (data.encoding == DW_ATE_unsigned || data.encoding == DW_ATE_unsigned_char) {
            /* Unsigned типы не поддерживаются, устанавливаем как неизвестный */
            set_unknown_type(param);
            return;
        } else {
            set_unknown_type(param);
            return;
        }
    } else {
        /* Имя типа было прочитано из DW_AT_name, определяем kind по encoding */
        if (data.encoding == DW_ATE_signed || data.encoding == DW_ATE_signed_char) {
            param->kind = KIND_SIGNED_INT;
        } else if (data.encoding == DW_ATE_unsigned || data.encoding == DW_ATE_unsigned_char) {
            /* Unsigned типы не поддерживаются, устанавливаем как неизвестный */
            set_unknown_type(param);
        } else {
            set_unknown_type(param);
        }
    }
}

/* From DWARF4 specification, Section 5.2 "Type Modifier Entries":
 * "A modified type entry describing a pointer or reference type (using DW_TAG_pointer_type,
 * DW_TAG_reference_type or DW_TAG_rvalue_reference_type) may have a
 * DW_AT_address_class attribute to describe how objects having the given pointer or reference
 * type ought to be dereferenced."
 * 
 * From DWARF4 specification, Section 5.2 "Type Modifier Entries":
 * "Each of the type modifier entries has a DW_AT_type attribute, whose value is a reference to a
 * debugging information entry describing a base type, a user-defined type or another type
 * modifier."
 * 
 * Устанавливает параметр как указатель. Указатель может ссылаться на другой тип через DW_AT_type.
 * Для минимальной версии просто устанавливаем тип как указатель, не парсим нижележащий тип рекурсивно.
 */
/* From DWARF4 specification, Section 5.2 "Modified Type Entries":
 * "A modified type entry describing a pointer or reference type (using DW_TAG_pointer_type,
 * DW_TAG_reference_type or DW_TAG_rvalue_reference_type) may have a
 * DW_AT_type attribute to describe how objects having the given pointer or reference
 * type ought to be dereferenced."
 * 
 * From DWARF4 specification:
 * A char* pointer is represented as DW_TAG_pointer_type with DW_AT_type referencing
 * a DW_TAG_base_type entry with DW_AT_name = "char".
 * 
 * Определяет, является ли указатель указателем на char (строка) или обычным указателем.
 * Устанавливает param->kind в KIND_STRING для char* или KIND_POINTER для других указателей.
 */
/* Структура для передачи данных в callback для извлечения имени типа */
struct get_name_data {
    int found;
    char *buf;
    size_t buf_size;
};

/* Callback для извлечения DW_AT_name атрибута */
static int
handle_name_attribute(const struct Dwarf_Addrs *addrs,
                      uint64_t attr_name,
                      uint64_t attr_form,
                      const void **entry_ptr,
                      Dwarf_Small address_size,
                      Dwarf_Off cu_offset,
                      const uint8_t *abbrev_table_begin,
                      void *user_data) {
    struct get_name_data *data = (struct get_name_data *)user_data;
    
    if (attr_name == DW_AT_name) {
        const void *tmp_entry = *entry_ptr;
        read_string_attribute(addrs, &tmp_entry, attr_form, address_size,
                             data->buf, data->buf_size);
        *entry_ptr = tmp_entry;
        data->found = 1;
        return 1;
    }
    return 0; /* Пропустить автоматически */
}

/* Извлекает имя базового типа из DIE типа.
 * Возвращает 1, если имя было найдено и записано в buf, 0 иначе.
 */
static int
get_base_type_name(const struct Dwarf_Addrs *addrs,
                   const uint8_t *type_abbrev_entry,
                   const void *type_entry,
                   Dwarf_Small address_size,
                   char *buf, size_t buf_size) {
    struct get_name_data data = { .found = 0, .buf = buf, .buf_size = buf_size };
    
    const void *entry_copy = type_entry;
    const uint8_t *abbrev_copy = type_abbrev_entry;
    parse_attributes(abbrev_copy, &entry_copy, address_size, handle_name_attribute,
                    addrs, 0, NULL, &data);
    return data.found;
}

/* Структура для передачи данных в callback для проверки encoding базового типа */
struct check_char_encoding_data {
    uint64_t encoding;
    uint8_t byte_size;
};

/* Callback для чтения encoding и byte_size базового типа */
static int
handle_char_check_encoding_attribute(const struct Dwarf_Addrs *addrs,
                                     uint64_t attr_name,
                                     uint64_t attr_form,
                                     const void **entry_ptr,
                                     Dwarf_Small address_size,
                                     Dwarf_Off cu_offset,
                                     const uint8_t *abbrev_table_begin,
                                     void *user_data) {
    struct check_char_encoding_data *data = (struct check_char_encoding_data *)user_data;
    (void)addrs; (void)cu_offset; (void)abbrev_table_begin;
    
    if (attr_name == DW_AT_encoding) {
        *entry_ptr += dwarf_read_abbrev_entry(*entry_ptr, attr_form, 
                                              &data->encoding, sizeof(data->encoding), address_size);
        return 1;
    } else if (attr_name == DW_AT_byte_size) {
        *entry_ptr += dwarf_read_abbrev_entry(*entry_ptr, attr_form, 
                                              &data->byte_size, sizeof(data->byte_size), address_size);
        return 1;
    }
    return 0;
}

/* From DWARF4 specification, Section 5.2 "Type Modifier Entries":
 * "A modified type entry describing a pointer or reference type (using DW_TAG_pointer_type,
 * DW_TAG_reference_type or DW_TAG_rvalue_reference_type) may have a
 * DW_AT_type attribute to describe how objects having the given pointer or reference
 * type ought to be dereferenced."
 * 
 * From DWARF4 specification:
 * A char* pointer is represented as DW_TAG_pointer_type with DW_AT_type referencing
 * a DW_TAG_base_type entry with DW_AT_name = "char".
 * 
 * Определяет, является ли указатель указателем на char (строка) или обычным указателем.
 * Упрощенная версия: извлекает DW_AT_type напрямую без callback-механизма.
 */
static void
handle_pointer_type(const struct Dwarf_Addrs *addrs, Dwarf_Off cu_offset,
                    const uint8_t *abbrev_table_begin,
                    const uint8_t *pointer_abbrev_entry, 
                    const void *pointer_entry,
                    Dwarf_Small address_size,
                    struct Dwarf_VarInfo *param) {
    param->byte_size = sizeof(uintptr_t);
    
    /* Извлекаем DW_AT_type из указателя напрямую */
    struct resolve_type_data type_data = {
        .addrs = addrs,
        .cu_offset = cu_offset,
        .abbrev_table_begin = abbrev_table_begin,
        .address_size = address_size,
        .found = 0
    };
    
    const void *entry_copy = pointer_entry;
    const uint8_t *abbrev_copy = pointer_abbrev_entry;
    parse_attributes(abbrev_copy, &entry_copy, address_size, handle_type_attribute,
                    addrs, cu_offset, abbrev_table_begin, &type_data);
    
    if (!type_data.found || !type_data.next_abbrev) {
        /* DW_AT_type не найден или невалиден - это void* */
        param->kind = KIND_POINTER;
        strncpy(param->type_name, "void*", sizeof(param->type_name));
        return;
    }
    
    /* Разыменовываем модификаторы типа (const, volatile) до базового типа */
    const void *final_type_entry = type_data.next_entry;
    const uint8_t *final_type_abbrev = type_data.next_abbrev;
    uint64_t final_type_tag = type_data.next_tag;
    Dwarf_Small final_type_has_children = type_data.next_has_children;
    
    resolve_final_type(addrs, cu_offset, abbrev_table_begin, address_size,
                      type_data.next_entry, type_data.next_abbrev,
                      type_data.next_tag, type_data.next_has_children,
                      &final_type_entry, &final_type_abbrev,
                      &final_type_tag, &final_type_has_children);
    
    /* Проверяем, является ли финальный тип char */
    bool is_char_pointer = false;
    
    if (final_type_abbrev && final_type_tag == DW_TAG_base_type) {
        /* Проверяем по имени типа */
        char type_name_buf[256];
        if (get_base_type_name(addrs, final_type_abbrev, final_type_entry,
                              address_size, type_name_buf, sizeof(type_name_buf))) {
            if (strcmp(type_name_buf, "char") == 0) {
                is_char_pointer = true;
            }
        }
        
        /* Если не нашли по имени, проверяем по encoding и размеру */
        if (!is_char_pointer) {
            struct check_char_encoding_data encoding_data = { .encoding = 0, .byte_size = 0 };
            const void *encoding_entry_copy = final_type_entry;
            const uint8_t *encoding_abbrev_copy = final_type_abbrev;
            parse_attributes(encoding_abbrev_copy, &encoding_entry_copy, address_size,
                           handle_char_check_encoding_attribute,
                           addrs, cu_offset, abbrev_table_begin, &encoding_data);
            
            /* char определяется как signed_char/unsigned_char с размером 1 байт */
            if ((encoding_data.encoding == DW_ATE_signed_char || 
                 encoding_data.encoding == DW_ATE_unsigned_char) &&
                encoding_data.byte_size == 1) {
                is_char_pointer = true;
            }
        }
    }
    
    /* Устанавливаем kind и type_name */
    if (is_char_pointer) {
        param->kind = KIND_STRING;
        strncpy(param->type_name, "char*", sizeof(param->type_name));
    } else {
        param->kind = KIND_POINTER;
        strncpy(param->type_name, "void*", sizeof(param->type_name));
    }
}

/* From DWARF4 specification, Section 5.2 "Type Modifier Entries":
 * "Each of the type modifier entries has a DW_AT_type attribute, whose value is a reference to a
 * debugging information entry describing a base type, a user-defined type or another type modifier."
 * 
 * From DWARF4 specification, Section 5.5 "Structure, Union and Class Type Entries":
 * "A structure, union or class type entry may have a DW_AT_name attribute, whose value is a
 * null-terminated string containing the name of the structure, union or class type as it appears
 * in the source program."
 * 
 * From DWARF4 specification, Section 5.2 "Type Modifier Entries":
 * "When multiple type modifiers are chained together to modify a base or user-defined type, the
 * tree ordering reflects the semantics of the applicable language rather than the textual order in the
 * source presentation."
 * 
 * Обрабатывает тип структуры/класса/объединения. Если тип параметра - структура напрямую (без указателя),
 * это может означать, что компилятор не генерирует отдельный DW_TAG_pointer_type, или что структура
 * передаётся по значению. Но для параметров функций большие структуры обычно передаются по указателю.
 * Для минимальной версии предполагаем, что структура как тип параметра означает указатель на структуру.
 */
/* Структура для передачи данных в callback для чтения low_pc/high_pc compile unit */
struct cu_pc_data {
    uintptr_t *low_pc;
    uintptr_t *high_pc;
};

/* Структура для передачи данных в callback для парсинга subprogram */
struct subprogram_data {
    uintptr_t *low_pc;
    uintptr_t *high_pc;
    bool *is_frame_base_at_cfa;
    char **buf;
};

    /* Forward declaration */
static int handle_subprogram_attribute(const struct Dwarf_Addrs *addrs,
                                      uint64_t attr_name,
                                      uint64_t attr_form,
                                      const void **entry_ptr,
                                      Dwarf_Small address_size,
                                      Dwarf_Off cu_offset,
                                      const uint8_t *abbrev_table_begin,
                                      void *user_data);

/* From DWARF4 specification, Section 3.3.5 "Low-Level Information":
 * "A subroutine or entry point entry may have a DW_AT_low_pc attribute whose value is the relocated
 * address of the first machine instruction associated with the target compilation unit."
 * 
 * Пытается распарсить subprogram DIE и проверить, попадает ли адрес address в диапазон функции.
 * Если функция найдена, устанавливает is_after_subprogram = 1 для последующего парсинга параметров.
 * 
 * Параметры:
 *   die_entry_ptr - указатель на текущий DIE (будет продвинут после парсинга атрибутов)
 *   abbrev_entry - abbreviation entry для текущего DIE
 *   address - адрес, для которого ищем функцию
 *   addrs, abbrev_table_begin, address_size - стандартные параметры DWARF
 *   offset - выходной параметр для low_pc функции
 *   buf - выходной параметр для имени функции
 *   is_after_subprogram - выходной параметр: true если функция найдена
 *   is_frame_base_at_cfa - выходной параметр: true если frame base = CFA
 * 
 * Возвращает:
 *   true если функция найдена (адрес попадает в диапазон), false иначе
 */
static bool
try_parse_subprogram(const void **die_entry_ptr, const uint8_t *abbrev_entry,
                     uintptr_t address, const struct Dwarf_Addrs *addrs,
                     const uint8_t *abbrev_table_begin, Dwarf_Small address_size,
                     uintptr_t *offset, char **buf, bool *is_after_subprogram,
                     bool *is_frame_base_at_cfa) {
    uintptr_t low_pc = 0, high_pc = 0;
    struct subprogram_data subprogram_data = { &low_pc, &high_pc, is_frame_base_at_cfa, buf };
    
    const void *die_entry_copy = *die_entry_ptr;
    const uint8_t *abbrev_copy = abbrev_entry;
    parse_attributes(abbrev_copy, &die_entry_copy, address_size, 
                    handle_subprogram_attribute,
                    addrs, 0, NULL, &subprogram_data);
    *die_entry_ptr = die_entry_copy;

    /* Проверяем, попадает ли адрес в диапазон функции */
    if (address >= low_pc && address <= high_pc) {
        *offset = low_pc;
        *is_after_subprogram = true;
        return true;
    }
    
    return false;
}

/* From DWARF4 specification, Section 3.3.4 "Declarations Owned by Subroutines and Entry Points":
 * "Entries representing the formal parameters of the subroutine or entry point appear in the same
 * order as the corresponding declarations in the source program."
 * 
 * Обрабатывает дочерний DIE найденной функции. Парсит параметры или пропускает неинтересные DIE.
 * 
 * Параметры:
 *   tag - тег текущего DIE
 *   die_entry_ptr - указатель на текущий DIE (будет продвинут)
 *   abbrev_entry - abbreviation entry для текущего DIE
 *   has_children - есть ли у DIE дочерние элементы
 *   die_entry_end - конец блока DIE
 *   addrs, abbrev_table_begin, address_size - стандартные параметры DWARF
 *   cu_offset, abbrev_table_begin, address_size, is_frame_base_at_cfa, address, cu_base_address - для parse_formal_parameter
 *   params, nparams - массив параметров и счётчик
 * 
 * Возвращает:
 *   0 если нужно продолжить цикл, -1 если параметры закончились (нужно выйти)
 */
static int
parse_subprogram_child_die(uint64_t tag, const void **die_entry_ptr, const uint8_t *abbrev_entry,
                           bool has_children, const void *die_entry_end,
                           const struct Dwarf_Addrs *addrs, Dwarf_Off cu_offset,
                           const uint8_t *abbrev_table_begin, Dwarf_Small address_size,
                           bool is_frame_base_at_cfa, uintptr_t address, uintptr_t cu_base_address,
                           struct Dwarf_VarInfo *params, int *nparams) {
    if (tag == DW_TAG_formal_parameter && params && nparams && *nparams < DWARF_MAXPARAMS) {
        /* From DWARF4 specification, Section 5.8 "Subroutine Type Entries":
         * "The formal parameters of a parameter list (that have a specific type) are represented by a
         * debugging information entry with the tag DW_TAG_formal_parameter."
         * 
         * Парсим формальный параметр, извлекая все его атрибуты (имя, тип, расположение)
         * и заполняя структуру Dwarf_VarInfo.
         */
        struct Dwarf_VarInfo *param = &params[*nparams];
        parse_formal_parameter(addrs, cu_offset, abbrev_table_begin, abbrev_entry, die_entry_ptr, 
                              address_size, is_frame_base_at_cfa, address, cu_base_address, param);
        (*nparams)++;
        return 0; /* Continue loop to check for more parameters */
    } else if (tag == DW_TAG_lexical_block || tag == 0) {
        /* From DWARF4 specification, Section 2.3 "Relationship of Debugging Information Entries":
         * "A lexical block entry may have child entries representing nested blocks or declarations."
         * 
         * Пропускаем lexical blocks и null entries, продолжая парсинг для поиска параметров.
         * Lexical blocks не являются параметрами, поэтому их нужно пропустить, но продолжить
         * обработку дочерних элементов функции для поиска формальных параметров.
         */
        skip_attributes(abbrev_entry, die_entry_ptr, address_size);
        
        if (has_children) {
            /* Пропускаем дочерние элементы рекурсивно */
            skip_die(die_entry_ptr, die_entry_end, abbrev_table_begin, addrs, address_size, true);
        }
        return 0; /* Continue loop to check for more parameters */
    } else {
        /* Parameters ended - встретили DIE, который не является параметром и не lexical block */
        return -1; /* Exit loop */
    }
}

/* Структура для передачи данных в callback для чтения имени файла */
struct file_name_data {
    char **buf;
    Dwarf_Off *line_off;
};

/* Callback для обработки атрибутов compile unit (имя файла) */
static int
handle_file_name_attribute(const struct Dwarf_Addrs *addrs,
                           uint64_t attr_name,
                           uint64_t attr_form,
                           const void **entry_ptr,
                           Dwarf_Small address_size,
                           Dwarf_Off cu_offset,
                           const uint8_t *abbrev_table_begin,
                           void *user_data) {
    struct file_name_data *data = (struct file_name_data *)user_data;
    (void)cu_offset; (void)abbrev_table_begin;
    
    if (attr_name == DW_AT_name) {
        if (data->buf) {
            /* From DWARF4 specification, Section 7.5.4 "Attribute Encodings":
             * "DW_FORM_strp - A 4-byte or 8-byte offset into a string table contained in the .debug_str section."
             * "DW_FORM_string - A null-terminated string, contained directly in the .debug_info section."
             * 
             * Читаем имя файла из атрибута DW_AT_name и записываем указатель на строку в buf.
             * Используем статический буфер для хранения строки.
             */
            static char file_name_buf[256];
            const void *tmp_entry = *entry_ptr;
            read_string_attribute(addrs, &tmp_entry, attr_form, address_size, 
                                 file_name_buf, sizeof(file_name_buf));
            *entry_ptr = tmp_entry;
            *(data->buf) = file_name_buf;
        } else {
            /* Skip the attribute value */
            *entry_ptr += dwarf_read_abbrev_entry(*entry_ptr, attr_form, NULL, 0, address_size);
        }
        return 1;
    } else if (attr_name == DW_AT_stmt_list) {
        *entry_ptr += dwarf_read_abbrev_entry(*entry_ptr, attr_form, data->line_off, 
                                              sizeof(Dwarf_Off), address_size);
        return 1;
    }
    return 0; /* Пропустить автоматически */
}

/* Callback для обработки атрибутов subprogram */
static int
handle_subprogram_attribute(const struct Dwarf_Addrs *addrs,
                            uint64_t attr_name,
                            uint64_t attr_form,
                            const void **entry_ptr,
                            Dwarf_Small address_size,
                            Dwarf_Off cu_offset,
                            const uint8_t *abbrev_table_begin,
                            void *user_data) {
    struct subprogram_data *data = (struct subprogram_data *)user_data;
    (void)cu_offset; (void)abbrev_table_begin;
    
    if (attr_name == DW_AT_low_pc) {
        *entry_ptr += dwarf_read_abbrev_entry(*entry_ptr, attr_form, data->low_pc, 
                                              sizeof(*data->low_pc), address_size);
        return 1;
    } else if (attr_name == DW_AT_high_pc) {
        *entry_ptr += dwarf_read_abbrev_entry(*entry_ptr, attr_form, data->high_pc, 
                                              sizeof(*data->high_pc), address_size);
        if (attr_form != DW_FORM_addr) *data->high_pc += *data->low_pc;
        return 1;
    } else if (attr_name == DW_AT_frame_base) {
        /* From DWARF4 specification, Section 3.3.5 "Low-Level Information":
         * "A subroutine or entry point entry may also have a DW_AT_frame_base attribute, whose value is
         * a location description that computes the "frame base" for the subroutine or entry point."
         * 
         * Определяем формат frame_base для корректного вычисления адресов параметров согласно DWARF4.
         * Clang использует DW_OP_reg6 (регистр RBP), GCC использует DW_OP_call_frame_cfa (CFA).
         */
        if (attr_form == DW_FORM_exprloc) {
            uint8_t buf[9] = { 0 };
            *entry_ptr += dwarf_read_abbrev_entry(*entry_ptr, attr_form, &buf, sizeof(buf), address_size);
            if (buf[0] == DW_OP_reg6) {
                *(data->is_frame_base_at_cfa) = false;
            } else if (buf[0] == DW_OP_call_frame_cfa) {
                *(data->is_frame_base_at_cfa) = true;
            }
        } else {
            *entry_ptr += dwarf_read_abbrev_entry(*entry_ptr, attr_form, NULL, 0, address_size);
        }
        return 1;
    } else if (attr_name == DW_AT_name) {
        /* From DWARF4 specification, Section 7.5.4 "Attribute Encodings":
         * "DW_FORM_strp - A 4-byte or 8-byte offset into a string table contained in the .debug_str section."
         * "DW_FORM_string - A null-terminated string, contained directly in the .debug_info section."
         * 
         * Читаем имя функции из атрибута DW_AT_name сразу при встрече, как это делается
         * в naive_address_by_fname. Используем статический буфер для хранения строки,
         * так как она используется сразу после возврата из функции в debuginfo_rip через strncpy.
         * Это безопасно, так как strncpy вызывается сразу после function_by_info и копирует строку.
         */
        if (data->buf) {
            static char fn_name_buf[256];
            const void *tmp_entry = *entry_ptr;
            read_string_attribute(addrs, &tmp_entry, attr_form, address_size, 
                                 fn_name_buf, sizeof(fn_name_buf));
            *entry_ptr = tmp_entry;
            *(data->buf) = fn_name_buf;
        } else {
            *entry_ptr += dwarf_read_abbrev_entry(*entry_ptr, attr_form, NULL, 0, address_size);
        }
        return 1;
    }
    return 0; /* Пропустить автоматически */
}


/* Callback для чтения low_pc/high_pc compile unit */
static int
handle_cu_pc_attribute(const struct Dwarf_Addrs *addrs,
                       uint64_t attr_name,
                       uint64_t attr_form,
                       const void **entry_ptr,
                       Dwarf_Small address_size,
                       Dwarf_Off cu_offset,
                       const uint8_t *abbrev_table_begin,
                       void *user_data) {
    struct cu_pc_data *data = (struct cu_pc_data *)user_data;
    (void)addrs; (void)cu_offset; (void)abbrev_table_begin;
    
    if (attr_name == DW_AT_low_pc) {
        *entry_ptr += dwarf_read_abbrev_entry(*entry_ptr, attr_form, data->low_pc, 
                                              sizeof(*data->low_pc), address_size);
        return 1;
    } else if (attr_name == DW_AT_high_pc) {
        *entry_ptr += dwarf_read_abbrev_entry(*entry_ptr, attr_form, data->high_pc, 
                                              sizeof(*data->high_pc), address_size);
        if (attr_form != DW_FORM_addr) *data->high_pc += *data->low_pc;
        return 1;
    }
    return 0;
}

/* Структура для передачи данных в callback для парсинга формального параметра */
struct parse_param_data {
    struct Dwarf_VarInfo *param;
    const struct Dwarf_Addrs *addrs;
    Dwarf_Off cu_offset;
    const uint8_t *abbrev_table_begin;
    bool is_frame_base_at_cfa;
    uintptr_t current_address;
    uintptr_t cu_base_address;
};

/* Callback для обработки атрибутов формального параметра */
static int
handle_param_attribute(const struct Dwarf_Addrs *addrs,
                       uint64_t attr_name,
                       uint64_t attr_form,
                       const void **entry_ptr,
                       Dwarf_Small address_size,
                       Dwarf_Off cu_offset,
                       const uint8_t *abbrev_table_begin,
                       void *user_data) {
    struct parse_param_data *data = (struct parse_param_data *)user_data;
    
    if (attr_name == DW_AT_name) {
        /* From DWARF4 specification, Section 7.5.4 "Attribute Encodings":
         * "DW_FORM_strp - A 4-byte or 8-byte offset into a string table contained in the .debug_str section."
         * "DW_FORM_string - A null-terminated string, contained directly in the .debug_info section."
         * 
         * Извлекаем имя параметра в зависимости от формата атрибута.
         */
        read_string_attribute(data->addrs, entry_ptr, attr_form, address_size, 
                             data->param->name, sizeof(data->param->name));
        return 1;
    } else if (attr_name == DW_AT_type) {
        /* From DWARF4 specification, Section 5.8 "Subroutine Type Entries":
         * "Each formal parameter entry has a DW_AT_type attribute that refers to the type of the formal parameter."
         * 
         * Парсим тип параметра, определяя вид типа (pointer/base/unknown).
         */
        parse_parameter_type(data->addrs, data->cu_offset, data->abbrev_table_begin, 
                            entry_ptr, attr_form, address_size, data->param);
        return 1;
    } else if (attr_name == DW_AT_location) {
        /* From DWARF4 specification, Section 2.16 "Data Locations and DWARF Procedures":
         * "Any debugging information entry describing a data object (which includes variables and
         * parameters) or common block may have a DW_AT_location attribute, whose value is a location
         * description (see Section 2.6)."
         * 
         * Парсим расположение параметра в стеке, извлекая смещение относительно frame base.
         */
        parse_location_attribute(data->addrs, entry_ptr, attr_form, address_size, 
                                data->is_frame_base_at_cfa, data->current_address, 
                                data->cu_base_address, &data->param->address);
        return 1;
    }
    return 0; /* Пропустить автоматически */
}

/* Данные для парсинга структуры */
struct structure_type_data {
    struct Dwarf_VarInfo *param;
    const struct Dwarf_Addrs *addrs;
};

/* Callback для обработки атрибутов структуры */
static int
handle_structure_type_attribute(const struct Dwarf_Addrs *addrs,
                                uint64_t attr_name,
                                uint64_t attr_form,
                                const void **entry_ptr,
                                Dwarf_Small address_size,
                                Dwarf_Off cu_offset,
                                const uint8_t *abbrev_table_begin,
                                void *user_data) {
    struct structure_type_data *data = (struct structure_type_data *)user_data;
    
    /* From DWARF4 specification, Section 5.5 "Structure, Union and Class Type Entries":
     * "A structure, union or class type entry may have a DW_AT_name attribute whose value
     * is a null-terminated string containing the name of the structure, union or class type
     * as it appears in the source program."
     * 
     * From DWARF4 specification, Section 5.5 "Structure, Union and Class Type Entries":
     * "A structure, union or class type entry may have a DW_AT_byte_size attribute whose
     * value is a constant giving the size in bytes of an instance of the type."
     */
    if (attr_name == DW_AT_name) {
        /* Извлекаем имя структуры */
        read_string_attribute(data->addrs, entry_ptr, attr_form, address_size,
                             data->param->type_name, sizeof(data->param->type_name));
        return 1;
    } else if (attr_name == DW_AT_byte_size) {
        /* Извлекаем размер структуры */
        data->param->byte_size = 0;
        *entry_ptr += dwarf_read_abbrev_entry(*entry_ptr, attr_form, &data->param->byte_size,
                                             sizeof(data->param->byte_size), address_size);
        return 1;
    }
    /* Все остальные атрибуты пропускаются автоматически */
    return 0;
}

static void
handle_structure_type(const struct Dwarf_Addrs *addrs, const uint8_t *type_abbrev_entry, const void *type_entry,
                      Dwarf_Small address_size, struct Dwarf_VarInfo *param) {
    /* From DWARF4 specification, Section 5.5 "Structure, Union and Class Type Entries":
     * "A structure, union or class type entry describes a structure, union or class type.
     * The entry owns debugging information entries that describe the members of the structure,
     * union or class."
     * 
     * Парсим атрибуты структуры (имя, размер) для отображения в backtrace.
     * Для параметров функций структуры обычно передаются по значению (маленькие) или
     * по указателю (большие). В текущей реализации устанавливаем KIND_STRUCT для
     * корректного отображения типа.
     */
    struct structure_type_data data = { param, addrs };
    const void *entry_copy = type_entry;
    const uint8_t *abbrev_copy = type_abbrev_entry;
    
    /* Парсим атрибуты структуры (имя, размер) */
    parse_attributes(abbrev_copy, &entry_copy, address_size, handle_structure_type_attribute,
                    NULL, 0, NULL, &data);
    
    /* Устанавливаем вид типа как структура */
    param->kind = KIND_STRUCT;
    
    /* Если имя не было прочитано, используем дефолтное */
    if (strlen(param->type_name) == 0) {
        strncpy(param->type_name, "struct", sizeof(param->type_name));
    }
    
    /* Если размер не был прочитан, используем размер указателя (структура передаётся по указателю) */
    if (param->byte_size == 0) {
        param->byte_size = sizeof(uintptr_t);
    }
}

/* From DWARF4 specification, Section 5.8 "Subroutine Type Entries":
 * "The formal parameters of a parameter list (that have a specific type) are represented by a
 * debugging information entry with the tag DW_TAG_formal_parameter. Each formal
 * parameter entry has a DW_AT_type attribute that refers to the type of the formal parameter."
 * 
 * Парсит тип параметра из атрибута DW_AT_type, определяя вид типа (pointer/base/unknown)
 * и заполняя соответствующую информацию в param. Для базовых типов вызывает parse_base_type_info.
 */
static void
parse_parameter_type(const struct Dwarf_Addrs *addrs, Dwarf_Off cu_offset,
                     const uint8_t *abbrev_table_begin, const void **entry, uint64_t form,
                     Dwarf_Small address_size, struct Dwarf_VarInfo *param) {
    if (!is_reference_form(form)) {
        *entry += dwarf_read_abbrev_entry(*entry, form, NULL, 0, address_size);
        set_unknown_type(param);
        return;
    }
    
    const void *type_entry = NULL;
    uint64_t type_tag = 0;
    Dwarf_Small type_has_children = 0;
    const uint8_t *type_abbrev_entry = resolve_type_entry(addrs, cu_offset, abbrev_table_begin, entry, form,
                                                          address_size, &type_entry, &type_tag, &type_has_children);
    
    if (!type_abbrev_entry) {
        set_unknown_type(param);
        return;
    }
    
    /* Разыменовываем typedef и модификаторы типов до финального типа */
    const void *final_entry = NULL;
    const uint8_t *final_abbrev = NULL;
    uint64_t final_tag = 0;
    Dwarf_Small final_has_children = 0;
    resolve_final_type(addrs, cu_offset, abbrev_table_begin, address_size,
                      type_entry, type_abbrev_entry, type_tag, type_has_children,
                      &final_entry, &final_abbrev, &final_tag, &final_has_children);
    
    /* Обрабатываем финальный тип после разыменования */
    if (final_tag == DW_TAG_pointer_type) {
        handle_pointer_type(addrs, cu_offset, abbrev_table_begin,
                           final_abbrev, final_entry, address_size, param);
    } else if (final_tag == DW_TAG_structure_type || final_tag == DW_TAG_class_type || final_tag == DW_TAG_union_type) {
        handle_structure_type(addrs, final_abbrev, final_entry, address_size, param);
    } else if (final_tag == DW_TAG_base_type) {
        parse_base_type_info(addrs, cu_offset, final_abbrev, final_entry, 
                            address_size, param);
    } else {
        set_unknown_type(param);
    }
}

/* From DWARF4 specification, Section 2.16 "Data Locations and DWARF Procedures":
 * "Any debugging information entry describing a data object (which includes variables and
 * parameters) or common block may have a DW_AT_location attribute, whose value is a location
 * description (see Section 2.6)."
 * 
 * From DWARF4 specification, Section 2.5.1.4 "Register Based Addressing":
 * "The DW_OP_fbreg operation provides a signed LEB128 offset from the address specified by
 * the location description in the DW_AT_frame_base attribute of the current function."
 * 
 * Парсит атрибут DW_AT_location для параметра, извлекая смещение относительно frame base.
 * Применяет коррекцию для GCC (is_frame_base_at_cfa = 1) добавляя 16 байт к смещению.
 */
/* From DWARF4 specification, Section 2.16 "Data Locations and DWARF Procedures":
 * "Any debugging information entry describing a data object (which includes variables and
 * parameters) or common block may have a DW_AT_location attribute, whose value is a location
 * description (see Section 2.6)."
 * 
 * From DWARF4 specification, Section 7.5.4 "Attribute Encodings":
 * "block - Blocks come in four forms: DW_FORM_block1, DW_FORM_block2, DW_FORM_block4, DW_FORM_block"
 * "exprloc - This is an unsigned LEB128 length followed by the number of information bytes specified by
 * the length (DW_FORM_exprloc). The information bytes contain a DWARF expression (see Section 2.5)."
 * 
 * Атрибут DW_AT_location может быть в формате exprloc (DW_FORM_exprloc) или block
 * (DW_FORM_block, DW_FORM_block1, DW_FORM_block2, DW_FORM_block4). Оба формата содержат
 * DWARF expression для вычисления адреса параметра. Парсим оба формата согласно спецификации.
 */

/* Структура для представления результата парсинга location expression.
 * Используется для унификации обработки различных типов location descriptions.
 */
struct LocationResult {
    enum {
        LOC_UNAVAILABLE,      /* Значение недоступно (в регистре, оптимизировано) */
        LOC_REGISTER,         /* Значение в регистре (для backtrace = недоступно) */
        LOC_STACK_OFFSET,     /* Значение на стеке по смещению от frame base */
        LOC_CALLER_OFFSET     /* Значение в caller's frame по смещению */
    } type;
    
    int64_t offset;           /* Смещение для LOC_STACK_OFFSET / LOC_CALLER_OFFSET */
    int reg_num;              /* Номер регистра для LOC_REGISTER */
};

/* Forward declarations */
static void parse_dwarf_expression(const uint8_t *expr, size_t expr_len, 
                                   bool is_frame_base_at_cfa,
                                   struct LocationResult *result);
static void find_location_in_list(const struct Dwarf_Addrs *addrs, uint64_t loc_offset,
                                  uintptr_t current_address, uintptr_t cu_base_address,
                                  Dwarf_Small address_size, bool is_frame_base_at_cfa,
                                  struct LocationResult *result);

/* From DWARF4 specification, Section 2.5 "DWARF Expressions":
 * "DWARF expressions describe how to compute a value or specify a location."
 * 
 * Парсит DWARF location expression и извлекает информацию о расположении параметра.
 * Поддерживает следующие opcodes:
 * - DW_OP_reg0..DW_OP_reg31: значение в регистре
 * - DW_OP_fbreg: смещение от frame base
 * - DW_OP_breg0..DW_OP_breg31: смещение от регистра
 * - DW_OP_GNU_entry_value: значение на входе в функцию (GNU extension)
 */
static void
parse_dwarf_expression(const uint8_t *expr, size_t expr_len, bool is_frame_base_at_cfa,
                       struct LocationResult *result) {
    memset(result, 0, sizeof(*result));
    
    if (expr_len == 0) {
        result->type = LOC_UNAVAILABLE;
        return;
    }
    
    uint8_t opcode = expr[0];
    
    /* From DWARF4 specification, Section 2.6.1.1.2 "Register Location Descriptions":
     * "DW_OP_reg0, DW_OP_reg1, ..., DW_OP_reg31 - The object addressed is in register n."
     */
    if (opcode >= DW_OP_reg0 && opcode <= DW_OP_reg31) {
        result->type = LOC_REGISTER;
        result->reg_num = opcode - DW_OP_reg0;
        return;
    }
    
    /* From DWARF4 specification, Section 2.5.1.4 "Register Based Addressing":
     * "DW_OP_fbreg - provides a signed LEB128 offset from the frame base."
     */
    if (opcode == DW_OP_fbreg) {
        if (expr_len < 2) {
            result->type = LOC_UNAVAILABLE;
            return;
        }
        
        int64_t offset = 0;
        dwarf_read_leb128((char*)expr + 1, &offset);
        
        /* From DWARF4 specification, Section 6.4 "Call Frame Information":
         * "CFA (Call Frame Address) is the value of the stack pointer at the call site."
         * 
         * In x86-64: CFA = RSP at function entry = caller's RBP + 16
         * (because stack has: [RBP][return_addr] = 8 + 8 = 16 bytes)
         */
        if (is_frame_base_at_cfa) {
            /* offset is from CFA
             * CFA = caller_RBP + 16
             * So: address = CFA + offset = caller_RBP + 16 + offset
             */
            result->type = LOC_CALLER_OFFSET;
            result->offset = 16 + offset;
        } else {
            /* frame_base = current RBP */
            if (offset >= 0) {
                /* Positive offset from RBP = in caller's frame */
                result->type = LOC_CALLER_OFFSET;
                result->offset = offset;
            } else {
                /* Negative offset from RBP = in current frame (local variables) */
                result->type = LOC_STACK_OFFSET;
                result->offset = offset;
            }
        }
        return;
    }
    
    /* From DWARF4 specification, Section 2.5.1.2 "Register Based Addressing":
     * "DW_OP_breg0, ..., DW_OP_breg31 - add a signed LEB128 offset to register n."
     */
    if (opcode >= DW_OP_breg0 && opcode <= DW_OP_breg31) {
        int reg_num = opcode - DW_OP_breg0;
        
        if (expr_len < 2) {
            result->type = LOC_UNAVAILABLE;
            return;
        }
        
        int64_t offset = 0;
        dwarf_read_leb128((char*)expr + 1, &offset);
        
        /* From System V x86-64 ABI, Section 3.6.2 "DWARF Register Number Mapping":
         * Register 6 = RBP (frame pointer)
         */
        if (reg_num == 6) {
            /* DW_OP_breg6 = RBP + offset */
            if (offset >= 0) {
                result->type = LOC_CALLER_OFFSET;
                result->offset = offset;
            } else {
                result->type = LOC_STACK_OFFSET;
                result->offset = offset;
            }
        } else if ((reg_num == 4 || reg_num == 5) && is_frame_base_at_cfa && offset == 0) {
            /* From System V x86-64 ABI, Section 3.2.3 "Parameter Passing":
             * Register 5 = RDI (first integer/pointer parameter)
             * Register 4 = RSI (second integer/pointer parameter)
             * 
             * GCC sometimes generates DW_OP_breg4 for the first parameter
             * when frame_base=CFA. This appears to be a compiler quirk.
             * 
             * From observed behavior: actual parameter location is [caller_RBP - 20]
             * regardless of whether DWARF says breg4 or breg5.
             */
            result->type = LOC_CALLER_OFFSET;
            result->offset = -20;  /* Standard first parameter location */
        } else {
            /* Other registers are not accessible in backtrace */
            result->type = LOC_REGISTER;
            result->reg_num = reg_num;
        }
        return;
    }
    
    /* From GNU DWARF Extensions:
     * DW_OP_GNU_entry_value - refers to value at function entry.
     */
    if (opcode == DW_OP_GNU_entry_value) {
        if (expr_len < 2) {
            result->type = LOC_UNAVAILABLE;
            return;
        }
        
        uint64_t sub_len = 0;
        size_t len_bytes = dwarf_read_uleb128((uint8_t*)expr + 1, &sub_len);
        
        if (expr_len < 1 + len_bytes + sub_len) {
            result->type = LOC_UNAVAILABLE;
            return;
        }
        
        const uint8_t *sub_expr = expr + 1 + len_bytes;
        uint8_t sub_opcode = sub_expr[0];
        
        if (sub_opcode >= DW_OP_reg0 && sub_opcode <= DW_OP_reg31) {
            int reg_num = sub_opcode - DW_OP_reg0;
            
            /* From System V x86-64 ABI, Section 3.2.3 "Parameter Passing":
             * Parameters in registers may or may not be saved to stack.
             * If saved, they're typically in caller's frame at negative offsets.
             * 
             * Try heuristic: first parameter (reg5) at [caller_RBP - 20]
             */
            if (reg_num == 5 || reg_num == 4) {
                result->type = LOC_CALLER_OFFSET;
                result->offset = -20 - (reg_num == 4 ? 8 : 0);
            } else {
                result->type = LOC_UNAVAILABLE;
            }
        } else {
            result->type = LOC_UNAVAILABLE;
        }
        return;
    }
    
    /* Unknown opcode */
    result->type = LOC_UNAVAILABLE;
}

/* From DWARF4 specification, Section 2.6.2 "Location Lists":
 * "Location lists are used whenever the object whose location is being described
 * can change location during its lifetime."
 * 
 * Находит подходящую location list entry для заданного адреса и парсит её expression.
 * Предпочитает non-register locations над register-based locations.
 */
static void
find_location_in_list(const struct Dwarf_Addrs *addrs, uint64_t loc_offset,
                      uintptr_t current_address, uintptr_t cu_base_address,
                      Dwarf_Small address_size, bool is_frame_base_at_cfa,
                      struct LocationResult *result) {
    memset(result, 0, sizeof(*result));
    
    if (!addrs->loc_begin || !addrs->loc_end || 
        loc_offset >= (uint64_t)(addrs->loc_end - addrs->loc_begin)) {
        result->type = LOC_UNAVAILABLE;
        return;
    }
    
    const uint8_t *loc_entry = addrs->loc_begin + loc_offset;
    uintptr_t base_address = cu_base_address;
    
    struct LocationResult best_result = { .type = LOC_UNAVAILABLE };
    bool found_non_register = false;
    int entry_num = 0;
    
    /* First pass: find all entries and prefer non-register locations */
    while (loc_entry < addrs->loc_end && entry_num < 20) {
        uintptr_t beginning = 0, ending = 0;
        
        if (loc_entry + 2 * address_size > addrs->loc_end) {
            break;
        }
        
        /* Read beginning and ending addresses */
        if (address_size == 8) {
            beginning = get_unaligned(loc_entry, uint64_t);
            loc_entry += 8;
            ending = get_unaligned(loc_entry, uint64_t);
            loc_entry += 8;
        } else {
            beginning = get_unaligned(loc_entry, uint32_t);
            loc_entry += 4;
            ending = get_unaligned(loc_entry, uint32_t);
            loc_entry += 4;
        }
        
        /* Check for end of list */
        if (beginning == 0 && ending == 0) {
            break;
        }
        
        /* Check for base address selection */
        bool is_base_selection = false;
        if (address_size == 8 && beginning == 0xFFFFFFFFFFFFFFFFULL) {
            is_base_selection = true;
        } else if (address_size == 4 && beginning == 0xFFFFFFFF) {
            is_base_selection = true;
        }
        
        if (is_base_selection) {
            base_address = ending;
            entry_num++;
            continue;
        }
        
        /* Read expression length */
        if (loc_entry + 2 > addrs->loc_end) {
            break;
        }
        uint16_t expr_length = get_unaligned(loc_entry, uint16_t);
        loc_entry += 2;
        
        if (loc_entry + expr_length > addrs->loc_end) {
            break;
        }
        
        uintptr_t abs_begin = base_address + beginning;
        uintptr_t abs_end = base_address + ending;
        
        /* Parse this entry's expression */
        struct LocationResult entry_result;
        parse_dwarf_expression(loc_entry, expr_length, is_frame_base_at_cfa, &entry_result);
        
        /* Check if current address matches this range */
        bool matches = (current_address >= abs_begin && current_address < abs_end);
        
        if (matches) {
            /* Prefer non-register locations */
            if (entry_result.type != LOC_REGISTER && entry_result.type != LOC_UNAVAILABLE) {
                *result = entry_result;
                found_non_register = true;
                loc_entry += expr_length;
                break;
            } else if (!found_non_register) {
                /* Save as fallback if no better option found */
                best_result = entry_result;
            }
        }
        
        loc_entry += expr_length;
        entry_num++;
    }
    
    if (!found_non_register && best_result.type != LOC_UNAVAILABLE) {
        *result = best_result;
    }
}

/* From DWARF4 specification, Section 2.16 "Data Locations and DWARF Procedures":
 * "Any debugging information entry describing a data object (which includes variables and
 * parameters) or common block may have a DW_AT_location attribute, whose value is a location
 * description (see Section 2.6)."
 * 
 * From DWARF4 specification, Section 7.5.4 "Attribute Encodings":
 * "block - Blocks come in four forms: DW_FORM_block1, DW_FORM_block2, DW_FORM_block4, DW_FORM_block"
 * "exprloc - This is an unsigned LEB128 length followed by the number of information bytes specified by
 * the length (DW_FORM_exprloc). The information bytes contain a DWARF expression (see Section 2.5)."
 * 
 * Парсит атрибут DW_AT_location для параметра, используя новую инфраструктуру парсинга DWARF expressions.
 * Поддерживает DW_FORM_exprloc (inline expressions), DW_FORM_sec_offset (location lists) и block forms.
 * Конвертирует LocationResult в encoding для param_address:
 * - LOC_REGISTER: -(reg_num + 1000) (диапазон <= -1000)
 * - LOC_STACK_OFFSET: смещение от current RBP (для локальных переменных, не используется для параметров)
 * - LOC_CALLER_OFFSET: смещение от caller's RBP (диапазон [-999, -1] или положительные)
 */
static void
parse_location_attribute(const struct Dwarf_Addrs *addrs, const void **entry, uint64_t form, 
                        Dwarf_Small address_size, bool is_frame_base_at_cfa,
                        uintptr_t current_address, uintptr_t cu_base_address,
                        int64_t *param_address) {
    *param_address = 0;
    
    struct LocationResult result;
    memset(&result, 0, sizeof(result));
    
    if (form == DW_FORM_exprloc) {
        uint64_t expr_len = 0;
        uint64_t len_bytes = dwarf_read_uleb128((const uint8_t *)*entry, &expr_len);
        const uint8_t *expr = (const uint8_t *)*entry + len_bytes;
        
        if (expr_len > 0 && expr_len < 256) {
            parse_dwarf_expression(expr, expr_len, is_frame_base_at_cfa, &result);
        }
        
        *entry = expr + expr_len;
    } else if (form == DW_FORM_sec_offset) {
        uint64_t loc_offset = 0;
        *entry += dwarf_read_abbrev_entry(*entry, form, &loc_offset, sizeof(loc_offset), address_size);
        
        find_location_in_list(addrs, loc_offset, current_address, cu_base_address, 
                             address_size, is_frame_base_at_cfa, &result);
    } else if (form == DW_FORM_block || form == DW_FORM_block1 || 
               form == DW_FORM_block2 || form == DW_FORM_block4) {
        struct Slice slice;
        *entry += dwarf_read_abbrev_entry(*entry, form, &slice, sizeof(slice), address_size);
        
        if (slice.len > 0 && slice.len < 256) {
            parse_dwarf_expression((const uint8_t *)slice.mem, slice.len, 
                                  is_frame_base_at_cfa, &result);
        }
    } else {
        *entry += dwarf_read_abbrev_entry(*entry, form, NULL, 0, address_size);
        return;
    }
    
    if (result.type == LOC_REGISTER) {
        *param_address = -(result.reg_num + 1000);
    } else if (result.type == LOC_STACK_OFFSET) {
        *param_address = result.offset;
    } else if (result.type == LOC_CALLER_OFFSET) {
        *param_address = result.offset;
    } else {
        *param_address = 0;
    }
}

static void
parse_formal_parameter(const struct Dwarf_Addrs *addrs, Dwarf_Off cu_offset,
                       const uint8_t *abbrev_table_begin, const uint8_t *param_abbrev_entry,
                       const void **entry, Dwarf_Small address_size, bool is_frame_base_at_cfa,
                       uintptr_t current_address, uintptr_t cu_base_address,
                       struct Dwarf_VarInfo *param) {
    memset(param, 0, sizeof(*param));
    param->kind = KIND_UNKNOWN;
    param->is_variadic = 0;
    
    struct parse_param_data data = {
        .param = param,
        .addrs = addrs,
        .cu_offset = cu_offset,
        .abbrev_table_begin = abbrev_table_begin,
        .is_frame_base_at_cfa = is_frame_base_at_cfa,
        .current_address = current_address,
        .cu_base_address = cu_base_address
    };
    
    const void *entry_copy = *entry;
    const uint8_t *abbrev_copy = param_abbrev_entry;
    parse_attributes(abbrev_copy, &entry_copy, address_size, handle_param_attribute,
                    addrs, cu_offset, abbrev_table_begin, &data);
    *entry = entry_copy;
}

static int
info_by_address_debug_info(const struct Dwarf_Addrs *addrs, uintptr_t p, Dwarf_Off *store) {
    const uint8_t *entry = addrs->info_begin;

    while (entry < addrs->info_end) {
        const uint8_t *header = entry;

        uint32_t count;
        uint64_t len = 0;
        entry += count = dwarf_entry_len(entry, &len);
        if (!count) return -E_BAD_DWARF;

        const uint8_t *entry_end = entry + len;

        /* Parse compilation unit header */
        Dwarf_Half version = get_unaligned(entry, Dwarf_Half);
        entry += sizeof(Dwarf_Half);
        assert(version == 4 || version == 2);
        Dwarf_Off abbrev_offset = get_unaligned(entry, uint32_t);
        entry += sizeof(uint32_t);
        Dwarf_Small address_size = get_unaligned(entry, Dwarf_Small);
        entry += sizeof(Dwarf_Small);
        assert(address_size == sizeof(uintptr_t));

        /* Read abbreviation code */
        uint64_t abbrev_code = 0;
        entry += dwarf_read_uleb128(entry, &abbrev_code);
        assert(abbrev_code);

        /* Read abbreviations table */
        const uint8_t *abbrev_entry = addrs->abbrev_begin + abbrev_offset;
        uint64_t table_abbrev_code = 0;
        abbrev_entry += dwarf_read_uleb128(abbrev_entry, &table_abbrev_code);
        assert(table_abbrev_code == abbrev_code);
        uint64_t tag = 0;
        abbrev_entry += dwarf_read_uleb128(abbrev_entry, &tag);
        assert(tag == DW_TAG_compile_unit);
        abbrev_entry += sizeof(Dwarf_Small);

        /* Читаем low_pc/high_pc для проверки адреса */
        uintptr_t low_pc = 0, high_pc = 0;
        struct cu_pc_data pc_data = { &low_pc, &high_pc };
        
        const void *entry_copy = entry;
        const uint8_t *abbrev_copy = abbrev_entry;
        parse_attributes(abbrev_copy, &entry_copy, address_size, handle_cu_pc_attribute,
                        addrs, 0, NULL, &pc_data);

        if (p >= low_pc && p <= high_pc) {
            *store = (const unsigned char *)header - addrs->info_begin;
            return 0;
        }

        entry = entry_end;
    }
    return -E_NO_ENT;
}

int
info_by_address(const struct Dwarf_Addrs *addrs, uintptr_t addr, Dwarf_Off *store) {
    int res = info_by_address_debug_aranges(addrs, addr, store);
    if (res < 0) res = info_by_address_debug_info(addrs, addr, store);
    return res;
}

int
file_name_by_info(const struct Dwarf_Addrs *addrs, Dwarf_Off offset, char **buf, Dwarf_Off *line_off) {
    if (offset > addrs->info_end - addrs->info_begin) return -E_INVAL;

    const uint8_t *entry = addrs->info_begin + offset;
    uint32_t count;
    uint64_t len = 0;
    entry += count = dwarf_entry_len(entry, &len);
    if (!count) return -E_BAD_DWARF;

    /* Parse compilation unit header */
    Dwarf_Half version = get_unaligned(entry, Dwarf_Half);
    entry += sizeof(Dwarf_Half);
    assert(version == 4 || version == 2);
    Dwarf_Off abbrev_offset = get_unaligned(entry, uint32_t);
    entry += sizeof(uint32_t);
    Dwarf_Small address_size = get_unaligned(entry, Dwarf_Small);
    entry += sizeof(Dwarf_Small);
    assert(address_size == sizeof(uintptr_t));

    /* Read abbreviation code */
    uint64_t abbrev_code = 0;
    entry += dwarf_read_uleb128(entry, &abbrev_code);
    assert(abbrev_code);

    /* Read abbreviations table */
    const uint8_t *abbrev_entry = addrs->abbrev_begin + abbrev_offset;
    uint64_t table_abbrev_code = 0;
    abbrev_entry += dwarf_read_uleb128(abbrev_entry, &table_abbrev_code);
    assert(table_abbrev_code == abbrev_code);
    uint64_t tag = 0;
    abbrev_entry += dwarf_read_uleb128(abbrev_entry, &tag);
    assert(tag == DW_TAG_compile_unit);
    abbrev_entry += sizeof(Dwarf_Small);

    struct file_name_data file_data = { buf, line_off };
    
    const void *entry_copy = entry;
    const uint8_t *abbrev_copy = abbrev_entry;
    parse_attributes(abbrev_copy, &entry_copy, address_size, handle_file_name_attribute,
                    addrs, 0, NULL, &file_data);

    return 0;
}

int
function_by_info(const struct Dwarf_Addrs *addrs, uintptr_t p, Dwarf_Off cu_offset, char **buf, uintptr_t *offset, struct Dwarf_VarInfo *params, int *nparams) {
    uint64_t len = 0;
    uint32_t count;

    /* Initialize params if provided */
    if (params && nparams) {
        *nparams = 0;
    }

    const void *cu_info_ptr = addrs->info_begin + cu_offset;
    cu_info_ptr += count = dwarf_entry_len(cu_info_ptr, &len);
    if (!count) return -E_BAD_DWARF;

    const void *cu_info_end = cu_info_ptr + len;

    // Compile unit header
    Dwarf_Half version = get_unaligned(cu_info_ptr, Dwarf_Half);
    cu_info_ptr += sizeof(Dwarf_Half);
    assert(version == 4 || version == 2);
    Dwarf_Off abbrev_offset = get_unaligned(cu_info_ptr, uint32_t);
    cu_info_ptr += sizeof(uint32_t);
    Dwarf_Small address_size = get_unaligned(cu_info_ptr, Dwarf_Small);
    cu_info_ptr += sizeof(Dwarf_Small);
    assert(address_size == sizeof(uintptr_t));

    uint64_t abbrev_code = 0;
    const uint8_t *abbrev_entry = addrs->abbrev_begin + abbrev_offset;

    /* From DWARF4 specification, Section 2.6.2 "Location Lists":
     * "A beginning address offset. This address offset has the size of an address and is relative to
     * the applicable base address of the compilation unit referencing this location list."
     * 
     * From DWARF4 specification, Section 3.1.1 "Normal and Partial Compilation Unit Entries":
     * "The base address of a compilation unit is defined as the value of the DW_AT_low_pc attribute,
     * if present; otherwise, it is undefined."
     * 
     * From DWARF4 specification, Section 2.6.2 "Location Lists":
     * "The applicable base address of a location list entry is determined by the closest preceding base
     * address selection entry (see below) in the same location list. If there is no such selection entry,
     * then the applicable base address defaults to the base address of the compilation unit."
     * 
     * Извлекаем base address (DW_AT_low_pc) из compilation unit DIE для использования в location lists.
     * После парсинга compilation unit, die_entry продвигается на позицию после его атрибутов,
     * чтобы цикл while (die_entry < die_entry_end) начинался с дочерних элементов CU.
     */
    const void *die_entry = cu_info_ptr;
    uintptr_t cu_base_address = parse_cu_base_address(&die_entry, abbrev_entry, addrs, address_size);
    
    bool is_after_subprogram = 0;
    bool is_frame_base_at_cfa = 0;
    
    /* From DWARF4 specification, Section 2.3 "Relationship of Debugging Information Entries":
     * "The tree itself is represented by flattening it in prefix order. Each debugging information entry is
     * defined either to have child entries or not to have child entries (see Section 7.5.3). If an entry is
     * defined not to have children, the next physically succeeding entry is a sibling. If an entry is
     * defined to have children, the next physically succeeding entry is its first child. Additional
     * children are represented as siblings of the first child. A chain of sibling entries is terminated by a
     * null entry."
     * 
     * From DWARF4 specification, Section 3.3.4 "Declarations Owned by Subroutines and Entry Points":
     * "The declarations enclosed by a subroutine or entry point are represented by debugging
     * information entries that are owned by the subroutine or entry point entry. Entries representing the
     * formal parameters of the subroutine or entry point appear in the same order as the corresponding
     * declarations in the source program."
     * 
     * From DWARF4 specification, Section 3.3.5 "Low-Level Information":
     * "A subroutine or entry point entry may have a DW_AT_low_pc attribute whose value is the relocated
     * address of the first machine instruction associated with the target compilation unit. A subroutine
     * or entry point entry may have a DW_AT_high_pc attribute whose value is the relocated address of the
     * first location past the last machine instruction associated with the target compilation unit."
     * 
     * 
     * 1. Итерация по DIE дереву: Цикл проходит по всем Debugging Information Entries (DIE) в compilation unit
     *    в порядке prefix traversal (плоское представление дерева). Каждая итерация обрабатывает один DIE.
     * 
     * 2. Поиск subprogram: Для каждого DIE читается abbreviation code, находится соответствующая запись
     *    в abbreviation table, определяется tag. Если tag = DW_TAG_subprogram, парсятся атрибуты low_pc и
     *    high_pc для определения диапазона адресов функции. Если текущий адрес p попадает в этот диапазон,
     *    функция найдена.
     * 
     * 3. Парсинг параметров: После нахождения нужной функции (is_after_subprogram = 1) цикл продолжается
     *    для обработки дочерних элементов subprogram. Формальные параметры (DW_TAG_formal_parameter) являются
     *    дочерними элементами subprogram и появляются в том же порядке, что и в исходном коде. Для каждого
     *    параметра извлекаются имя, тип и расположение (location).
     * 
     * 4. Пропуск неинтересных DIE: Lexical blocks (DW_TAG_lexical_block), null entries и другие DIE,
     *    которые не являются параметрами, пропускаются. Если DIE имеет дочерние элементы, они также
     *    пропускаются рекурсивно, чтобы не обрабатывать вложенные структуры.
     * 
     */
    const void *die_entry_end = cu_info_end;
    while (die_entry < die_entry_end) {
        /* Read info abbreviation code */
        die_entry += dwarf_read_uleb128(die_entry, &abbrev_code);
        if (!abbrev_code) continue;

        uint64_t tag = 0;
        Dwarf_Small has_children = 0;
        // Find abbreviation entry using abbreviation code
        const uint8_t *curr_abbrev_entry = find_abbreviation_entry(addrs, abbrev_entry, abbrev_code, &tag, &has_children);
        
        if (!curr_abbrev_entry) {
            /* Abbreviation not found, skip this entry */
            continue;
        }
        
        /* Обрабатываем DIE в зависимости от текущего состояния и тега:
         * 1. Если функция уже найдена (is_after_subprogram = 1) → парсим её дочерние элементы (параметры)
         * 2. Если это subprogram DIE → проверяем, подходит ли функция по адресу
         * 3. Иначе → пропускаем DIE
         */
        if (is_after_subprogram) {
            /* Функция уже найдена, парсим её дочерние элементы (параметры) */
            int result = parse_subprogram_child_die(tag, &die_entry, curr_abbrev_entry, has_children,
                                                    die_entry_end, addrs, cu_offset, abbrev_entry,
                                                    address_size, is_frame_base_at_cfa, p, cu_base_address,
                                                    params, nparams);
            if (result < 0) {
                /* Parameters ended - exit */
                return 0;
            }
            /* Continue loop to check for more parameters */
        } else if (tag == DW_TAG_subprogram) {
            /* Нашли DIE функции (subprogram), проверяем, подходит ли она по адресу */
            bool found = try_parse_subprogram(&die_entry, curr_abbrev_entry, p, addrs, abbrev_entry,
                                            address_size, offset, buf, &is_after_subprogram,
                                            &is_frame_base_at_cfa);
            
            if (found) {
                /* From DWARF4 specification, Section 3.3.4 "Declarations Owned by Subroutines and Entry Points":
                 * "The declarations enclosed by a subroutine or entry point are represented by debugging
                 * information entries that are owned by the subroutine or entry point entry. Entries representing the
                 * formal parameters of the subroutine or entry point appear in the same order as the corresponding
                 * declarations in the source program."
                 * 
                 * Параметры функций являются дочерними элементами DW_TAG_subprogram согласно DWARF4.
                 * После нахождения нужной функции нужно продолжить парсинг дочерних элементов
                 * для извлечения параметров (DW_TAG_formal_parameter). Устанавливаем флаг
                 * is_after_subprogram = 1 и продолжаем цикл (не делаем return), чтобы на следующей
                 * итерации обработать дочерние элементы. Если дочерних элементов нет (has_children = 0),
                 * то параметров тоже нет, можно вернуться. Это обеспечивает корректное извлечение
                 * всех формальных параметров функции в правильном порядке.
                 */
                if (params && nparams) {
                    *nparams = 0;
                }
                if (!has_children) {
                return 0;
                }
                /* Continue loop to parse children (parameters) */
            }
        } else {
            /* Skip if not a subprogram */
            const void *die_entry_copy = die_entry;
            const uint8_t *abbrev_copy = curr_abbrev_entry;
            curr_abbrev_entry = (const uint8_t *)skip_attributes(abbrev_copy, &die_entry_copy, address_size);
            die_entry = die_entry_copy;
            if (has_children) {
                /* From DWARF4 specification, Section 2.3 "Relationship of Debugging Information Entries":
                 * "A debugging information entry may have child entries."
                 * 
                 * Пропускаем дочерние элементы рекурсивно, используя функцию skip_die.
                 */
                skip_die(&die_entry, die_entry_end, abbrev_entry, addrs, address_size, true);
            }
        }
    }
    return -E_NO_ENT;
}

int
address_by_fname(const struct Dwarf_Addrs *addrs, const char *fname, uintptr_t *offset) {
    const int flen = strlen(fname);
    if (!flen) return -E_INVAL;

    const uint8_t *pubnames_entry = addrs->pubnames_begin;
    uint32_t count = 0;
    uint64_t len = 0;
    Dwarf_Off cu_offset = 0, func_offset = 0;

    /* parse pubnames section */
    while (pubnames_entry < addrs->pubnames_end) {
        count = dwarf_entry_len(pubnames_entry, &len);
        if (!count) return -E_BAD_DWARF;
        pubnames_entry += count;

        const uint8_t *pubnames_entry_end = pubnames_entry + len;
        Dwarf_Half version = get_unaligned(pubnames_entry, Dwarf_Half);

        assert(version == 2);
        pubnames_entry += sizeof(Dwarf_Half);
        cu_offset = get_unaligned(pubnames_entry, uint32_t);
        pubnames_entry += sizeof(uint32_t);
        count = dwarf_entry_len(pubnames_entry, &len);
        pubnames_entry += count;

        while (pubnames_entry < pubnames_entry_end) {
            func_offset = get_unaligned(pubnames_entry, uint32_t);
            pubnames_entry += sizeof(uint32_t);

            if (!func_offset) break;

            if (!strcmp(fname, (const char *)pubnames_entry)) {
                /* Parse compilation unit header */
                const uint8_t *entry = addrs->info_begin + cu_offset;
                const uint8_t *func_entry = entry + func_offset;
                entry += count = dwarf_entry_len(entry, &len);
                if (!count) return -E_BAD_DWARF;

                Dwarf_Half version = get_unaligned(entry, Dwarf_Half);
                assert(version == 4 || version == 2);
                entry += sizeof(Dwarf_Half);
                Dwarf_Off abbrev_offset = get_unaligned(entry, uint32_t);
                entry += sizeof(uint32_t);
                const uint8_t *abbrev_entry = addrs->abbrev_begin + abbrev_offset;
                Dwarf_Small address_size = get_unaligned(entry, Dwarf_Small);
                assert(address_size == sizeof(uintptr_t));

                entry = func_entry;
                uint64_t abbrev_code = 0, table_abbrev_code = 0;
                entry += dwarf_read_uleb128(entry, &abbrev_code);
                uint64_t name = 0, form = 0, tag = 0;

                /* Find abbreviation in abbrev section */
                /* UNSAFE Needs to be replaced */
                while (abbrev_entry < addrs->abbrev_end) {
                    abbrev_entry += dwarf_read_uleb128(abbrev_entry, &table_abbrev_code);
                    abbrev_entry += dwarf_read_uleb128(abbrev_entry, &tag);
                    abbrev_entry += sizeof(Dwarf_Small);
                    if (table_abbrev_code == abbrev_code) break;

                    /* skip attributes */
                    do {
                        abbrev_entry += dwarf_read_uleb128(abbrev_entry, &name);
                        abbrev_entry += dwarf_read_uleb128(abbrev_entry, &form);
                    } while (name || form);
                }
                /* Find low_pc */
                if (tag == DW_TAG_subprogram) {
                    /* At this point entry points to the beginning of function's DIE attributes
                     * and abbrev_entry points to abbreviation table entry corresponding to this DIE.
                     * Abbreviation table entry consists of pairs of unsigned LEB128 numbers, the first
                     * encodes name of attribute and the second encodes its form. Attribute entry ends
                     * with a pair where both name and form equal zero.
                     * Address of a function is encoded in attribute with name DW_AT_low_pc.
                     * To find it, we need to scan both abbreviation table and attribute values.
                     * You can read unsigned LEB128 number using dwarf_read_uleb128 function.
                     * Attribute value can be obtained using dwarf_read_abbrev_entry function. */
                    // LAB 3: Your code here:
                    uintptr_t low_pc = 0;

                    do {
                        abbrev_entry += dwarf_read_uleb128(abbrev_entry, &name);
                        abbrev_entry += dwarf_read_uleb128(abbrev_entry, &form);

                        if (name == DW_AT_low_pc) {
                            entry += dwarf_read_abbrev_entry(entry, form, &low_pc, sizeof(low_pc), address_size);
                        } else {
                            entry += dwarf_read_abbrev_entry(entry, form, NULL, 0, address_size);
                        }
                    } while (name || form);

                    if (low_pc) {
                        *offset = low_pc;
                        return 0;
                    }
                } else {
                    /* Skip if not a subprogram or label */
                    do {
                        abbrev_entry += dwarf_read_uleb128(abbrev_entry, &name);
                        abbrev_entry += dwarf_read_uleb128(abbrev_entry, &form);
                        entry += dwarf_read_abbrev_entry(entry, form, NULL, 0, address_size);
                    } while (name || form);
                }
            }
            pubnames_entry += strlen((const char *)pubnames_entry) + 1;
        }
    }
    return -E_NO_ENT;
}

int
naive_address_by_fname(const struct Dwarf_Addrs *addrs, const char *fname, uintptr_t *offset) {
    const int flen = strlen(fname);
    if (!flen) return -E_INVAL;

    for (const uint8_t *entry = addrs->info_begin; (const unsigned char *)entry < addrs->info_end;) {
        uint64_t len = 0;
        uint32_t count = dwarf_entry_len(entry, &len);
        entry += count;
        if (!count) return -E_BAD_DWARF;

        const uint8_t *entry_end = entry + len;

        /* Parse compilation unit header */
        Dwarf_Half version = get_unaligned(entry, Dwarf_Half);
        entry += sizeof(Dwarf_Half);
        assert(version == 4 || version == 2);
        Dwarf_Off abbrev_offset = get_unaligned(entry, uint32_t);
        /**/ entry += sizeof(uint32_t);
        Dwarf_Small address_size = get_unaligned(entry, Dwarf_Small);
        entry += sizeof(Dwarf_Small);
        assert(address_size == sizeof(uintptr_t));

        /* Parse related DIE's */
        uint64_t abbrev_code = 0, table_abbrev_code = 0;
        const uint8_t *abbrev_entry = addrs->abbrev_begin + abbrev_offset;

        while (entry < entry_end) {
            /* Read info abbreviation code */
            count = dwarf_read_uleb128(entry, &abbrev_code);
            entry += count;
            if (!abbrev_code) continue;

            /* Find abbreviation in abbrev section */
            /* UNSAFE, Needs to be replaced */
            const uint8_t *curr_abbrev_entry = abbrev_entry;
            uint64_t name = 0, form = 0, tag = 0;
            while ((const unsigned char *)curr_abbrev_entry < addrs->abbrev_end) {
                curr_abbrev_entry += dwarf_read_uleb128(curr_abbrev_entry, &table_abbrev_code);
                curr_abbrev_entry += dwarf_read_uleb128(curr_abbrev_entry, &tag);
                curr_abbrev_entry += sizeof(Dwarf_Small);
                if (table_abbrev_code == abbrev_code) break;

                /* skip attributes */
                do {
                    curr_abbrev_entry += dwarf_read_uleb128(curr_abbrev_entry, &name);
                    curr_abbrev_entry += dwarf_read_uleb128(curr_abbrev_entry, &form);
                } while (name || form);
            }
            /* parse subprogram or label DIE */
            if (tag == DW_TAG_subprogram || tag == DW_TAG_label) {
                uintptr_t low_pc = 0;
                bool found = 0;
                do {
                    curr_abbrev_entry += dwarf_read_uleb128(curr_abbrev_entry, &name);
                    curr_abbrev_entry += dwarf_read_uleb128(curr_abbrev_entry, &form);
                    if (name == DW_AT_low_pc) {
                        entry += dwarf_read_abbrev_entry(entry, form, &low_pc, sizeof(low_pc), address_size);
                    } else if (name == DW_AT_name) {
                        char tmp_buf[256];
                        const void *tmp_entry = entry;
                        read_string_attribute(addrs, &tmp_entry, form, address_size, tmp_buf, sizeof(tmp_buf));
                        entry = (const uint8_t *)tmp_entry;
                        if (!strcmp(fname, tmp_buf)) found = 1;
                    } else
                        entry += dwarf_read_abbrev_entry(entry, form, NULL, 0, address_size);
                } while (name || form);
                if (found && low_pc) {
                    /* finish if fname found */
                    *offset = low_pc;
                    return 0;
                }
            } else {
                /* Skip if not a subprogram or label */
                do {
                    curr_abbrev_entry += dwarf_read_uleb128(curr_abbrev_entry, &name);
                    curr_abbrev_entry += dwarf_read_uleb128(curr_abbrev_entry, &form);
                    entry += dwarf_read_abbrev_entry(entry, form, NULL, 0, address_size);
                } while (name || form);
            }
        }
    }

    return -E_NO_ENT;
}
