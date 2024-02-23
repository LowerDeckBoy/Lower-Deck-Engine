#pragma once

/*
	Core/CoreTypes.hpp
	Aliases for commonly used types.
*/

#include <cstdint>
//#include <type_traits>

using int8  = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;

using uint8  = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

using usize = size_t;

// Ensure that all aliases are of desired size.

static_assert(sizeof(int8)  == 1, "Expected to be a size of 1 bytes.");
static_assert(sizeof(int16) == 2, "Expected to be a size of 2 bytes.");
static_assert(sizeof(int32) == 4, "Expected to be a size of 4 bytes.");
static_assert(sizeof(int64) == 8, "Expected to be a size of 8 bytes.");

static_assert(sizeof(uint8)  == 1, "Expected to be a size of 1 bytes.");
static_assert(sizeof(uint16) == 2, "Expected to be a size of 2 bytes.");
static_assert(sizeof(uint32) == 4, "Expected to be a size of 4 bytes.");
static_assert(sizeof(uint64) == 8, "Expected to be a size of 8 bytes.");
