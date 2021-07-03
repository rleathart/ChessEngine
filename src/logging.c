#include <chess/logging.h>
#include "defs.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ANSI escape codes for colouring the terminal */
#define LOC_COL "\033[38;2;7;102;114m"
#define ERR_COL "\033[38;2;220;50;47m"
#define WAR_COL "\033[38;2;181;157;0m"
#define INF_COL "\033[38;2;53;193;90m"
#define DBG_COL "\033[38;2;88;109;210m"
#define ESC     "\033[0m"

#ifdef DEBUG
_Thread_local DebugLevel t_debug_level = DebugLevelDebug;
#else
_Thread_local DebugLevel t_debug_level = DebugLevelInfo;
#endif

static char* debuglevel_tostring(DebugLevel level)
{
  switch (level)
  {
    case DebugLevelError:
      return ERR_COL "ERROR" ESC;
    case DebugLevelWarning:
      return WAR_COL "WARN " ESC;
    case DebugLevelInfo:
      return INF_COL "INFO " ESC;
    case DebugLevelDebug:
      return DBG_COL "DEBUG" ESC;
    default:
      return "";
  }
}
static char* col(DebugLevel level)
{
  switch (level)
  {
    case DebugLevelError:
      return ERR_COL;
    case DebugLevelWarning:
      return WAR_COL;
    case DebugLevelInfo:
      return INF_COL;
    case DebugLevelDebug:
      return DBG_COL;
    default:
      return "";
  }
}
static char* conditional_escape(DebugLevel level)
{
  switch (level)
  {
    case DebugLevelError:
    case DebugLevelWarning:
      return "";
    default:
      return ESC;
  }

}
void chess_logger(DebugLevel level, char* file,
                  int line, const char* func, const char* fmt, ...)
{
  DebugLevel target_level = t_debug_level;
  va_list args;
  va_start(args, fmt);

  FILE* streams[] = {
      stderr,
      fopen("chess.log", "a"),
  };

  time_t log_time = time(NULL);
  struct tm* time_info = localtime(&log_time);
  char* time_str = calloc(1, 4096);
  // ISO Datetime
  strftime(time_str, 4096, "%Y-%m-%dT%H:%M:%S%z", time_info);

  for (int i = 0; i < sizeof(streams) / sizeof(streams[0]); i++)
    if (streams[i] == stderr && target_level < level)
    {
      continue;
    }
    else
    {
      if (file) // If file is NULL, don't print the header
      {
        char buffer[128];
        sprintf(buffer, LOC_COL "%s:%03d:%s()%s: %s",
                file, line, func, col(level), conditional_escape(level));
        fprintf(streams[i],
                "%s: [%s] %s",
                debuglevel_tostring(level), time_str, buffer);
      }
      vfprintf(streams[i], fmt, args);
      fprintf(streams[i], ESC);
      if (!(streams[i] == stderr || streams[i] == stdout))
        fclose(streams[i]);
    }

  free(time_str);
  va_end(args);
}

#undef LOC_COL
#undef ERR_COL
#undef WAR_COL
#undef INF_COL
#undef DBG_COL
#undef ESC
