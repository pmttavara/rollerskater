#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define _NO_CRT_STDIO_INLINE

#include <stdint.h>
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include "enum.hpp"
#include <windows.h>

using u8 = uint8_t;
using s8 = int8_t;
using u08 = uint8_t;
using s08 = int8_t;
using u16 = uint16_t;
using s16 = int16_t;
using u32 = uint32_t;
using s32 = int32_t;
using u64 = uint64_t;
using s64 = int64_t;

#define logn(s, n) WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), s, n, NULL, NULL)
#define log(s) logn(s, sizeof(s) - 1)

