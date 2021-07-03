#include <chess/logging.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
      return "ERROR";
    case DebugLevelWarning:
      return "WARN ";
    case DebugLevelInfo:
      return "INFO ";
    case DebugLevelDebug:
      return "DEBUG";
    default:
      return "";
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
        fprintf(streams[i],
                "%s: [%s] %s:%03d:%s(): ", debuglevel_tostring(level), time_str,
                file, line, func);
      vfprintf(streams[i], fmt, args);
      if (!(streams[i] == stderr || streams[i] == stdout))
        fclose(streams[i]);
    }

  free(time_str);
  va_end(args);
}
