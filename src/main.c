#define _CRT_SECURE_NO_DEPRECATE
#define _USE_MATH_DEFINES

#include <rgl/logging.h>

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
int depth = 4;

int main(int argc, char* argv[])
{

  // Set up our logger
  LoggerStream streams[] = {
      {
          .stream = stderr,
      },
      {
          .filename = strdup("chess.log"),
      },
  };
  for (int i = 0; i < sizeof(streams) / sizeof(streams[0]); i++)
    rgl_logger_add_stream(streams[i]);

  ipcError err = 0;
  Socket sock;
  socket_init(&sock, get_dotnet_pipe_name("ChessIPC_Messages"), SocketServer);

  Board board;
  board_new(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

  DLOG("\n%s\n", board_tostring(board));
  bool isWhitesTurn = true;

  Node* root = node_new(NULL, move_new(-1, -1), isWhitesTurn);
  Tree* tree = tree_new(root, board, depth);

  clock_t start_time = clock();
  Move best_move = search(tree);
  clock_t end_time = clock();
  double total_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
  DLOG("Best move: %s\n", move_tostring(best_move));
  DLOG("Time taken: %f\n\n", total_time);

  int nodes_freed = tree_free(&tree);

  for (;;)
  {
    while (socket_connect(&sock))
      sleep_ms(200);

    board_new(&board,
              "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    Move player_move;

    for (;;)
    {
      if (!socket_is_connected(&sock))
      {
        DLOG("Error: Socket lost connection\n");
        break;
      }

      Message mess_in, mess_out;
      message_receive(&mess_in, &sock);

      Move move;
      Move* moves = NULL;
      size_t nmoves = 0;
      Board board_cpy = board;
      switch (mess_in.type)
      {
      case MessageTypeLegalMoveRequest:
        mess_out.type = MessageTypeLegalMoveReply;
        mess_out.len = sizeof(int);
        mess_out.data = malloc(sizeof(int));
        int response = 0;
        move = *(Move*)mess_in.data;
        board_get_moves(board, move.from, &moves, &nmoves, ConsiderChecks);
        for (int i = 0; i < nmoves; i++)
          if (moves[i].from == move.from && moves[i].to == move.to)
            response = 1;

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
        board_update(&board, &move);
        ILOG("Board Updated:\n%s\n", board_tostring(board));
        break;

      case MessageTypeBestMoveRequest:
        mess_out.type = MessageTypeBestMoveReply;
        mess_out.len = sizeof(Move);
        mess_out.data = malloc(mess_out.len);
        Node* server_root = node_new(NULL, player_move, false);
        Tree* server_tree = tree_new(server_root, board, depth);
        move = search(server_tree);
        tree_free(&server_tree);
        board_update(&board, &move);
        ILOG("Server move: %s\n", move_tostring(move));
        ILOG("Board Updated:\n%s\n", board_tostring(board));
        memcpy(mess_out.data, &move, mess_out.len);
        break;

      case MessageTypeBoardStateRequest:
        mess_out.type = MessageTypeBoardStateReply;
        mess_out.len = sizeof(board.state);
        mess_out.data = malloc(mess_out.len);
        memcpy(mess_out.data, board.state, mess_out.len);
        break;

      case MessageTypeGetMovesRequest:
        mess_out.type = MessageTypeGetMovesReply;
        int pos = *(int*)mess_in.data;
        board_get_moves(board, pos, &moves, &nmoves, ConsiderChecks);
        mess_out.len = nmoves * sizeof(Move);
        mess_out.data = malloc(mess_out.len);
        memcpy(mess_out.data, moves, mess_out.len);
        break;

      // Sets the board from a FEN string
      case MessageTypeSetBoardRequest:
        mess_out.type = MessageTypeSetBoardReply;
        char* fen = (char*)mess_in.data;
        board_new(&board, fen);
        mess_out.len = 1;
        mess_out.data = malloc(mess_out.len);
        ILOG("Board Set:\n%s\n", board_tostring(board));
        break;

      // @@FIXME This will mess up precomputation since promoting after move.
      case MessageTypePromotionRequest:
        mess_out.type = MessageTypePromotionReply;
        ChessPiece piece = *(ChessPiece*)mess_in.data;
        ILOG("Promoting to: %d\n", piece);
        for (int i = 0; i < 8; i++)
          if (board.state[i] & ChessPiecePawn)
            board.state[i] = piece | (board.state[i] & ChessPieceIsWhite);
        for (int i = topos64(0x70); i < 64; i++)
          if (board.state[i] & ChessPiecePawn)
            board.state[i] = piece | (board.state[i] & ChessPieceIsWhite);
        mess_out.len = 1;
        mess_out.data = malloc(mess_out.len);
        ILOG("Board Promotion:\n%s\n", board_tostring(board));
        break;

      default:
        break;
      }

      message_send(mess_out, &sock);
      free(mess_out.data);
      free(mess_in.data);
      if (moves)
        free(moves);
    }
  }

  return 0;
}
