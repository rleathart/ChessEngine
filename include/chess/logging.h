#pragma once

// These all read the thread local variable t_debug_level.

#include <chess/defs.h>

extern _Thread_local DebugLevel t_debug_level;

#define ILOG(fmt, ...) chess_logger(DebugLevelInfo, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define WLOG(fmt, ...) chess_logger(DebugLevelWarning, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define ELOG(fmt, ...) chess_logger(DebugLevelError, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define DLOG(fmt, ...) chess_logger(DebugLevelDebug, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define IPRINT(fmt, ...) chess_logger(DebugLevelInfo, NULL, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define WPRINT(fmt, ...) chess_logger(DebugLevelWarning, NULL, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define EPRINT(fmt, ...) chess_logger(DebugLevelError, NULL, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define DPRINT(fmt, ...) chess_logger(DebugLevelDebug, NULL, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)

void chess_logger(DebugLevel level, char* file, int line, const char* func, const char* fmt, ...);
void tdebug_level_set(DebugLevel level);
DebugLevel tdebug_level_get();
void tdebug_level_set_prev();
