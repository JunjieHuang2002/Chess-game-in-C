#include "Resources.h"

int main() {
    int listenfd, connfd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set options to reuse the IP address and IP port if either is already in use
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))");
        return -1;
    }
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))");
        return -1;
    }

    // Bind socket to port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(listenfd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(listenfd, 1) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    INFO("Server listening on port %d", PORT);
    // Accept incoming connection
    if ((connfd = accept(listenfd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    INFO("Server accepted connection");

    ChessGame game;
    initialize_game(&game);
    display_chessboard(&game);

    char buffer[BUFFER_SIZE];
    int client_command, server_command;
    while (1) {
        // Read from client
        while (1) {
            memset(buffer, 0, BUFFER_SIZE);
            if (read(connfd, buffer, BUFFER_SIZE) < 0) {
                fprintf(stdout, "[Server] Read error\n");
                break;
            }

            // Receive command
            client_command = receive_command(&game, buffer, connfd, 1);
            if (COMMAND_NONE != client_command) {
                fprintf(stdout, "[Client] Client enter: %s\n", buffer);
                    if (COMMAND_FORFEIT == server_command)
                        break;
                if (COMMAND_LOAD == client_command && game.currentPlayer == WHITE_PLAYER) {
                    fprintf(stdout, "[Client] Current player is client. Switch control to client.\n");
                    send_command(&game, "/none", connfd, 0);
                } else {
                    break;
                }
            } else {
                fprintf(stdout, "[Client] Client switches control.\n");
                break;
            }
        }

        // Server enter
        while (1) {
            fprintf(stdout, "[Server] Enter message: ");
            memset(buffer, 0, BUFFER_SIZE);
            fgets(buffer, BUFFER_SIZE, stdin);
            buffer[strlen(buffer)-1] = '\0';
            fprintf(stdout, "\n");

            // Send command
            server_command = send_command(&game, buffer, connfd, 0);
            if (COMMAND_UNKNOWN == server_command || COMMAND_ERROR == server_command) {
                fprintf(stdout, "[Server] Bad command. Enter again.\n");
            } else if (COMMAND_FORFEIT == server_command) {
                close(listenfd);
                close(connfd);
                return 0;
            } else if (COMMAND_DISPLAY != server_command  && COMMAND_SAVE != server_command) {
                break;
            }
        }
    }

    close(listenfd);
    close(connfd);
    return EXIT_SUCCESS;
}
