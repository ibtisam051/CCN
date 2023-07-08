#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define PORT 8080

int main() {
    int clientSocket;
    struct sockaddr_in serverAddress;
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0) {
        perror("inet_pton failed");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server. Send messages in lowercase or uppercase.\n");
    printf("Enter 'exit' to quit.\n");

    while (1) {
        printf("Enter a letter: ");
        fgets(buffer, BUFFER_SIZE, stdin);

        // Remove the newline character
        buffer[strcspn(buffer, "\n")] = '\0';

        // Send the letter to the server
        send(clientSocket, buffer, strlen(buffer), 0);

        // If the user enters 'exit', close the connection and exit the loop
        if (strcmp(buffer, "exit") == 0) {
            break;
        }

        // Clear the buffer
        memset(buffer, 0, BUFFER_SIZE);

        // Receive and display the response from the server
        if (recv(clientSocket, buffer, BUFFER_SIZE, 0) < 0) {
            perror("recv failed");
            break;
        }

        printf("Server response: %s\n", buffer);
    }

    // Close the socket
    close(clientSocket);

    return 0;
}
