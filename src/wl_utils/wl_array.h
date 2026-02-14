#pragma once

#include "wl_types.h"

#include <cstring>
#include <iostream>
#include <vector>

/**
	@brief Specialisation of std::vector representing
	a wl_array.

	Possible values of T are restricted to those with both
	WL_WORD_SIZE alignment and a maximum size of WL_WORD_SIZE.


*/
template<class T>
class wl_array : public std::vector<T> {
	static_assert(alignof(T) == WL_WORD_SIZE, "Alignment of T must be WL_WORD_SIZE");
	static_assert(sizeof(T) <= WL_WORD_SIZE, "Size of T must be <= WL_WORD_SIZE");

	public:

	wl_array<T>(const wl_uint size) : std::vector<T>(size) {}

	wl_array<T>(const T* const data, const wl_uint size) : wl_array<T>(size) {
		memcpy(this->data(), data, size * WL_WORD_SIZE);
	}

	static wl_array<T> from_data(const void* const data) {
		const wl_uint size = read_wl_uint(data);
		const T* const arr_data = static_cast<const T* const>(data);
	}
};