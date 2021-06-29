#include "chess/defs.h"
#include <check.h>
#include <chess/board.h>
#include <chess/util.h>
#include <stdlib.h>

START_TEST(test_pawn_moves)
{
  Board board;
  Move* moves;
  size_t nmoves = 0;

  board_new_from_string(&board, "0 0 0 0 0 0 0 0"
                                "p 0 0 0 0 0 0 0"
                                "0 P 0 0 0 0 0 P"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0");

  // Pawn should be able to move 1 or 2 squares forward and capture enemy pawn.
  // It should not wrap around the board to capture the pawn on the other side
  board_get_moves(board, topos64(0x10), &moves, &nmoves, 0);

  int expected_nmoves = 3;
  ck_assert_int_eq(nmoves, expected_nmoves);

  bool found_moves[expected_nmoves];
  for (int i = 0; i < expected_nmoves; i++)
    found_moves[i] = false;

  int idx = 0;
  for (int i = 0; i < nmoves; i++)
  {
    if (moves[i].from == topos64(0x10) && moves[i].to == topos64(0x20))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x10) && moves[i].to == topos64(0x30))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x10) && moves[i].to == topos64(0x21))
      found_moves[idx++] = true;
  }

  for (int i = 0; i < expected_nmoves; i++)
    fail_if(!found_moves[i]);

  // @@Implement explicitly check that pawns cant capture a piece directly in
  // front of them

  // @@Implement explicitly check that pawns can only move one square after they
  // leave their starting rank
}
END_TEST

START_TEST(test_en_passant)
{
  Board board;
  Move* moves;
  size_t nmoves = 0;

  board_new_from_string(&board, "0 0 0 0 0 0 0 0"
                                "0 0 0 0 p 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 P 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0");
  Move tmp = move_new(topos64(0x14), topos64(0x34));
  board_update(&board, &tmp); // Double move black pawn
  ck_assert_int_eq(board.en_passant_tile, topos64(0x24));

  board_get_moves(board, topos64(0x33), &moves, &nmoves, 0);

  int expected_nmoves = 2;
  ck_assert_int_eq(nmoves, expected_nmoves);

  bool found_moves[expected_nmoves];
  for (int i = 0; i < expected_nmoves; i++)
    found_moves[i] = false;

  int idx = 0;
  for (int i = 0; i < nmoves; i++)
  {
    if (moves[i].from == topos64(0x33) && moves[i].to == topos64(0x23))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x33) && moves[i].to == topos64(0x24))
      found_moves[idx++] = true;
  }

  for (int i = 0; i < expected_nmoves; i++)
    fail_if(!found_moves[i]);
}
END_TEST

START_TEST(test_knight_moves)
{
  Board board;
  Move* moves;
  size_t nmoves = 0;

  board_new_from_string(&board, "0 0 0 0 0 0 0 0"
                                "n 0 0 0 0 0 0 0"
                                "0 0 P 0 0 0 P 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0");

  board_get_moves(board, topos64(0x10), &moves, &nmoves, 0);

  int expected_nmoves = 3;
  ck_assert_int_eq(nmoves, expected_nmoves);

  bool found_moves[expected_nmoves];
  for (int i = 0; i < expected_nmoves; i++)
    found_moves[i] = false;

  int idx = 0;
  for (int i = 0; i < nmoves; i++)
  {
    if (moves[i].from == topos64(0x10) && moves[i].to == topos64(0x02))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x10) && moves[i].to == topos64(0x22))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x10) && moves[i].to == topos64(0x31))
      found_moves[idx++] = true;
  }

  for (int i = 0; i < expected_nmoves; i++)
    fail_if(!found_moves[i]);

  // @@Implement check knights moves in the middle of the board (with jumping
  // over pieces)
}
END_TEST

START_TEST(test_castle_moves)
{
  Board board;
  Move* moves;
  size_t nmoves = 0;

  board_new_from_string(&board, "0 0 0 0 0 0 0 0"
                                "P r 0 0 0 0 0 0"
                                "P 0 0 0 0 0 0 0"
                                "0 p 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 P 0 0 0 0 0 0");

  board_get_moves(board, topos64(0x11), &moves, &nmoves, 0);

  int expected_nmoves = 9;
  ck_assert_int_eq(nmoves, expected_nmoves);

  bool found_moves[expected_nmoves];
  for (int i = 0; i < expected_nmoves; i++)
    found_moves[i] = false;

  int idx = 0;
  for (int i = 0; i < nmoves; i++)
  {
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x01))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x10))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x12))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x13))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x14))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x15))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x16))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x17))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x21))
      found_moves[idx++] = true;
  }

  for (int i = 0; i < expected_nmoves; i++)
    fail_if(!found_moves[i]);

  // @@Implement Castling
}
END_TEST

START_TEST(test_bishop_moves)
{
  Board board;
  Move* moves;
  size_t nmoves = 0;

  board_new_from_string(&board, "0 0 0 0 0 0 0 0"
                                "0 b 0 0 0 0 0 0"
                                "P 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 p 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0");

  board_get_moves(board, topos64(0x11), &moves, &nmoves, 0);

  int expected_nmoves = 5;
  ck_assert_int_eq(nmoves, expected_nmoves);

  bool found_moves[expected_nmoves];
  for (int i = 0; i < expected_nmoves; i++)
    found_moves[i] = false;

  int idx = 0;
  for (int i = 0; i < nmoves; i++)
  {
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x00))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x02))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x20))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x22))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x33))
      found_moves[idx++] = true;
  }

  for (int i = 0; i < expected_nmoves; i++)
    fail_if(!found_moves[i]);
}
END_TEST

START_TEST(test_queen_moves)
{
  Board board;
  Move* moves;
  size_t nmoves = 0;

  board_new_from_string(&board, "0 0 0 0 0 0 0 0"
                                "0 q 0 0 0 p 0 P"
                                "P 0 0 0 0 0 0 0"
                                "0 p 0 0 0 0 0 0"
                                "0 0 0 0 p 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 P 0 0 0 0 0 0");

  board_get_moves(board, topos64(0x11), &moves, &nmoves, 0);

  int expected_nmoves = 11;
  ck_assert_int_eq(nmoves, expected_nmoves);

  bool found_moves[expected_nmoves];
  for (int i = 0; i < expected_nmoves; i++)
    found_moves[i] = false;

  int idx = 0;
  for (int i = 0; i < nmoves; i++)
  {
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x00))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x01))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x02))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x10))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x12))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x13))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x14))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x20))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x21))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x22))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x33))
      found_moves[idx++] = true;
  }

  for (int i = 0; i < expected_nmoves; i++)
    fail_if(!found_moves[i]);
}
END_TEST

START_TEST(test_king_moves)
{
  Board board;
  Move* moves;
  size_t nmoves = 0;

  // First just do a simple test for the king moving 1 square
  board_new_from_string(&board, "0 0 k 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 P 0 0 0 0 0 0");

  board_get_moves(board, topos64(0x02), &moves, &nmoves, 0);

  int expected_nmoves = 5;
  ck_assert_int_eq(nmoves, expected_nmoves);

  bool found_moves[expected_nmoves];
  for (int i = 0; i < expected_nmoves; i++)
    found_moves[i] = false;

  int idx = 0;
  for (int i = 0; i < nmoves; i++)
  {
    if (moves[i].from == topos64(0x02) && moves[i].to == topos64(0x01))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x02) && moves[i].to == topos64(0x03))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x02) && moves[i].to == topos64(0x11))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x02) && moves[i].to == topos64(0x12))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x02) && moves[i].to == topos64(0x13))
      found_moves[idx++] = true;
  }

  for (int i = 0; i < expected_nmoves; i++)
    fail_if(!found_moves[i]);

  // @@Implement test for check condition
}
END_TEST

START_TEST(test_castling)
{
  Board board;
  Move* moves;
  size_t nmoves = 0;

  board_new_from_string(&board, "r 0 0 0 k 0 0 r"
                                "0 0 0 p p p 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0");

  board_get_moves(board, topos64(0x04), &moves, &nmoves, 0);

  int expected_nmoves = 4;
  ck_assert_int_eq(nmoves, expected_nmoves);

  bool found_moves[expected_nmoves];
  for (int i = 0; i < expected_nmoves; i++)
    found_moves[i] = false;

  int idx = 0;
  for (int i = 0; i < nmoves; i++)
  {
    if (moves[i].from == topos64(0x04) && moves[i].to == topos64(0x03))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x04) && moves[i].to == topos64(0x05))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x04) && moves[i].to == topos64(0x06))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x04) && moves[i].to == topos64(0x02))
      found_moves[idx++] = true;
  }

  for (int i = 0; i < expected_nmoves; i++)
    fail_if(!found_moves[i], "!found_moves[%d]", i);

  Move tmp = move_new(topos64(0x04), topos64(0x02));
  board_update(&board, &tmp);
  ck_assert_int_eq(board.state[0], ChessPieceNone);
  ck_assert_int_eq(board.state[2], ChessPieceKing);
  ck_assert_int_eq(board.state[3], ChessPieceCastle);

  tmp = move_new(topos64(0x02), topos64(0x03));
  board_update(&board, &tmp);
  tmp = move_new(topos64(0x03), topos64(0x04));
  board_update(&board, &tmp);

  // We shouldnt be able to castle kingside now
  board_get_moves(board, topos64(0x04), &moves, &nmoves, 0);
  ck_assert_int_eq(nmoves, 2);

  // We can't castle through check
  board_new_from_string(&board, "r 0 0 0 k 0 0 r"
                                "0 0 0 0 p p 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 Q 0 0 0 0");

  board_get_moves(board, topos64(0x04), &moves, &nmoves, ConsiderChecks);

  // Should only be able to castle kingside here
  expected_nmoves = 2;
  ck_assert_int_eq(nmoves, expected_nmoves);

  idx = 0;
  for (int i = 0; i < nmoves; i++)
  {
    if (moves[i].from == topos64(0x04) && moves[i].to == topos64(0x05))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x04) && moves[i].to == topos64(0x06))
      found_moves[idx++] = true;
  }

  for (int i = 0; i < expected_nmoves; i++)
    fail_if(!found_moves[i], "!found_moves[%d]", i);

  // Check that white can castle too
  board_new_from_string(&board, "r n b q k b n r"
                                "p p p p p p p p"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 P 0 0 0"
                                "0 0 0 0 0 N 0 0"
                                "P P P P B P P P"
                                "R N B Q K 0 0 R");

  board_get_moves(board, topos64(0x74), &moves, &nmoves, ConsiderChecks);

  ck_assert_int_eq(nmoves, 2);
}
END_TEST

// Pinned pieces should still be able to check the enemy king
START_TEST(test_pinned_check)
{
  Board board;
  Move* moves;
  size_t nmoves = 0;

  board_new_from_string(&board, "k 0 0 0 0 0 0 0"
                                "0 r 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 B 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 K 0 0 0 0 0 0");
  for (int i = 0; i < 2; i++)
    board.can_castle_ks[i] = board.can_castle_qs[i] = false;

  board_get_moves(board, topos64(0x44), &moves, &nmoves, ConsiderChecks);

  int expected_nmoves = 1;
  ck_assert_int_eq(nmoves, expected_nmoves);
  expected_nmoves = 4;
  board_get_moves(board, topos64(0x71), &moves, &nmoves, ConsiderChecks);
  ck_assert_int_eq(nmoves, expected_nmoves);
}
END_TEST

START_TEST(test_starting_moves)
{
  Board board;
  Move* moves;
  size_t nmoves = 0;

  board_new_from_string(&board, "r n b q k b n r"
                                "p p p p p p p p"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "P P P P P P P P"
                                "R N B Q K B N R");
  board_get_moves_all(board, &moves, &nmoves, GetMovesWhite | GetMovesBlack);

  ck_assert_int_eq(nmoves, 40);
}
END_TEST

START_TEST(test_checkmate)
{
  Board board;
  Move* moves;
  size_t nmoves = 0;

  board_new_from_string(&board, "r 0 b 0 k b n r"
                                "p p p p 0 p p p"
                                "0 0 n 0 0 0 0 0"
                                "0 0 0 0 p 0 0 0"
                                "0 0 0 0 0 0 P q"
                                "0 0 0 0 0 P 0 0"
                                "P P P P P 0 0 P"
                                "R N B Q K B N R");

  for (int i = 0; i < 64; i++)
  {
    if (!(board.state[i] & ChessPieceIsWhite))
      continue;
    board_get_moves(board, i, &moves, &nmoves, ConsiderChecks);

    fail_if(nmoves != 0, "Found move %s at %d", move_tostring(moves[0]), i);
  }
}
END_TEST

void rook_nmoves(Board board, int defending_rook_pos)
{
  Move* moves;
  size_t nmoves;
  int expected_nmoves = 1;

  board_get_moves(board, defending_rook_pos, &moves, &nmoves, ConsiderChecks);
  ck_assert_int_eq(nmoves, expected_nmoves);
  free(moves);
}
// Does knight correctly put king in check.
START_TEST(test_knight_check_king)
{
  Board board;
  board_new_from_string(&board, "0 n 0 0 0 0 0 R"
                                "0 0 0 0 0 0 0 0"
                                "0 0 K 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0");
  rook_nmoves(board, 7);
}
END_TEST
START_TEST(test_queen_vertical_check_king)
{
  Board board;
  board_new_from_string(&board, "q 0 0 0 0 0 0 R"
                                "0 0 0 0 0 0 0 0"
                                "0 0 K 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0");
  rook_nmoves(board, 7);
}
END_TEST
START_TEST(test_queen_horizontal_check_king)
{
  Board board;
  board_new_from_string(&board, "0 0 q 0 0 0 0 R"
                                "0 0 0 0 0 0 0 0"
                                "0 0 K 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0");
  rook_nmoves(board, 7);
}
END_TEST
START_TEST(test_bishop_check_king)
{
  Board board;
  board_new_from_string(&board, "b 0 0 0 0 0 0 R"
                                "0 0 0 0 0 0 0 0"
                                "0 0 K 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0");
  rook_nmoves(board, 7);
}
END_TEST
START_TEST(test_castle_check_king)
{
  Board board;
  board_new_from_string(&board, "0 0 r 0 0 0 0 R"
                                "0 0 0 0 0 0 0 0"
                                "0 0 K 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0");
  rook_nmoves(board, 7);
}
END_TEST
START_TEST(test_pawn_check_king)
{
  Board board;
  board_new_from_string(&board, "0 0 0 0 0 0 0 0"
                                "0 p 0 0 0 0 0 R"
                                "0 0 K 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0");
  rook_nmoves(board, 15);
}
END_TEST

START_TEST(test_same_move)
{
  Board board;
  Move* moves;
  size_t nmoves = 0;

  board_new_from_string(&board, "0 0 0 0 0 r 0 k"
                                "0 R 0 0 0 0 0 0"
                                "0 0 0 0 0 K P P"
                                "0 P 0 0 0 P N 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0");
  Move tmp = move_new(topos64(0x11), topos64(0x15));
  board_update(&board, &tmp);

  board_get_moves_all(board, &moves, &nmoves, GetMovesBlack | GetMovesWhite);

  for (int i = 0; i < nmoves; i++)
    fail_if(moves[i].from == topos64(0x11));
}
END_TEST

START_TEST(test_parse_fen)
{
  Board board;
  board_new(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b Kkq 41 45 15");

  fail_if(board.can_castle_ks[1] != true);
  fail_if(board.can_castle_qs[1] != false);
  fail_if(board.can_castle_ks[0] != true);
  fail_if(board.can_castle_qs[0] != true);
  fail_if(board.white_to_move != false);
  ck_assert_int_eq(board.en_passant_tile, 41);
  ck_assert_int_eq(board.halfmove_clock, 45);
  ck_assert_int_eq(board.fullmove_count, 15);
}
END_TEST

int main(int argc, char** argv)
{
  Suite* s1 = suite_create("Engine");
  TCase* tc1_1 = tcase_create("Engine");
  SRunner* sr = srunner_create(s1);
  int num_failed;

  tcase_add_test(tc1_1, test_pawn_moves);
  tcase_add_test(tc1_1, test_en_passant);
  tcase_add_test(tc1_1, test_knight_moves);
  tcase_add_test(tc1_1, test_bishop_moves);
  tcase_add_test(tc1_1, test_castle_moves);
  tcase_add_test(tc1_1, test_queen_moves);
  tcase_add_test(tc1_1, test_king_moves);
  tcase_add_test(tc1_1, test_castling);
  tcase_add_test(tc1_1, test_pinned_check);
  tcase_add_test(tc1_1, test_starting_moves);
  tcase_add_test(tc1_1, test_checkmate);
  tcase_add_test(tc1_1, test_same_move);
  tcase_add_test(tc1_1, test_parse_fen);
  tcase_add_test(tc1_1, test_knight_check_king);
  tcase_add_test(tc1_1, test_queen_vertical_check_king);
  tcase_add_test(tc1_1, test_queen_horizontal_check_king);
  tcase_add_test(tc1_1, test_bishop_check_king);
  tcase_add_test(tc1_1, test_castle_check_king);
  tcase_add_test(tc1_1, test_pawn_check_king);
  suite_add_tcase(s1, tc1_1);

  srunner_run_all(sr, CK_ENV);
  num_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return num_failed == 0 ? 0 : 1;
}
