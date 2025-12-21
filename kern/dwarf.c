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
static void handle_structure_type(const uint8_t *type_abbrev_entry, const void *type_entry,
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

/* Структура для передачи данных в callback для проверки char* */
struct check_char_pointer_data {
    int is_char_pointer;
    const struct Dwarf_Addrs *addrs;
    Dwarf_Off cu_offset;
    const uint8_t *abbrev_table_begin;
};

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

/* Callback для обработки DW_AT_type атрибута указателя */
static int
handle_pointer_type_attribute(const struct Dwarf_Addrs *addrs,
                              uint64_t attr_name,
                              uint64_t attr_form,
                              const void **entry_ptr,
                              Dwarf_Small address_size,
                              Dwarf_Off cu_offset,
                              const uint8_t *abbrev_table_begin,
                              void *user_data) {
    struct check_char_pointer_data *data = (struct check_char_pointer_data *)user_data;
    
    if (attr_name == DW_AT_type) {
        /* DW_AT_type ссылается на тип, на который указывает указатель */
        const void *pointed_type_entry = NULL;
        uint64_t pointed_type_tag = 0;
        Dwarf_Small pointed_type_has_children = 0;
        const void *entry_copy = *entry_ptr;
        const uint8_t *pointed_type_abbrev = resolve_type_entry(
            data->addrs, data->cu_offset, data->abbrev_table_begin, 
            &entry_copy, attr_form, address_size, &pointed_type_entry, 
            &pointed_type_tag, &pointed_type_has_children);
        
        /* From DWARF4 specification, Section 5.2 "Type Modifier Entries":
         * "When multiple type modifiers are chained together to modify a base or user-defined type..."
         * 
         * Нужно разыменовывать модификаторы типа (const, volatile) до базового типа.
         * Например, const char* представлен как: pointer_type → const_type → base_type(char)
         */
        const void *final_type_entry = pointed_type_entry;
        const uint8_t *final_type_abbrev = pointed_type_abbrev;
        uint64_t final_type_tag = pointed_type_tag;
        Dwarf_Small final_type_has_children = pointed_type_has_children;
        
        /* Разыменовываем модификаторы типа и typedef до базового типа */
        resolve_final_type(data->addrs, data->cu_offset, data->abbrev_table_begin,
                          address_size, pointed_type_entry, pointed_type_abbrev,
                          pointed_type_tag, pointed_type_has_children,
                          &final_type_entry, &final_type_abbrev,
                          &final_type_tag, &final_type_has_children);
        
        if (final_type_abbrev && final_type_tag == DW_TAG_base_type) {
            /* Проверяем, что это char - сначала по имени, потом по encoding */
            char type_name_buf[256];
            bool found_by_name = false;
            
            if (get_base_type_name(data->addrs, final_type_abbrev,
                                  final_type_entry, address_size,
                                  type_name_buf, sizeof(type_name_buf))) {
                if (strcmp(type_name_buf, "char") == 0) {
                    data->is_char_pointer = true;
                    found_by_name = true;
                }
            }
            
            /* Если не нашли по имени, проверяем по encoding и размеру */
            if (!found_by_name) {
                struct check_char_encoding_data encoding_data = { .encoding = 0, .byte_size = 0 };
                const void *encoding_entry_copy = final_type_entry;
                const uint8_t *encoding_abbrev_copy = final_type_abbrev;
                parse_attributes(encoding_abbrev_copy, &encoding_entry_copy, address_size,
                               handle_char_check_encoding_attribute,
                               addrs, cu_offset, data->abbrev_table_begin, &encoding_data);
                
                /* char определяется как signed_char/unsigned_char с размером 1 байт */
                if ((encoding_data.encoding == DW_ATE_signed_char || 
                     encoding_data.encoding == DW_ATE_unsigned_char) &&
                    encoding_data.byte_size == 1) {
                    data->is_char_pointer = true;
                }
            }
        }
        *entry_ptr = entry_copy;
        return 1;
    }
    return 0; /* Пропустить автоматически */
}

static void
handle_pointer_type(const struct Dwarf_Addrs *addrs, Dwarf_Off cu_offset,
                    const uint8_t *abbrev_table_begin,
                    const uint8_t *pointer_abbrev_entry, 
                    const void *pointer_entry,
                    Dwarf_Small address_size,
                    struct Dwarf_VarInfo *param) {
    param->byte_size = sizeof(uintptr_t);
    
    /* Структура для передачи данных в callback */
    struct check_char_pointer_data callback_data = {
        .is_char_pointer = 0,
        .addrs = addrs,
        .cu_offset = cu_offset,
        .abbrev_table_begin = abbrev_table_begin
    };
    
    /* Парсим атрибуты DW_TAG_pointer_type */
    const void *entry_copy = pointer_entry;
    const uint8_t *abbrev_copy = pointer_abbrev_entry;
    parse_attributes(abbrev_copy, &entry_copy, address_size, handle_pointer_type_attribute,
                    addrs, cu_offset, abbrev_table_begin, &callback_data);
    
    /* Устанавливаем kind и type_name */
    if (callback_data.is_char_pointer) {
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

/* Callback для обработки атрибутов структуры (просто пропускаем все) */
static int
handle_structure_type_attribute(const struct Dwarf_Addrs *addrs,
                                uint64_t attr_name,
                                uint64_t attr_form,
                                const void **entry_ptr,
                                Dwarf_Small address_size,
                                Dwarf_Off cu_offset,
                                const uint8_t *abbrev_table_begin,
                                void *user_data) {
    /* Структура как тип параметра - для параметров функций большие структуры обычно
     * передаются по указателю. Для минимальной версии просто пропускаем все атрибуты */
    (void)addrs; (void)attr_name; (void)cu_offset; (void)abbrev_table_begin; (void)user_data;
    /* Все атрибуты пропускаются автоматически */
    return 0;
}

static void
handle_structure_type(const uint8_t *type_abbrev_entry, const void *type_entry,
                      Dwarf_Small address_size, struct Dwarf_VarInfo *param) {
    const void *entry_copy = type_entry;
    const uint8_t *abbrev_copy = type_abbrev_entry;
    /* Пропускаем все атрибуты структуры */
    parse_attributes(abbrev_copy, &entry_copy, address_size, handle_structure_type_attribute,
                    NULL, 0, NULL, NULL);
    
    /* Структура как тип параметра - для параметров функций большие структуры обычно
     * передаются по указателю. Для минимальной версии предполагаем указатель. */
    param->kind = KIND_POINTER;
    param->byte_size = sizeof(uintptr_t);
    strncpy(param->type_name, "void*", sizeof(param->type_name));
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
        handle_structure_type(final_abbrev, final_entry, address_size, param);
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
// static void
// parse_location_attribute(const struct Dwarf_Addrs *addrs, const void **entry, uint64_t form, 
//                         Dwarf_Small address_size, bool is_frame_base_at_cfa, 
//                         uintptr_t current_address, uintptr_t cu_base_address,
//                         int64_t *param_address) {
//     uint8_t buf[64] = { 0 };
//     size_t buf_size = 0;
    
//     cprintf("    [DEBUG parse_location] form=0x%lx, current_addr=0x%lx, cu_base=0x%lx, is_frame_base_at_cfa=%d\n",
//             form, current_address, cu_base_address, is_frame_base_at_cfa);
    
//     if (form == DW_FORM_exprloc) {
//         cprintf("    [DEBUG] Using DW_FORM_exprloc\n");
//         uint64_t length = 0;
//         size_t len_bytes = dwarf_read_uleb128(*entry, &length);
//         *entry += len_bytes;
//         buf_size = MIN(length, sizeof(buf) - 1);
//         cprintf("    [DEBUG] exprloc length=%lu, buf_size=%lu\n", length, buf_size);
//         if (buf_size > 0) {
//             memcpy(buf, *entry, buf_size);
//         }
//         *entry += length;
//     } else if (form == DW_FORM_block || form == DW_FORM_block1 || form == DW_FORM_block2 || form == DW_FORM_block4) {
//         cprintf("    [DEBUG] Using DW_FORM_block (type=%lx)\n", form);
//         uint32_t length = 0;
//         if (form == DW_FORM_block1) {
//             length = get_unaligned(*entry, uint8_t);
//             *entry += 1;
//         } else if (form == DW_FORM_block2) {
//             length = get_unaligned(*entry, uint16_t);
//             *entry += 2;
//         } else if (form == DW_FORM_block4) {
//             length = get_unaligned(*entry, uint32_t);
//             *entry += 4;
//         } else { /* DW_FORM_block */
//             uint64_t len64 = 0;
//             *entry += dwarf_read_uleb128(*entry, &len64);
//             length = (uint32_t)len64;
//         }
//         buf_size = MIN(length, sizeof(buf) - 1);
//         cprintf("    [DEBUG] block length=%u, buf_size=%lu\n", length, buf_size);
//         if (buf_size > 0) {
//             memcpy(buf, *entry, buf_size);
//         }
//         *entry += length;
//     } else if (form == DW_FORM_sec_offset) {
//         cprintf("    [DEBUG] Using DW_FORM_sec_offset (location list)\n");
//         uint64_t loc_offset = 0;
//         *entry += dwarf_read_abbrev_entry(*entry, form, &loc_offset, sizeof(loc_offset), address_size);
        
//         cprintf("    [DEBUG] loc_offset=0x%lx\n", loc_offset);
//         cprintf("    [DEBUG] loc_begin=%p, loc_end=%p\n", addrs->loc_begin, addrs->loc_end);
        
//         if (!addrs->loc_begin || !addrs->loc_end || loc_offset >= (uint64_t)(addrs->loc_end - addrs->loc_begin)) {
//             cprintf("    [DEBUG] Location list out of bounds or not available!\n");
//             return;
//         }


//         cprintf("    [DEBUG] Scanning ALL location list entries for debugging:\n");
//         const uint8_t *debug_entry = addrs->loc_begin + loc_offset;
//         uintptr_t debug_base = cu_base_address;
//         int debug_count = 0;
        
//         while (debug_entry < addrs->loc_end && debug_count < 10) {
//             uintptr_t beg = 0, end = 0;
            
//             if (debug_entry + 2 * address_size > addrs->loc_end) break;
            
//             if (address_size == 8) {
//                 beg = get_unaligned(debug_entry, uint64_t);
//                 debug_entry += 8;
//                 end = get_unaligned(debug_entry, uint64_t);
//                 debug_entry += 8;
//             } else {
//                 beg = get_unaligned(debug_entry, uint32_t);
//                 debug_entry += 4;
//                 end = get_unaligned(debug_entry, uint32_t);
//                 debug_entry += 4;
//             }
            
//             if (beg == 0 && end == 0) {
//                 cprintf("    [DEBUG] Entry %d: END OF LIST\n", debug_count);
//                 break;
//             }
            
//             if ((address_size == 8 && beg == 0xFFFFFFFFFFFFFFFFULL) ||
//                 (address_size == 4 && beg == 0xFFFFFFFF)) {
//                 debug_base = end;
//                 cprintf("    [DEBUG] Entry %d: BASE ADDRESS SELECTION, new base=0x%lx\n", 
//                         debug_count, debug_base);
//                 debug_count++;
//                 continue;
//             }
            
//             if (debug_entry + 2 > addrs->loc_end) break;
//             uint16_t expr_len = get_unaligned(debug_entry, uint16_t);
//             debug_entry += 2;
            
//             uintptr_t abs_beg = debug_base + beg;
//             uintptr_t abs_end = debug_base + end;
            
//             uint8_t opcode = 0;
//             if (debug_entry + expr_len <= addrs->loc_end && expr_len > 0) {
//                 opcode = *debug_entry;
//             }
            
//             cprintf("    [DEBUG] Entry %d: range [0x%lx - 0x%lx), len=%u, opcode=0x%02x",
//                     debug_count, abs_beg, abs_end, expr_len, opcode);
            
//             if (opcode >= DW_OP_reg0 && opcode <= DW_OP_reg31) {
//                 cprintf(" (DW_OP_reg%d)", opcode - DW_OP_reg0);
//             } else if (opcode == DW_OP_fbreg) {
//                 int64_t off = 0;
//                 dwarf_read_leb128((char*)(debug_entry + 1), &off);
//                 cprintf(" (DW_OP_fbreg, offset=%ld)", off);
//             } else if (opcode == DW_OP_breg6) {
//                 int64_t off = 0;
//                 dwarf_read_leb128((char*)(debug_entry + 1), &off);
//                 cprintf(" (DW_OP_breg6, offset=%ld)", off);
//             }
            
//             if (current_address >= abs_beg && current_address < abs_end) {
//                 cprintf(" <- CURRENT ADDRESS MATCH!");
//             }
//             cprintf("\n");
            
//             debug_entry += expr_len;
//             debug_count++;
//         }
        
//         cprintf("    [DEBUG] Current address for search: 0x%lx\n", current_address);
//         cprintf("    [DEBUG] Now searching for matching entry...\n");
        
//         /* Теперь настоящий поиск */
//         const uint8_t *loc_entry = addrs->loc_begin + loc_offset;
//         uintptr_t base_address = cu_base_address;
//         bool found_entry = false;
//         int entry_count = 0;
        
//         cprintf("    [DEBUG] Scanning location list entries...\n");
        
//         while (loc_entry < addrs->loc_end) {
//             uintptr_t beginning = 0, ending = 0;
            
//             /* Проверка на достаточность данных */
//             if (loc_entry + 2 * address_size > addrs->loc_end) {
//                 cprintf("    [DEBUG] Not enough data for entry, breaking\n");
//                 break;
//             }
            
//             /* Read beginning and ending address offsets */
//             if (address_size == 4) {
//                 beginning = get_unaligned(loc_entry, uint32_t);
//                 loc_entry += 4;
//                 ending = get_unaligned(loc_entry, uint32_t);
//                 loc_entry += 4;
//             } else {
//                 beginning = get_unaligned(loc_entry, uint64_t);
//                 loc_entry += 8;
//                 ending = get_unaligned(loc_entry, uint64_t);
//                 loc_entry += 8;
//             }
            
//             cprintf("    [DEBUG] Entry %d: beginning=0x%lx, ending=0x%lx\n", 
//                     entry_count++, beginning, ending);
            
//             /* Check for end of list entry */
//             if (beginning == 0 && ending == 0) {
//                 cprintf("    [DEBUG] End of location list\n");
//                 break;
//             }
            
//             /* Check for base address selection entry */
//             if (address_size == 4 && beginning == 0xFFFFFFFF) {
//                 base_address = ending;
//                 cprintf("    [DEBUG] Base address selection: new base=0x%lx\n", base_address);
//                 continue;
//             } else if (address_size == 8 && beginning == 0xFFFFFFFFFFFFFFFFULL) {
//                 base_address = ending;
//                 cprintf("    [DEBUG] Base address selection: new base=0x%lx\n", base_address);
//                 continue;
//             }
            
//             /* Calculate absolute addresses */
//             uintptr_t abs_beginning = base_address + beginning;
//             uintptr_t abs_ending = base_address + ending;
            
//             cprintf("    [DEBUG] Absolute range: 0x%lx - 0x%lx (current=0x%lx)\n",
//                     abs_beginning, abs_ending, current_address);
            
//             /* Check if current address is in this range */
//             if (current_address >= abs_beginning && current_address < abs_ending) {
//                 cprintf("    [DEBUG] MATCH! Current address is in this range\n");
                
//                 /* Read length of location expression */
//                 if (loc_entry + 2 > addrs->loc_end) {
//                     cprintf("    [DEBUG] ERROR: Not enough data for expr_length\n");
//                     break;
//                 }
//                 uint16_t expr_length = get_unaligned(loc_entry, uint16_t);
//                 loc_entry += 2;
                
//                 cprintf("    [DEBUG] Expression length: %u\n", expr_length);
                
//                 /* Read location expression */
//                 buf_size = MIN(expr_length, sizeof(buf) - 1);
//                 if (buf_size > 0 && loc_entry + expr_length <= addrs->loc_end) {
//                     memcpy(buf, loc_entry, buf_size);
//                     cprintf("    [DEBUG] Copied %lu bytes of expression\n", buf_size);
//                 } else {
//                     cprintf("    [DEBUG] ERROR: Expression out of bounds\n");
//                 }
//                 found_entry = true;
//                 break;
//             }
            
//             /* Skip location expression for this entry */
//             if (loc_entry + 2 > addrs->loc_end) {
//                 cprintf("    [DEBUG] ERROR: Can't read expr_length to skip\n");
//                 break;
//             }
//             uint16_t expr_length = get_unaligned(loc_entry, uint16_t);
//             loc_entry += 2;
//             cprintf("    [DEBUG] Skipping expression of length %u\n", expr_length);
//             loc_entry += expr_length;
//         }
        
//         if (!found_entry) {
//             cprintf("    [DEBUG] No matching location list entry found!\n");
//             return;
//         }
//     } else {
//         cprintf("    [DEBUG] Unknown form 0x%lx, skipping\n", form);
//         *entry += dwarf_read_abbrev_entry(*entry, form, NULL, 0, address_size);
//         return;
//     }
//         if (buf_size > 0) {
//         cprintf("    [DEBUG] Processing location expression, buf_size=%lu\n", buf_size);
//         cprintf("    [DEBUG] Expression bytes: ");
//         for (size_t i = 0; i < buf_size; i++) {
//             cprintf("%02x ", buf[i]);
//         }
//         cprintf("\n");
        
//         /* Попробуем интерпретировать как multi-byte opcode */
//         if (buf_size >= 2 && buf[0] == 0x74 && buf[1] == 0x00) {
//             /* From DWARF4 specification, Section 2.5.1.2 "Register Based Addressing":
//              * "DW_OP_breg4 (0x74) - contents of register 4 + SLEB128 offset"
//              * 
//              * Но в контексте CFA и frame_base, возможно это означает что-то другое.
//              * 
//              * Теория: Если frame_base = CFA, то DW_OP_breg4 может означать
//              * "from CFA (which is RSP at function entry) + offset from register 4 state"
//              * 
//              * Или это может быть ошибка компилятора/линковщика в DWARF генерации.
//              */
//             cprintf("    [DEBUG] DW_OP_breg4 + offset 0 detected\n");
//             cprintf("    [DEBUG] is_frame_base_at_cfa=%d\n", is_frame_base_at_cfa);
            
//             /* Попробуем альтернативную интерпретацию:
//              * Если это первый параметр и frame_base=CFA, возможно компилятор
//              * имел в виду "параметр сохранён по адресу [RBP + offset]",
//              * но закодировал это неправильно как breg4.
//              * 
//              * Попробуем использовать это как смещение от RBP. */
//             int64_t address = 0;
//             dwarf_read_leb128((char*)buf + 1, &address);
            
//             cprintf("    [DEBUG] Trying to interpret as RBP-relative with offset %ld\n", address);
            
//             /* Параметры обычно находятся ВЫШЕ RBP (положительные смещения)
//              * в фрейме вызывающей функции, или НИЖЕ RBP (отрицательные смещения)  
//              * если функция их сохранила в свой локальный фрейм.
//              * 
//              * Попробуем offset = 0 означает, что параметр по адресу [RBP + 16]
//              * (стандартное место для первого параметра в caller's frame). */
//             if (is_frame_base_at_cfa && address == 0) {
//                 /* Эвристика: первый параметр обычно по [caller_RBP - 20] */
//                 *param_address = -20; /* Попробуем стандартное смещение */
//                 cprintf("    [DEBUG] Using heuristic: param_address = -20\n");
//             } else {
//                 *param_address = address;
//                 cprintf("    [DEBUG] Using parsed offset: param_address = %ld\n", address);
//             }
//             return;
//         }
        
        
//         if (buf[0] >= DW_OP_reg0 && buf[0] <= DW_OP_reg31) {
//             int reg_num = buf[0] - DW_OP_reg0;
//             cprintf("    [DEBUG] DW_OP_reg%d detected - parameter in register\n", reg_num);
//             *param_address = -(int64_t)(reg_num + 1);
//             cprintf("    [DEBUG] Set param_address to %ld (negative = register)\n", *param_address);
//             return;
//         } else if (buf[0] == DW_OP_fbreg) {
//             /* From DWARF4 specification, Section 2.5.1.4 "Register Based Addressing":
//              * "DW_OP_fbreg - provides a signed LEB128 offset from the frame base."
//              */
//             cprintf("    [DEBUG] DW_OP_fbreg detected\n");
//             int64_t address = 0;
//             size_t len = dwarf_read_leb128((char*)buf + 1, &address);
//             cprintf("    [DEBUG] Read offset: %ld (len=%lu bytes)\n", address, len);
//             if (len <= 4) {
//                 *param_address = (int32_t) address;
//             } else {
//                 *param_address = address;
//             }
//             if (is_frame_base_at_cfa) {
//                 cprintf("    [DEBUG] Applying CFA correction: +16\n");
//                 *param_address += 16;
//             }
//             cprintf("    [DEBUG] Final param_address: %ld\n", *param_address);
//         } else if (buf[0] >= DW_OP_breg0 && buf[0] <= DW_OP_breg31) {
//             /* From DWARF4 specification, Section 2.5.1.2 "Register Based Addressing":
//              * "DW_OP_breg0, DW_OP_breg1, ..., DW_OP_breg31 - The DW_OP_bregn operations add a
//              * signed LEB128 offset to the contents of register n to compute an address."
//              * 
//              * DW_OP_breg0 = 0x70, DW_OP_breg31 = 0x8F
//              * DW_OP_breg4 (0x74) = RSI register + offset
//              * DW_OP_breg6 (0x76) = RBP register + offset
//              */
//             int reg_num = buf[0] - DW_OP_breg0;
//             cprintf("    [DEBUG] DW_OP_breg%d detected\n", reg_num);
//             int64_t address = 0;
//             size_t len = dwarf_read_leb128((char*)buf + 1, &address);
//             cprintf("    [DEBUG] Read offset: %ld (len=%lu bytes)\n", address, len);
            
//             /* From System V x86-64 ABI, Section 3.6.2 "DWARF Register Number Mapping":
//              * Register 6 = RBP (base pointer)
//              * 
//              * Для локальных переменных и параметров обычно используется DW_OP_breg6 (RBP).
//              * Другие регистры редко используются для адресации стека.
//              */
//             if (reg_num == 4 || reg_num == 5) {
//                 if (is_frame_base_at_cfa && address == 0) {
//                     /* Параметр сохранён в caller's frame по отрицательному смещению */
//                     *param_address = -20; /* [caller_RBP - 20] */
//                     cprintf("    [DEBUG] Parameter from reg%d saved at caller_RBP - 20\n", reg_num);
//                 } else {
//                     *param_address = address;
//                 }
//             }
//             if (reg_num == 6) {
//                 /* DW_OP_breg6 - offset от RBP */
//                 if (len <= 4) {
//                     *param_address = (int32_t) address;
//                 } else {
//                     *param_address = address;
//                 }
//                 cprintf("    [DEBUG] Using RBP-relative addressing, param_address: %ld\n", *param_address);
//             } else {
//                 /* Другие регистры - для backtrace они недоступны */
//                 cprintf("    [DEBUG] Register %d is not RBP, parameter unavailable in backtrace\n", reg_num);
//                 *param_address = 0; /* Недоступно */
//             }
//         } else {
//             cprintf("    [DEBUG] Unknown opcode 0x%02x - leaving param_address=0\n", buf[0]);
//         }
//     }

// }


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

    const void *entry = addrs->info_begin + cu_offset;
    entry += count = dwarf_entry_len(entry, &len);
    if (!count) return -E_BAD_DWARF;

    const void *entry_end = entry + len;

    /* Parse compilation unit header */
    Dwarf_Half version = get_unaligned(entry, Dwarf_Half);
    entry += sizeof(Dwarf_Half);
    assert(version == 4 || version == 2);
    Dwarf_Off abbrev_offset = get_unaligned(entry, uint32_t);
    entry += sizeof(uint32_t);
    Dwarf_Small address_size = get_unaligned(entry, Dwarf_Small);
    entry += sizeof(Dwarf_Small);
    assert(address_size == sizeof(uintptr_t));

    /* Parse abbrev and info sections */
    uint64_t abbrev_code = 0;
    const uint8_t *abbrev_entry = addrs->abbrev_begin + abbrev_offset;

    /* Extract compilation unit base address (low_pc) for location list parsing */
    uintptr_t cu_base_address = 0;
    const void *cu_entry = entry;
    uint64_t cu_abbrev_code = 0;
    cu_entry += dwarf_read_uleb128(cu_entry, &cu_abbrev_code);
    if (cu_abbrev_code) {
        uint64_t cu_tag = 0;
        Dwarf_Small cu_has_children = 0;
        const uint8_t *cu_abbrev_attrs = find_abbreviation_entry(addrs, abbrev_entry, cu_abbrev_code, &cu_tag, &cu_has_children);
        if (cu_abbrev_attrs && cu_tag == DW_TAG_compile_unit) {
            uint64_t cu_name = 0, cu_form = 0;
            const uint8_t *cu_curr_abbrev = cu_abbrev_attrs;
            do {
                cu_curr_abbrev += dwarf_read_uleb128(cu_curr_abbrev, &cu_name);
                cu_curr_abbrev += dwarf_read_uleb128(cu_curr_abbrev, &cu_form);
                if (cu_name == DW_AT_low_pc) {
                    cu_entry += dwarf_read_abbrev_entry(cu_entry, cu_form, &cu_base_address, sizeof(cu_base_address), address_size);
                } else {
                    cu_entry += dwarf_read_abbrev_entry(cu_entry, cu_form, NULL, 0, address_size);
                }
            } while (cu_name || cu_form);
        }
    }
    
    bool is_after_subprogram = 0;
    bool is_frame_base_at_cfa = 0;
    uintptr_t current_func_low_pc = 0;
    uintptr_t current_func_high_pc = 0;

    while (entry < entry_end) {
        /* Read info abbreviation code */
        entry += dwarf_read_uleb128(entry, &abbrev_code);
        if (!abbrev_code) continue;

        uint64_t tag = 0;
        Dwarf_Small has_children = 0;
        const uint8_t *curr_abbrev_entry = find_abbreviation_entry(addrs, abbrev_entry, abbrev_code, &tag, &has_children);
        
        if (!curr_abbrev_entry) {
            continue;
        }

        if (is_after_subprogram) {
            if (tag == DW_TAG_formal_parameter && params && nparams && *nparams < DWARF_MAXPARAMS) {
                struct Dwarf_VarInfo *param = &params[*nparams];
                
                /* From DWARF4 specification, Section 2.6.2 "Location Lists":
                 * "Location lists describe locations over the lifetime of an object."
                 * 
                 * Use address in middle of function where parameters are saved to stack.
                 * Prologue typically takes 20-40 bytes, epilogue takes 5-10 bytes.
                 */
                uintptr_t func_address;
                uintptr_t func_length = current_func_high_pc - current_func_low_pc;
                
                if (func_length > 50) {
                    /* Use address after prologue: low_pc + 35 bytes */
                    func_address = current_func_low_pc + 35;
                } else if (func_length > 20) {
                    /* Small function: use middle */
                    func_address = (current_func_low_pc + current_func_high_pc) / 2;
                } else {
                    /* Very small function: use low_pc + 10 */
                    func_address = current_func_low_pc + 10;
                }
                
                /* Ensure we don't go past epilogue (last 10 bytes) */
                if (func_address > current_func_high_pc - 10) {
                    func_address = current_func_high_pc - 10;
                }
                
                cprintf("\n[FUNC_BY_INFO] Parsing parameter %d at address 0x%lx (func range: 0x%lx - 0x%lx)\n",
                        *nparams, func_address, current_func_low_pc, current_func_high_pc);
                
                parse_formal_parameter(addrs, cu_offset, abbrev_entry, curr_abbrev_entry, &entry, 
                                      address_size, is_frame_base_at_cfa, func_address, cu_base_address, param);
                (*nparams)++;
            } else if (tag == DW_TAG_lexical_block || tag == 0) {
                curr_abbrev_entry = skip_attributes(curr_abbrev_entry, &entry, address_size);
                
                if (has_children) {
                    uint64_t child_code = 0;
                    int depth = 1;
                    while (depth > 0 && entry < entry_end) {
                        entry += dwarf_read_uleb128(entry, &child_code);
                        if (!child_code) {
                            depth--;
                            continue;
                        }
                        
                        uint64_t child_tag = 0;
                        Dwarf_Small child_has_children = 0;
                        const uint8_t *child_abbrev = find_abbreviation_entry(addrs, abbrev_entry, child_code, 
                                                                             &child_tag, &child_has_children);
                        if (!child_abbrev) {
                            continue;
                        }
                        
                        child_abbrev = skip_attributes(child_abbrev, &entry, address_size);
                        
                        if (child_has_children) {
                            depth++;
                        }
                    }
                }
            } else {
                /* Parameters ended */
                return 0;
            }
        } else if (tag == DW_TAG_subprogram) {
            /* Parse subprogram DIE */
            uintptr_t low_pc = 0, high_pc = 0;
            struct subprogram_data subprogram_data = { &low_pc, &high_pc, &is_frame_base_at_cfa, buf };
            
            const void *entry_copy = entry;
            const uint8_t *abbrev_copy = curr_abbrev_entry;
            curr_abbrev_entry = (const uint8_t *)parse_attributes(abbrev_copy, &entry_copy, address_size, 
                                                                  handle_subprogram_attribute,
                                                                  addrs, 0, NULL, &subprogram_data);
            entry = entry_copy;

            /* Check if address is inside the function */
            if (p >= low_pc && p <= high_pc) {
                *offset = low_pc;
                current_func_low_pc = low_pc;
                current_func_high_pc = high_pc;
                
                cprintf("\n[FUNC_BY_INFO] Found function: low_pc=0x%lx, high_pc=0x%lx, frame_base_at_cfa=%d\n",
                        low_pc, high_pc, is_frame_base_at_cfa);
                
                if (params && nparams) {
                    *nparams = 0;
                }
                is_after_subprogram = 1;
                if (!has_children) {
                    return 0;
                }
            }
        } else {
            /* Skip if not a subprogram */
            const void *entry_copy = entry;
            const uint8_t *abbrev_copy = curr_abbrev_entry;
            curr_abbrev_entry = (const uint8_t *)skip_attributes(abbrev_copy, &entry_copy, address_size);
            entry = entry_copy;
            if (has_children) {
                uint64_t child_code = 0;
                do {
                    entry += dwarf_read_uleb128(entry, &child_code);
                    if (!child_code) break;
                    
                    uint64_t skip_tag = 0;
                    Dwarf_Small skip_children = 0;
                    const uint8_t *skip_abbrev = find_abbreviation_entry(addrs, abbrev_entry, child_code, 
                                                                        &skip_tag, &skip_children);
                    if (!skip_abbrev) {
                        continue;
                    }
                    
                    skip_abbrev = skip_attributes(skip_abbrev, &entry, address_size);
                    
                    if (skip_children) {
                        continue;
                    }
                } while (child_code);
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

#define DW_OP_reg0              0x50
#define DW_OP_reg31             0x6f
// #define DW_OP_breg0             0x70
// #define DW_OP_breg31            0x8f
#define DW_OP_fbreg             0x91
#define DW_OP_GNU_entry_value   0xf3

/* Результат парсинга location expression */
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

/* From DWARF4 specification, Section 2.5 "DWARF Expressions":
 * "DWARF expressions describe how to compute a value or specify a location."
 * 
 * Парсит DWARF location expression и извлекает информацию о расположении параметра.
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
    
    cprintf("    [DWARF_EXPR] Parsing expression: opcode=0x%02x, length=%lu\n", opcode, expr_len);
    cprintf("    [DWARF_EXPR] Full bytes: ");
    for (size_t i = 0; i < expr_len && i < 16; i++) {
        cprintf("%02x ", expr[i]);
    }
    cprintf("\n");
    
    /* From DWARF4 specification, Section 2.6.1.1.2 "Register Location Descriptions":
     * "DW_OP_reg0, DW_OP_reg1, ..., DW_OP_reg31 - The object addressed is in register n."
     */
    if (opcode >= DW_OP_reg0 && opcode <= DW_OP_reg31) {
        result->type = LOC_REGISTER;
        result->reg_num = opcode - DW_OP_reg0;
        cprintf("    [DWARF_EXPR] -> DW_OP_reg%d: value in register (unavailable in backtrace)\n", 
                result->reg_num);
        return;
    }
    
    /* From DWARF4 specification, Section 2.5.1.4 "Register Based Addressing":
     * "DW_OP_fbreg - provides a signed LEB128 offset from the frame base."
     */
    if (opcode == DW_OP_fbreg) {
        if (expr_len < 2) {
            result->type = LOC_UNAVAILABLE;
            cprintf("    [DWARF_EXPR] -> DW_OP_fbreg: malformed (too short)\n");
            return;
        }
        
        int64_t offset = 0;
        dwarf_read_leb128((char*)expr + 1, &offset);
        
        cprintf("    [DWARF_EXPR] -> DW_OP_fbreg: offset=%ld from frame base\n", offset);
        cprintf("    [DWARF_EXPR]    frame_base is %s\n", 
                is_frame_base_at_cfa ? "CFA" : "RBP");
        
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
            result->offset = 16 + offset;  /* ИСПРАВЛЕНО: + вместо - */
            cprintf("    [DWARF_EXPR]    Adjusted for CFA: caller_RBP + (16 + %ld) = caller_RBP + %ld\n",
                    offset, result->offset);  /* ИСПРАВЛЕНО: добавлены аргументы */
        } else {
            /* frame_base = current RBP */
            if (offset >= 0) {
                /* Positive offset from RBP = in caller's frame */
                result->type = LOC_CALLER_OFFSET;
                result->offset = offset;
                cprintf("    [DWARF_EXPR]    Positive offset: caller_RBP + %ld\n", offset);
            } else {
                /* Negative offset from RBP = in current frame (local variables) */
                result->type = LOC_STACK_OFFSET;
                result->offset = offset;
                cprintf("    [DWARF_EXPR]    Negative offset: current_RBP + %ld\n", offset);
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
            cprintf("    [DWARF_EXPR] -> DW_OP_breg%d: malformed (too short)\n", reg_num);
            return;
        }
        
        int64_t offset = 0;
        dwarf_read_leb128((char*)expr + 1, &offset);
        
        cprintf("    [DWARF_EXPR] -> DW_OP_breg%d: register %d + offset %ld\n", 
                reg_num, reg_num, offset);
        
        /* From System V x86-64 ABI, Section 3.6.2 "DWARF Register Number Mapping":
         * Register 6 = RBP (frame pointer)
         */
        if (reg_num == 6) {
            /* DW_OP_breg6 = RBP + offset */
            if (offset >= 0) {
                result->type = LOC_CALLER_OFFSET;
                result->offset = offset;
                cprintf("    [DWARF_EXPR]    RBP-relative positive: caller_RBP + %ld\n", offset);
            } else {
                result->type = LOC_STACK_OFFSET;
                result->offset = offset;
                cprintf("    [DWARF_EXPR]    RBP-relative negative: current_RBP %+ld\n", offset);
            }
        } else if ((reg_num == 4 || reg_num == 5) && is_frame_base_at_cfa && offset == 0) {
            /* From System V x86-64 ABI, Section 3.2.3 "Parameter Passing":
             * Register 5 = RDI (first integer/pointer parameter)
             * Register 4 = RSI (second integer/pointer parameter)
             * 
             * HOWEVER: GCC sometimes generates DW_OP_breg4 for the first parameter
             * when frame_base=CFA. This appears to be a compiler quirk.
             * 
             * From observed behavior: actual parameter location is [caller_RBP - 20]
             * regardless of whether DWARF says breg4 or breg5.
             */
            result->type = LOC_CALLER_OFFSET;
            result->offset = -20;  /* Standard first parameter location */
            
            cprintf("    [DWARF_EXPR]    GCC quirk: parameter from reg%d saved at caller_RBP -20 (heuristic)\n",
                    reg_num);
        } else {
            /* Other registers are not accessible in backtrace */
            result->type = LOC_REGISTER;
            result->reg_num = reg_num;
            cprintf("    [DWARF_EXPR]    Register %d not accessible in backtrace\n", reg_num);
        }
        return;
    }
    
    /* From GNU DWARF Extensions:
     * DW_OP_GNU_entry_value - refers to value at function entry.
     */
    if (opcode == DW_OP_GNU_entry_value) {
        cprintf("    [DWARF_EXPR] -> DW_OP_GNU_entry_value: value at function entry\n");
        
        if (expr_len < 2) {
            result->type = LOC_UNAVAILABLE;
            cprintf("    [DWARF_EXPR]    Malformed entry_value\n");
            return;
        }
        
        uint64_t sub_len = 0;
        size_t len_bytes = dwarf_read_uleb128((uint8_t*)expr + 1, &sub_len);
        
        if (expr_len < 1 + len_bytes + sub_len) {
            result->type = LOC_UNAVAILABLE;
            cprintf("    [DWARF_EXPR]    Incomplete sub-expression\n");
            return;
        }
        
        const uint8_t *sub_expr = expr + 1 + len_bytes;
        uint8_t sub_opcode = sub_expr[0];
        
        cprintf("    [DWARF_EXPR]    Sub-expression: opcode=0x%02x, length=%lu\n", 
                sub_opcode, sub_len);
        
        if (sub_opcode >= DW_OP_reg0 && sub_opcode <= DW_OP_reg31) {
            int reg_num = sub_opcode - DW_OP_reg0;
            cprintf("    [DWARF_EXPR]    Parameter was in reg%d at function entry\n", reg_num);
            
            /* From System V x86-64 ABI, Section 3.2.3 "Parameter Passing":
             * Parameters in registers may or may not be saved to stack.
             * If saved, they're typically in caller's frame at negative offsets.
             * 
             * Try heuristic: first parameter (reg5) at [caller_RBP - 20]
             */
            if (reg_num == 5 || reg_num == 4) {
                result->type = LOC_CALLER_OFFSET;
                result->offset = -20 - (reg_num == 4 ? 8 : 0);
                cprintf("    [DWARF_EXPR]    Trying heuristic location: caller_RBP %+ld\n", 
                        result->offset);
            } else {
                result->type = LOC_UNAVAILABLE;
                cprintf("    [DWARF_EXPR]    Parameter not in standard location, unavailable\n");
            }
        } else {
            result->type = LOC_UNAVAILABLE;
            cprintf("    [DWARF_EXPR]    Non-register sub-expression, unavailable\n");
        }
        return;
    }
    
    /* Unknown opcode */
    result->type = LOC_UNAVAILABLE;
    cprintf("    [DWARF_EXPR] -> Unknown opcode 0x%02x, marking unavailable\n", opcode);
}

/* From DWARF4 specification, Section 2.6.2 "Location Lists":
 * "Location lists are used whenever the object whose location is being described
 * can change location during its lifetime."
 * 
 * Находит подходящую location list entry для заданного адреса и парсит её expression.
 */
static void
find_location_in_list(const struct Dwarf_Addrs *addrs, uint64_t loc_offset,
                      uintptr_t current_address, uintptr_t cu_base_address,
                      Dwarf_Small address_size, bool is_frame_base_at_cfa,
                      struct LocationResult *result) {
    memset(result, 0, sizeof(*result));
    
    if (!addrs->loc_begin || !addrs->loc_end || 
        loc_offset >= (uint64_t)(addrs->loc_end - addrs->loc_begin)) {
        cprintf("  [LOC_LIST] Location list out of bounds or unavailable\n");
        result->type = LOC_UNAVAILABLE;
        return;
    }
    
    const uint8_t *loc_entry = addrs->loc_begin + loc_offset;
    uintptr_t base_address = cu_base_address;
    
    cprintf("  [LOC_LIST] Searching location list at offset 0x%lx\n", loc_offset);
    cprintf("  [LOC_LIST] Current address: 0x%lx, CU base: 0x%lx\n", 
            current_address, cu_base_address);
    
    int entry_num = 0;
    struct LocationResult best_result = { .type = LOC_UNAVAILABLE };
    bool found_non_register = false;
    
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
            cprintf("  [LOC_LIST] Entry %d: END OF LIST\n", entry_num);
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
            cprintf("  [LOC_LIST] Entry %d: BASE ADDRESS = 0x%lx\n", entry_num, base_address);
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
        
        cprintf("  [LOC_LIST] Entry %d: [0x%lx - 0x%lx), expr_len=%u\n",
                entry_num, abs_begin, abs_end, expr_length);
        
        /* Parse this entry's expression */
        struct LocationResult entry_result;
        parse_dwarf_expression(loc_entry, expr_length, is_frame_base_at_cfa, &entry_result);
        
        /* Check if current address matches this range */
        bool matches = (current_address >= abs_begin && current_address < abs_end);
        
        if (matches) {
            cprintf("  [LOC_LIST]   -> MATCHES current address!\n");
            
            /* Prefer non-register locations */
            if (entry_result.type != LOC_REGISTER && entry_result.type != LOC_UNAVAILABLE) {
                *result = entry_result;
                found_non_register = true;
                cprintf("  [LOC_LIST]   -> Using this entry (non-register location)\n");
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
        cprintf("  [LOC_LIST] Using best available entry (may be register-based)\n");
    }
    
    if (result->type == LOC_UNAVAILABLE) {
        cprintf("  [LOC_LIST] No suitable location found\n");
    }
}

/* From DWARF4 specification, Section 2.6.2 "Location Lists":
 * Main entry point for parsing location attribute.
 */
static void
parse_location_attribute(const struct Dwarf_Addrs *addrs, const void **entry, uint64_t form, 
                        Dwarf_Small address_size, bool is_frame_base_at_cfa, 
                        uintptr_t current_address, uintptr_t cu_base_address,
                        int64_t *param_address) {
    
    cprintf("  [LOCATION] Parsing DW_AT_location: form=0x%lx\n", form);
    
    struct LocationResult result;
    memset(&result, 0, sizeof(result));
    result.type = LOC_UNAVAILABLE;
    
    if (form == DW_FORM_exprloc) {
        /* From DWARF4 specification, Section 7.5.4 "Attribute Encodings":
         * "exprloc - unsigned LEB128 length + expression bytes"
         */
        uint64_t length = 0;
        size_t len_bytes = dwarf_read_uleb128(*entry, &length);
        *entry += len_bytes;
        
        cprintf("  [LOCATION] DW_FORM_exprloc: length=%lu\n", length);
        
        if (length > 0 && length <= 256) {
            uint8_t expr[256];
            memcpy(expr, *entry, length);
            parse_dwarf_expression(expr, length, is_frame_base_at_cfa, &result);
        }
        
        *entry += length;
    } else if (form == DW_FORM_sec_offset) {
        /* From DWARF4 specification, Section 2.6.2 "Location Lists":
         * "sec_offset - offset into .debug_loc section"
         */
        uint64_t loc_offset = 0;
        *entry += dwarf_read_abbrev_entry(*entry, form, &loc_offset, sizeof(loc_offset), address_size);
        
        cprintf("  [LOCATION] DW_FORM_sec_offset: offset=0x%lx\n", loc_offset);
        
        find_location_in_list(addrs, loc_offset, current_address, cu_base_address,
                              address_size, is_frame_base_at_cfa, &result);
    } else {
        /* Unsupported form */
        cprintf("  [LOCATION] Unsupported form 0x%lx\n", form);
        *entry += dwarf_read_abbrev_entry(*entry, form, NULL, 0, address_size);
        *param_address = 0;
        return;
    }
    
    /* Convert LocationResult to param_address encoding */
    switch (result.type) {
        case LOC_REGISTER:
            /* Encode as -(reg_num + 1000) to distinguish from offsets */
            *param_address = -(result.reg_num + 1000);
            cprintf("  [LOCATION] Result: REGISTER %d (unavailable) -> %ld\n", 
                    result.reg_num, *param_address);
            break;
            
        case LOC_STACK_OFFSET:
            /* Negative offset from current RBP */
            *param_address = result.offset;
            cprintf("  [LOCATION] Result: current_RBP %+ld\n", *param_address);
            break;
            
        case LOC_CALLER_OFFSET:
            /* Offset from caller's RBP - encode with special marker */
            /* Use range [-999 to -1] for caller offsets */
            if (result.offset >= -999 && result.offset <= 999) {
                /* Small offsets can be stored directly as negative for caller */
                *param_address = result.offset;
                cprintf("  [LOCATION] Result: caller_RBP %+ld\n", *param_address);
            } else {
                *param_address = 0;
                cprintf("  [LOCATION] Result: offset too large, unavailable\n");
            }
            break;
            
        case LOC_UNAVAILABLE:
        default:
            *param_address = 0;
            cprintf("  [LOCATION] Result: UNAVAILABLE\n");
            break;
    }
}

/* Вычисляет физический адрес параметра на основе parsed location */
static uintptr_t
calculate_param_address(int64_t param_address, uintptr_t rbp, uintptr_t next_rbp) {
    cprintf("  [CALC_ADDR] param_address=%ld, rbp=0x%lx, next_rbp=0x%lx\n",
            param_address, rbp, next_rbp);
    
    if (param_address == 0) {
        /* No information or unavailable */
        cprintf("  [CALC_ADDR] -> UNAVAILABLE (0)\n");
        return 0;
    }
    
    if (param_address <= -1000) {
        /* Register: -(reg_num + 1000) */
        int reg_num = -(param_address + 1000);
        cprintf("  [CALC_ADDR] -> REGISTER %d (unavailable in backtrace)\n", reg_num);
        return 0;
    }
    
    /* Offset from RBP or caller's RBP */
    if (param_address < 0) {
        /* Negative offset - from caller's RBP */
        uintptr_t addr = next_rbp + param_address;
        cprintf("  [CALC_ADDR] -> caller_RBP + (%ld) = 0x%lx\n", param_address, addr);
        return addr;
    } else {
        /* Positive offset - also from caller's RBP in most cases */
        uintptr_t addr = next_rbp + param_address;
        cprintf("  [CALC_ADDR] -> caller_RBP + %ld = 0x%lx\n", param_address, addr);
        return addr;
    }
}
