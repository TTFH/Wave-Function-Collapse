#ifndef TICTACTOE_H
#define TICTACTOE_H

/*
const char X = 'X';
const char O = 'O';
const char EMPTY = ' ';
const char ONGOING = '-';
*/

// I may have went a bit overboard with the abstractions here

enum class BoardStatus { ONGOING, X_WINS, O_WINS, TIE };

class Tile {
public:
	enum Value { EMPTY, X, O };
	Tile();
	Tile(Value v);
	Tile(char t);
	char getChar() const;
	BoardStatus toStatus() const;
	bool operator==(const Tile& other) const;
	bool operator!=(const Tile& other) const;
private:
	Value value;
};

class Turn {
public:
	enum Value { X, O };
	Turn(Value v);
	Turn(char t);
	Turn switchTurn() const;
	Tile getTile() const;
	char getChar() const;
	bool operator==(const Turn& other) const;
private:
	Value value;
};

class Board {
private:
	Tile cells[9];
public:
	Board() = default;
	void print() const;
	BoardStatus getStatus() const;
	Tile& operator[](int index);
	const Tile& operator[](int index) const;
};

class TicTacToe {
protected:
	Turn turn;
	Board board;
	bool is_player_x_ai, is_player_o_ai;
	void handlePlayerMove();
	virtual void playAIMoveX() = 0;
	virtual void playAIMoveO() = 0;
public:
	TicTacToe(bool is_player_x_ai, bool is_player_o_ai);
	void playTurn();
	void printBoard() const;
	void showWinner() const;
	BoardStatus getStatus() const;
	virtual ~TicTacToe() = default;
};

class TTT_MM_MM : public TicTacToe {
private:
	void playAIMoveMinimax();
	void playAIMoveX() override;
	void playAIMoveO() override;
public:
	TTT_MM_MM(bool is_player_x_ai, bool is_player_o_ai);
};

class TTT_WFC_MM : public TicTacToe {
private:
	void playAIMoveX() override;
	void playAIMoveO() override;
public:
	TTT_WFC_MM(bool is_player_x_ai, bool is_player_o_ai);
};

#endif
