#include <chess/message.h>
#include <chess/util.h>

enum
{
  MessageSleepMs = 10
};

ipcError message_receive(Socket* sock, void** buffer, size_t* buflen)
{
  int message_len = 0;
  while (socket_read_bytes(sock, &message_len, 4) == ipcErrorSocketHasMoreData)
    sleep_ms(MessageSleepMs);
  // Maybe have another few bytes here for indicating the type of message?
  *buffer = malloc(message_len);
  *buflen = message_len;
  while (socket_read_bytes(sock, *buffer, message_len) ==
         ipcErrorSocketHasMoreData)
    sleep_ms(MessageSleepMs);
  return ipcErrorNone;
}

ipcError message_send(Socket* sock, Message message)
{
  while (socket_write_bytes(sock, &(message.lengthInBytes),
                            sizeof(message.lengthInBytes)) ==
         ipcErrorSocketHasMoreData)
    ;
  while (socket_write_bytes(sock, message.data, message.lengthInBytes) ==
         ipcErrorSocketHasMoreData)
    ;
  return ipcErrorNone;
}
