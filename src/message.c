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

  if (mess->len > 0)
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
  if (mess.len > 0)
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
