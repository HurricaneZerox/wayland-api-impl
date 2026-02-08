#include "wl_id.h"

wl_new_id wl_id_assigner::request_id() {
    for (wl_new_id id = WL_NEW_ID_MIN; id < WL_NEW_ID_MAX; id++) {
        if (ids.count(id) != 0) { continue; }
        ids.insert(id);
        return id;
    }

    throw std::runtime_error("Failed to get new ID.");
}

void wl_id_assigner::release_id(const wl_object id) {
    if (ids.count(id) == 0) {
        throw std::runtime_error("Attempt to destroy ID that is not bound to anything");
    }

    ids.erase(id);
}

std::shared_ptr<wl_obj*> wl_id_map::get(const wl_object id) {
    if (objects.count(id) == 0) {
        return nullptr;
    }

    return objects.at(id);
}

std::shared_ptr<wl_obj*> wl_id_map::create(wl_obj& object) {
    std::shared_ptr<wl_obj*> object_ptr = std::make_shared<wl_obj*>(&object);
    objects[object.ID()] = object_ptr;
    return object_ptr;
}

void wl_id_map::destroy(const wl_object id) {
    objects.erase(id);
}