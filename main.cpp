#include <iostream>
#include <cstdlib>
#include <ctime>
using namespace std;

/**
 * Enumerate the possible players in the game. Each is given
 * a unique prime integer so that the math involved with determining
 * available winning moves on the board is made easier.
 *
 * See the `ai_*` functions and the `drawBoard` for relevant logic
 */
enum {EMPTY=0, USER=7, COMPUTER=11};

/**
 * Enumerate possible states the game can be in. The game is either
 *  IN_PROGRESS  - neither player has won, and there are still valid moves
 *  USER_WON     - user won by matching three in a row
 *  COMPUTER_WON - computer won by matching three in a row
 *  DRAW         - neither player has won, but there are no valid moves left
 *
 * See the function `isGameOver` for related logic
 */
enum {IN_PROGRESS=0, USER_WON=USER, COMPUTER_WON=COMPUTER, DRAW};

/**
 * Enumerate possible strategies the computer might employ to try to
 * win the game. Available strategies are:
 *   0 RANDOM       - Randomly pick one of the available cells
 *   1 SMART        - Prefer strategic locations if available
 *   2 GENIOUS      - Defend and attack in all situations
 *
 * GENIOUS is the default strategy used if no other is requested. See the
 * function `nextComputerMove` for relevant logic
 */
enum {RANDOM, SMART, GENIOUS};

/**
 * Container to represent a single cell on the board. This makes
 * it possible to use a board location as the return value of a
 * function ex: `playerCanWin`.
 */
struct Cell {
  Cell(int r, int c): row(r), col(c) {};
  int row;
  int col;
};

/**
 * Draw a 3x3 tic-tac-toe board with row and column labels
 * The cell data in board will be interpreted as follows:
 *   1 = owned by 'x'
 *   2 = owned by 'o'
 *   all other values = empty
 *
 * @param  int[3][3] board The current state of each board cell
 * @return void
 */
void drawBoard(int board[][3]) {
  cout << "  " << "  A   B   C  " << endl;
  cout << "  " << "+---+---+---+" << endl;
  for (int row = 0; row < 3; row++) {
    cout << row << " ";
    for (int col = 0; col < 3; col++) {
      switch (board[row][col]) {
        case USER:     cout << "| " << 'x' << " "; break;
        case COMPUTER: cout << "| " << 'o' << " "; break;
        default:       cout << "| " << ' ' << " "; break;
      }
    }
    cout << "|" << endl;
    cout << " " << " " << "+---+---+---+" << endl;
  }
}

/**
 * Get the next move from the player, and make sure that the
 * desired move is valid. Validity means:
 *   - column value is within bounds [A, B, C]
 *   - row value is within bounds    [0, 1, 2]
 *   - the selected cell is empty    board[row][col] == 0
 * Once input has been validated, update the board.
 *
 * @param  int[3][3] board  The current state of the board
 * @return void
 */
void nextPlayerMove(int board[][3]) {
  bool row_valid;
  bool col_valid;
  char col_c;             // container for the character value of the column
  int  col;               // container for the integer value of the column
  int  row;               // container for the integer value of the row

  cout << "Your turn. Where would you like to move next?" << endl;
  cout << "Type your move as two characters separated by a space (ex: A 1)" << endl;

  do {
    col_valid = true;  // assume correct input until proven wrong
    row_valid = true;  // ...

    // Obtain input from the user (one character for column, one int for row)
    cin >> col_c >> row;

    // Validate the provided column value
    if      (col_c == 'a' || col_c == 'A') { col = 0; }
    else if (col_c == 'b' || col_c == 'B') { col = 1; }
    else if (col_c == 'c' || col_c == 'C') { col = 2; }
    else {
      cout << "! Invalid column value entered. Your choices are: [A, B, C] " << endl;
      col_valid = false;
    }

    // Validate the provided row value
    if (row < 0 || row > 2) {
      cout << "! Invalid row value entered. Your choices are: [0, 1, 2] " << endl;
      row_valid = false;
    }

    // Ensure that the choice corresponds to an empty cell
    if (row_valid && col_valid && board[row][col] != 0) {
      cout << "! That cell is not empty. Please try a different cell " << endl;
      col_valid = false;
      row_valid = false;
    }
  } while (!col_valid || !row_valid);

  // Update the board with the user's latest choice
  board[row][col] = USER;
}

/**
 * AI strategy based on randomly picking available cells
 *
 * @param  int[3][3]  board  The current state of the board
 * @return void
 **/
void ai_random(int board[][3]) {
  int row, col;
  do {
    row = rand() % 3;  // Choose a random row
    col = rand() % 3;  // Choose a random column
  } while (board[row][col] != EMPTY); // If taken, try again

  // Update the board state
  board[row][col] = COMPUTER;
}

/**
 * AI strategy based on preferring strategic cells if they
 * are available. Defaults to randomly picking a cell if they
 * are not.
 *
 * @param  int[3][3] board  The current state of the board
 * @return void
 */
void ai_smart(int board[][3]) {
  int row, col;

  // Prefer B1 if it is available
  if (board[1][1] == EMPTY) { row = 1; col = 1; }
  else {
    // Prefer corners if they are available
    if (board[0][0] == EMPTY)      { row = 0; col = 0; }
    else if (board[0][2] == EMPTY) { row = 0; col = 2; }
    else if (board[2][0] == EMPTY) { row = 2; col = 0; }
    else if (board[2][2] == EMPTY) { row = 2; col = 2; }
    else {
      // Resort to random available location
      ai_random(board);
      row = -1;
      col = -1; // set to prevent later logic
    }
  }

  if (row >= 0 && col >= 0) {
    board[row][col] = COMPUTER;
  }
}

/**
 * Determine whether a user could win by claiming
 * one more cell along any of the possible winning
 * axes.
 * There are 8 possible winning axes. The user can
 * win if the sum of the cells on that axis is currently
 * 2 and if one of the cells is empty (=0).
 *
 * @param  int[3][3] board  The current state of the board
 * @param  int       which  Which player (USER or COMPUTER)
 * @return cell  The location of the cell the user could win with
 */
Cell playerCanWin(int board[][3], int which) {
  // Target sum depends on which player we're calculating for
  int target = which * 2;
  // Iterate through the 8 possible winning axes:
  // Rows:
  for (int r = 0; r < 3; r++) {
    int sum  = board[r][0] + board[r][1] + board[r][2];
    if (sum == target ) {
      for (int c = 0; c < 3; c++) {
        if (board[r][c] == EMPTY) { return Cell(r,c); }
      }
    }
  }

  // Columns:
  for (int c = 0; c < 3; c++) {
    int sum  = board[0][c] + board[1][c] + board[2][c];
    if (sum == target) {
      for (int r = 0; r < 3; r++) {
        if (board[r][c] == EMPTY) { return Cell(r,c); }
      }
    }
  }

  // Diagonals:
  int sum_ltr = board[0][0] + board[1][1] + board[2][2];
  if (sum_ltr == target) {
    if (board[0][0] == EMPTY) { return Cell(0,0); }
    if (board[1][1] == EMPTY) { return Cell(1,1); }
    if (board[2][2] == EMPTY) { return Cell(2,2); }
  }
  int sum_rtl = board[0][2] + board[1][1] + board[2][0];
  if (sum_rtl == target) {
    if (board[0][2] == EMPTY) { return Cell(0,2); }
    if (board[1][1] == EMPTY) { return Cell(1,1); }
    if (board[2][0] == EMPTY) { return Cell(2,0); }
  }
  // No possible win
  return Cell(-1,-1);
}

Cell userCanWin(int board[][3]) {
  return playerCanWin(board, USER);
}

Cell computerCanWin(int board[][3]) {
  return playerCanWin(board, COMPUTER);
}

void ai_genious(int board[][3]) {

  // Prefer B1 if it is available
  if (board[1][1] == EMPTY) {
    board[1][1] = COMPUTER;
  } else {
    // Determine if there's any way for the computer
    // to win on this turn
    Cell c = computerCanWin(board);

    // If the computer can win, then make it happen:
    if (c.row >= 0 && c.col >= 0) {
      board[c.row][c.col] = COMPUTER; // computer is always 'o'
    } else {
      // Otherwise, determine whether there's any way for
      // the user to win on their next turn
      Cell c = userCanWin(board);

      // If the user can win on the next turn, attempt to
      // block that action now:
      if (c.row >= 0 && c.col >= 0) {
        board[c.row][c.col] = COMPUTER; // computer is always 'o'
      } else {
        // Otherwise, try to pick a strategic location
        ai_smart(board);
      }
    }
  }
}


/**
 * Determine the next move the computer should make. For now
 * the strategy will be simple: randomly pick an available
 * cell.
 * Once a cell has been picked, update the board
 *
 * @param  int[3][3] board   The current state of the board
 * @param  int    strategy   The strategy to use
 * @return void
 */
void nextComputerMove(int board[][3], int strategy) {
  switch (strategy) {
    case SMART:
      ai_smart(board);
      break;
    case GENIOUS:
      ai_genious(board);
      break;
    case RANDOM:
    default:
      ai_random(board);
      break;
  }
}

/**
 * Determine the status of the game given the current state
 * of the board. The status can be one of 4 values:
 *   0 - valid: no one has won yet, and there are valid moves remaining
 *   1 - invalid, user has won
 *   2 - invalid, computer has won
 *   3 - invalid, draw - no one has won but there are no valid moves remaining
 *
 * @param  int[3][3] board   The current state of the board
 * @return int               The status of the board in its current state
 */
int isGameOver (int board[][3]) {

  // Strategy: examine the "middle" square for each axis. There are only a
  //           limited number of these squares on the board. Specifically:
  //     A   B   C
  //   +---+---+---+
  // 0 |   | x |   |
  //   +---+---+---+
  // 1 | x | x | x |
  //   +---+---+---+
  // 2 |   | x |   |
  //   +---+---+---+
  // If a given user owns one of these middle squares, and also owns two
  // neighbors along the same axis, then that user has won. Each "middle
  // square" has only one axis to check, except B1, which has four axes.
  // In other words you have won if you own:
  //   B0  (and A0 and C0)
  //   A1  (and A0 and A2)
  //   B2  (and A2 and C2)
  //   C1  (and C0 and C2)
  //   B1  (and B0 and B2), (and A1 and C1), (and A0 and C2), (and A2 and C0)

  // Check each player sequentially:
  for (int player = USER; player <= COMPUTER; player += (COMPUTER - USER)) {

    // Determine if the current player won
    if ((board[0][1] == player && board[0][0] == player && board[0][2] == player)
    	||(board[1][0] == player && board[0][0] == player && board[2][0] == player)
    	||(board[2][1] == player && board[2][0] == player && board[2][2] == player)
    	||(board[1][2] == player && board[0][2] == player && board[2][2] == player)
    	||(board[1][1] == player && (
  	   (board[0][1] == player && board[2][1] == player)
  	   ||(board[1][0] == player && board[1][2] == player)
  	   ||(board[0][0] == player && board[2][2] == player)
  	   ||(board[2][0] == player && board[0][2] == player))
	    )
	  ){
      return player;
    }
  }

  // Determine if there are no further moves available
  bool moreMoves = false;
  for (int col = 0; col < 3; col++) {
    for (int row = 0; row < 3; row++) {
      if (board[row][col] == EMPTY) { moreMoves = true; }
    }
  }

  if (moreMoves) {
    return IN_PROGRESS;// game still valid;
  } else {
    return DRAW;       // game over without a winner
  }
}



int main (int argc, char* argv[])
{
  // State variables
  //
  // Board:
  //     0   1   2
  //   +---+---+---+
  // 0 |   |   | x | x = board[0][2]
  //   +---+---+---+
  // 1 |   |   |   |
  //   +---+---+---+
  // 2 |   |   |   |
  //   +---+---+---+
  //
  // Each board square will be in one of three possible
  // states:  0=empty, 1=owned by 'x', 2=owned by 'o'.
  int board[3][3] = {};

  // Variable to hold the current status of the game. See the
  // enum declaration at the top of this file for the possible
  // values of status.
  int gameStatus = IN_PROGRESS;

  // Flag to determine whose turn it is
  // false: it is the computer's turn to make a move
  // true:  it is the player's turn to make a move
  bool playerTurn = true;


  /* Game Flow */

  // 1. determine who goes first
  playerTurn = rand() % 2; // coin flip

  // 2. Enter the main game "loop"
  while (gameStatus == IN_PROGRESS) {
    // a. draw the board
    drawBoard(board);

    // b. current player makes a move
    //    the logic here depends on whether or not the
    //    computer or the player is the current player
    if (playerTurn) {
      // Some function for asking the user what the
      // next move should be...
      nextPlayerMove(board);

    } else {
      // Some function for determining what the next
      // move should be...
      nextComputerMove(board, GENIOUS);
    }

    // c. Check the current status of the game to determine
    //    if the game can continue...
    gameStatus = isGameOver(board);

    // d. swap current player
    playerTurn = (playerTurn) ? false : true;
  }

  // 3. print final game result message
  cout << "Game over! Here's what the final board looked like:" << endl;
  cout << endl;
  drawBoard(board);
  cout << endl;
  switch (gameStatus) {
    case USER_WON:     cout << "^.^ Congratulations! ^.^ You win! ^.^ "       << endl; break;
    case COMPUTER_WON: cout << "~.~ Sorry! ~.~ You lose! ~.~ "                << endl; break;
    case DRAW:         cout << "O.o Whoa, that was close! O.o You tied! O.o " << endl; break;
    default: cout << "Hmm... something really went wrong here. Please let my programmer know. " << endl;
  }
  cout << endl;
}
