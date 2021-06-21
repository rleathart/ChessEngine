#pragma once

#include "defs.h"

#include <ipc/socket.h>

ipcError message_receive(Socket* sock, void** buffer, size_t* buflen);
ipcError message_send(Socket* sock, Message message);
