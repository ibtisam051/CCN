#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 5
#define BUFFER_SIZE 1024
#define PORT 8080

void convertToUpper(char *str) {
    int i = 0;
    while (str[i]) {
        str[i] = toupper(str[i]);
        i++;
    }
}

int main() {
    int serverSocket, clientSockets[MAX_CLIENTS], activity, i, valread, sd, max_sd;
    struct sockaddr_in address;
    fd_set readfds;

    // Create server socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the specified address and port
    if (bind(serverSocket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(serverSocket, MAX_CLIENTS) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server started. Waiting for connections...\n");

    // Initialize client sockets to 0
    for (i = 0; i < MAX_CLIENTS; i++) {
        clientSockets[i] = 0;
    }

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(serverSocket, &readfds);
        max_sd = serverSocket;

        // Add child sockets to set
        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = clientSockets[i];

            if (sd > 0)
                FD_SET(sd, &readfds);

            if (sd > max_sd)
                max_sd = sd;
        }

        // Wait for activity on one of the sockets
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            perror("select error");
        }

        // If there's a new incoming connection
        if (FD_ISSET(serverSocket, &readfds)) {
            struct sockaddr_in clientAddress;
            int clientAddressLength = sizeof(clientAddress);

            // Accept the new connection
            int newSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, (socklen_t *)&clientAddressLength);
            if (newSocket < 0) {
                perror("accept failed");
                exit(EXIT_FAILURE);
            }

            printf("New connection, socket fd is %d, IP is: %s, port is: %d\n",
                   newSocket, inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));

            // Add new socket to array of sockets
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (clientSockets[i] == 0) {
                    clientSockets[i] = newSocket;
                    break;
                }
            }
        }

        // Check for IO operations on client sockets
        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = clientSockets[i];

            if (FD_ISSET(sd, &readfds)) {
                char buffer[BUFFER_SIZE] = {0};

                // Read the incoming message
                valread = read(sd, buffer, BUFFER_SIZE);
                if (valread == 0) {
                    // Client disconnected
                    getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
                    printf("Client disconnected, IP: %s, port: %d\n",
                           inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                    // Close the socket and mark as 0 in the client array for reuse
                    close(sd);
                    clientSockets[i] = 0;
                } else {
                    // Convert received message to uppercase
                    convertToUpper(buffer);

                    // Send the converted message back to the client
                    send(sd, buffer, strlen(buffer), 0);
                }
            }
        }
    }

    return 0;
}
