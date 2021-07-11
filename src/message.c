#include <rgl/logging.h>
#include <rgl/util.h>

#include <chess/defs.h>
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
  DLOG("Message data (%s) (len: %d): ", messagetype_tostring(mess->type), mess->len);
  for (int i = 0; i < message_len; i++)
    DPRINT("%d ", mess->data[i]);
  DPRINT("\n");
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
  DLOG("Message data (%s) (len: %d): ", messagetype_tostring(mess.type), mess.len);
  for (int i = 0; i < mess.len; i++)
    DPRINT("%d ", mess.data[i]);
  DPRINT("\n");
  return ipcErrorNone;
}

char* messagetype_tostring(MessageType type)
{
  switch (type)
  {
    case MessageTypeLegalMoveReply:
      return "LegalMoveReply";
    case MessageTypeLegalMoveRequest:
      return "LegalMoveRequest";
    case MessageTypeMakeMoveReply:
      return "MakeMoveReply";
    case MessageTypeMakeMoveRequest:
      return "MakeMoveRequest";
    case MessageTypeBestMoveReply:
      return "BestMoveReply";
    case MessageTypeBestMoveRequest:
      return "BestMoveRequest";
    case MessageTypeBoardStateReply:
      return "BoardStateReply";
    case MessageTypeBoardStateRequest:
      return "BoardStateRequest";
    case MessageTypeGetMovesReply:
      return "GetMovesReply";
    case MessageTypeGetMovesRequest:
      return "GetMovesRequest";
    case MessageTypeSetBoardReply:
      return "SetBoardReply";
    case MessageTypeSetBoardRequest:
      return "SetBoardRequest";
    default:
      break;
  }

  return "Unknown MessageType";
}
