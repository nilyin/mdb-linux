%module mdv

%inline %{
#include <mdv_objid.h>
%}

%rename(ObjectId) mdv_objid;

typedef union
{
    uint8_t  u8[12];

    struct
    {
        uint32_t node;
        uint64_t id;
    };
} mdv_objid;

%extend mdv_objid
{
    mdv_objid()
    {
        mdv_objid *objid = malloc(sizeof(mdv_objid));
        if (objid)
            memset(objid, 0, sizeof(mdv_objid));
        return objid;
    }

    ~mdv_objid()
    {
        free($self);
    }

    char * toString()
    {
        char *buf = malloc(MDV_OBJID_STR_LEN);
        if (buf)
            mdv_objid_to_str($self, buf);
        return buf;
    }
}