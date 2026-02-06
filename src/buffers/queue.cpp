#include "queue.h"

#include <stdexcept>
#include <sys/socket.h>

using namespace wl;

void recv_queue::Recv(const wl_fd_t socket) {
    const ssize_t new_size = recv(socket, *buffer.get(), PAGE_SIZE, 0);

    if (new_size < 0) {
        throw std::runtime_error("Failed to receive data");
    }

    if (new_size > std::numeric_limits<size_type>::max()) {
        throw std::runtime_error("Received more data than protocol allows");
    }

    if (!is_aligned(new_size)) {
        lumber::err("[Wayland::ERR]: Fatal stream frame misalignment (did not receive multiple of WORD bytes).");
    }

    current_size = new_size;
}

recv_queue::iterator recv_queue::begin() const noexcept {
    return recv_queue::iterator(*buffer.get(), current_size);
}

recv_queue::iterator recv_queue::end() const noexcept {
    return recv_queue::iterator(*buffer.get() + current_size, current_size);
}

recv_queue::~recv_queue() {
    
}

recv_queue::iterator::iterator(recv_queue::value_ptr buffer, size_type size)
  : buffer(buffer), current_size(size), m_ptr(buffer) {}

recv_queue::iterator& recv_queue::iterator::operator++() {
    m_ptr += *reinterpret_cast<const wl_uint16* const>(m_ptr + 6);

    if (m_ptr - buffer > current_size) {
        throw std::runtime_error("Fatal stream frame misalignment");
    }

    return *this;
}

recv_queue::iterator recv_queue::iterator::operator++(int) noexcept {
    iterator tmp = *this;
    ++(*this);
    return tmp;
}

wl_message recv_queue::iterator::operator*() const {
    const wl_object object_id = read_wl_object(m_ptr);
    const wl_uint size = *reinterpret_cast<const wl_uint16* const>(m_ptr + 6);
    const wl_uint opcode = *reinterpret_cast<const wl_uint16* const>(m_ptr + 4);
    char* payload = (char*)m_ptr + WL_EVENT_HEADER_SIZE;

    return wl_message(object_id, opcode, size, payload);
}

bool recv_queue::iterator::operator==(const recv_queue::iterator& other) const noexcept {
    return m_ptr == other.m_ptr;
}

bool recv_queue::iterator::operator!=(const recv_queue::iterator& other) const noexcept {
    return m_ptr != other.m_ptr;
}

send_queue::value_ptr send_queue::Allocate(const wl_uint bytes) {
    const value_ptr current = access_ptr;
    access_ptr += bytes;

    if (Offset() >= PAGE_SIZE) {
        throw std::runtime_error("Exceeded message buffer size");
    }

    msg_n++;

    return current;
}

void send_queue::SetAncillary(int data) noexcept {
    fd = data;
}

wl_uint send_queue::Send(const wl_fd_t socket) {
    if (fd != -1) {

        char cmsgbuf[CMSG_SPACE(sizeof(int))];

        struct iovec vec {
            .iov_base = buffer,
            .iov_len = Offset(),
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
        cmsg->cmsg_len = CMSG_LEN(WL_FD_SIZE);

        memcpy(CMSG_DATA(cmsg), &fd, WL_FD_SIZE);

        if (sendmsg(socket, &msg, 0) != Offset()) {
            throw std::runtime_error("Failed to send command");
        }
    } else {
        if (send(socket, buffer, Offset(), 0) != Offset()) {
            throw std::runtime_error("Failed to send entire send queue");
        }
    }

    const wl_uint prev_msg_n = msg_n;

    fd = -1;
    access_ptr = buffer;
    return prev_msg_n;
}

bool send_queue::Empty() const noexcept {
    return Offset() == 0;
}

send_queue::difference_type send_queue::Offset() const noexcept {
    return access_ptr - buffer;
}

send_queue::~send_queue() {
    free(buffer);
}