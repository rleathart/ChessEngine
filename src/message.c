#include <rgl/logging.h>
#include <rgl/util.h>

#include <chess/defs.h>
#include <chess/message.h>
#include <chess/util.h>

enum
{
  MessageSleepMs = 1
};

ipcError message_receive(Message* mess, Socket* sock)
{
  ipcError more_data = ipcErrorSocketHasMoreData;

  int bytes_to_read = sizeof(*mess) - sizeof(mess->data);
  while (socket_read_bytes(sock, mess, bytes_to_read) == more_data)
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

  int bytes_to_write = sizeof(mess) - sizeof(mess.data);
  while (socket_write_bytes(sock, &mess, bytes_to_write) == more_data)
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
    case MessageTypeLegalMoveRequest:
      return "LegalMoveRequest";
    case MessageTypeLegalMoveReply:
      return "LegalMoveReply";
    case MessageTypeMakeMoveRequest:
      return "MakeMoveRequest";
    case MessageTypeMakeMoveReply:
      return "MakeMoveReply";
    case MessageTypeBestMoveRequest:
      return "BestMoveRequest";
    case MessageTypeBestMoveReply:
      return "BestMoveReply";
    case MessageTypeBoardStateRequest:
      return "BoardStateRequest";
    case MessageTypeBoardStateReply:
      return "BoardStateReply";
    case MessageTypeGetMovesRequest:
      return "GetMovesRequest";
    case MessageTypeGetMovesReply:
      return "GetMovesReply";
    case MessageTypeSetBoardRequest:
      return "SetBoardRequest";
    case MessageTypeSetBoardReply:
      return "SetBoardReply";
    case MessageTypePromotionRequest:
      return "PromotionRequest";
    case MessageTypePromotionReply:
      return "PromotionReply";
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
