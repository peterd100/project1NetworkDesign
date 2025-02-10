#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fstream>

#define SERVER_PORT 8080
#define SIZE 1024

using namespace std;

void receive_file(int serverSocket, struct sockaddr_in &clientAddr, socklen_t &addrlen) {
    ofstream outFile("received.bmp", ios::binary);
    char buffer[SIZE];
    ssize_t recvLen;

    if (!outFile) {
        cerr << "Error opening file for writing." << endl;
        return;
    }

    cout << "Receiving file..." << endl;
    while ((recvLen = recvfrom(serverSocket, buffer, SIZE, 0, (struct sockaddr *)&clientAddr, &addrlen)) > 0) {
        outFile.write(buffer, recvLen);
        if (recvLen < SIZE) break;
    }
    outFile.close();
    cout << "File received successfully." << endl;
}

int main() {
    int serverSocket;
    char buffer[SIZE];
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrlen = sizeof(clientAddr);

    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket < 0) {
        perror("Socket creation failed");
        return 1;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(SERVER_PORT);

    if (bind(serverSocket, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        close(serverSocket);
        return 1;
    }

    cout << "UDP Server listening on port " << SERVER_PORT << endl;

    while (true) {
        ssize_t recvLen = recvfrom(serverSocket, buffer, SIZE, 0, 
                                   (struct sockaddr *)&clientAddr, &addrlen);
        if (recvLen < 0) {
            perror("Receive failed");
            break;
        }
        buffer[recvLen] = '\0';
        cout << "Received from client (port " << ntohs(clientAddr.sin_port) << "): " << buffer << endl;

        if (strcmp(buffer, "START_FILE_TRANSFER") == 0) {
            receive_file(serverSocket, clientAddr, addrlen);
            continue;
        }

        sendto(serverSocket, buffer, recvLen, 0, (struct sockaddr *)&clientAddr, addrlen);
        cout << "Echoed back to client (port " << ntohs(clientAddr.sin_port) << "): " << buffer << endl;
    }

    close(serverSocket);
    return 0;
}
