#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_CLIENTS 5
#define BUFFER_SIZE 1024
#define PORT 8080

void* handleClient(void* arg) {
    int clientSocket = *(int*)arg;
    char buffer[BUFFER_SIZE] = {0};
    int valread;

    while (1) {
        valread = read(clientSocket, buffer, BUFFER_SIZE);
        if (valread <= 0)
            break;

        // Convert received message to uppercase
        for (int i = 0; i < valread; i++) {
            buffer[i] = toupper(buffer[i]);
        }

        // Send the converted message back to the client
        send(clientSocket, buffer, valread, 0);

        memset(buffer, 0, BUFFER_SIZE);
    }

    close(clientSocket);
    free(arg);
    pthread_exit(NULL);
}

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in address;
    pthread_t thread;
    pthread_attr_t threadAttr;

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

    // Initialize thread attribute
    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);

    while (1) {
        // Accept the new connection
        int clientAddrLength = sizeof(address);
        if ((clientSocket = accept(serverSocket, (struct sockaddr *)&address, (socklen_t *)&clientAddrLength)) < 0) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        printf("New connection, socket fd is %d, IP is: %s, port is: %d\n",
               clientSocket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

        // Create a new thread to handle the client
        int* socketPtr = malloc(sizeof(int));
        *socketPtr = clientSocket;
        if (pthread_create(&thread, &threadAttr, handleClient, (void*)socketPtr) != 0) {
            perror("pthread_create failed");
            exit(EXIT_FAILURE);
        }
    }

    pthread_attr_destroy(&threadAttr);
    close(serverSocket);

    return 0;
}
