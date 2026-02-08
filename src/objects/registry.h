#pragma once

#include "../wl_utils/wl_types.h"
#include "../wl_utils/wl_state.h"
#include <string>

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
        std::cout << "Registry ID: " << id << '\n';
    }

    void set_listener(struct listener* listener) {
        this->listener = listener;
    }

    void handle_event(uint16_t opcode, wl_message::reader reader) override {
        if (!listener) {
            std::cout << "[Wayland::WARN]: Missing event listener.\n";
            return;
        }

        if (opcode == EV_GLOBAL_OPCODE) {
            const wl_uint name(reader.read_uint());
            const wl_string interface(reader.read_string());
            const wl_uint version(reader.read_uint());
            listener->global(*this, name, interface, version);
        } else if (opcode == EV_GLOBAL_REMOVE_OPCODE) {
            listener->global_remove(*this, reader.read_uint());
        }
    }

    /**
        Binds a server-side global to a client-side ID.
    */
    void bind(wl_uint name, const wl_string& interface, wl_uint version, wl_new_id id) {
        wl_message client_msg(this->id, BIND_OPCODE, 3 + (interface.word_size() + WL_WORD_SIZE));
        wl_message::writer writer = client_msg.new_writer(send_queue_alloc);
    
        writer.write(name);
        writer.write(interface);
        writer.write(version);
        writer.write(id);
    }

    wl_object ID() const noexcept override {
        return id;
    }
};

inline wl_registry* create_wl_registry(const wl_new_id id) {
    return new wl_registry(id);
}