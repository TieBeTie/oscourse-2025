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
            param->kind = KIND_UNSIGNED_INT;
            if (data.byte_size == 8) {
                strncpy(param->type_name, "uint64_t", sizeof(param->type_name));
            } else if (data.byte_size == 4) {
                strncpy(param->type_name, "unsigned int", sizeof(param->type_name));
            } else {
                strncpy(param->type_name, "unsigned int", sizeof(param->type_name));
            }
        } else {
            set_unknown_type(param);
            return;
        }
    } else {
        /* Имя типа было прочитано из DW_AT_name, определяем kind по encoding */
        if (data.encoding == DW_ATE_signed || data.encoding == DW_ATE_signed_char) {
            param->kind = KIND_SIGNED_INT;
        } else if (data.encoding == DW_ATE_unsigned || data.encoding == DW_ATE_unsigned_char) {
            param->kind = KIND_UNSIGNED_INT;
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
        
        if (pointed_type_abbrev && pointed_type_tag == DW_TAG_base_type) {
            /* Проверяем, что это char */
            char type_name_buf[256];
            if (get_base_type_name(data->addrs, pointed_type_abbrev,
                                  pointed_type_entry, address_size,
                                  type_name_buf, sizeof(type_name_buf))) {
                if (strcmp(type_name_buf, "char") == 0) {
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
    
    if (type_tag == DW_TAG_pointer_type) {
        handle_pointer_type(addrs, cu_offset, abbrev_table_begin,
                           type_abbrev_entry, type_entry, address_size, param);
    } else if (type_tag == DW_TAG_structure_type || type_tag == DW_TAG_class_type || type_tag == DW_TAG_union_type) {
        handle_structure_type(type_abbrev_entry, type_entry, address_size, param);
    } else if (type_tag == DW_TAG_base_type) {
        parse_base_type_info(addrs, cu_offset, type_abbrev_entry, type_entry, 
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
static void
parse_location_attribute(const struct Dwarf_Addrs *addrs, const void **entry, uint64_t form, 
                        Dwarf_Small address_size, bool is_frame_base_at_cfa, 
                        uintptr_t current_address, uintptr_t cu_base_address,
                        int64_t *param_address) {
    uint8_t buf[64] = { 0 };
    size_t buf_size = 0;
    
    if (form == DW_FORM_exprloc) {
        /* From DWARF4 specification, Section 7.5.4 "Attribute Encodings":
         * "exprloc - This is an unsigned LEB128 length followed by the number of information bytes"
         * 
         * DW_FORM_exprloc содержит unsigned LEB128 длину, за которой следуют байты выражения.
         */
        uint64_t length = 0;
        size_t len_bytes = dwarf_read_uleb128(*entry, &length);
        *entry += len_bytes;
        buf_size = MIN(length, sizeof(buf) - 1);
        if (buf_size > 0) {
            memcpy(buf, *entry, buf_size);
        }
        *entry += length;
    } else if (form == DW_FORM_block || form == DW_FORM_block1 || form == DW_FORM_block2 || form == DW_FORM_block4) {
        /* From DWARF4 specification, Section 7.5.4 "Attribute Encodings":
         * "block - Blocks come in four forms: DW_FORM_block1 (1-byte length), DW_FORM_block2 (2-byte length),
         * DW_FORM_block4 (4-byte length), DW_FORM_block (unsigned LEB128 length)"
         * 
         * Block формы содержат длину, за которой следуют байты выражения.
         * Это старый формат для location expressions, используемый в DWARF2/3.
         */
        uint32_t length = 0;
        if (form == DW_FORM_block1) {
            length = get_unaligned(*entry, uint8_t);
            *entry += 1;
        } else if (form == DW_FORM_block2) {
            length = get_unaligned(*entry, uint16_t);
            *entry += 2;
        } else if (form == DW_FORM_block4) {
            length = get_unaligned(*entry, uint32_t);
            *entry += 4;
        } else { /* DW_FORM_block */
            uint64_t len64 = 0;
            *entry += dwarf_read_uleb128(*entry, &len64);
            length = (uint32_t)len64;
        }
        buf_size = MIN(length, sizeof(buf) - 1);
        if (buf_size > 0) {
            memcpy(buf, *entry, buf_size);
        }
        *entry += length;
    } else if (form == DW_FORM_sec_offset) {
        /* From DWARF4 specification, Section 2.6.2 "Location Lists":
         * "Location lists are used in place of location expressions whenever the object whose location is
         * being described can change location during its lifetime. Location lists are contained in a separate
         * object file section called .debug_loc."
         * 
         * From DWARF4 specification, Section 7.5.4 "Attribute Encodings":
         * "loclistptr - This is an offset into the .debug_loc section (DW_FORM_sec_offset)."
         * 
         * From DWARF4 specification, Section 7.7.3 "Location Lists":
         * "A location list entry consists of two address offsets followed by a 2-byte length, followed by a
         * block of contiguous bytes that contains a DWARF location description."
         * 
         * Форма DW_FORM_sec_offset (0x17) для DW_AT_location означает loclistptr - смещение
         * в секцию .debug_loc. Читаем offset, находим location list, парсим записи и ищем ту, которая
         * соответствует текущему адресу (beginning <= current_address < ending). Извлекаем location expression
         * из найденной записи и обрабатываем её как обычный location expression.
         */
        uint64_t loc_offset = 0;
        *entry += dwarf_read_abbrev_entry(*entry, form, &loc_offset, sizeof(loc_offset), address_size);
        
        if (!addrs->loc_begin || !addrs->loc_end || loc_offset >= (uint64_t)(addrs->loc_end - addrs->loc_begin)) {
            return;
        }
        
        const uint8_t *loc_entry = addrs->loc_begin + loc_offset;
        uintptr_t base_address = cu_base_address;
        bool found_entry = false;
        
        while (loc_entry < addrs->loc_end) {
            uintptr_t beginning = 0, ending = 0;
            
            /* Read beginning and ending address offsets */
            if (address_size == 4) {
                beginning = get_unaligned(loc_entry, uint32_t);
                loc_entry += 4;
                ending = get_unaligned(loc_entry, uint32_t);
                loc_entry += 4;
            } else {
                beginning = get_unaligned(loc_entry, uint64_t);
                loc_entry += 8;
                ending = get_unaligned(loc_entry, uint64_t);
                loc_entry += 8;
            }
            
            /* Check for end of list entry */
            if (beginning == 0 && ending == 0) {
                break;
            }
            
            /* Check for base address selection entry */
            if (address_size == 4 && beginning == 0xFFFFFFFF) {
                base_address = ending;
                continue;
            } else if (address_size == 8 && beginning == 0xFFFFFFFFFFFFFFFFULL) {
                base_address = ending;
                continue;
            }
            
            /* Calculate absolute addresses */
            uintptr_t abs_beginning = base_address + beginning;
            uintptr_t abs_ending = base_address + ending;
            
            /* Check if current address is in this range */
            if (current_address >= abs_beginning && current_address < abs_ending) {
                /* Read length of location expression */
                uint16_t expr_length = get_unaligned(loc_entry, uint16_t);
                loc_entry += 2;
                
                /* Read location expression */
                buf_size = MIN(expr_length, sizeof(buf) - 1);
                if (buf_size > 0 && loc_entry + expr_length <= addrs->loc_end) {
                    memcpy(buf, loc_entry, buf_size);
                }
                found_entry = true;
                break;
            }
            
            /* Skip location expression for this entry */
            uint16_t expr_length = get_unaligned(loc_entry, uint16_t);
            loc_entry += 2;
            loc_entry += expr_length;
        }
        
        if (!found_entry) {
            return;
        }
    } else {
        /* From DWARF4 specification, Section 7.5.4 "Attribute Encodings":
         * "DW_AT_location may be encoded using class exprloc or loclistptr"
         * 
         * Если форма не exprloc, не block и не loclistptr, это неизвестная форма.
         * Для минимальной версии пропускаем такие случаи.
         */
        *entry += dwarf_read_abbrev_entry(*entry, form, NULL, 0, address_size);
        return;
    }
    
    if (buf_size > 0) {
        if (buf[0] == DW_OP_fbreg) {
            int64_t address = 0;
            size_t len = dwarf_read_leb128((char*)buf + 1, &address);
            if (len <= 4) {
                *param_address = (int32_t) address;
            } else {
                *param_address = address;
            }
            if (is_frame_base_at_cfa) {
                *param_address += 16;
            }
        } else if (buf[0] == DW_OP_breg6) {
            /* From DWARF4 specification, Section 2.5.1.2 "Register Based Addressing":
             * "DW_OP_breg0, DW_OP_breg1, ..., DW_OP_breg31 - The DW_OP_bregn operations add a
             * signed LEB128 offset to the contents of register n to compute an address."
             * 
             * DW_OP_breg6 добавляет signed LEB128 offset к значению регистра 6 (RBP) для вычисления адреса.
             * Это смещение сохраняется в param_address для последующего использования при вычислении
             * адреса параметра (rbp + offset).
             */
            int64_t address = 0;
            size_t len = dwarf_read_leb128((char*)buf + 1, &address);
            if (len <= 4) {
                *param_address = (int32_t) address;
            } else {
                *param_address = address;
            }
        }
    }
}

/* From DWARF4 specification, Section 3.3.4 "Declarations Owned by Subroutines and Entry Points":
 * "The declarations enclosed by a subroutine or entry point are represented by debugging
 * information entries that are owned by the subroutine or entry point entry. Entries representing the
 * formal parameters of the subroutine or entry point appear in the same order as the corresponding
 * declarations in the source program."
 * 
 * Парсит все атрибуты формального параметра (DW_TAG_formal_parameter), извлекая
 * имя (DW_AT_name), тип (DW_AT_type) и расположение (DW_AT_location). Заполняет структуру
 * Dwarf_VarInfo всей необходимой информацией о параметре.
 */
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

    while (entry < entry_end) {
        /* Read info abbreviation code */
        entry += dwarf_read_uleb128(entry, &abbrev_code);
        if (!abbrev_code) continue;

        uint64_t tag = 0;
        Dwarf_Small has_children = 0;
        const uint8_t *curr_abbrev_entry = find_abbreviation_entry(addrs, abbrev_entry, abbrev_code, &tag, &has_children);
        
        if (!curr_abbrev_entry) {
            /* Abbreviation not found, skip this entry */
            continue;
        }
        /* From DWARF4 specification, Section 3.3.4 "Declarations Owned by Subroutines and Entry Points":
         * "The declarations enclosed by a subroutine or entry point are represented by debugging
         * information entries that are owned by the subroutine or entry point entry. Entries representing the
         * formal parameters of the subroutine or entry point appear in the same order as the corresponding
         * declarations in the source program."
         * 
         * Параметры функций являются дочерними элементами DW_TAG_subprogram согласно DWARF4.
         * После нахождения нужной функции (is_after_subprogram = 1) продолжаем парсинг
         * дочерних элементов для извлечения параметров. Формальные параметры функции имеют тег
         * DW_TAG_formal_parameter и появляются в том же порядке, что и в исходном коде.
         */
        if (is_after_subprogram) {
            if (tag == DW_TAG_formal_parameter && params && nparams && *nparams < DWARF_MAXPARAMS) {
                /* From DWARF4 specification, Section 5.8 "Subroutine Type Entries":
                 * "The formal parameters of a parameter list (that have a specific type) are represented by a
                 * debugging information entry with the tag DW_TAG_formal_parameter."
                 * 
                 * Парсим формальный параметр, извлекая все его атрибуты (имя, тип, расположение)
                 * и заполняя структуру Dwarf_VarInfo. Вызываем parse_formal_parameter для обработки всех атрибутов.
                 * Передаём abbrev_entry (начало таблицы abbreviations) для правильного поиска типов.
                 */
                struct Dwarf_VarInfo *param = &params[*nparams];
                parse_formal_parameter(addrs, cu_offset, abbrev_entry, curr_abbrev_entry, &entry, 
                                      address_size, is_frame_base_at_cfa, p, cu_base_address, param);
                (*nparams)++;
            /* From DWARF4 specification, Section 3.3.4 "Declarations Owned by Subroutines and Entry Points":
             * "The unspecified parameters of a variable parameter list are represented by a debugging
             * information entry with the tag DW_TAG_unspecified_parameters."
             * 
             * Тег DW_TAG_unspecified_parameters означает наличие variadic-аргументов (...)
             * в сигнатуре функции согласно DWARF4. Для таких параметров создаём запись с is_variadic = 1
             * и именем "...". Значения variadic-аргументов не выводятся, так как их количество
             * и типы неизвестны на этапе компиляции, что соответствует требованию спецификации.
             */
            } else if (tag == DW_TAG_unspecified_parameters && params && nparams && *nparams < DWARF_MAXPARAMS) {
                /* From DWARF4 specification, Section 3.3.4 "Declarations Owned by Subroutines and Entry Points":
                 * "The unspecified parameters of a variable parameter list are represented by a debugging
                 * information entry with the tag DW_TAG_unspecified_parameters."
                 * 
                 * Пропускаем атрибуты variadic-параметра и создаём запись для него.
                 * Variadic-параметры не имеют конкретных типов и имён, поэтому создаём специальную запись.
                 */
                curr_abbrev_entry = skip_attributes(curr_abbrev_entry, &entry, address_size);

                struct Dwarf_VarInfo *param = &params[*nparams];
                memset(param, 0, sizeof(*param));
                /* Устанавливаем имя "..." для variadic-параметра согласно DWARF4.
                 * Поле is_variadic = 1 указывает, что это variadic-параметр функции.
                 * Значения не выводятся, поэтому размер не нужен (byte_size = 0).
                 */
                strncpy(param->name, "...", sizeof(param->name));
                strncpy(param->type_name, UNKNOWN_TYPE, sizeof(param->type_name));
                param->is_variadic = 1;
                param->kind = KIND_UNKNOWN;
                param->byte_size = 0;
                (*nparams)++;
            } else if (tag == DW_TAG_lexical_block || tag == 0) {
                /* From DWARF4 specification, Section 2.3 "Relationship of Debugging Information Entries":
                 * "A lexical block entry may have child entries representing nested blocks or declarations."
                 * 
                 * Пропускаем lexical blocks и null entries, продолжая парсинг для поиска параметров.
                 * Lexical blocks не являются параметрами, поэтому их нужно пропустить, но продолжить
                 * обработку дочерних элементов функции для поиска формальных параметров.
                 */
                curr_abbrev_entry = skip_attributes(curr_abbrev_entry, &entry, address_size);
                
                if (has_children) {
                    /* From DWARF4 specification, Section 2.3 "Relationship of Debugging Information Entries":
                     * "A debugging information entry may have child entries."
                     * 
                     * Пропускаем дочерние элементы рекурсивно, используя depth для отслеживания
                     * уровня вложенности. Для каждого дочернего элемента находим его abbreviation и пропускаем
                     * все его атрибуты и дочерние элементы.
                     */
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
                            /* Abbreviation not found, skip */
                            continue;
                        }
                        
                        child_abbrev = skip_attributes(child_abbrev, &entry, address_size);
                        
                        if (child_has_children) {
                            depth++;
                        }
                    }
                }
                /* Continue loop to check for more parameters */
            } else {
                /* Parameters ended - just exit */
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

            /* Load info and finish if address is inside of the function */
            if (p >= low_pc && p <= high_pc) {
                *offset = low_pc;
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
                is_after_subprogram = 1;
                if (!has_children) {
                return 0;
                }
                /* Continue loop to parse children (parameters) */
            }
        } else {
            /* Skip if not a subprogram */
            const void *entry_copy = entry;
            const uint8_t *abbrev_copy = curr_abbrev_entry;
            curr_abbrev_entry = (const uint8_t *)skip_attributes(abbrev_copy, &entry_copy, address_size);
            entry = entry_copy;
            if (has_children) {
                /* From DWARF4 specification, Section 2.3 "Relationship of Debugging Information Entries":
                 * "A debugging information entry may have child entries."
                 * 
                 * Пропускаем дочерние элементы упрощённым способом (без рекурсии).
                 * Для каждого дочернего элемента находим его abbreviation и пропускаем атрибуты.
                 */
                uint64_t child_code = 0;
                do {
                    entry += dwarf_read_uleb128(entry, &child_code);
                    if (!child_code) break;
                    
                    uint64_t skip_tag = 0;
                    Dwarf_Small skip_children = 0;
                    const uint8_t *skip_abbrev = find_abbreviation_entry(addrs, abbrev_entry, child_code, 
                                                                        &skip_tag, &skip_children);
                    if (!skip_abbrev) {
                        /* Abbreviation not found, skip */
                        continue;
                    }
                    
                    skip_abbrev = skip_attributes(skip_abbrev, &entry, address_size);
                    
                    if (skip_children) {
                        /* Recursively skip - simplified, just continue */
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
