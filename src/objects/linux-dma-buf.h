#pragma once

#include "../wl_utils/wl_obj.h"
#include "surface.h"

#include <cstdint>
#include <drm/drm_fourcc.h>
#include <drm/drm_mode.h>

/**
    @brief Allows Wayland clients and compositors to
    share buffers created on a graphics device.
*/
namespace zwp::linux_dmabuf {

    /**
        @brief Parameters for the creation of a dmabuf-
        based wl_buffer.
    */
    class params : public wl_obj {
		const wl_uint id;

		static constexpr wl_uint DESTROY_OPCODE = 0;
		static constexpr wl_uint ADD_OPCODE = 1;
		static constexpr wl_uint CREATE_OPCODE = 2;
		static constexpr wl_uint CREATE_IMMED_OPCODE = 3;

		static constexpr wl_uint EV_CREATED_OPCODE = 0;
		static constexpr wl_uint EV_FAILED_OPCODE = 1;

		public:

		params(const wl_uint id) : id(id) {}

		void handle_event(uint16_t opcode, wl_message::reader reader) override {
			std::cout << "Params ev\n";
		}

		wl_object ID() const noexcept override {
			return id;
		}

		void add(wl_fd_t fd, wl_uint plane_idx, wl_uint offset, wl_uint stride, uint64_t modifiers) {
			wl_message client_msg(id, ADD_OPCODE, 6);
			wl_message::writer writer = client_msg.new_writer(send_queue_alloc);

			//writer.write(fd);
			//writer.write(plane_idx);
			//writer.write(offset);
			//writer.write(stride);
			//writer.write((modifiers >> 32) & 0xFFFF);
			//writer.write((modifiers) & 0xFFFF);

		}

    };

    class feedback : public wl_obj {
		const wl_uint id;

		static constexpr wl_uint DESTROY_OPCODE = 0;

		static constexpr wl_uint EV_DONE_OPCODE = 0;
		static constexpr wl_uint EV_FORMAT_TABLE_OPCODE = 1;
		static constexpr wl_uint EV_MAIN_DEVICE_OPCODE = 2;
		static constexpr wl_uint EV_TRANCHE_DONE_OPCODE = 3;
		static constexpr wl_uint EV_TRANCHE_TARGET_DEVICE_OPCODE = 4;
		static constexpr wl_uint EV_TRANCHE_FORMATS_OPCODE = 5;
		static constexpr wl_uint EV_TRANCHE_FLAGS_OPCODE = 6;

		public:

		feedback(const wl_uint id) : id(id) {}

		void handle_event(uint16_t opcode, wl_message::reader reader) override {
			std::cout << "feedback ev\n";

			if (opcode == EV_DONE_OPCODE) {
				//std::cout << "Done\n";
			} else if (opcode == EV_FORMAT_TABLE_OPCODE) {
				//std::cout << "Format table\n";
			} else if (opcode == EV_MAIN_DEVICE_OPCODE) {
				//std::cout << "Tranche main device\n";
			} else if (opcode == EV_TRANCHE_DONE_OPCODE) {
				//std::cout << "Tranche done\n";
			} else if (opcode == EV_TRANCHE_TARGET_DEVICE_OPCODE) {
				//std::cout << "Tranche target device\n";
			} else if (opcode == EV_TRANCHE_FORMATS_OPCODE) {
				//std::cout << "Tranche formats\n";

				
			} else if (opcode == EV_TRANCHE_FLAGS_OPCODE) {
				//std::cout << "Tranche flags\n";
			} else {
				lumber::warn("[Wayland::WARN]: Unimplemented dmabuf feedback ev");
			}
		}

		wl_object ID() const noexcept override {
			return id;
		}
    };

	/**
        @brief Factory object for creating dmabuf-based
        wl_buffers.

        @note
    */
    class dmabuf : public wl_obj {
		const wl_uint id;

		static constexpr wl_uint DESTROY_OPCODE = 0;
		static constexpr wl_uint CREATE_PARAMS_OPCODE = 1;
		static constexpr wl_uint GET_DEFAULT_FEEDBACK_OPCODE = 2;
		static constexpr wl_uint GET_SURFACE_FEEDBACK_OPCODE = 3;

		public:

		dmabuf(const wl_uint id) : id(id) {}

		void handle_event(uint16_t opcode, wl_message::reader reader) override {
			std::cout << "dmabuf ev\n";
		}

		wl_object ID() const noexcept override {
			return id;
		}

		void destroy();

		params& create_params() {
			params* params = new class params(wl_id_assigner.request_id());

			wl_message client_msg(id, CREATE_PARAMS_OPCODE, 1);
			wl_message::writer writer = client_msg.new_writer(send_queue_alloc);

			writer.write(params->ID());

			wl_id_map.create(*params);

			return *params;
		}

		feedback& get_default_feedback();

		

		feedback& get_surface_feedback(wl_surface& surface) {
			feedback* feedback = new class feedback(wl_id_assigner.request_id());

			wl_message client_msg(id, GET_SURFACE_FEEDBACK_OPCODE, 2);
			wl_message::writer writer = client_msg.new_writer(send_queue_alloc);

			writer.write(feedback->ID());
			writer.write(surface.ID());

			wl_id_map.create(*feedback);

			return *feedback;
		}
    };


}