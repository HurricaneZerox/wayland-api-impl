#pragma once

#include "../wl_types.h"
#include "../wl_state.h"

class wl_registry : public wl_obj {
    const wl_object id;

    static constexpr uintmax_t BIND_OPCODE = 0;

    static constexpr wl_uint EV_GLOBAL_OPCODE = 0;
    static constexpr wl_uint EV_GLOBAL_REMOVE_OPCODE = 1;

    public:

    struct listener {
        void (*global)(wl_registry& registry, wl_uint name, const wl_string& interface, wl_uint version);
        void (*global_remove)(wl_registry& registry, wl_uint name);
    };

    listener* listener = nullptr;

    wl_registry(const wl_new_id id) : id(id) {
        
    }

    void set_listener(struct listener* listener) {
        this->listener = listener;
    }

    void handle_event(uint16_t opcode, void* data, size_t size) override {
        if (!listener) {
            std::cout << "[Wayland::WARN]: Missing event listener.\n";
            return;
        }

        const wl_uint name(read_wl_uint(data));
        const wl_string interface((char*)data + WL_UINT_SIZE);
        const wl_uint version(read_wl_uint((char*)data + wl_align((2 * WL_UINT_SIZE) + interface.Size())));

        if (opcode == EV_GLOBAL_OPCODE) {
            listener->global(*this, name, interface, version);
        } else if (opcode == EV_GLOBAL_REMOVE_OPCODE) {
            listener->global_remove(*this, read_wl_uint(data));
        }
    }

    /**
        Binds a server-side global to a client-side ID.
    */
    void bind(wl_uint name, const wl_string& interface, wl_uint version, wl_new_id id) {
        WaylandMessage client_msg(send_queue_alloc, this->id, BIND_OPCODE, 3 + (interface.WordSize() + WL_WORD_SIZE));
    
        client_msg.Write(name);
        client_msg.Write(interface);
        client_msg.Write(version);
        client_msg.Write(id);
    }

    wl_object ID() const noexcept override {
        return id;
    }
};

inline wl_registry* create_wl_registry(const wl_new_id id) {
    return new wl_registry(id);
}