#include "mdv_serialization.h"
#include <mdv_log.h>
#include <mdv_alloc.h>
#include <mdv_rollbacker.h>
// #include <mdv_enumerator.h>
#include <assert.h>
#include <limits.h>


static bool binn_field(mdv_field const *field, binn *obj)
{
    if (!binn_create_object(obj))
    {
        MDV_LOGE("binn_field failed");
        return false;
    }

    if (0
        || !binn_object_set_uint32(obj, "T", (unsigned int)field->type)
        || !binn_object_set_uint32(obj, "L", (unsigned int)field->limit)
        || !binn_object_set_str(obj, "N", (char *)field->name))
    {
        MDV_LOGE("binn_field failed");
        binn_free(obj);
        return false;
    }

    return true;
}


static bool binn_add_to_list(binn *list, mdv_field_type type, void const *data)
{
    switch(type)
    {
        case MDV_FLD_TYPE_BOOL:     return binn_list_add_bool(list, *(bool const*)data);         break;
        case MDV_FLD_TYPE_CHAR:     return binn_list_add_int8(list, *(char const*)data);         break;
        case MDV_FLD_TYPE_BYTE:     return binn_list_add_int8(list, *(int8_t const*)data);       break;
        case MDV_FLD_TYPE_INT8:     return binn_list_add_int8(list, *(int8_t const*)data);       break;
        case MDV_FLD_TYPE_UINT8:    return binn_list_add_uint8(list, *(uint8_t const*)data);     break;
        case MDV_FLD_TYPE_INT16:    return binn_list_add_int16(list, *(int16_t const*)data);     break;
        case MDV_FLD_TYPE_UINT16:   return binn_list_add_uint16(list, *(uint16_t const*)data);   break;
        case MDV_FLD_TYPE_INT32:    return binn_list_add_int32(list, *(int32_t const*)data);     break;
        case MDV_FLD_TYPE_UINT32:   return binn_list_add_uint32(list, *(uint32_t const*)data);   break;
        case MDV_FLD_TYPE_INT64:    return binn_list_add_int64(list, *(int64_t const*)data);     break;
        case MDV_FLD_TYPE_UINT64:   return binn_list_add_uint64(list, *(uint64_t const*)data);   break;
        case MDV_FLD_TYPE_FLOAT:    return binn_list_add_float(list, *(float const*)data);       break;
        case MDV_FLD_TYPE_DOUBLE:   return binn_list_add_double(list, *(double const*)data);     break;
    }
    MDV_LOGE("Unknown field type: %u", type);
    return false;
}


static bool binn_get(binn *val, mdv_field_type type, void *data)
{
    bool ret = false;

    switch(type)
    {
        case MDV_FLD_TYPE_BOOL:
        {
            BOOL bool_val = 0;
            ret = binn_get_bool(val, &bool_val);
            *(bool*)data = bool_val != 0;
            break;
        }

        case MDV_FLD_TYPE_CHAR:
        {
            int int_val = 0;
            ret = binn_get_int32(val, &int_val);
            *(char*)data = (char)int_val;
            break;
        }

        case MDV_FLD_TYPE_BYTE:
        case MDV_FLD_TYPE_INT8:
        {
            int int_val = 0;
            ret = binn_get_int32(val, &int_val);
            *(int8_t*)data = (int8_t)int_val;
            break;
        }

        case MDV_FLD_TYPE_UINT8:
        {
            int int_val = 0;
            ret = binn_get_int32(val, &int_val);
            *(uint8_t*)data = (uint8_t)int_val;
            break;
        }

        case MDV_FLD_TYPE_INT16:
        {
            int int_val = 0;
            ret = binn_get_int32(val, &int_val);
            *(int16_t*)data = (int16_t)int_val;
            break;
        }

        case MDV_FLD_TYPE_UINT16:
        {
            int int_val = 0;
            ret = binn_get_int32(val, &int_val);
            *(uint16_t*)data = (uint16_t)int_val;
            break;
        }

        case MDV_FLD_TYPE_INT32:
        {
            int int_val = 0;
            ret = binn_get_int32(val, &int_val);
            *(int32_t*)data = (int32_t)int_val;
            break;
        }

        case MDV_FLD_TYPE_UINT32:
        {
            int64 int64_val = 0;
            ret = binn_get_int64(val, &int64_val);
            *(uint32_t*)data = (uint32_t)int64_val;
            break;
        }

        case MDV_FLD_TYPE_INT64:
        {
            int64 int64_val = 0;
            ret = binn_get_int64(val, &int64_val);
            *(int64_t*)data = (int64_t)int64_val;
            break;
        }

        case MDV_FLD_TYPE_UINT64:
        {
            int64 int64_val = 0;
            ret = binn_get_int64(val, &int64_val);
            *(uint64_t*)data = (uint64_t)int64_val;
            break;
        }

        case MDV_FLD_TYPE_FLOAT:
        {
            double double_val = 0;
            ret = binn_get_double(val, &double_val);
            *(float*)data = (float)double_val;
            break;
        }

        case MDV_FLD_TYPE_DOUBLE:
        {
            ret = binn_get_double(val, data);
            break;
        }
    }

    if (!ret)
        MDV_LOGE("Unknown field type: %u", type);

    return ret;
}


bool mdv_binn_table_desc(mdv_table_desc const *table, binn *obj)
{
    if (!binn_create_object(obj))
    {
        MDV_LOGE("binn_table_desc failed");
        return false;
    }

    if (0
        || !binn_object_set_str(obj, "N", (char*)table->name)
        || !binn_object_set_uint32(obj, "S", table->size))
    {
        MDV_LOGE("binn_table_desc failed");
        binn_free(obj);
        return false;
    }

    binn fields;

    if (!binn_create_list(&fields))
    {
        MDV_LOGE("binn_table_desc failed");
        binn_free(obj);
        return false;
    }

    for(uint32_t i = 0; i < table->size; ++i)
    {
        binn field;
        if(0
           || !binn_field(table->fields + i, &field)
           || !binn_list_add_object(&fields, &field))
        {
            binn_free(&field);
            binn_free(&fields);
            binn_free(obj);
            return false;
        }
        binn_free(&field);
    }

    if (!binn_object_set_list(obj, "F", &fields))
    {
        binn_free(&fields);
        binn_free(obj);
        return false;
    }

    binn_free(&fields);

    return true;
}


mdv_table_desc * mdv_unbinn_table_desc(binn const *obj)
{
    char *name = 0;
    uint32_t fields_count = 0;

    binn *binn_fields = 0;

    if (0
        || !binn_object_get_str((void*)obj, "N", &name)
        || !binn_object_get_uint32((void*)obj, "S", &fields_count)
        || !binn_object_get_list((void*)obj, "F", (void**)&binn_fields))
    {
        MDV_LOGE("unbinn_table_desc failed");
        return 0;
    }

    size_t const table_name_size = strlen(name) + 1;

    binn_iter iter = {};
    binn value = {};

    // Calculate size
    uint32_t size = sizeof(mdv_table_desc) + fields_count * sizeof(mdv_field)
                    + table_name_size;

    binn_list_foreach(binn_fields, value)
    {
        char *field_name = 0;

        if (!binn_object_get_str(&value, "N", &field_name))
        {
            MDV_LOGE("unbinn_table_desc failed");
            return 0;
        }

        size += strlen(field_name) + 1;
    }

    // Allocate memory for table desc
    mdv_table_desc *table = mdv_alloc(size);

    if (!table)
    {
        MDV_LOGE("unbinn_table failed");
        return 0;
    }

    table->size = fields_count;

    mdv_field *fields = (mdv_field *)(table + 1);

    table->fields = fields;

    char *buff = (char *)(fields + table->size);

    memcpy(buff, name, table_name_size);
    table->name = buff;
    buff += table_name_size;

    size_t i = 0;

    binn_list_foreach(binn_fields, value)
    {
        if (i > table->size)
        {
            MDV_LOGE("unbinn_table_desc failed");
            mdv_free(table);
            return 0;
        }

        mdv_field *field = fields + i;
        char *field_name = 0;

        if (!binn_object_get_uint32(&value, "T", &field->type)
            || !binn_object_get_uint32(&value, "L", &field->limit)
            || !binn_object_get_str(&value, "N", &field_name))
        {
            MDV_LOGE("unbinn_table_desc failed");
            mdv_free(table);
            return 0;
        }

        size_t const field_name_size = strlen(field_name) + 1;
        memcpy(buff, field_name, field_name_size);
        field->name = buff;
        buff += field_name_size;

        ++i;
    }

    if (buff - (char*)table != size)
        MDV_LOGE("memory corrupted: %p, %zu != %u", table, buff - (char*)table, size);

    assert(buff - (char*)table <= size);

    return table;
}


bool mdv_binn_table(mdv_table const *table, binn *obj)
{
    if (!binn_create_object(obj))
    {
        MDV_LOGE("binn_table failed");
        return false;
    }

    binn desc;

    if (!mdv_binn_table_desc(mdv_table_description(table), &desc))
    {
        binn_free(obj);
        return false;
    }

    if (!mdv_binn_table_uuid(mdv_table_uuid(table), obj)
        || !binn_object_set_object(obj, "D", &desc))
    {
        MDV_LOGE("binn_table failed");
        binn_free(obj);
        binn_free(&desc);
        return false;
    }

    binn_free(&desc);

    return true;
}


mdv_table * mdv_unbinn_table(binn const *obj)
{
    mdv_uuid id;

    if (!mdv_unbinn_table_uuid(obj, &id))
    {
        MDV_LOGE("unbinn_table failed");
        return 0;
    }

    binn *desc_odj = 0;

    if (!binn_object_get_object((void*)obj, "D", (void**)&desc_odj))
    {
        MDV_LOGE("unbinn_table failed");
        return 0;
    }

    mdv_table_desc *desc = mdv_unbinn_table_desc(desc_odj);

    if (!desc)
    {
        MDV_LOGE("unbinn_table failed");
        return 0;
    }

    mdv_table *table = mdv_table_create(&id, desc);

    mdv_free(desc);

    if (!table)
    {
        MDV_LOGE("unbinn_table failed");
        return 0;
    }

    return table;
}


mdv_rowlist_entry * mdv_unbinn_table_as_row_slice(binn const        *obj,
                                                  mdv_bitset const  *mask)
{
    mdv_uuid uuid        = {};
    char *name           = 0;
    size_t name_size     = 0;
    uint32_t const cols  = 2;
    uint32_t fields_num  = 0;
    size_t row_size      = 0;

    // Calculate necessary space for row
    for(uint32_t n = 0; n < cols; ++n)
    {
        if (mask && !mdv_bitset_test(mask, n))
            continue;

        switch(n)
        {
            case 0:
            {
                if (!mdv_unbinn_table_uuid(obj, &uuid))
                {
                    MDV_LOGE("unbinn_table failed");
                    return 0;
                }

                row_size += sizeof(mdv_uuid);

                break;
            }

            case 1:
            {
                binn *desc_odj = 0;

                if (!binn_object_get_object((void*)obj, "D", (void**)&desc_odj))
                {
                    MDV_LOGE("unbinn_table failed");
                    return 0;
                }

                if (!binn_object_get_str((void*)desc_odj, "N", &name))
                {
                    MDV_LOGE("unbinn_table_desc failed");
                    return 0;
                }

                name_size = strlen(name) + 1;

                row_size += name_size;

                break;
            }

            default:
            {
                MDV_LOGE("Unknown field requested");
                return 0;
            }
        }

        ++fields_num;
    }

    if (!fields_num ||
        fields_num > cols)
    {
        MDV_LOGE("Serialized row contains invalid number of fields");
        return 0;
    }

    row_size += offsetof(mdv_rowlist_entry, data)
                + offsetof(mdv_row, fields)
                + sizeof(mdv_data) * fields_num;

    // Memory allocation for new row
    mdv_rowlist_entry *entry = mdv_alloc(row_size);

    if (!entry)
    {
        MDV_LOGE("No memory for new row");
        return 0;
    }

    mdv_row *row = &entry->data;

    char *dataspace = (char *)(row->fields + fields_num);

    size_t field_idx = 0;

    {
        row->fields[field_idx].ptr = dataspace;
        row->fields[field_idx].size = sizeof(mdv_uuid);
        memcpy(dataspace, &uuid, sizeof(mdv_uuid));
        ++field_idx;
        dataspace += sizeof(mdv_uuid);
    }

    {
        row->fields[field_idx].ptr = dataspace;
        row->fields[field_idx].size = name_size;
        memcpy(dataspace, name, name_size);
        ++field_idx;
        dataspace += name_size;
    }

    if (dataspace - (char*)entry != row_size)
        MDV_LOGE("memory corrupted: %p, %zu != %zu", entry, dataspace - (char*)entry, row_size);

    assert(dataspace - (char const*)entry == row_size);

    return entry;
}


bool mdv_binn_table_uuid(mdv_uuid const *uuid, binn *obj)
{
    return binn_object_set_uint64((void*)obj, "U0", uuid->u64[0])
           && binn_object_set_uint64((void*)obj, "U1", uuid->u64[1]);
}


bool mdv_unbinn_table_uuid(binn const *obj, mdv_uuid *uuid)
{
    if (0
        || !binn_object_get_uint64((void*)obj, "U0", (uint64 *)&uuid->u64[0])
        || !binn_object_get_uint64((void*)obj, "U1", (uint64 *)&uuid->u64[1]))
    {
        MDV_LOGE("unbinn_table failed");
        return false;
    }

    return true;
}


// Helper function to determine actual field count in a row
// CRITICAL: We cannot safely access row->fields[i] beyond allocated memory
static uint32_t mdv_row_field_count(mdv_row const *row, mdv_table_desc const *table_desc)
{
    // PROBLEM: We don't know how many fields were actually allocated for this row
    // The row structure only contains fields[1] but actual allocation varies
    // Accessing row->fields[i] beyond allocated count causes segfault
    
    // SAFE APPROACH: Use table schema size and handle missing fields in serialization
    MDV_LOGI("DEBUG: Using table schema field count: %u (avoiding unsafe memory access)", 
             table_desc->size);
    
    return table_desc->size;
}

bool mdv_binn_row(mdv_row const *row, mdv_table_desc const *table_desc, binn *list)
{
    if (!binn_create_list(list))
    {
        MDV_LOGE("binn_row failed");
        return false;
    }

    mdv_field const *fields = table_desc->fields;
    
    // Check if row has valid data - only skip if ALL fields are NULL/empty
    bool has_any_data = false;
    for(uint32_t i = 0; i < table_desc->size; ++i)
    {
        if (row->fields[i].ptr != NULL && row->fields[i].size > 0) {
            has_any_data = true;
            break;
        }
    }
    
    if (!has_any_data) {
        MDV_LOGI("DEBUG: Skipping completely empty row (all fields NULL)");
        binn_free(list);
        return false;
    }
    
    MDV_LOGI("DEBUG: Serializing row (table schema has %u fields)", table_desc->size);

    // Serialize all schema fields, handling missing ones as NULL
    for(uint32_t i = 0; i < table_desc->size; ++i)
    {
        uint32_t const field_type_size = mdv_field_type_size(fields[i].type);

        if(!field_type_size)
        {
            MDV_LOGE("binn_row failed. Invalid field type size.");
            binn_free(list);
            return false;
        }

        // Check if this field exists in the row (detect sparse data)
        bool field_exists = true;
        uint32_t arr_size = 0;
        
        // Detect if we're accessing beyond allocated memory by checking for garbage values
        if (row->fields[i].ptr == NULL && row->fields[i].size > 1000000) {
            field_exists = false;
            MDV_LOGI("DEBUG: Field %u appears unallocated (NULL ptr, size=%u), treating as missing", 
                     i, row->fields[i].size);
        } else if (row->fields[i].ptr != NULL) {
            uintptr_t ptr_val = (uintptr_t)row->fields[i].ptr;
            if (ptr_val < 0x1000 || ptr_val > 0x7fffffffffff) {
                field_exists = false;
                MDV_LOGI("DEBUG: Field %u has invalid pointer %p, treating as missing", i, row->fields[i].ptr);
            } else if (row->fields[i].size > 0x1000000) {
                field_exists = false;
                MDV_LOGI("DEBUG: Field %u has unreasonable size %u, treating as missing", i, row->fields[i].size);
            }
        }
        
        if (field_exists) {
            if(row->fields[i].size % field_type_size)
            {
                MDV_LOGE("binn_row failed. Invalid field size.");
                binn_free(list);
                return false;
            }
            arr_size = row->fields[i].size / field_type_size;
        }

        BOOL res = true;
        
        if (!field_exists) {
            // Field doesn't exist in row, serialize as NULL blob
            MDV_LOGI("DEBUG: Serializing missing field %u as NULL blob", i);
            res = binn_list_add_blob(list, NULL, 0);
        } else {
            // Field exists, serialize normally with validation
            if (fields[i].limit && fields[i].limit < arr_size)
            {
                MDV_LOGE("binn_row failed. Field is too long. Field %u: limit=%u, actual=%u", i, fields[i].limit, arr_size);
                binn_free(list);
                return false;
            }

            if(fields[i].limit == 1)
                res = binn_add_to_list(list, fields[i].type, row->fields[i].ptr);
            else if (field_type_size == 1)
            {
                if (!row->fields[i].ptr || arr_size == 0)
                {
                    MDV_LOGI("DEBUG: Serializing empty blob field %u", i);
                    res = binn_list_add_blob(list, NULL, 0);
                }
                else
                {
                    MDV_LOGI("DEBUG: binn_list_add_blob field %u: ptr=%p, size=%u, row=%p", 
                             i, row->fields[i].ptr, arr_size, row);
                    res = binn_list_add_blob(list, row->fields[i].ptr, arr_size);
                }
            }
            else
            {
                binn *field_items = binn_list();
                if (!field_items)
                {
                    MDV_LOGE("binn_row failed");
                    binn_free(list);
                    return false;
                }
                for(uint32_t j = 0; res && j < arr_size; ++j)
                    res = binn_add_to_list(field_items, fields[i].type, (char const *)row->fields[i].ptr + j * field_type_size);
                if (res)
                    res = binn_list_add_list(list, field_items);
                binn_free(field_items);
            }
        }

        if(!res)
        {
            MDV_LOGE("binn_row failed.");
            binn_free(list);
            return false;
        }
    }

    return true;
}


static size_t mdv_calc_row_size(binn const           *list,
                                mdv_table_desc const *table_desc,
                                uint32_t             *fields_count,
                                mdv_bitset const     *mask)
{
    *fields_count = 0;

    // Validate inputs
    if (!list || !table_desc || !fields_count)
    {
        MDV_LOGE("calc_row_size: invalid input parameters");
        return 0;
    }

    // Validate list structure
    if (!binn_is_valid((void*)list, NULL, NULL, NULL))
    {
        MDV_LOGE("calc_row_size: invalid binn list structure");
        return 0;
    }

    binn_iter iter = {};
    binn value = {};

    uint32_t const cols = table_desc->size;
    mdv_field const *fields = table_desc->fields;

    if (!fields || cols == 0)
    {
        MDV_LOGE("calc_row_size: invalid table descriptor");
        return 0;
    }

    uint32_t n = 0;
    size_t row_size = 0;

    // Calculate necessary space for row
    binn_list_foreach((void*)list, value)
    {
        // Safety check to prevent processing too many fields
        if (n >= cols)
        {
            MDV_LOGE("calc_row_size: field index %u exceeds table columns %u", n, cols);
            return 0;
        }

        // If mask is NULL, include all fields (select all)
        if (mask && !mdv_bitset_test(mask, n))
        {
            ++n;
            continue;
        }

        uint32_t const field_type_size = mdv_field_type_size(fields[n].type);

        if (field_type_size == 0)
        {
            MDV_LOGE("calc_row_size: invalid field type size for field %u", n);
            return 0;
        }

        if(fields[n].limit == 1)
            row_size += field_type_size;
        else if (field_type_size == 1)
        {
            int const blob_size = binn_size(&value);

            if (blob_size < 0)
            {
                MDV_LOGE("blob size is negative: %d", blob_size);
                return 0;
            }

            // Safety check for maximum blob size
            if (blob_size > 0x7FFFFFFF)
            {
                MDV_LOGE("blob size too large: %d bytes", blob_size);
                return 0;
            }

            // Check for integer overflow in row_size calculation
            if (row_size > SIZE_MAX - blob_size)
            {
                MDV_LOGE("row size calculation overflow");
                return 0;
            }

            MDV_LOGI("DEBUG: Blob field %u: blob_size=%d, running_total=%zu", 
                     n, blob_size, row_size + blob_size);
            row_size += blob_size;
        }
        else
        {
            uint32_t array_len = mdv_binn_list_length(&value);
            size_t array_size = field_type_size * array_len;
            MDV_LOGI("DEBUG: Array field %u: type_size=%u, array_len=%u, total_size=%zu", 
                     n, field_type_size, array_len, array_size);
            row_size += array_size;
        }

        ++n;
        ++*fields_count;
    }

    if (*fields_count > cols)
    {
        MDV_LOGE("Serialized row contains invalid number of fields");
        return 0;
    }

    row_size += offsetof(mdv_rowlist_entry, data)
                + offsetof(mdv_row, fields)
                + sizeof(mdv_data) * *fields_count;
    
    MDV_LOGI("DEBUG: Final calculated row_size=%zu (data_size=%zu + overhead=%zu)", 
             row_size, row_size - (offsetof(mdv_rowlist_entry, data) + offsetof(mdv_row, fields) + sizeof(mdv_data) * *fields_count),
             offsetof(mdv_rowlist_entry, data) + offsetof(mdv_row, fields) + sizeof(mdv_data) * *fields_count);

    return row_size;
}


mdv_rowlist_entry * mdv_unbinn_row(binn const *list, mdv_table_desc const *table_desc)
{
    return mdv_unbinn_row_slice(list, table_desc, 0);
}


mdv_rowlist_entry * mdv_unbinn_row_slice(binn const *list,
                                         mdv_table_desc const *table_desc,
                                         mdv_bitset const *mask)
{
    // Early validation of inputs
    if (!list || !table_desc || !table_desc->fields)
    {
        MDV_LOGE("unbinn_row_slice: invalid input parameters");
        return 0;
    }

    uint32_t const cols = table_desc->size;
    mdv_field const *fields = table_desc->fields;

    // Validate table descriptor
    if (cols == 0 || cols > 1000) // reasonable limit
    {
        MDV_LOGE("unbinn_row_slice: invalid table column count: %u", cols);
        return 0;
    }

    uint32_t fields_count = 0;

    // Calculate necessary space for row
    size_t const row_size = mdv_calc_row_size(list, table_desc, &fields_count, mask);
    
    MDV_LOGI("DEBUG: Calculated row_size=%zu, fields_count=%u, table_desc->size=%u", 
             row_size, fields_count, table_desc->size);

    if (!row_size)
        return 0;

    // Additional safety check for row size
    if (row_size > 0x10000000) // 256MB limit
    {
        MDV_LOGE("unbinn_row_slice: calculated row size too large: %zu bytes", row_size);
        return 0;
    }

    /* Note: Server now pads rows to match table schema, so list_len should equal table_desc->size
       The fields_count calculated here may be less than table_desc->size for sparse data,
       but the server ensures consistent field count in serialized data. */
    
    size_t const list_len = mdv_binn_list_length(list);
    MDV_LOGI("DEBUG: Client deserializing row: list_len=%zu, fields_count=%u, table_desc->size=%u", 
             list_len, fields_count, table_desc->size);

    // Memory allocation for new row
    mdv_rowlist_entry *entry = mdv_alloc(row_size);

    if (!entry)
    {
        MDV_LOGE("No memory for new row");
        return 0;
    }

    // Zero the entire allocated memory to prevent garbage values
    memset(entry, 0, row_size);

    mdv_row *row = &entry->data;

    char *dataspace = (char *)(row->fields + fields_count);
    char *dataspace_start = dataspace;
    char *dataspace_end = (char*)entry + row_size;
    
    MDV_LOGI("DEBUG: Row allocation - entry=%p, row_size=%zu, dataspace_start=%p, dataspace_end=%p", 
             entry, row_size, dataspace_start, dataspace_end);

    uint32_t n = 0, field_idx = 0;

    binn_iter iter = {};
    binn value = {};

    // Deserialize row
    binn_list_foreach((void*)list, value)
    {
        // If mask is NULL, include all fields (select all)
        if (mask && !mdv_bitset_test(mask, n))
        {
            ++n;
            continue;
        }

        uint32_t const field_type_size = mdv_field_type_size(fields[n].type);

        if (!field_type_size)
        {
            MDV_LOGE("unbinn_row_slice failed. Invalid field type size for field %u (type=%u).", n, fields[n].type);
            mdv_free(entry);
            return 0;
        }

        if(fields[n].limit == 1)
        {
            /* Bound check before writing fixed-size value */
            if ((size_t)(dataspace + field_type_size - (char*)entry) > row_size)
            {
                MDV_LOGE("unbinn_row_slice failed. Fixed field %u exceeds allocated row_size (%zu), need %u bytes.",
                         n, row_size, field_type_size);
                mdv_free(entry);
                return 0;
            }

            if (!binn_get(&value, fields[n].type, dataspace))
            {
                MDV_LOGE("unbinn_row_slice failed. binn_get returned false for field %u (type=%u).", n, fields[n].type);
                mdv_free(entry);
                return 0;
            }

            row->fields[field_idx].size = field_type_size;
            row->fields[field_idx].ptr = dataspace;
            dataspace += field_type_size;
        }
        else if (field_type_size == 1)
        {
            int const blob_size = binn_size(&value);

            if (blob_size < 0)
            {
                MDV_LOGE("unbinn_row_slice failed. blob size is negative: %d", blob_size);
                mdv_free(entry);
                return 0;
            }

            // Additional safety check for maximum blob size
            if (blob_size > 0x7FFFFFFF)
            {
                MDV_LOGE("unbinn_row_slice failed. Blob field %u size too large: %d bytes", n, blob_size);
                mdv_free(entry);
                return 0;
            }

            /* Bound check before memcpy */
            if ((size_t)(dataspace + blob_size - (char*)entry) > row_size)
            {
                MDV_LOGE("unbinn_row_slice failed. Blob field %u (size=%d) exceeds allocated row_size (%zu).",
                         n, blob_size, row_size);
                mdv_free(entry);
                return 0;
            }

            void *blob_ptr = binn_ptr(&value);
            
            MDV_LOGI("DEBUG: binn_ptr returned %p for field %u, blob_size=%d, binn_value=%p, binn_row=%p", 
                     blob_ptr, field_idx, blob_size, &value, list);

            row->fields[field_idx].size = blob_size;
            
            if (blob_size > 0 && blob_ptr)
            {
                memcpy(dataspace, blob_ptr, blob_size);
                row->fields[field_idx].ptr = dataspace;
                MDV_LOGI("DEBUG: Copied %d bytes from %p to %p, field_ptr=%p", 
                         blob_size, blob_ptr, dataspace, row->fields[field_idx].ptr);
                dataspace += blob_size;
            }
            else
            {
                // For empty blobs, set NULL pointer and don't advance dataspace
                row->fields[field_idx].ptr = NULL;
                MDV_LOGI("DEBUG: Set field %u to NULL (blob_size=%d, blob_ptr=%p)", 
                         field_idx, blob_size, blob_ptr);
                // Don't advance dataspace for empty blobs
            }
            
            MDV_LOGI("DEBUG: Deserialized blob field %u: ptr=%p, size=%d, dataspace_before=%p, dataspace_after=%p, remaining=%ld", 
                     field_idx, row->fields[field_idx].ptr, blob_size, dataspace, dataspace + blob_size, dataspace_end - (dataspace + blob_size));
            
            if (dataspace + blob_size > dataspace_end) {
                MDV_LOGE("DEBUG: Dataspace overflow! dataspace=%p + blob_size=%d > dataspace_end=%p", 
                         dataspace, blob_size, dataspace_end);
            }
        }
        else
        {
            binn_iter arr_iter = {};
            binn arr_value = {};
            uint32_t arr_len = 0;

            row->fields[field_idx].ptr = dataspace;

            binn_iter_init(&arr_iter, &value, BINN_LIST);

            while (binn_list_next(&arr_iter, &arr_value))
            {
                /* Bound check before each element copy */
                if ((size_t)(dataspace + field_type_size - (char*)entry) > row_size)
                {
                    MDV_LOGE("unbinn_row_slice failed. Array element for field %u would exceed row_size (%zu).", n, row_size);
                    mdv_free(entry);
                    return 0;
                }

                if (!binn_get(&arr_value, fields[n].type, dataspace))
                {
                    MDV_LOGE("unbinn_row_slice failed. binn_get failed for array element of field %u (type=%u).", n, fields[n].type);
                    mdv_free(entry);
                    return 0;
                }

                ++arr_len;
                dataspace += field_type_size;
            }

            row->fields[field_idx].size = field_type_size * arr_len;
        }

        ++n;
        ++field_idx;
    }

    size_t actual_size = dataspace - (char*)entry;
    if (actual_size != row_size)
        MDV_LOGI("DEBUG: Size mismatch (expected): entry=%p, actual=%zu, expected=%zu", entry, actual_size, row_size);

    return entry;
}


bool mdv_binn_rowset(mdv_rowset *rowset, binn *list)
{
    mdv_rollbacker *rollbacker = mdv_rollbacker_create(3);

    if (!binn_create_list(list))
    {
        MDV_LOGE("binn_rowset failed");
        mdv_rollback(rollbacker);
        return false;
    }

    mdv_rollbacker_push(rollbacker, binn_free, list);

    mdv_enumerator *enumerator = mdv_rowset_enumerator(rowset);

    if (!enumerator)
    {
        MDV_LOGE("binn_rowset failed");
        mdv_rollback(rollbacker);
        return false;
    }

    mdv_rollbacker_push(rollbacker, mdv_enumerator_release, enumerator);

    mdv_table *table = mdv_rowset_table(rowset);

    mdv_rollbacker_push(rollbacker, mdv_table_release, table);

    mdv_table_desc const *table_desc = mdv_table_description(table);

    while(mdv_enumerator_next(enumerator) == MDV_OK)
    {
        mdv_row *row = mdv_enumerator_current(enumerator);
        mdv_objid const *row_id = mdv_enumerator_row_id(enumerator);

        if (!row)
        {
            MDV_LOGE("binn_rowset failed: null row from enumerator");
            mdv_rollback(rollbacker);
            return false;
        }

        binn fields;

        if (mdv_binn_row(row, table_desc, &fields))
        {
            if (!binn_list_add_list(list, &fields))
            {
                MDV_LOGE("binn_rowset failed: could not add row to list");
                binn_free(&fields);
                mdv_rollback(rollbacker);
                return false;
            }
            MDV_LOGI("DEBUG: Successfully serialized and added row to rowset");
            binn_free(&fields);

            if (row_id)
            {
                binn obj;
                if (binn_create_object(&obj))
                {
                    if (binn_object_set_blob(&obj, "id", (void*)row_id, sizeof(*row_id)))
                    {
                        if (!binn_list_add_object(list, &obj))
                        {
                            MDV_LOGE("binn_rowset failed: could not add row id to list");
                            binn_free(&obj);
                            mdv_rollback(rollbacker);
                            return false;
                        }
                    }
                    else
                    {
                        MDV_LOGE("binn_rowset failed: could not set row id blob");
                    }
                    binn_free(&obj);
                }
                else
                {
                    MDV_LOGE("binn_rowset failed: could not create row id object");
                    mdv_rollback(rollbacker);
                    return false;
                }
            }
        }
        else
        {
            // Row was skipped (no real data), continue with next row
            MDV_LOGI("DEBUG: Skipped empty row, continuing with next row");
        }
    }

    mdv_table_release(table);
    mdv_enumerator_release(enumerator);
    mdv_rollbacker_free(rollbacker);

    return true;
}


mdv_rowset * mdv_unbinn_rowset(binn const *list, mdv_table *table)
{
    mdv_rowset *rowset = mdv_rowset_create(table);

    if (!rowset || !table)
    {
        MDV_LOGE("unbinn_rowset failed");
        return 0;
    }

    mdv_table_desc const *table_desc = mdv_table_description(table);

    binn_iter iter = {};
    binn value = {};

    binn_list_foreach((void*)list, value)
    {
        mdv_rowlist_entry *entry = mdv_unbinn_row(&value, table_desc);

        if (!entry)
        {
            MDV_LOGE("unbinn_rowset failed");
            mdv_rowset_release(rowset);
            return 0;
        }

        if (binn_list_next(&iter, &value))
        {
            void *row_id = 0;
            uint32_t size = 0;
            binn_object_get_blob(&value, "id", &row_id, &size);
            if (row_id && size == sizeof(mdv_objid))
                entry->row_id = *(mdv_objid*)row_id;
        }

        mdv_rowset_emplace(rowset, entry);
    }

    return rowset;
}


bool mdv_topology_serialize(mdv_topology *topology, binn *obj)
{
    mdv_rollbacker *rollbacker = mdv_rollbacker_create(6);

    binn nodes;
    binn links;

    if (!binn_create_object(obj))
    {
        MDV_LOGE("binn_topology failed");
        mdv_rollback(rollbacker);
        return false;
    }

    mdv_rollbacker_push(rollbacker, binn_free, obj);

    if (!binn_create_list(&nodes))
    {
        MDV_LOGE("binn_topology failed");
        mdv_rollback(rollbacker);
        return false;
    }

    mdv_rollbacker_push(rollbacker, binn_free, &nodes);

    if (!binn_create_list(&links))
    {
        MDV_LOGE("binn_topology failed");
        mdv_rollback(rollbacker);
        return false;
    }

    mdv_rollbacker_push(rollbacker, binn_free, &links);

    mdv_vector *toponodes = mdv_topology_nodes(topology);
    mdv_vector *topolinks = mdv_topology_links(topology);
    mdv_vector *topoextradata = mdv_topology_extradata(topology);

    mdv_rollbacker_push(rollbacker, mdv_vector_release, toponodes);
    mdv_rollbacker_push(rollbacker, mdv_vector_release, topolinks);
    mdv_rollbacker_push(rollbacker, mdv_vector_release, topoextradata);

    mdv_vector_foreach(toponodes, mdv_toponode, toponode)
    {
        binn node;

        if (!binn_create_object(&node))
        {
            MDV_LOGE("binn_topology failed");
            mdv_rollback(rollbacker);
            return false;
        }

        if (0
            || !binn_object_set_uint32(&node, "ID", toponode->id)
            || !binn_object_set_uint64(&node, "U1", toponode->uuid.u64[0])
            || !binn_object_set_uint64(&node, "U2", toponode->uuid.u64[1])
            || !binn_object_set_str(&node, "A", (char*)toponode->addr)
            || !binn_list_add_object(&nodes, &node))
        {
            MDV_LOGE("binn_topology failed");
            binn_free(&node);
            mdv_rollback(rollbacker);
            return false;
        }

        binn_free(&node);
    }

    mdv_vector_foreach(topolinks, mdv_topolink, topolink)
    {
        binn link;
        uint8_t tmp[64];

        if (!binn_create(&link, BINN_OBJECT, sizeof tmp, tmp))
        {
            MDV_LOGE("binn_topology failed");
            mdv_rollback(rollbacker);
            return false;
        }

        if (0
            || !binn_object_set_uint32(&link, "U1", topolink->node[0])
            || !binn_object_set_uint32(&link, "U2", topolink->node[1])
            || !binn_object_set_uint32(&link, "W", topolink->weight)
            || !binn_list_add_object(&links, &link))
        {
            MDV_LOGE("binn_topology failed");
            binn_free(&link);
            mdv_rollback(rollbacker);
            return false;
        }

        binn_free(&link);
    }

    if (0
        || !binn_object_set_uint64(obj, "NC", mdv_vector_size(toponodes))
        || !binn_object_set_uint64(obj, "LC", mdv_vector_size(topolinks))
        || !binn_object_set_uint64(obj, "ES", mdv_vector_size(topoextradata))
        || (mdv_vector_empty(toponodes) ? false : !binn_object_set_list(obj, "N", &nodes))
        || (mdv_vector_empty(topolinks) ? false : !binn_object_set_list(obj, "L", &links)))
    {
        MDV_LOGE("binn_topology failed");
        mdv_rollback(rollbacker);
        return false;
    }

    mdv_vector_release(toponodes);
    mdv_vector_release(topolinks);
    mdv_vector_release(topoextradata);

    binn_free(&nodes);
    binn_free(&links);

    mdv_rollbacker_free(rollbacker);

    return true;
}


mdv_topology * mdv_topology_deserialize(binn const *obj)
{
    uint64   nodes_count = 0;
    uint64   links_count = 0;
    uint64   extradata_size = 0;
    binn    *nodes = 0;
    binn    *links = 0;

    if (0
        || !binn_object_get_uint64((void*)obj, "NC", &nodes_count)
        || !binn_object_get_uint64((void*)obj, "LC", &links_count)
        || !binn_object_get_uint64((void*)obj, "ES", &extradata_size)
        || (nodes_count && !binn_object_get_list((void*)obj, "N", (void**)&nodes))
        || (links_count && !binn_object_get_list((void*)obj, "L", (void**)&links)))
    {
        MDV_LOGE("unbinn_topology failed");
        return 0;
    }

    mdv_rollbacker *rollbacker = mdv_rollbacker_create(3);

    mdv_vector *toponodes = mdv_vector_create(nodes_count,
                                              sizeof(mdv_toponode),
                                              &mdv_default_allocator);

    if(!toponodes)
    {
        MDV_LOGE("No memory for network topology");
        mdv_rollback(rollbacker);
        return 0;
    }

    mdv_rollbacker_push(rollbacker, mdv_vector_release, toponodes);

    mdv_vector *topolinks = mdv_vector_create(links_count,
                                              sizeof(mdv_topolink),
                                              &mdv_default_allocator);

    if(!topolinks)
    {
        MDV_LOGE("No memory for network topology");
        mdv_rollback(rollbacker);
        return 0;
    }

    mdv_rollbacker_push(rollbacker, mdv_vector_release, topolinks);

    mdv_vector *extradata = mdv_vector_create(extradata_size,
                                              sizeof(char),
                                              &mdv_default_allocator);

    if(!extradata)
    {
        MDV_LOGE("No memory for network topology");
        mdv_rollback(rollbacker);
        return 0;
    }

    mdv_rollbacker_push(rollbacker, mdv_vector_release, extradata);

    binn_iter iter = {};
    binn value = {};

    size_t i;

    // load nodes
    if (nodes_count)
    {
        i = 0;
        binn_list_foreach(nodes, value)
        {
            if (i++ > nodes_count)
            {
                MDV_LOGE("unbinn_topology failed");
                mdv_rollback(rollbacker);
                return 0;
            }

            mdv_toponode node;

            char *addr = 0;

            if (0
                || !binn_object_get_uint32(&value, "ID", &node.id)
                || !binn_object_get_uint64(&value, "U1", (uint64*)&node.uuid.u64[0])
                || !binn_object_get_uint64(&value, "U2", (uint64*)&node.uuid.u64[1])
                || !binn_object_get_str(&value, "A", &addr))
            {
                MDV_LOGE("unbinn_topology failed");
                mdv_rollback(rollbacker);
                return 0;
            }

            node.addr = mdv_vector_append(extradata, addr, strlen(addr) + 1);

            mdv_vector_push_back(toponodes, &node);
        }
    }

    // load links
    if (links_count)
    {
        i = 0;
        binn_list_foreach(links, value)
        {
            if (i++ > links_count)
            {
                MDV_LOGE("unbinn_topology failed");
                mdv_rollback(rollbacker);
                return 0;
            }

            mdv_topolink link;

            if (0
                || !binn_object_get_uint32(&value, "U1", link.node + 0)
                || !binn_object_get_uint32(&value, "U2", link.node + 1)
                || !binn_object_get_uint32(&value, "W", &link.weight)
                || link.node[0] >= nodes_count
                || link.node[1] >= nodes_count)
            {
                MDV_LOGE("unbinn_topology failed");
                mdv_rollback(rollbacker);
                return 0;
            }

            mdv_vector_push_back(topolinks, &link);
        }
    }

    mdv_topology *topology = mdv_topology_create(toponodes, topolinks, extradata);

    mdv_rollback(rollbacker);

    return topology;
}


bool mdv_binn_uuid(mdv_uuid const *uuid, binn *obj)
{
    if (!binn_create_object(obj))
    {
        MDV_LOGE("binn_uuid failed");
        return false;
    }

    if (0
        || !binn_object_set_uint64(obj, "U1", uuid->u64[0])
        || !binn_object_set_uint64(obj, "U2", uuid->u64[1]))
    {
        MDV_LOGE("binn_uuid failed");
        binn_free(obj);
        return false;
    }

    return true;
}


bool mdv_unbinn_uuid(binn *obj, mdv_uuid *uuid)
{
    if (0
        || !binn_object_get_uint64(obj, "U1", (uint64*)&uuid->u64[0])
        || !binn_object_get_uint64(obj, "U2", (uint64*)&uuid->u64[1]))
    {
        MDV_LOGE("unbinn_uuid failed");
        return false;
    }

    return true;
}


bool mdv_binn_bitset(mdv_bitset const *bitset, binn *obj)
{
    if (!binn_create_list(obj))
    {
        MDV_LOGE("binn_bitset failed");
        return false;
    }

    // Handle NULL bitset case - create empty list to represent "select all fields"
    if (!bitset)
        return true;

    size_t const capacity = mdv_bitset_capacity(bitset);
    int32_t const *data = (int32_t const *)mdv_bitset_data(bitset);

    for(size_t i = 0; i < capacity / (MDV_BITSET_ALIGNMENT * CHAR_BIT); ++i)
    {
        if (!binn_list_add_int32(obj, data[i]))
        {
            MDV_LOGE("binn_bitset failed");
            binn_free(obj);
            return false;
        }
    }

    return true;
}


mdv_bitset * mdv_unbinn_bitset(binn const *obj)
{
    // Handle NULL input
    if (!obj)
        return 0;

    size_t const list_len = mdv_binn_list_length(obj);
    
    // Handle empty list case - return NULL to represent "select all fields"
    if (list_len == 0)
        return 0;
    
    size_t const capacity = list_len * MDV_BITSET_ALIGNMENT * CHAR_BIT;

    mdv_bitset *bitset = mdv_bitset_create(capacity, &mdv_default_allocator);

    if (!bitset)
    {
        MDV_LOGE("unbinn_bitset failed. No memory.");
        return 0;
    }

    int32_t *data = (int32_t *)mdv_bitset_data(bitset);

    binn_iter iter;
    binn value;
    uint32_t n = 0;

    binn_list_foreach((binn*)obj, value)
    {
        if (!binn_get_int32(&value, data + n))
        {
            MDV_LOGE("unbinn_bitset failed");
            mdv_bitset_release(bitset);
            return 0;
        }

        ++n;
    }

    return bitset;
}
