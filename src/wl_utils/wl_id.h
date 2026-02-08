#pragma once

#include <map>
#include <memory>
#include <set>
#include <stdexcept>

#include "wl_types.h"
#include "wl_obj.h"

class wl_id_assigner {
    std::set<wl_object> ids = {};

    public:

    wl_new_id get_id() {
        for (wl_new_id id = WL_NEW_ID_MIN; id < WL_NEW_ID_MAX; id++) {
            if (ids.count(id) != 0) { continue; }
            ids.insert(id);
            return id;
        }

        throw std::runtime_error("Failed to get new ID.");
    }

    void destroy_id(const wl_object id) {
        if (ids.count(id) == 0) {
            throw std::runtime_error("Attempt to destroy ID that is not bound to anything");
        }

        ids.erase(id);
    }
};

class wl_id_map {
    std::map<wl_object, std::shared_ptr<wl_obj*>> objects;

    public:

    std::shared_ptr<wl_obj*> get(const wl_object id) {
        if (objects.count(id) == 0) {
            return nullptr;
        }

        return objects.at(id);
    }

    std::shared_ptr<wl_obj*> create(wl_obj& object) {
        std::shared_ptr<wl_obj*> object_ptr = std::make_shared<wl_obj*>(&object);
        objects[object.ID()] = object_ptr;
        return object_ptr;
    }

    void destroy(const wl_object id) {
        objects.erase(id);
    }
};