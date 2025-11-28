#pragma once
#include <mdv_def.h>
#include "mdv_objid.h"


/// Data
typedef struct mdv_data
{
    size_t      size;       ///< Data size
    void       *ptr;        ///< Data pointer
} mdv_data;


/// Key and value pair
typedef struct mdv_kvdata
{
    mdv_data key;           ///< Key
    mdv_data value;         ///< Value
    mdv_objid row_id;       ///< row identifier
} mdv_kvdata;
