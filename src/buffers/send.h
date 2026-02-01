#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <sys/socket.h>
#include <vector>

#include "../wl_types.h"

class SendQueue {
    static constexpr size_t max_alloc = 4096;

    size_t offset = 0;
    size_t msg_n = 0;
    void* buffer = malloc(max_alloc);

    int fd = -1;

    public:

    /**
        Allocate a region of the message
        buffer to the caller.

        This region is considered valid
        until ::Flush() is called.
    */
    void* Allocate(const size_t bytes) {
        void* current = (char*)buffer + offset;
        offset += bytes;

        if (offset >= max_alloc) {
            throw std::runtime_error("Exceeded message buffer size");
        }

        msg_n++;

        return current;
    }

    void SetAncillary(int data) {
        fd = data;
    }

    void Flush(const int socket) {
        if (fd != -1) {
            char* fd_data = new char[4];
            from_int(fd, fd_data);

            char cmsgbuf[CMSG_SPACE(sizeof(int))];

            struct iovec vec {
                .iov_base = buffer,
                .iov_len = offset,
            };

            struct msghdr msg {
                .msg_iov = &vec,
                .msg_iovlen = 1,
                .msg_control = cmsgbuf,
                .msg_controllen = CMSG_SPACE(sizeof(int)),
            };
            
            struct cmsghdr* cmsg;
            cmsg = CMSG_FIRSTHDR(&msg);
            cmsg->cmsg_level = SOL_SOCKET;
            cmsg->cmsg_type = SCM_RIGHTS;
            cmsg->cmsg_len = CMSG_LEN(sizeof(int));

            memcpy(CMSG_DATA(cmsg), &fd, sizeof(int));

            if (sendmsg(socket, &msg, 0) !=  offset) {
                throw std::runtime_error("Failed to send command");
            }

            delete[] fd_data;
        } else {
            if (send(socket, buffer, offset, 0) != offset) {
                throw std::runtime_error("Failed to send entire send queue");
            }
        }

        fd = -1;

        offset = 0;
        msg_n = 0;
    }

    bool Empty() const noexcept {
        return offset == 0;
    }
};