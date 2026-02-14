#pragma once

#include "../wl_utils/wl_types.h"
#include "../wl_utils/wl_obj.h"

namespace wl {
	/**
		@brief Describes part of the compositor
		geometry, typically corresponding to a monitor.
	*/
	class output : public wl_obj {
		const wl_object id;

		static constexpr wl_opcode_t RELEASE_OPCODE = 0;

		static constexpr wl_opcode_t EV_GEOMETRY_OPCODE = 0;
		static constexpr wl_opcode_t EV_MODE_OPCODE = 1;
		static constexpr wl_opcode_t EV_DONE_OPCODE = 2;
		static constexpr wl_opcode_t EV_SCALE_OPCODE = 3;
		static constexpr wl_opcode_t EV_NAME_OPCODE = 4;
		static constexpr wl_opcode_t EV_DESCRIPTION_OPCODE = 5;

		public:

		output(const wl_object id) : id(id) {}

		void handle_event(uint16_t opcode, wl_message::reader reader) override {
			if (opcode == EV_GEOMETRY_OPCODE) {
				//std::cout << "Geometry\n";
			} else if (opcode == EV_MODE_OPCODE) {
				const wl_uint flags = reader.read_uint();
				const wl_int width = reader.read_int();
				const wl_int height = reader.read_int();
				const wl_int refresh_rate = reader.read_int();
				/*std::cout << "wl::output mode:\n";
				std::cout << "\tflags: " << flags << '\n';
				std::cout << "\twidth: " << width << '\n';
				std::cout << "\theight: " << height << '\n';
				std::cout << "\trefresh rate: " << refresh_rate << '\n';*/

			} else if (opcode == EV_DONE_OPCODE) {
				//lumber::info("[Wayland::INFO]: wl::output done.");
			} else if (opcode == EV_SCALE_OPCODE) {
				const wl_int scale = reader.read_int();
				//std::cout << "wl::output scale: " << scale << '\n';
			} else if (opcode == EV_NAME_OPCODE) {
				const wl_string name = reader.read_string();
				//std::cout << "wl::output name: " << name << '\n';
			} else if (opcode == EV_DESCRIPTION_OPCODE) {
				const wl_string description = reader.read_string();
				//std::cout << "wl::output description: " << description << '\n';
			} else {
				lumber::warn("[Wayland::WARN]: Unimplemented event opcode for wl::output.");
			}
		}

		wl_object ID() const noexcept override {
			return id;
		}
	};
}