#define _CRT_SECURE_NO_DEPRECATE
#define _USE_MATH_DEFINES

#include "defs.h"
#include "matrix.h"
#include "message.h"
#include "move.h"
#include "util.h"

#include <assert.h>
#include <ipc/socket.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * ###########################################################################
 * All board indexes are 64 based! If you need to convert to 0x88 format for
 * off-board checks etc. use the helper functions in util.h.
 * ###########################################################################
 */

int minimax(Board board, size_t depth, s64 alpha, s64 beta,
            bool maximising_player, bool is_initial_call, Move* out_move)
{
  int rv = 0;
  if (depth == 0)
  {
    // How many pieces does each player have?
    for (int i = 0; i < 64; i++)
    {
      if (board.state[i] && (board.state[i] & ChessPieceIsWhite))
        rv++;
      if (board.state[i] && !(board.state[i] & ChessPieceIsWhite))
        rv--;
    }
    return rv;
  }

  int best_eval = maximising_player ? -INT_MAX : INT_MAX;
  Board new_board = board;
  Move* moves;
  size_t nmoves = 0;
  board_get_moves_all(board, &moves, &nmoves,
                      maximising_player ? GetMovesWhite : GetMovesBlack);
  for (size_t i = 0; i < nmoves; i++)
  {
    board_update(&new_board, &(moves[i]));
    int eval = minimax(new_board, depth - 1, alpha, beta, !maximising_player,
                       false, NULL);
    new_board = board; // Restore board state after trying a move
    int last_best_eval = best_eval;
    best_eval = maximising_player ? max(best_eval, eval) : min(best_eval, eval);
    if (maximising_player)
    {
      alpha = max(alpha, eval);
      if (best_eval > last_best_eval && is_initial_call)
        if (out_move)
          *out_move = moves[i];
    }
    else
    {
      beta = min(beta, eval);
      if (best_eval < last_best_eval && is_initial_call)
        if (out_move)
          *out_move = moves[i];
    }
    if (beta <= alpha)
      break;
  }
  free(moves);
  return best_eval;
}

char* sockname = "ChessIPC";

int main(int argc, char* argv[])
{
  ipcError err = 0;
  Move move = {};
  Socket sock;
  Board board;
  board_new(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  printf("%s\n", board_tostring(board));
  bool isWhitesTurn = true;

  Move* moves;
  size_t nmoves;

  Move best_move;
  printf("minimax: %d\n",
         minimax(board, 7, -INT_MAX, INT_MAX, true, true, &best_move));
  printf("Best move: %s\n", move_tostring(best_move));
  socket_init(&sock, get_dotnet_pipe_name(sockname), SocketServer);
  for (;;)
  {
    while ((err = socket_connect(&sock)))
      sleep_ms(200);

    for (;;)
    {
      if (!socket_is_connected(&sock))
      {
        fprintf(stderr, "Error: Socket connection lost\n");
        break;
      }

      while (socket_read_bytes(&sock, &move, sizeof(move)) ==
             ipcErrorSocketHasMoreData)
        sleep_ms(10);

      printf("Client move: %s\n", move_tostring(move));

      board_update(&board, &move);
      isWhitesTurn = !isWhitesTurn;

      Move server_move;
      minimax(board, 7, -INT_MAX, INT_MAX, isWhitesTurn, true, &server_move);

      printf("Server move: %s\n", move_tostring(server_move));
      while (socket_write_bytes(&sock, &server_move, sizeof(move)) ==
             ipcErrorSocketHasMoreData)
        sleep_ms(10);
      isWhitesTurn = !isWhitesTurn;

      // Test code for Message passing between client/server
      /* void* buf; */
      /* size_t buflen = 0; */
      /* message_receive(&sock, &buf, &buflen); */
      /* printf("Got message %d\n", *(char*)buf); */

      board_update(&board, &server_move);
      printf("%s\n", board_tostring(board));
    }
  }

  return 0;
}
