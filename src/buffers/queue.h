#pragma once

#include "../wl_utils/wl_event.h"

#include <memory>
#include <span>
#include <vector>

namespace wl {
    /**
        @brief Represents a generic queue used for
        client-server communication.
    */
    class recv_queue {
        public:

        using value_type = wl_message::value_type;
        using value_ptr = wl_message::value_ptr;
        using reference = value_type&;
        using const_reference = const value_type&;
        using size_type = wl_uint;
        using difference_type = wl_uint;

        class iterator;

        private:

        static constexpr size_type PAGE_SIZE = 4096;

        const std::unique_ptr<value_ptr> buffer = std::make_unique<value_ptr>(static_cast<value_ptr>(malloc(PAGE_SIZE)));
        size_type current_size = 0;

        public:

        /**
            @brief Receive data.

            Calling `Recv` invalidates any iterators pointing
            to this queue.
        */
        void Recv(const wl_fd_t socket);

        iterator begin() const noexcept;
        iterator end() const noexcept;

        ~recv_queue();
    };

    class recv_queue::iterator {

        const value_type* buffer = nullptr;
        const size_type current_size = 0;
        const value_type* m_ptr = nullptr;

        public:

        iterator(value_ptr buffer, size_type size);

        iterator& operator++();

        iterator operator++(int) noexcept;

        wl_message operator*() const;

        bool operator==(const iterator& other) const noexcept;

        bool operator!=(const iterator& other) const noexcept;
    };

    class send_queue {
        public:

        using value_type = wl_message::value_type;
        using value_ptr = wl_message::value_ptr;
        using reference = wl_message::value_type&;
        using size_type = wl_uint;
        using difference_type = wl_uint;

        private:

        static constexpr size_type PAGE_SIZE = 4096;

        value_ptr buffer = static_cast<value_ptr>(malloc(PAGE_SIZE));
        size_type current_size = 0;
        value_ptr access_ptr = buffer;
        wl_uint msg_n = 0;
        std::vector<int> fds;

        public:

        /**
            @brief Allocates a region of the message
            buffer to the caller.

            This region is considered valid
            until ::Flush() is called.

            @warning The caller should NOT attempt to
            write outside of the range [value_ptr,
            value_ptr + bytes). Doing so could
            cause a memory corruption.
        */
        value_ptr Allocate(const wl_uint bytes);

        void AddFD(int data) noexcept;

        wl_uint Send(const wl_fd_t socket);

        bool Empty() const noexcept;

        difference_type Offset() const noexcept;
        
        ~send_queue();
    };
}