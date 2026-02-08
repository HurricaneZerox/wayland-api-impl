#pragma once

#include <map>
#include <memory>
#include <set>

#include "wl_types.h"
#include "wl_obj.h"

/**
    @brief 
*/
class wl_id_assigner {
    std::set<wl_object> ids = {};

    public:

    wl_new_id request_id();
    void release_id(const wl_object id);
};

class wl_id_map {
    std::map<wl_object, std::shared_ptr<wl_obj*>> objects;

    public:

    std::shared_ptr<wl_obj*> get(const wl_object id);

    std::shared_ptr<wl_obj*> create(wl_obj& object);

    void destroy(const wl_object id);
};