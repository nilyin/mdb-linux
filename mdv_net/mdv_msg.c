#include "mdv_msg.h"
#include "mdv_socket.h"
#include <mdv_alloc.h>
#include <mdv_log.h>
#include <mdv_limits.h>
#include <string.h>


mdv_errno mdv_write_msg(mdv_descriptor fd, mdv_msg const *msg)
{
    if (msg->hdr.size > MDV_MSG_SIZE_MAX)
    {
        MDV_LOGE("Message is too long");
        return MDV_FAILED;
    }

    mdv_msghdr const hdr =
    {
        .id     = mdv_hton16(msg->hdr.id),
        .number = mdv_hton16(msg->hdr.number),
        .size   = mdv_hton32(msg->hdr.size)
    };

    mdv_errno err = mdv_write_all(fd, &hdr, sizeof hdr);

    if (err != MDV_OK)
        return err;

    return mdv_write_all(fd, msg->payload, msg->hdr.size);
}


mdv_errno mdv_read_msg(mdv_descriptor fd, mdv_msg *msg)
{
    MDV_LOGI("DEBUG: mdv_read_msg start, fd=%d, msg=%p", fd, msg);
    
    if (!msg) {
        MDV_LOGE("DEBUG: NULL message pointer");
        return MDV_FAILED;
    }
    
    // Read header
    while(msg->available_size < sizeof(mdv_msghdr))
    {
        size_t len = sizeof(mdv_msghdr) - msg->available_size;

        mdv_errno err = mdv_read(fd, ((char*)&msg->hdr) + msg->available_size, &len);

        if (err != MDV_OK)
            return err;

        msg->available_size += len;

        if (msg->available_size == sizeof(mdv_msghdr))
        {
            msg->hdr.id     = mdv_ntoh16(msg->hdr.id);
            msg->hdr.number = mdv_ntoh16(msg->hdr.number);
            msg->hdr.size   = mdv_ntoh32(msg->hdr.size);
            
            MDV_LOGI("DEBUG: Header read - id=%u, number=%u, size=%u", 
                     msg->hdr.id, msg->hdr.number, msg->hdr.size);

            if (msg->hdr.size > MDV_MSG_SIZE_MAX)
            {
                MDV_LOGE("Incoming message is too long: %u > %u", msg->hdr.size, MDV_MSG_SIZE_MAX);
                memset(msg, 0, sizeof *msg);
                return MDV_FAILED;
            }


            if (msg->hdr.size)
            {
                msg->payload = mdv_alloc(msg->hdr.size);

                if (!msg->payload)
                {
                    MDV_LOGE("No memory for incoming message of size %u", msg->hdr.size);
                    memset(msg, 0, sizeof *msg);
                    return MDV_NO_MEM;
                }
                
                MDV_LOGI("DEBUG: Allocated payload %p for size %u", msg->payload, msg->hdr.size);
            }
            else
                msg->payload = 0;

            break;
        }
    }

    while(msg->available_size - sizeof(mdv_msghdr) < msg->hdr.size)
    {
        uint32_t const available_size = msg->available_size - sizeof(mdv_msghdr);

        size_t len = msg->hdr.size - available_size;
        
        if (!msg->payload && msg->hdr.size > 0) {
            MDV_LOGE("DEBUG: NULL payload but size > 0: %u", msg->hdr.size);
            return MDV_FAILED;
        }

        mdv_errno err = mdv_read(fd, (char*)msg->payload + available_size, &len);

        if (err != MDV_OK)
            return err;

        msg->available_size += len;
    }
    
    MDV_LOGI("DEBUG: Message read complete - id=%u, size=%u, payload=%p", 
             msg->hdr.id, msg->hdr.size, msg->payload);

    return MDV_OK;
}


void mdv_free_msg(mdv_msg *msg)
{
    mdv_free(msg->payload);
    memset(msg, 0, sizeof *msg);
}
