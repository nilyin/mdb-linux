#ifndef VALIDATE_ROW_INTEGRITY_H
#define VALIDATE_ROW_INTEGRITY_H

#include <mdv_def.h>
#include <mdv_log.h>

// Simple row integrity check - validate fields that actually exist in the row
static inline bool validate_row_integrity_with_count(mdv_rowlist_entry *entry, uint32_t field_count, const char *location) {
    if (!entry) {
        MDV_LOGE("Row integrity check at %s: NULL entry", location);
        return false;
    }
    
    // Check if field pointers are within reasonable bounds relative to the entry
    char *entry_start = (char*)entry;
    char *entry_end = entry_start + 0x10000; // Reasonable upper bound
    
    for (uint32_t i = 0; i < field_count; ++i) { // Only check fields that exist
        if (entry->data.fields[i].ptr) { // Only validate non-NULL pointers
            char *field_ptr = (char*)entry->data.fields[i].ptr;
            if (field_ptr < entry_start || field_ptr >= entry_end) {
                MDV_LOGE("Row integrity check at %s: Field %u pointer %p outside entry bounds [%p, %p)", 
                         location, i, field_ptr, entry_start, entry_end);
                return false;
            }
        }
    }
    
    return true;
}

// Backward compatibility wrapper
static inline bool validate_row_integrity(mdv_rowlist_entry *entry, const char *location) {
    return validate_row_integrity_with_count(entry, 3, location);
}

#endif // VALIDATE_ROW_INTEGRITY_H