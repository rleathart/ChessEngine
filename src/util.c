#include <chess/util.h>
#include <chess/piece.h>

#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

/* Return a random int between lower and upper inclusive. */
int randrange(int lower, int upper)
{
  return (rand() % (upper - lower + 1)) + lower;
}

/// {{{ 64 to 0x88 helper functions

u8 topos64(u8 pos88)
{
  return (pos88 + (pos88 & 7)) >> 1;
}
u8 topos64fr(u8 file, u8 rank)
{
  return file + 8 * rank;
}
u8 topos88(u8 pos64)
{
  return pos64 + (pos64 & ~7);
}
u8 topos88fr(u8 file, u8 rank)
{
  return file + 16 * rank;
}
u8 tofile64(u8 pos64)
{
  return pos64 % 8;
}
u8 torank64(u8 pos64)
{
  return pos64 / 8;
}
u8 tofile88(u8 pos88)
{
  return pos88 % 16;
}
u8 torank88(u8 pos88)
{
  return pos88 / 16;
}

// }}}

char* get_dotnet_pipe_name(char* name)
{
  char* pipename = calloc(1, 4096);
#ifdef _WIN32
  strcat(pipename, "\\\\.\\pipe\\");
  strcat(pipename, name);
#else
  char* tmpdir = getenv("TMPDIR");
  if (tmpdir)
    strcat(pipename, tmpdir);
  else
    strcat(pipename, "/tmp/");
  if (pipename[strlen(pipename) - 1] != '/')
    strcat(pipename, "/");
  strcat(pipename, "CoreFxPipe_");
  strcat(pipename, name);
#endif
  pipename = realloc(pipename, strlen(pipename) + 1);
  return pipename;
}
