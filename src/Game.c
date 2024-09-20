#include "Resources.h"
#include <fcntl.h>

/*
 * @brief This function checks if a piece belongs to a white player.
 */
int is_white(const char piece) {
    return piece > 'A' && piece < 'Z';
}

/**
 * @brief Display the current state of chessboard.
 */
void display_chessboard(const ChessGame* game) {
    if (NULL == game)
        return;
    fprintf(stdout, "\nChessboard:\n");
    fprintf(stdout, "  a b c d e f g h\n");
    for (int row = 0; row < 8; ++row) {
        fprintf(stdout, "%d ", 8 - row);
        for (int col = 0; col < 8; ++col) 
            fprintf(stdout, "%c ", game->chessboard[row][col]);
        fprintf(stdout, "%d\n", 8 - row);
    }
    fprintf(stdout, "  a b c d e f g h\n");
}

/**
 * @brief Initialize the chessboard to the starting state.
 * 
 * @param game Chessboard.
 * @return 1 if chessboard initialzed success, 0 if chessboard is NULL.
 */
int initialize_game(ChessGame* game) {
    if (NULL == game) 
        return -1;

    game->moveCount = 0;
    game->capturedCount = 0;
    game->currentPlayer = WHITE_PLAYER; 
    const char* b_row = "rnbqkbnr";
    const char* w_row = "RNBQKBNR";
    for (int col = 0; col < 8; ++col) {
        game->chessboard[0][col] = b_row[col];
        game->chessboard[1][col] = 'p';
        for (int row = 2; row <= 5; ++row) 
            game->chessboard[row][col] = '.';
        game->chessboard[6][col] = 'P';
        game->chessboard[7][col] = w_row[col];
    }
    return 0;
}

/**
 * @brief Storage the state of game in a string array in format of FEN.
 */
void chessboard_to_fen(char fen[], const ChessGame* game) {
    int index = 0;
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            if ('.' != game->chessboard[row][col]) {
                fen[index] = game->chessboard[row][col];
                index++;
            } else {
                int count = 0;
                for (; col < 8; ++col) {
                    if ('.' == game->chessboard[row][col])
                        count++;
                    else 
                        break;
                }
                fen[index] = count + '0';
                index++;
                if (8 != col) {
                    fen[index] = game->chessboard[row][col];
                    index++;
                }
            }
        }
        if (7 == row) fen[index] = ' ';
        else fen[index] = '/';
        index++;
    }

    fen[index] = game->currentPlayer == WHITE_PLAYER ? 'w' : 'b';
    index++;
    fen[index] = '\0';
}

/**
 * @brief Parse the FEN string and modify the state of game.
 */
void fen_to_chessboard(const char* fen, ChessGame* game) {
    int index = 0, col = 0;
    char current;
    for (int row = 0; row < 8; ++row) {
        while ('/' != (current = fen[index]) && ' ' != current) {
            index++;
            if (current >= '0' && current <= '9') {
                for (int i = 0; i < current - '0'; ++i) {
                    game->chessboard[row][col] = '.';
                    col++;
                }
            } else {
                game->chessboard[row][col] = current;
                col++;
            }
        }
        index++;
        col = 0;
    }

    current = fen[index];
    if (current == 'w')
        game->currentPlayer = WHITE_PLAYER;
    else 
        game->currentPlayer = BLACK_PLAYER;
}

/**
 * @brief Verify if the pawn piece is moving correctly.
 * @details A pawn can only move 2 spaces on its starting row.
 *          Otherwise, it can only move 1 space up (for white player) or down (for black player).
 *          A pawn can move 1 space diagonally if it is able to catch another piece.
 * 
 * @return 1 if pawn moves valid, 0 otherwise.
 */
int is_valid_pawn_move(char piece, int src_row, int src_col, int dest_row, int dest_col, const ChessGame* game) {
    int length = dest_row - src_row;
    if ('P' == piece) {   // While piece
        if (src_col != dest_col) {
            if (-1 != length)
                return 0;
            int h = src_col - dest_col;
            if (-1 != h && 1 != h)
                return 0;
            return !is_white(game->chessboard[dest_row][dest_col]) && 
                    '.' != game->chessboard[dest_row][dest_col];
        }

        if (-2 == length) {
            if (6 == src_row)
                return '.' == game->chessboard[dest_row + 1][dest_col];
            return 0;
        }
        return -1 == length && '.' == game->chessboard[dest_row][dest_col];
    } else if ('p' == piece) {   // Black piece
        if (src_col != dest_col) {
            if (1 == length)
                return 0;
            int h = src_col - dest_col;
            if (-1 != h && 1 != h)
                return 0;
            return is_white(game->chessboard[dest_row][dest_col]) && 
                    '.' != game->chessboard[dest_row][dest_col];
        }

        if (2 == length) {
            if (1 == src_row)
                return '.' == game->chessboard[dest_row - 1][dest_col];
            return 0;
        }
        return 1 == length && '.' == game->chessboard[dest_row][dest_col];
    } else {   // Invalid piece
        return 0;
    }
}

/**
 * @brief Verify if the rook piece is moving correctly.
 * @details A rook can move either vertically or horizontally in any direction.
 * 
 * @return 1 if rook moves valid, 0 otherwise.
 */
int is_valid_rook_move(int src_row, int src_col, int dest_row, int dest_col, const ChessGame* game) {
    if (src_row != dest_row && src_col != dest_col)
        return 0;

    int length = abs(src_row - dest_row) + abs(src_col - dest_col) - 1;
    int row_incre = (dest_row > src_row) - (dest_row < src_row);
    int col_incre = (dest_col > src_col) - (dest_col < src_col);
    for (int i = 0; i < length; ++i) {
        src_row += row_incre;
        src_col += col_incre;
        if ('.' != game->chessboard[src_row][src_col])
            return 0;
    }
    return 1;
}

/**
 * @brief Verify if the knight piece is moving correctly.
 * @details A knight can only move like a shape of "L".
 * 
 * @return 1 if knight moves valid, 0 otherwise.
 */
int is_valid_knight_move(int src_row, int src_col, int dest_row, int dest_col) {
    if (src_row == dest_row || src_col == dest_col)
        return 0;
    return 6 == (abs(src_row - dest_row) + 1) * (abs(src_col - dest_col) + 1);
}

/**
 * @brief Verify if the bishop piece is moving correctly.
 * @details A bishop can only move diagonally.
 * 
 * @return 1 if bishop moves valid, 0 otherwise.
 */
int is_valid_bishop_move(int src_row, int src_col, int dest_row, int dest_col, const ChessGame* game) {
    if (abs(src_row - dest_row) != abs(src_col - dest_col))
        return 0;
    
    int length = abs(src_row - dest_row) - 1;
    int row_incre = src_row < dest_row;
    int col_incre = src_col < dest_col;
    for (int i = 0; i < length; ++i) {
        src_row += (row_incre ? 1 : -1);
        src_col += (col_incre ? 1 : -1);
        if ('.' != game->chessboard[src_row][src_col])
            return 0;
    }
    return 1;
}

/**
 * @brief Verify if the queen piece is moving correctly.
 * @details A queen can move either vertically, horizontally, or diagonally.
 * 
 * @return 1 if queen moves valid, 0 otherwise.
 */
int is_valid_queen_move(int src_row, int src_col, int dest_row, int dest_col, const ChessGame* game) {
    if (src_row == dest_row || src_col == dest_col) 
        return is_valid_rook_move(src_row, src_col, dest_row, dest_col, game);
    return is_valid_bishop_move(src_row, src_col, dest_row, dest_col, game);
}

/**
 * @brief Verify if the king piece is moving correctly.
 * @details A king can only move 1 space either vertically, horizontally or diagonally.
 * 
 * @return 1 if king moves valid, 0 otherwise.
 */
int is_valid_king_move(int src_row, int src_col, int dest_row, int dest_col) {
    if (src_row == dest_row)
        return 1 == abs(src_col - dest_col);
    if (src_col == dest_col)
        return 1 == abs(src_row - dest_row);
    return 1 == abs(src_row - dest_row) && 1 == abs(src_col - dest_col);
}

/**
 * @brief verify if the piece is moving correctly.
 * @details This function will be passed the starting and ending location of a piece,
 * then verify if this piece can be able to move.
 * 
 * @return 1 if the piece moves valid, 0 otherwise.
 */
int is_valid_move(char piece, int src_row, int src_col, int dest_row, int dest_col, const ChessGame* game) {
    if (src_row == dest_row && src_col == dest_col)
        return 0;
    if (game->chessboard[src_row][src_col] != piece)
        return 0;

    int ret;
    switch (piece) {
        case 'P':
        case 'p':
            ret = is_valid_pawn_move(piece, src_row, src_col, dest_row, dest_col, game);
            break;
        case 'R':
        case 'r':
            ret = is_valid_rook_move(src_row, src_col, dest_row, dest_col, game);
            break;
        case 'N':
        case 'n':
            ret = is_valid_knight_move(src_row, src_col, dest_row, dest_col);
            break;
        case 'B':
        case 'b':
            ret = is_valid_bishop_move(src_row, src_col, dest_row, dest_col, game);
            break;
        case 'Q':
        case 'q':
            ret = is_valid_queen_move(src_row, src_col, dest_row, dest_col, game);
            break;
        case 'K':
        case 'k':
            ret = is_valid_king_move(src_row, src_col, dest_row, dest_col);
            break;
        default:
            return 0;
    }

    if (!ret) 
        return 0;
    // Check if capture same color
    return is_white(game->chessboard[src_row][src_col]) != is_white(game->chessboard[dest_row][dest_col]) || 
            '.' == game->chessboard[dest_row][dest_col];
}

/**
 * @brief Parse the given string and modify the ChessMove structure.
 * @details The move string is essentially holding 4 or 5 characters. 
 * The 1st and 2nd characters represent the starting location, 
 * and the 3rd and 4th characters represent the ending location. 
 * For example, "b3b5" means moving the piece on "b3" to "b5".
 * There might be a 5th character which represents the promotion of a pawn.
 * For example, "e7e8q" means a pawn on "e7" moves to "e8" and is promoted to a queen.
 * This function will parse the move string, and initialze the information in the ChessMove structure.
 * 
 * @param move Move string
 * @param parsed_move ChessMove holding the moving information
 * @return State of this moving
 */
int parse_move(const char* move, ChessMove* parsed_move) {
    int length = (int) strlen(move);
    if (4 != length && 5 != length) 
        return PARSE_MOVE_INVALID_FORMAT;
    if (move[0] < 'a' || move[0] > 'h' || move[2] < 'a' || move[2] > 'h')   // Incorrect location
        return PARSE_MOVE_INVALID_FORMAT;
    if (move[1] < '1' || move[1] > '8' || move[3] < '1' || move[3] > '8')   // Incorrect location
        return PARSE_MOVE_OUT_OF_BOUNDS;
    
    // Parse promotion
    if (5 == length) {
        if ('8' != move[3] && '1' != move[3])  // Not reach to the last row
            return PARSE_MOVE_INVALID_DESTINATION;
        if ('r' != move[4] && 'b' != move[4] && 'n' != move[4] && 'q' != move[4])
            return PARSE_MOVE_INVALID_PROMOTION;
        parsed_move->endSquare[2] = move[4];
        parsed_move->endSquare[3] = '\0';
    } else {
        parsed_move->endSquare[2] = '\0';
    }

    parsed_move->startSquare[0] = move[0];
    parsed_move->startSquare[1] = move[1];
    parsed_move->startSquare[2] = '\0';

    parsed_move->endSquare[0] = move[2];
    parsed_move->endSquare[1] = move[3];
    return 0;
}

/**
 * @brief Implement the ChessMove on the chess board.
 * 
 * @param game Chess board
 * @param move Move
 * @param is_client If it's the client site calling the function
 * @param validate_move 1 if assume ChessMove itself is correct, 0 otherwise
 * @return 0 if the move is success made.
 */
int make_move(ChessGame* game, const ChessMove* move, int is_client, int validate_move) {
    int src_row = '8' - move->startSquare[1];
    int src_col = move->startSquare[0] - 'a';
    int dest_row = '8' - move->endSquare[1];
    int dest_col = move->endSquare[0] - 'a';
    char start = game->chessboard[src_row][src_col];
    char end = game->chessboard[dest_row][dest_col];
    int endLength = (int) strlen(move->endSquare);

    // Validate ChessMove
    if (validate_move) {
        if (is_client == game->currentPlayer)  // Out of turn
            return MOVE_OUT_OF_TURN;
        if ('.' == start) 
            return MOVE_NOTHING;
        if ((is_client && !is_white(start)) || (!is_client && is_white(start)))
            return MOVE_WRONG_COLOR;
        if ('.' != end) {
            if ((is_client && is_white(end)) || (!is_client && !is_white(end)))
                return MOVE_SUS;
        }

        if (('P' != start && 'p' != start) && 3 == endLength) 
            return MOVE_NOT_A_PAWN;
        if (3 != endLength) {
            if ('P' == start && 0 == dest_row) 
                return MOVE_MISSING_PROMOTION;
            else if ('p' == start && 7 == dest_row)
                return MOVE_MISSING_PROMOTION;
        }
        
        if (!is_valid_move(start, src_row, src_col, dest_row, dest_col, game))
            return MOVE_WRONG;
    }

    game->chessboard[src_row][src_col] = '.';
    game->chessboard[dest_row][dest_col] = start;
    if ('P' == start && 3 == endLength)        // Promotion white
        game->chessboard[dest_row][dest_col] = toupper(move->endSquare[2]);
    else if ('p' == start && 3 == endLength)   // Promotion black
        game->chessboard[dest_row][dest_col] = move->endSquare[2]; 

    game->moves[game->moveCount] = *move;
    game->moveCount++;
    if ('.' != end) {  // Capture
        game->capturedPieces[game->capturedCount] = end;
        game->capturedCount++;
    }
    
    // Update player
    game->currentPlayer = game->currentPlayer ? WHITE_PLAYER : BLACK_PLAYER;
    return 0;
}

/*
 * @brief Splite the command with " ", each element represents an argument.
 * 
 * @return Number of arguments from the command. Return -1 if it is an invalid command.
 */
int parse_command(const char* message, char *args[3]) {
    int arg_size = 0, arg_index = 0;
    while ('\0' != *message) {
        if (3 == arg_size)
            return -1;
        if (' ' == *message) {
            args[arg_size][arg_index] = '\0';
            arg_size++;
            arg_index = 0;
        } else {
            args[arg_size][arg_index] = *message;
            arg_index++;
        }
        message++;
    }
    args[arg_size][arg_index] = '\0';
    return arg_size + 1;
}

/*
 * @brief Send /move command to another player and make move.
 */
int send_move_command(ChessGame* game, int arg_size, char* args[3], 
                        const char* message, int socketfd, int is_client) {
    if (0 != strcmp(args[0], "/move"))
        return COMMAND_UNKNOWN;
    if (2 != arg_size)
        return COMMAND_ERROR;

    ChessMove move;
    if (0 == parse_move(args[1], &move)) {
        if (0 == make_move(game, &move, is_client, 1)) {
            send(socketfd, message, strlen(message), 0);
            return COMMAND_MOVE;
        } else {
            return COMMAND_ERROR;
        }
    } else {
        return COMMAND_ERROR;
    }
}

/*
 * @brief Send /forfeit command to another player.
 */
int send_forfeit_command(int arg_size, char* arg, const char* message, int socketfd) {
    if (0 != strcmp(arg, "/forfeit"))
        return COMMAND_UNKNOWN;
    if (1 != arg_size)
        return COMMAND_ERROR;
    send(socketfd, message, strlen(message), 0);
    return COMMAND_FORFEIT;
}

/*
 * @brief Send /chessboard command to another player.
 */
int send_chessboard_command(const ChessGame* game, int arg_size, char* arg) {
    if (0 != strcmp(arg, "/chessboard"))
        return COMMAND_UNKNOWN;
    if (1 != arg_size)
        return COMMAND_ERROR;
    display_chessboard(game);
    return COMMAND_DISPLAY;
}

/*
 * @details Send /import command to another player. 
 * Only the server site can send the /import command.
 */
int send_import_command(ChessGame* game, int arg_size, char* args[3], 
                        const char* message, int socketfd, int is_client) {
    if (0 != strcmp(args[0], "/import"))
        return COMMAND_UNKNOWN;
    if (3 != arg_size)
        return COMMAND_ERROR;

    if (!is_client) {
        char fen[BUFFER_SIZE];
        strcpy(fen, args[1]);
        strcat(fen, " ");
        strcat(fen, args[2]);
        fen_to_chessboard(fen, game);
        send(socketfd, message, strlen(message), 0);
        return COMMAND_IMPORT;
    }
    return COMMAND_ERROR;
}

/*
 * @details Send /load command to another player. 
 * It is defaultly looking for file "game_database.txt" in src directory.
 */
int send_load_command(ChessGame* game, int arg_size, char* args[3], const char* message, int socketfd) {
    if (0 != strcmp(args[0], "/load"))
        return COMMAND_UNKNOWN;
    if (3 != arg_size)
        return COMMAND_ERROR;
            
    char *endptr;
    int save_number = (int)strtol(args[2], &endptr, 10);
    if (save_number <= 0)
        return COMMAND_ERROR;
    if (0 != load_game(game, args[1], "../src/game_database.txt", save_number))
        return COMMAND_ERROR;
    send(socketfd, message, strlen(message), 0);
    return COMMAND_LOAD;
}

/*
 * @details Apply /save command, but don't have to send to another player.
 * It is defaultly looking for file "game_database.txt" in src.
 */
int send_save_command(const ChessGame* game, int arg_size, char* args[3]) {
    if (0 != strcmp(args[0], "/save"))
        return COMMAND_UNKNOWN;
    if (2 != arg_size)
        return COMMAND_ERROR;
    if (0 != save_game(game, args[1], "../src/game_database.txt"))
        return COMMAND_ERROR;
    return COMMAND_SAVE;
}

/*
 * @brief Send /none command in order to switch controller.
 */
int send_none_command(int arg_size, char* arg, 
                        const char* message, int socketfd, int is_client) {
    if (0 != strcmp(arg, "/none"))
        return COMMAND_UNKNOWN;
    if (1 != arg_size)
        return COMMAND_ERROR;
    send(socketfd, message, strlen(message), is_client);
    return COMMAND_NONE;
}

/**
 * @brief Send command to another player and modify the chess board if applied.
 * 
 * @return Type of command sent.
 */
int send_command(ChessGame* game, const char* message, int socketfd, int is_client) {
    char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], arg3[BUFFER_SIZE];
    char* args[3] = { arg1, arg2, arg3 };
    int arg_size = parse_command(message, args);
    if (arg_size <= 0)
        return COMMAND_ERROR;

    switch (arg1[1]) {
        case 'm':
            return send_move_command(game, arg_size, args, message, socketfd, is_client);
        case 'f':
            return send_forfeit_command(arg_size, args[0], message, socketfd);
        case 'c':
            return send_chessboard_command(game, arg_size, args[0]);
        case 'i':
            return send_import_command(game, arg_size, args, message, socketfd, is_client);
        case 'l':
            return send_load_command(game, arg_size, args, message, socketfd);
        case 's':
            return send_save_command(game, arg_size, args);
        case 'n':
            return send_none_command(arg_size, args[0], message, socketfd, is_client);
        default:
            return COMMAND_UNKNOWN;
    }
}

/*
 * @brief Receive /move command from another player and make move.
 */
int receive_move_command(ChessGame* game, int arg_size, char* args[3], int is_client) {
    if (0 != strcmp(args[0], "/move"))
        return -1;
    if (2 != arg_size)
        return COMMAND_ERROR;

    ChessMove move;
    if (0 == parse_move(args[1], &move)) {
        if (0 == make_move(game, &move, is_client, 0)) 
            return COMMAND_MOVE;
        else 
            return COMMAND_ERROR;
    }
    return COMMAND_ERROR;
}

/*
 * @brief Receive /forfeit command from another player and terminate the game.
 */
int receive_forfeit_command(int arg_size, char* arg, int socketfd) {
    if (0 != strcmp(arg, "/forfeit"))
        return -1;
    if (1 != arg_size)
        return COMMAND_ERROR;
    close(socketfd);
    return COMMAND_FORFEIT;
}

/*
 * @details Receive /import command from another player and update chess board. 
 * Only the client site can receive the /import command.
 */
int receive_import_command(ChessGame *game, int arg_size, char* args[3], int is_client) {
    if (0 != strcmp(args[0], "/import"))
        return -1;
    if (3 != arg_size)
        return COMMAND_ERROR;
    if (is_client) {
        char fen[BUFFER_SIZE];
        strcpy(fen, args[1]);
        strcat(fen, " ");
        strcat(fen, args[2]);
        fen_to_chessboard(fen, game);
        return COMMAND_IMPORT;
    }
    return COMMAND_ERROR;
}

/*
 * @details Receive /load command from another player. 
 * It is defaultly looking for file "game_database.txt" in src.
 */
int receive_load_command(ChessGame *game, int arg_size, char* args[3], int socketfd, int is_client) {
    if (0 != strcmp(args[0], "/load"))
        return -1;
    if (3 != arg_size)
        return COMMAND_ERROR;
            
    char *endptr;
    int save_number = (int) strtol(args[2], &endptr, 10);
    if (save_number <= 0)
        return COMMAND_ERROR;
    if (0 != load_game(game, args[1], "../src/game_database.txt", save_number))
        return COMMAND_ERROR;
    if (is_client && game->currentPlayer != WHITE_PLAYER)
        return COMMAND_NONE;
    return COMMAND_LOAD;
}

/*
 * @brief Receive /none command from another player. 
 */
int receive_none_command(int arg_size, char* arg) {
    if (0 != strcmp(arg, "/none"))
        return -1;
    if (1 != arg_size)
        return COMMAND_ERROR;
    return COMMAND_NONE;
}

/**
 * @brief Receive command from another player and modify the chess board if applied.
 * 
 * @return Type of command received.
 */
int receive_command(ChessGame *game, const char *message, int socketfd, int is_client) {
    char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], arg3[BUFFER_SIZE];
    char *args[3] = { arg1, arg2, arg3 };
    int arg_size = parse_command(message, args);
    if (arg_size <= 0)
        return COMMAND_ERROR;

    switch (arg1[1]) {
        case 'm':
            return receive_move_command(game, arg_size, args, is_client);
        case 'f':
            return receive_forfeit_command(arg_size, args[0], socketfd);
        case 'i':
            return receive_import_command(game, arg_size, args, is_client);
        case 'l':
            return receive_load_command(game, arg_size, args, socketfd, is_client);
        case 'n':
            return receive_none_command(arg_size, args[0]);
        default:
            return -1;
    }
}

/*
 * @brief Verify if the username is valid.
 * @details Username should not contain white spaces.
 * 
 * @return 1 if username valid.
 */
int username_valid(const char* username) {
    if (NULL == username) 
        return 0;
    int length = (int) strlen(username);
    if (0 == length) 
        return 0;
    for (int i = 0; i < length; ++i) {
        if (' ' == username[i])
            return 0;
    }
    return 1;
}

/**
 * @brief Save the game state and username in a given file.
 * 
 * @return 0 if saved game success, -1 otherwise.
 */
int save_game(const ChessGame* game, const char* username, const char* db_filename) {
    if (!username_valid(username)) 
        return -1;

    FILE *db = fopen(db_filename, "a");
    if (!db)
        return -1;
    char fen[BUFFER_SIZE];
    chessboard_to_fen(fen, game);
    fprintf(db, "%s:%s\n", username, fen);
    fclose(db);
    return 0;
}

/*
 * @brief Parse the username from a line.
 * @details Username is located in the beginning of the line,
 * and ended with a ':'.
 */
void get_username(char* username, const char* line) {
    memset(username, 0, BUFFER_SIZE);
    int index = 0;
    while (':' != *line) {
        username[index] = *line;
        index++;
        line++;
    }
    username[index] = '\0';
}

/*
 * @brief Parse a FEN string from a line.
 * @details FEN string is located in the end of line, 
 * and started after a ':'.
 */
void get_fen(char* fen, const char* line) {
    while (':' != *line) line++;
    int index = 0;
    line++;
    while ('\0' != *line) {
        fen[index] = *line;
        index++;
        line++;
    }
    fen[index] = '\0';
}

/**
 * @brief Load the game state from a given file.
 * 
 * @param save_number The number of game state
 * @return 0 if loaded success, -1 otherwise.
 */
int load_game(ChessGame* game, const char* username, const char* db_filename, int save_number) {
    if (save_number <= 0 || !username_valid(username)) 
        return -1;

    FILE *db = fopen(db_filename, "r");
    if (!db) 
        return -1;
    
    int count = 0;
    char line[BUFFER_SIZE], last_line[BUFFER_SIZE], u_name[BUFFER_SIZE];
    while (NULL != fgets(line, BUFFER_SIZE, db)) {
        line[strlen(line)-1] = '\0';
        get_username(u_name, line);
        if (0 == strcmp(username, u_name)) {   // Match username
            memset(last_line, 0, BUFFER_SIZE);
            strcpy(last_line, line);           // Hold the last line data
            count++;
            if (count == save_number)          // Match save number
                break;
        }
        memset(line, 0, BUFFER_SIZE);
    }
    
    if (count != save_number) {
        fclose(db);
        return -1;
    }

    char fen[BUFFER_SIZE];
    get_fen(fen, last_line);
    fen_to_chessboard(fen, game);
    fclose(db);
    return 0;
}
