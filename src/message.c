#include <chess/message.h>
#include <chess/util.h>

enum
{
  MessageSleepMs = 10
};

ipcError message_receive(Message* mess, Socket* sock)
{
  int message_len = 0;
  while (socket_read_bytes(sock, &message_len, 4) == ipcErrorSocketHasMoreData)
    sleep_ms(MessageSleepMs);

  while (socket_read_bytes(sock, &mess->type, 4) == ipcErrorSocketHasMoreData)
    sleep_ms(MessageSleepMs);

  mess->data = malloc(message_len);
  mess->len = message_len;

  while (socket_read_bytes(sock, mess->data, message_len) ==
         ipcErrorSocketHasMoreData)
    sleep_ms(MessageSleepMs);
  return ipcErrorNone;
}

ipcError message_send(Message mess, Socket* sock)
{
  while (socket_write_bytes(sock, &(mess.len),
                            sizeof(mess.len)) ==
         ipcErrorSocketHasMoreData)
    sleep_ms(MessageSleepMs);
  while (socket_write_bytes(sock, &(mess.type),
                            sizeof(mess.type)) ==
         ipcErrorSocketHasMoreData)
    sleep_ms(MessageSleepMs);
  while (socket_write_bytes(sock, mess.data, mess.len) ==
         ipcErrorSocketHasMoreData)
    sleep_ms(MessageSleepMs);
  return ipcErrorNone;
}
