#define _CRT_SECURE_NO_DEPRECATE
#define _USE_MATH_DEFINES

#include "defs.h"
#include "evaluate.h"
#include "matrix.h"
#include "message.h"
#include "move.h"
#include "search.h"
#include "util.h"
#include "tree.h"

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

char* sockname = "ChessIPC";

int main(int argc, char* argv[])
{

  char* logger_filepath = "chess.log";
  FILE* logger_fd = fopen(logger_filepath, "a");
  fprintf(logger_fd, "\n********************\n");
  fclose(logger_fd);

  srand(10);
  ipcError err = 0;
  Move move = {};
  Socket sock;
  Board board;
  board_new(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  printf("%s\n", board_tostring(board));
  table_black_init();
  bool isWhitesTurn = true;
  int depth = 5;

  Move* moves;
  size_t nmoves;

  Node* root = node_new(NULL, move_new(-1, -1), isWhitesTurn);
  Tree* tree = tree_new(root, board);

  Move best_move;
  printf("minimax: %d\n",
         minimax(board, depth, -INT_MAX, INT_MAX, true, &best_move, root));
  printf("Best move: %s\n\n", move_tostring(best_move));

  tree_print_best_line(*tree);

  int nodes_freed = tree_free(tree);

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
      minimax(board, depth, -INT_MAX, INT_MAX, isWhitesTurn, &server_move, NULL);

      printf("Server move: %s\n", move_tostring(server_move));
      while (socket_write_bytes(&sock, &server_move, sizeof(move)) ==
             ipcErrorSocketHasMoreData)
        sleep_ms(10);

      logger_fd = fopen(logger_filepath, "a");
      Move* line = board_calculate_line(board, depth, isWhitesTurn);
      fprintf(logger_fd, "Line:\n");
      Board test_board = board;
      fprintf(logger_fd, "%s\n\n", board_tostring(test_board));
      for (int i = 0; i < depth; i++)
      {
        board_update(&test_board, &line[i]);
        fprintf(logger_fd, "%s\n\n", board_tostring(test_board));
      }
      fclose(logger_fd);

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

  fclose(logger_fd);

  return 0;
}
