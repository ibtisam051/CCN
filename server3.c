#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>

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
    int serverSocket, clientSocket, valread;
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE] = {0};

    // Create server socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
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
    if (listen(serverSocket, 1) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server started. Waiting for a client to connect...\n");

    // Accept the new connection
    int addressLength = sizeof(address);
    if ((clientSocket = accept(serverSocket, (struct sockaddr *)&address, (socklen_t *)&addressLength)) < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }

    printf("Client connected. Waiting for messages...\n");

    while (1) {
        // Read the incoming message
        valread = read(clientSocket, buffer, BUFFER_SIZE);
        if (valread <= 0)
            break;

        // Convert received message to uppercase
        convertToUpper(buffer);

        // Send the converted message back to the client
        send(clientSocket, buffer, strlen(buffer), 0);

        memset(buffer, 0, BUFFER_SIZE);
    }

    close(clientSocket);
    close(serverSocket);

    return 0;
}
