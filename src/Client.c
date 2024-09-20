#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "Resources.h"

int main() {
    ChessGame game;
    int connfd = 0;
    struct sockaddr_in serv_addr;

    // Connect to the server
    if ((connfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    if (connect(connfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }

    initialize_game(&game);
    display_chessboard(&game);

    char buffer[BUFFER_SIZE];
    int server_command, client_command;
    while (1) {
        // Client enter
        while (1) {
            fprintf(stdout, "[Client] Enter message: ");
            memset(buffer, 0, BUFFER_SIZE);
            fgets(buffer, BUFFER_SIZE, stdin);
            buffer[strlen(buffer)-1] = '\0';
            fprintf(stdout, "\n");

            // Send command
            client_command = send_command(&game, buffer, connfd, 1);
            if (COMMAND_UNKNOWN == client_command || COMMAND_ERROR == client_command) {
                fprintf(stdout, "[Client] Bad command. Enter again.\n");
            } else if (COMMAND_FORFEIT == client_command) {
                FILE *temp = fopen("./fen.txt", "w");
                char fen[200];
                chessboard_to_fen(fen, &game);
                fprintf(temp, "%s", fen);
                fclose(temp);
                close(connfd);
                return 0;
            } else if (COMMAND_DISPLAY != client_command && COMMAND_SAVE != client_command) { 
                break;
            } 
        }

        // Read from server
        while (1) {
            memset(buffer, 0, BUFFER_SIZE);
            if (read(connfd, buffer, BUFFER_SIZE) < 0) {
                fprintf(stdout, "[Client] Read error\n");
                break;
            }

            // Receive command
            server_command = receive_command(&game, buffer, connfd, 1);
            if (COMMAND_NONE != server_command) {
                fprintf(stdout, "[Client] Server enter: %s\n", buffer);
                    if (COMMAND_FORFEIT == server_command)
                        break;
                if (COMMAND_LOAD == server_command && game.currentPlayer != WHITE_PLAYER) {
                    fprintf(stdout, "[Client] Current player is server. Switch control to server.\n");
                    send_command(&game, "/none", connfd, 1);
                } else {
                    break;
                }
            } else {
                fprintf(stdout, "[Client] Server switches control.\n");
                break;
            }
        }
    }

    // Save the game in current directory
    FILE *temp = fopen("./fen.txt", "w");
    char fen[200];
    chessboard_to_fen(fen, &game);
    fprintf(temp, "%s", fen);
    fclose(temp);
    close(connfd);
    return 0;
}
