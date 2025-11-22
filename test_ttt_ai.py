WINS = [
	(0, 1, 2), (3, 4, 5), (6, 7, 8),
	(0, 3, 6), (1, 4, 7), (2, 5, 8),
	(0, 4, 8), (2, 4, 6)
]

def opposite(p):
	return "O" if p == "X" else "X"

def available_moves(board):
	return [i for i, cell in enumerate(board) if cell == " "]

def test_ai(initial_player):
	empty_board = 9 * [" "]
	results = {"win": 0, "tie": 0, "lose": 0}
	simulate(empty_board, initial_player, "O", results)
	return results

def simulate(board, current_player, ai_player, results):
	if game_over(board):
		outcome = evaluate(board, ai_player)
		results[outcome] += 1
		return

	if current_player == ai_player:
		move = ai_move(board, ai_player)
		board[move] = ai_player
		simulate(board, opposite(ai_player), ai_player, results)
		board[move] = " "
	else:
		for move in available_moves(board):
			board[move] = current_player
			simulate(board, opposite(current_player), ai_player, results)
			board[move] = " "

def game_over(board):
	for a, b, c in WINS:
		if board[a] != " " and board[a] == board[b] == board[c]:
			return True
	return " " not in board

losing_boards = set()

def evaluate(board, ai_player):
	opponent = opposite(ai_player)
	for a, b, c in WINS:
		if board[a] == board[b] == board[c]:
			if board[a] == ai_player:
				return "win"
			elif board[a] == opponent:
				losing_boards.add(tuple(board))
				return "lose"
	return "tie"

# ------------------------- Minimax AI Implementation -------------------------

def heuristic(board, player):
	if game_over(board):
		outcome = evaluate(board, player)
		if outcome == "win":
			return 1
		elif outcome == "lose":
			return -1
		else:
			return 0

def minimax(player, board, is_max):
	if game_over(board):
		return heuristic(board, player)

	min_max = -1 if is_max else 1
	current_turn = player if is_max else opposite(player)

	for i in range(9):
		if board[i] == " ":
			board[i] = current_turn
			value = minimax(player, board, not is_max)
			if (is_max and value > min_max) or (not is_max and value < min_max):
				min_max = value
			board[i] = " "

	return min_max

def ai_move_minimax(board, ai_player):
	max_value = -1
	best_move = None

	for i in range(9):
		if board[i] == " ":
			board[i] = ai_player
			value = minimax(ai_player, board, False)
			if value > max_value:
				max_value = value
				best_move = i
			board[i] = " "
	return best_move

# -----------------------------------------------------------------------------
def place_opposite_corner(board, human_player):
	# Place on opposite corner from opponent
	if board[8] == human_player and board[0] == " ":
		return 0
	if board[6] == human_player and board[2] == " ":
		return 2
	if board[2] == human_player and board[6] == " ":
		return 6
	if board[0] == human_player and board[8] == " ":
		return 8
	return None

def place_adjacent_corner(board, human_player):
	# Place on adjacent corner if opponent has two inner diagonals
	if board[1] == human_player and board[3] == human_player and board[0] == " ":
		return 0
	if board[1] == human_player and board[5] == human_player and board[2] == " ":
		return 2
	if board[3] == human_player and board[7] == human_player and board[6] == " ":
		return 6
	if board[5] == human_player and board[7] == human_player and board[8] == " ":
		return 8
	return None

def place_side(board, human_player):
	# Place on any side if opponent has two opposite corners
	if board[0] == human_player and board[8] == human_player:
		for move in [1, 3, 5, 7]:
			if board[move] == " ":
				return move
	if board[2] == human_player and board[6] == human_player:
		for move in [1, 3, 5, 7]:
			if board[move] == " ":
				return move
	return None

def ai_move(board, ai_player):
	human_player = opposite(ai_player)

	# Check for immediate win or block
	for a, b, c in WINS:
		if board[b] == board[c] != " " and board[a] == " ":
			return a
		if board[a] == board[c] != " " and board[b] == " ":
			return b
		if board[a] == board[b] != " " and board[c] == " ":
			return c

	# If AI has center
	if board[4] == ai_player:
		move = place_opposite_corner(board, human_player)
		if move is not None:
			return move
		move = place_adjacent_corner(board, human_player)
		if move is not None:
			return move
		move = place_side(board, human_player)
		if move is not None:
			return move

	# Prioritize playing on center / corner / side
	for move in [4, 0, 2, 6, 8, 1, 3, 5, 7]:
		if board[move] == " ":
			return move

# -----------------------------------------------------------------------------

def print_board(board):
	for i in range(3):
		print("", " | ".join(board[3*i:3*i+3]))
		if i < 2:
			print("---+---+---")
	print()

def main():
	results_X = test_ai("X")
	results_O = test_ai("O")

	total_games = sum(results_X.values()) + sum(results_O.values())
	print("Player starts:\t", results_X)
	print("AI starts:\t", results_O)
	print("Total games:\t", total_games)
	print("Win rate:", '{:.2f}'.format((results_X["win"] + results_O["win"]) / total_games * 100), "%")

	for i, board in enumerate(losing_boards):
		print(f"Losing Board {i+1}:")
		print_board(board)

if __name__ == "__main__":
	main()
