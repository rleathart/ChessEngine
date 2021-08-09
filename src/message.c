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
  ipcError more_data = ipcErrorSocketHasMoreData;
  while (socket_read_bytes(sock, &mess->len, 4) == more_data)
    sleep_ms(MessageSleepMs);

  while (socket_read_bytes(sock, &mess->type, 4) == more_data)
    sleep_ms(MessageSleepMs);

  while (socket_read_bytes(sock, &mess->guid, 16) == more_data)
    sleep_ms(MessageSleepMs);

  mess->data = calloc(mess->len, 1);

  if (mess->len > 0)
    while (socket_read_bytes(sock, mess->data, mess->len) == more_data)
      sleep_ms(MessageSleepMs);

  DLOG("Message data (%s) (len: %d): ", messagetype_tostring(mess->type),
       mess->len);
  for (int i = 0; i < mess->len; i++)
    DPRINT("%d ", mess->data[i]);
  DPRINT("\n");
  return ipcErrorNone;
}

ipcError message_send(Message mess, Socket* sock)
{
  ipcError more_data = ipcErrorSocketHasMoreData;
  while (socket_write_bytes(sock, &mess.len, sizeof(mess.len)) == more_data)
    sleep_ms(MessageSleepMs);

  while (socket_write_bytes(sock, &mess.type, sizeof(mess.type)) == more_data)
    sleep_ms(MessageSleepMs);

  while (socket_write_bytes(sock, &mess.guid, sizeof(mess.guid)) == more_data)
    sleep_ms(MessageSleepMs);

  if (mess.len > 0)
    while (socket_write_bytes(sock, mess.data, mess.len) == more_data)
      sleep_ms(MessageSleepMs);

  DLOG("Message data (%s) (len: %d): ", messagetype_tostring(mess.type),
       mess.len);
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
    case MessageTypeIsInCheckRequest:
      return "IsInCheckRequest";
    case MessageTypeIsInCheckReply:
      return "IsInCheckReply";
    case MessageTypeIsInCheckmateRequest:
      return "IsInCheckmateRequest";
    case MessageTypeIsInCheckmateReply:
      return "IsInCheckmateReply";
    case MessageTypeIsInStalemateRequest:
      return "IsInStalemateRequest";
    case MessageTypeIsInStalemateReply:
      return "IsInStalemateReply";

    default:
      break;
  }

  return "Unknown MessageType";
}
