#include <rgl/logging.h>

#include "chess/defs.h"
#include "chess/search.h"
#include "chess/tree.h"
#include <chess/board.h>
#include <chess/move.h>
#include <chess/util.h>

#include <check.h>
#include <stdlib.h>

START_TEST(test_pawn_moves)
{
  Board board;

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
  Array moves = board_get_moves(board, topos64(0x10), 0);

  int expected_nmoves = 3;
  u64 nmoves = moves.used;
  ck_assert_int_eq(nmoves, expected_nmoves);

  bool found_moves[expected_nmoves];
  for (int i = 0; i < expected_nmoves; i++)
    found_moves[i] = false;

  int idx = 0;
  for (int i = 0; i < nmoves; i++)
  {
    if (array_get_as(&moves, i, Move).from == topos64(0x10) &&
        array_get_as(&moves, i, Move).to == topos64(0x20))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x10) &&
        array_get_as(&moves, i, Move).to == topos64(0x30))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x10) &&
        array_get_as(&moves, i, Move).to == topos64(0x21))
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
  Array moves;

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

  moves = board_get_moves(board, topos64(0x33), 0);

  int expected_nmoves = 2;
  ck_assert_int_eq(moves.used, expected_nmoves);

  bool found_moves[expected_nmoves];
  for (int i = 0; i < expected_nmoves; i++)
    found_moves[i] = false;

  int idx = 0;
  for (int i = 0; i < moves.used; i++)
  {
    if (!array_index_is_allocated(&moves, i))
      continue;
    if (array_get_as(&moves, i, Move).from == topos64(0x33) &&
        array_get_as(&moves, i, Move).to == topos64(0x23))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x33) &&
        array_get_as(&moves, i, Move).to == topos64(0x24))
      found_moves[idx++] = true;
  }

  for (int i = 0; i < expected_nmoves; i++)
    fail_if(!found_moves[i]);
}
END_TEST

START_TEST(test_knight_moves)
{
  Board board;
  Array moves;

  board_new_from_string(&board, "0 0 0 0 0 0 0 0"
                                "n 0 0 0 0 0 0 0"
                                "0 0 P 0 0 0 P 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0");

  moves = board_get_moves(board, topos64(0x10), 0);

  int expected_nmoves = 3;
  ck_assert_int_eq(moves.used, expected_nmoves);

  bool found_moves[expected_nmoves];
  for (int i = 0; i < expected_nmoves; i++)
    found_moves[i] = false;

  int idx = 0;
  for (int i = 0; i < moves.capacity; i++)
  {
    if (!array_index_is_allocated(&moves, i))
      continue;
    if (array_get_as(&moves, i, Move).from == topos64(0x10) &&
        array_get_as(&moves, i, Move).to == topos64(0x02))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x10) &&
        array_get_as(&moves, i, Move).to == topos64(0x22))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x10) &&
        array_get_as(&moves, i, Move).to == topos64(0x31))
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
  Array moves;

  board_new_from_string(&board, "0 0 0 0 0 0 0 0"
                                "P r 0 0 0 0 0 0"
                                "P 0 0 0 0 0 0 0"
                                "0 p 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 P 0 0 0 0 0 0");

  moves = board_get_moves(board, topos64(0x11), 0);

  int expected_nmoves = 9;
  ck_assert_int_eq(moves.used, expected_nmoves);

  bool found_moves[expected_nmoves];
  for (int i = 0; i < expected_nmoves; i++)
    found_moves[i] = false;

  int idx = 0;
  for (int i = 0; i < moves.capacity; i++)
  {
    if (!array_index_is_allocated(&moves, i))
      continue;
    if (array_get_as(&moves, i, Move).from == topos64(0x11) &&
        array_get_as(&moves, i, Move).to == topos64(0x01))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x11) &&
        array_get_as(&moves, i, Move).to == topos64(0x10))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x11) &&
        array_get_as(&moves, i, Move).to == topos64(0x12))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x11) &&
        array_get_as(&moves, i, Move).to == topos64(0x13))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x11) &&
        array_get_as(&moves, i, Move).to == topos64(0x14))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x11) &&
        array_get_as(&moves, i, Move).to == topos64(0x15))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x11) &&
        array_get_as(&moves, i, Move).to == topos64(0x16))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x11) &&
        array_get_as(&moves, i, Move).to == topos64(0x17))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x11) &&
        array_get_as(&moves, i, Move).to == topos64(0x21))
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
  Array moves;

  board_new_from_string(&board, "0 0 0 0 0 0 0 0"
                                "0 b 0 0 0 0 0 0"
                                "P 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 p 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0");

  moves = board_get_moves(board, topos64(0x11), 0);

  int expected_nmoves = 5;
  ck_assert_int_eq(moves.used, expected_nmoves);

  bool found_moves[expected_nmoves];
  for (int i = 0; i < expected_nmoves; i++)
    found_moves[i] = false;

  int idx = 0;
  for (int i = 0; i < moves.capacity; i++)
  {
    if (!array_index_is_allocated(&moves, i))
      continue;
    if (array_get_as(&moves, i, Move).from == topos64(0x11) &&
        array_get_as(&moves, i, Move).to == topos64(0x00))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x11) &&
        array_get_as(&moves, i, Move).to == topos64(0x02))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x11) &&
        array_get_as(&moves, i, Move).to == topos64(0x20))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x11) &&
        array_get_as(&moves, i, Move).to == topos64(0x22))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x11) &&
        array_get_as(&moves, i, Move).to == topos64(0x33))
      found_moves[idx++] = true;
  }

  for (int i = 0; i < expected_nmoves; i++)
    fail_if(!found_moves[i]);
}
END_TEST

START_TEST(test_queen_moves)
{
  Board board;
  Array moves;

  board_new_from_string(&board, "0 0 0 0 0 0 0 0"
                                "0 q 0 0 0 p 0 P"
                                "P 0 0 0 0 0 0 0"
                                "0 p 0 0 0 0 0 0"
                                "0 0 0 0 p 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 P 0 0 0 0 0 0");

  moves = board_get_moves(board, topos64(0x11), 0);

  int expected_nmoves = 11;
  ck_assert_int_eq(moves.used, expected_nmoves);

  bool found_moves[expected_nmoves];
  for (int i = 0; i < expected_nmoves; i++)
    found_moves[i] = false;

  int idx = 0;
  for (int i = 0; i < moves.capacity; i++)
  {
    if (!array_index_is_allocated(&moves, i))
      continue;
    if (array_get_as(&moves, i, Move).from == topos64(0x11) &&
        array_get_as(&moves, i, Move).to == topos64(0x00))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x11) &&
        array_get_as(&moves, i, Move).to == topos64(0x01))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x11) &&
        array_get_as(&moves, i, Move).to == topos64(0x02))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x11) &&
        array_get_as(&moves, i, Move).to == topos64(0x10))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x11) &&
        array_get_as(&moves, i, Move).to == topos64(0x12))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x11) &&
        array_get_as(&moves, i, Move).to == topos64(0x13))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x11) &&
        array_get_as(&moves, i, Move).to == topos64(0x14))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x11) &&
        array_get_as(&moves, i, Move).to == topos64(0x20))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x11) &&
        array_get_as(&moves, i, Move).to == topos64(0x21))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x11) &&
        array_get_as(&moves, i, Move).to == topos64(0x22))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x11) &&
        array_get_as(&moves, i, Move).to == topos64(0x33))
      found_moves[idx++] = true;
  }

  for (int i = 0; i < expected_nmoves; i++)
    fail_if(!found_moves[i]);
}
END_TEST

START_TEST(test_king_moves)
{
  Board board;
  Array moves;

  // First just do a simple test for the king moving 1 square
  board_new_from_string(&board, "0 0 k 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 P 0 0 0 0 0 0");

  moves = board_get_moves(board, topos64(0x02), 0);

  int expected_nmoves = 5;
  ck_assert_int_eq(moves.used, expected_nmoves);

  bool found_moves[expected_nmoves];
  for (int i = 0; i < expected_nmoves; i++)
    found_moves[i] = false;

  int idx = 0;
  for (int i = 0; i < moves.used; i++)
  {
    if (array_get_as(&moves, i, Move).from == topos64(0x02) &&
        array_get_as(&moves, i, Move).to == topos64(0x01))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x02) &&
        array_get_as(&moves, i, Move).to == topos64(0x03))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x02) &&
        array_get_as(&moves, i, Move).to == topos64(0x11))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x02) &&
        array_get_as(&moves, i, Move).to == topos64(0x12))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x02) &&
        array_get_as(&moves, i, Move).to == topos64(0x13))
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
  Array moves;

  board_new_from_string(&board, "r 0 0 0 k 0 0 r"
                                "0 0 0 p p p 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0");

  moves = board_get_moves(board, topos64(0x04), 0);

  int expected_nmoves = 4;
  ck_assert_int_eq(moves.used, expected_nmoves);

  bool found_moves[expected_nmoves];
  for (int i = 0; i < expected_nmoves; i++)
    found_moves[i] = false;

  int idx = 0;
  for (int i = 0; i < moves.capacity; i++)
  {
    if (!array_index_is_allocated(&moves, i))
      continue;
    if (array_get_as(&moves, i, Move).from == topos64(0x04) &&
        array_get_as(&moves, i, Move).to == topos64(0x03))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x04) &&
        array_get_as(&moves, i, Move).to == topos64(0x05))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x04) &&
        array_get_as(&moves, i, Move).to == topos64(0x06))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x04) &&
        array_get_as(&moves, i, Move).to == topos64(0x02))
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
  moves = board_get_moves(board, topos64(0x04), 0);
  ck_assert_int_eq(moves.used, 2);

  // We can't castle through check
  board_new_from_string(&board, "r 0 0 0 k 0 0 r"
                                "0 0 0 0 p p 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 Q 0 0 0 0");

  moves = board_get_moves(board, topos64(0x04), ConsiderChecks);

  // Should only be able to castle kingside here
  expected_nmoves = 2;
  ck_assert_int_eq(moves.used, expected_nmoves);

  idx = 0;
  for (int i = 0; i < moves.capacity; i++)
  {
    if (!array_index_is_allocated(&moves, i))
      continue;
    if (array_get_as(&moves, i, Move).from == topos64(0x04) &&
        array_get_as(&moves, i, Move).to == topos64(0x05))
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).from == topos64(0x04) &&
        array_get_as(&moves, i, Move).to == topos64(0x06))
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

  moves = board_get_moves(board, topos64(0x74), ConsiderChecks);

  ck_assert_int_eq(moves.used, 2);
}
END_TEST

// Pinned pieces should still be able to check the enemy king
START_TEST(test_pinned_check)
{
  Board board;
  Array moves;

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

  moves = board_get_moves(board, topos64(0x44), ConsiderChecks);

  int expected_nmoves = 1;
  ck_assert_int_eq(moves.used, expected_nmoves);
  array_free(&moves);
  expected_nmoves = 4;
  moves = board_get_moves(board, topos64(0x71), ConsiderChecks);
  ck_assert_int_eq(moves.used, expected_nmoves);
}
END_TEST

START_TEST(test_starting_moves)
{
  Board board;

  board_new_from_string(&board, "r n b q k b n r"
                                "p p p p p p p p"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "P P P P P P P P"
                                "R N B Q K B N R");
  Array moves = board_get_moves_all(board, GetMovesWhite | GetMovesBlack);

  ck_assert_int_eq(moves.used, 40);
}
END_TEST

START_TEST(test_checkmate)
{
  Board board;

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
    Array moves = board_get_moves(board, i, ConsiderChecks);

    fail_if(moves.used != 0, "Found move %s at %d",
            move_tostring(array_get_as(&moves, 0, Move)), i);
  }
}
END_TEST

void rook_nmoves(Board board, int defending_rook_pos)
{
  int expected_nmoves = 1;

  Array moves = board_get_moves(board, defending_rook_pos, ConsiderChecks);
  ck_assert_int_eq(moves.used, expected_nmoves);
  array_free(&moves);
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

  Array moves = board_get_moves_all(board, GetMovesBlack | GetMovesWhite);

  for (int i = 0; i < moves.capacity; i++)
  {
    if (!array_index_is_allocated(&moves, i))
      continue;
    fail_if(array_get_as(&moves, i, Move).from == topos64(0x11));
  }
}
END_TEST

START_TEST(test_parse_fen)
{
  Board board;
  board_new(&board,
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b Kkq 41 45 15");

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

START_TEST(test_promotion)
{
  Board board;
  int expected_nmoves;
  board_new_from_string(&board, "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 k 0 0 0 0 0 0"
                                "0 n n K 0 0 0 0"
                                "0 0 0 p p 0 0 0"
                                "0 0 R 0 0 0 0 0");
  Array moves = board_get_moves(board, topos64(0x63), ConsiderChecks);
  expected_nmoves = 8; // 4 promotions for each move
  ck_assert_int_eq(moves.used, expected_nmoves);

  int idx = 0;
  bool* found_moves = calloc(expected_nmoves, sizeof(bool));
  for (int i = 0; i < moves.capacity; i++)
  {
    if (!array_index_is_allocated(&moves, i))
      continue;
    if (array_get_as(&moves, i, Move).to == topos64(0x72) &&
        array_get_as(&moves, i, Move).promotion == ChessPieceKnight)
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).to == topos64(0x72) &&
        array_get_as(&moves, i, Move).promotion == ChessPieceCastle)
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).to == topos64(0x72) &&
        array_get_as(&moves, i, Move).promotion == ChessPieceBishop)
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).to == topos64(0x72) &&
        array_get_as(&moves, i, Move).promotion == ChessPieceQueen)
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).to == topos64(0x73) &&
        array_get_as(&moves, i, Move).promotion == ChessPieceKnight)
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).to == topos64(0x73) &&
        array_get_as(&moves, i, Move).promotion == ChessPieceCastle)
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).to == topos64(0x73) &&
        array_get_as(&moves, i, Move).promotion == ChessPieceBishop)
      found_moves[idx++] = true;
    if (array_get_as(&moves, i, Move).to == topos64(0x73) &&
        array_get_as(&moves, i, Move).promotion == ChessPieceQueen)
      found_moves[idx++] = true;
  }

  for (int i = 0; i < expected_nmoves; i++)
    fail_if(!found_moves[i], "!found_moves[%d]", i);
}

START_TEST(test_node_copy)
{
  Node* root = node_new(NULL, move_new(-1, -1), false);
  for (int i = 0; i < 20; i++)
  {
    Node* node = node_new(root, move_new(rand() % 8, rand() % 8), true);
    node->value = rand();
    for (int leaf = 0; leaf < 10; leaf++)
    {
      Node* node_leaf = node_new(node, move_new(rand() % 8, rand() % 8), true);
      node->value = rand();
    }
  }

  Node* copy = node_copy(*root);
  size_t orig_length;
  size_t copy_length;
  Node** nodes_orig = tree_traverse(root, tree_get_all_condition, &orig_length);
  Node** nodes_copy = tree_traverse(copy, tree_get_all_condition, &copy_length);

  fail_if(orig_length != copy_length);
  for (int i = 0; i < orig_length; i++)
  {
    fail_if(nodes_orig[i]->value != nodes_copy[i]->value);
    fail_if(nodes_orig[i]->nchilds != nodes_copy[i]->nchilds);
    fail_if(!move_equals(nodes_orig[i]->move, nodes_copy[i]->move));
  }
}
END_TEST

START_TEST(test_can_force_mate)
{
  Board board;
  board_new(&board, "8/8/8/8/7k/8/6qr/K7 w KQkq - 0 1");

  Node* node = node_new(NULL, move_new(57, 56), false);
  Tree* tree = tree_new(node, board, 3);
  Move move = search(tree);
  board_update(&board, &move);

  Array moves = board_get_moves(board, 57, ConsiderChecks);
  if (moves.used == 0 && is_in_check(board, true))
    return;

  move = move_new(56, 57);
  board_update(&board, &move);

  tree_free(&tree);
  node = node_new(NULL, move_new(56, 57), false);
  tree = tree_new(node, board, 3);
  move = search(tree);
  board_update(&board, &move);

  array_free(&moves);

  moves = board_get_moves(board, 57, ConsiderChecks);
  fail_unless(moves.used == 0 && is_in_check(board, true));
  tree_free(&tree);
  array_free(&moves);
}
END_TEST

int main(int argc, char** argv)
{
  t_debug_level_set(DebugLevelError);

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
  tcase_add_test(tc1_1, test_promotion);
  tcase_add_test(tc1_1, test_node_copy);
  tcase_add_test(tc1_1, test_can_force_mate);

  suite_add_tcase(s1, tc1_1);

  srunner_run_all(sr, CK_ENV);
  num_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return num_failed == 0 ? 0 : 1;
}
