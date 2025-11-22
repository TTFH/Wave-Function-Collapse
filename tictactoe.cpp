#include <vector>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>

#include "tictactoe.h"

#define RED    "\x1b[91m"
#define GREEN  "\x1b[92m"
#define YELLOW "\x1b[93m"
#define BLUE   "\x1b[94m"
#define NORMAL "\x1b[0m"

using namespace std;

Tile::Tile() : value(EMPTY) {}

Tile::Tile(Value v) : value(v) {}

Tile::Tile(char t) {
	switch (t) {
		case 'X': value = X; break;
		case 'O': value = O; break;
		case ' ': value = EMPTY; break;
		default: throw invalid_argument("Invalid tile character");
	}
}

char Tile::getChar() const {
	switch (value) {
		case EMPTY: return ' ';
		case X: return 'X';
		case O: return 'O';
		default: throw logic_error("Invalid tile value");
	}
}

BoardStatus Tile::toStatus() const {
	switch (value) {
		case X: return BoardStatus::X_WINS;
		case O: return BoardStatus::O_WINS;
		default: throw logic_error("Invalid conversion from tile to status");
	}
}

bool Tile::operator==(const Tile& other) const {
	return value == other.value;
}

bool Tile::operator!=(const Tile& other) const {
	return value != other.value;
}

Turn::Turn(Value v) : value(v) {}

Turn::Turn(char t) {
	switch (t) {
		case 'X': value = X; break;
		case 'O': value = O; break;
		default: throw invalid_argument("Invalid turn character");
	}
}

Turn Turn::switchTurn() const {
	return value == X ? O : X;
}

Tile Turn::getTile() const {
	return value == X ? Tile::X : Tile::O;
}

char Turn::getChar() const {
	return value == X ? 'X' : 'O';
}

bool Turn::operator==(const Turn& other) const {
	return value == other.value;
}

static bool equals(Tile a, Tile b, Tile c) {
	return a != Tile::EMPTY && a == b && b == c;
}

void Board::print() const {
	printf("\n");
	printf(" %c | %c | %c", cells[6].getChar(), cells[7].getChar(), cells[8].getChar());
	printf("\n---+---+---\n");
	printf(" %c | %c | %c", cells[3].getChar(), cells[4].getChar(), cells[5].getChar());
	printf("\n---+---+---\n");
	printf(" %c | %c | %c", cells[0].getChar(), cells[1].getChar(), cells[2].getChar());
	printf("\n\n");
}

BoardStatus Board::getStatus() const {
	if (equals(cells[6], cells[7], cells[8])) return cells[6].toStatus();
	if (equals(cells[3], cells[4], cells[5])) return cells[3].toStatus();
	if (equals(cells[0], cells[1], cells[2])) return cells[0].toStatus();

	if (equals(cells[0], cells[3], cells[6])) return cells[0].toStatus();
	if (equals(cells[1], cells[4], cells[7])) return cells[1].toStatus();
	if (equals(cells[2], cells[5], cells[8])) return cells[2].toStatus();

	if (equals(cells[0], cells[4], cells[8])) return cells[4].toStatus();
	if (equals(cells[2], cells[4], cells[6])) return cells[4].toStatus();

	for (int i = 0; i < 9; i++)
		if (cells[i] == Tile::EMPTY)
			return BoardStatus::ONGOING;
	return BoardStatus::TIE;
}

Tile& Board::operator[](int index) {
	if (index < 0 || index >= 9)
		throw out_of_range("Board index out of range");
	return cells[index];
}

const Tile& Board::operator[](int index) const {
	if (index < 0 || index >= 9)
		throw out_of_range("Board index out of range");
	return cells[index];
}

void TicTacToe::handlePlayerMove() {
	printf("\nIt's %c's turn, enter cell: ", turn.getChar());
	int cell = 0;
	scanf("%d", &cell);
	while (cell < 1 || cell > 9 || board[cell - 1] != Tile::EMPTY) {
		printf("Invalid, re-enter cell: ");
		scanf("%d", &cell);
	}
	board[cell - 1] = turn.getTile();
}

TicTacToe::TicTacToe(bool is_player_x_ai, bool is_player_o_ai)
	: turn(Turn::X), is_player_x_ai(is_player_x_ai), is_player_o_ai(is_player_o_ai) {}

void TicTacToe::playTurn() {
	if (turn == Turn::X && is_player_x_ai)
		playAIMoveX();
	else if (turn == Turn::O && is_player_o_ai)
		playAIMoveO();
	else
		handlePlayerMove();
	turn = turn.switchTurn();
}

void TicTacToe::printBoard() const {
	board.print();
}

BoardStatus TicTacToe::getStatus() const {
	return board.getStatus();
}

void TicTacToe::showWinner() const {
	BoardStatus status = board.getStatus();
	if (status == BoardStatus::X_WINS)
		printf("X Wins\n");
	else if (status == BoardStatus::O_WINS)
		printf("O Wins\n");
	else
		printf("It's a tie\n");
}

static int heuristic(Turn turn, const Board& board) {
	BoardStatus status = board.getStatus();
	if (status == BoardStatus::TIE) return 0;
	if (status == BoardStatus::ONGOING) return 0;
	//if (turn == Turn::X && status == BoardStatus::X_WINS) return 1;
	//if (turn == Turn::O && status == BoardStatus::O_WINS) return 1;
	if (turn.getTile().toStatus() == status) return 1;
	return -1;
}

static int minimax(Turn turn, Board& board, bool is_max) {
	if (board.getStatus() != BoardStatus::ONGOING)
		return heuristic(turn, board);

	int min_max = is_max ? -1 : 1;
	Turn current_turn = is_max ? turn : turn.switchTurn();
	for (int i = 0; i < 9; i++) {
		if (board[i] == Tile::EMPTY) {
			board[i] = current_turn.getTile();
			int value = minimax(turn, board, !is_max);
			if ((is_max && value > min_max) || (!is_max && value < min_max))
				min_max = value;
			board[i] = Tile::EMPTY;
		}
	}
	return min_max;
}

TTT_MM_MM::TTT_MM_MM(bool is_player_x_ai, bool is_player_o_ai)
	: TicTacToe(is_player_x_ai, is_player_o_ai) {}

void TTT_MM_MM::playAIMoveMinimax() {
	int max = -1;
	vector<int> best_moves;
	for (int i = 0; i < 9; i++) {
		if (board[i] == Tile::EMPTY) {
			board[i] = turn.getTile();
			int value = minimax(turn, board, false);
			if (value > max) {
				max = value;
				best_moves.clear();
			}
			if (value >= max)
				best_moves.push_back(i);
			board[i] = Tile::EMPTY;
		}
	}
	int count = best_moves.size();
	if (count == 0)
		throw logic_error("AI found no valid moves");
	//printf("Best moves count: %d\n", count);
	int best_move = best_moves[rand() % count];
	board[best_move] = turn.getTile();
}

void TTT_MM_MM::playAIMoveX() {
	playAIMoveMinimax();
}

void TTT_MM_MM::playAIMoveO() {
	playAIMoveMinimax();
}

static void showTitle() {
	printf(GREEN "  mm   " RED "      db      " GREEN "mm            mm   " RED "`7MMF'\n");
	printf(GREEN "  MM   " RED "     ;MM:     " GREEN "MM            MM   " RED "  MM\n");
	printf(GREEN "mmMMmm " RED "    ,V^MM.  " GREEN "mmMMmm .gP\"Ya mmMMmm" RED "   MM\n");
	printf(GREEN "  MM   " RED "   ,M  `MM    " GREEN "MM  ,M'   Yb  MM   " RED "  MM\n");
	printf(GREEN "  MM   " RED "   AbmmmqMA   " GREEN "MM  8M\"\"\"\"\"\"  MM" RED "     MM\n");
	printf(GREEN "  MM   " RED "  A'     VML  " GREEN "MM  YM.    ,  MM   " RED "  MM\n");
	printf(GREEN "  `Mbmo" RED ".AMA.   .AMMA." GREEN "`Mbmo`Mbmmd'  `Mbmo" RED ".JMML.\n");
	printf(NORMAL);
}

static void showMenu() {
	printf("\n\t-------- GAME MODES --------\n");
	printf("\t1) Human       - Human\n");
	printf("\t2) Human   (X) - Machine (O)\n");
	printf("\t3) Machine (X) - Human   (O)\n");
	printf("\t4) Machine     - Machine\n");
	printf("\t0) Exit\n");
	printf("\n> ");
}

int main() {
	srand(time(NULL));
	showTitle();
	
	while (true) {
		int option;
		showMenu();
		scanf("%d", &option);
		bool is_player_x_ai, is_player_o_ai;
		switch (option) {
			case 1: is_player_x_ai = false; is_player_o_ai = false; break;
			case 2: is_player_x_ai = false; is_player_o_ai = true; break;
			case 3: is_player_x_ai = true; is_player_o_ai = false; break;
			case 4: is_player_x_ai = true; is_player_o_ai = true; break;
			default: exit(EXIT_SUCCESS); break;
		}

		TicTacToe* game = new TTT_MM_MM(is_player_x_ai, is_player_o_ai);
		do {
			game->playTurn();
			game->printBoard();
		} while (game->getStatus() == BoardStatus::ONGOING);
		game->showWinner();
		delete game;
	}

	return 0;
}
