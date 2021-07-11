#pragma once

#include "defs.h"

int randrange(int lower, int upper);

u8 topos64(u8 pos88);
u8 topos64fr(u8 file, u8 rank);
u8 topos88(u8 pos64);
u8 topos88fr(u8 file, u8 rank);
u8 tofile64(u8 pos64);
u8 torank64(u8 pos64);
u8 tofile88(u8 pos88);
u8 torank88(u8 pos88);

char* get_dotnet_pipe_name(char* name);
