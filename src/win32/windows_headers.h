#pragma once
#if defined(_WIN32_WINNT)
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0800

#include <winsock2.h>
#include <windows.h>
#include <winuser.h>

