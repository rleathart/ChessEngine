#pragma once

#include "defs.h"

#include <ipc/socket.h>

ipcError message_receive(Message* mess, Socket* sock);
ipcError message_send(Message mess, Socket* sock);
