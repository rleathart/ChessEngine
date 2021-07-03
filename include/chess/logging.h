#pragma once

// These all read the thread local variable t_debug_level.

#include <chess/defs.h>

#define ILOG(fmt, ...) chess_logger(DebugLevelInfo, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__)
#define WLOG(fmt, ...) chess_logger(DebugLevelWarning, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__)
#define ELOG(fmt, ...) chess_logger(DebugLevelError, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__)
#define DLOG(fmt, ...) chess_logger(DebugLevelDebug, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__)
#define IPRINT(fmt, ...) chess_logger(DebugLevelInfo, NULL, __LINE__, __FUNCTION__, fmt, __VA_ARGS__)
#define WPRINT(fmt, ...) chess_logger(DebugLevelWarning, NULL, __LINE__, __FUNCTION__, fmt, __VA_ARGS__)
#define EPRINT(fmt, ...) chess_logger(DebugLevelError, NULL, __LINE__, __FUNCTION__, fmt, __VA_ARGS__)
#define DPRINT(fmt, ...) chess_logger(DebugLevelDebug, NULL, __LINE__, __FUNCTION__, fmt, __VA_ARGS__)

#define WLOG(fmt, ...) chess_logger(DebugLevelWarning, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__)
#define ELOG(fmt, ...) chess_logger(DebugLevelError, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__)
#define WPRINT(fmt, ...) chess_logger(DebugLevelWarning, NULL, __LINE__, __FUNCTION__, fmt, __VA_ARGS__)
#define EPRINT(fmt, ...) chess_logger(DebugLevelError, NULL, __LINE__, __FUNCTION__, fmt, __VA_ARGS__)

#ifndef DEBUG
#undef ILOG
#undef DLOG
#undef IPRINT
#undef DPRINT
#define ILOG(fmt, ...)
#define DLOG(fmt, ...)
#define IPRINT(fmt, ...)
#define DPRINT(fmt, ...)
#endif

void chess_logger(DebugLevel level, char* file, int line, const char* func, const char* fmt, ...);
