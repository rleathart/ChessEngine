#define _CRT_SECURE_NO_DEPRECATE
#define _USE_MATH_DEFINES

#include <rgl/logging.h>
#include <rgl/threadpool.h>
#include <rgl/util.h>

#include <chess/defs.h>
#include <chess/evaluate.h>
#include <chess/matrix.h>
#include <chess/message.h>
#include <chess/move.h>
#include <chess/search.h>
#include <chess/tree.h>
#include <chess/util.h>

#include <assert.h>
#include <ipc/socket.h>
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*
 * ###########################################################################
 * All board indexes are 64 based! If you need to convert to 0x88 format for
 * off-board checks etc. use the helper functions in util.h.
 * ###########################################################################
 */

char* sockname = "ChessIPC";
int depth = 5;

void signal_handler(int sig)
{
  switch (sig)
  {
  case SIGSEGV:
    // Need to reset signal handler so we don't get stuck in a loop
    signal(SIGSEGV, SIG_DFL);
    // Calling printf in signal handler not allowed but we're crashing anyway
    ELOG("Segfault!\n");
    break;
  }
}

static Array g_logger_streams;

typedef struct
{
  Message mess_in;
  Board* board;
  Socket sock_in;
  Socket sock_out;
  Move player_move; // The last move made by the player
  Task* task;       // Need this so we can free the task memory once we're done.
} MessageHandlerArgs;

void* message_handler(void* void_args)
{
  static _Thread_local bool did_thread_setup = false;
  if (!did_thread_setup)
  {
    rgl_logger_thread_setup();
    for (int i = 0; i < g_logger_streams.capacity; i++)
    {
      if (!array_index_is_allocated(&g_logger_streams, i))
        continue;

      LoggerStream ls = *(LoggerStream*)array_get(&g_logger_streams, i);
      if (ls.stream)
        rgl_logger_thread_add_stream(ls.stream);
      if (ls.filename)
        rgl_logger_thread_add_file(ls.filename);
    }
    did_thread_setup = true;
  }

  MessageHandlerArgs args = *(MessageHandlerArgs*)void_args;
  Message mess_in = args.mess_in;
  Message mess_out;
  Board* board = args.board;
  Socket sock_in = args.sock_in;
  Socket sock_out = args.sock_out;
  Move player_move = args.player_move;

  Move move;
  Array moves;
  memset(&moves, 0, sizeof(moves));
  Board board_cpy = *board;
  switch (mess_in.type)
  {
  case MessageTypeLegalMoveRequest:
    mess_out.type = MessageTypeLegalMoveReply;
    mess_out.len = sizeof(int);
    mess_out.data = malloc(sizeof(int));
    int response = 0;
    move = *(Move*)mess_in.data;
    moves = board_get_moves(*board, move.from, ConsiderChecks);
    for (int i = 0; i < moves.capacity; i++)
    {
      if (!array_index_is_allocated(&moves, i))
        continue;
      if (array_get_as(&moves, i, Move).from == move.from &&
          array_get_as(&moves, i, Move).to == move.to)
        response = 1;
    }

    memcpy(mess_out.data, &response, sizeof(int));
    break;

  case MessageTypeMakeMoveRequest:
    mess_out.type = MessageTypeMakeMoveReply;
    mess_out.len = 1;
    mess_out.data = malloc(mess_out.len);
    Move tmp = *(Move*)mess_in.data;
    move = move_new(tmp.from, tmp.to);
    ILOG("Client move: %s\n", move_tostring(move));
    player_move = move;
    board_update(board, &move);
    ILOG("Board Updated:\n%s\n", board_tostring(*board));
    break;

  case MessageTypeBestMoveRequest:
    mess_out.type = MessageTypeBestMoveReply;
    mess_out.len = sizeof(Move);
    mess_out.data = malloc(mess_out.len);
    Node* server_root = node_new(NULL, player_move, false);
    Tree* server_tree = tree_new(server_root, *board, depth);
    move = search(server_tree);
    tree_free(&server_tree);
    board_update(board, &move);
    ILOG("Server move: %s\n", move_tostring(move));
    ILOG("Board Updated:\n%s\n", board_tostring(*board));
    memcpy(mess_out.data, &move, mess_out.len);
    break;

  case MessageTypeBoardStateRequest:
    mess_out.type = MessageTypeBoardStateReply;
    mess_out.len = sizeof(board->state);
    mess_out.data = malloc(mess_out.len);
    memcpy(mess_out.data, board->state, mess_out.len);
    break;

  case MessageTypeGetMovesRequest:
    mess_out.type = MessageTypeGetMovesReply;
    int pos = *(int*)mess_in.data;
    moves = board_get_moves(*board, pos, ConsiderChecks);
    mess_out.len = moves.used * sizeof(Move);
    mess_out.data = malloc(mess_out.len);
    u64 written = 0;
    for (int i = 0; i < moves.capacity; i++)
    {
      if (!array_index_is_allocated(&moves, i))
        continue;
      memcpy(mess_out.data + written, array_get(&moves, i), moves.data_size);
      written += moves.data_size;
    }
    break;

  // Sets the board from a FEN string
  case MessageTypeSetBoardRequest:
    mess_out.type = MessageTypeSetBoardReply;
    char* fen = (char*)mess_in.data;
    board_new(board, fen);
    mess_out.len = 1;
    mess_out.data = malloc(mess_out.len);
    ILOG("Board Set:\n%s\n", board_tostring(*board));
    break;

  // @@FIXME This will mess up precomputation since promoting after move.
  case MessageTypePromotionRequest:
    mess_out.type = MessageTypePromotionReply;
    ChessPiece piece = *(ChessPiece*)mess_in.data;
    ILOG("Promoting to: %d\n", piece);
    for (int i = 0; i < 8; i++)
      if (board->state[i] & ChessPiecePawn)
        board->state[i] = piece | (board->state[i] & ChessPieceIsWhite);
    for (int i = topos64(0x70); i < 64; i++)
      if (board->state[i] & ChessPiecePawn)
        board->state[i] = piece | (board->state[i] & ChessPieceIsWhite);
    mess_out.len = 1;
    mess_out.data = malloc(mess_out.len);
    ILOG("Board Promotion:\n%s\n", board_tostring(*board));
    break;
  case MessageTypeIsInCheckRequest:
    mess_out.type = MessageTypeIsInCheckReply;
    mess_out.len = 2;
    mess_out.data = malloc(mess_out.len);
    mess_out.data[0] = is_in_check(*board, true);
    mess_out.data[1] = is_in_check(*board, false);
    break;

  case MessageTypeIsInCheckmateRequest:
    if (mess_in.len != 1)
      ELOG("MessageTypeIsInCheckmateRequest does not have length of 1.");
    bool is_white = mess_in.data[0];
    mess_out.type = MessageTypeIsInCheckmateReply;
    mess_out.len = 1;
    mess_out.data = malloc(mess_out.len);
    mess_out.data[0] = is_in_checkmate(*board, is_white);
    break;
  case MessageTypeIsInStalemateRequest:
    if (mess_in.len != 1)
      ELOG("MessageTypeIsInCheckmateRequest does not have length of 1.");
    is_white = mess_in.data[0];
    mess_out.type = MessageTypeIsInStalemateReply;
    mess_out.len = 1;
    mess_out.data = malloc(mess_out.len);
    mess_out.data[0] = is_in_stalemate(*board, is_white);
    break;

  default:
    WLOG("Unknown message type %d\n", mess_in.type);
    break;
  }

  memcpy(&mess_out.guid, &mess_in.guid, sizeof(mess_in.guid));
  message_send(mess_out, &sock_out);
  free(mess_out.data);
  free(mess_in.data);
  array_free(&moves);
  free(args.task);
  free(void_args);
  return NULL;
}

int main(int argc, char* argv[])
{

  signal(SIGSEGV, signal_handler);

  // Set up our logger {{{
  // We have the global g_logger_streams so that in future it's easier to have
  // new threads inherit the logger streams that we define here.
  rgl_logger_thread_setup();
  array_new(&g_logger_streams, 8, sizeof(LoggerStream));
  // Add new global log streams here
  LoggerStream streams[] = {
      {.filename = strdup("chess.log")},
      {.stream = stderr},
  };
  for (int i = 0; i < sizeof(streams) / sizeof(streams[0]); i++)
    array_push(&g_logger_streams, &streams[i]);

  for (int i = 0; i < g_logger_streams.capacity; i++)
  {
    if (!array_index_is_allocated(&g_logger_streams, i))
      continue;

    LoggerStream stream = *(LoggerStream*)array_get(&g_logger_streams, i);
    if (stream.filename)
      rgl_logger_thread_add_file(stream.filename);
    if (stream.stream)
      rgl_logger_thread_add_stream(stream.stream);
  }
  // }}}

  ThreadPool pool;
  threadpool_new(&pool, 4);

  ipcError err = 0;
  Socket sock_in;
  Socket sock_out;
  socket_init(&sock_in, get_dotnet_pipe_name("ChessIPC_Requests"), SocketServer);
  socket_init(&sock_out, get_dotnet_pipe_name("ChessIPC_Replies"), SocketServer);

  Board board;
  board_new(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

  DLOG("\n%s\n", board_tostring(board));
  bool isWhitesTurn = true;

  for (;;)
  {
    while (socket_connect(&sock_in))
      sleep_ms(200);
    while (socket_connect(&sock_out))
      sleep_ms(200);

    board_new(&board,
              "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    Move player_move;

    for (;;)
    {
      if (!socket_is_connected(&sock_in) || !socket_is_connected(&sock_out))
      {
        ELOG("Socket lost connection\n");
        break;
      }

      Message mess_in, mess_out;
      message_receive(&mess_in, &sock_in);

      MessageHandlerArgs* args = calloc(1, sizeof(*args));
      args->board = &board;
      args->mess_in = mess_in;
      args->player_move = player_move;
      args->sock_in = sock_in;
      args->sock_out = sock_out;
      args->task = task_new(NULL, message_handler, args);

      threadpool_queue_task(&pool, args->task);
    }
  }

  return 0;
}
