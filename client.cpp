#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fstream>

#define SERVER_PORT 8080
#define CLIENT_PORT 8081
#define SIZE 1024

using namespace std;

void send_file(int clientSocket, struct sockaddr_in &serverAddr) {
    ifstream inFile("input.bmp", ios::binary);
    char buffer[SIZE];
    socklen_t addrlen = sizeof(serverAddr);

    if (!inFile) {
        cerr << "Error opening file for reading." << endl;
        return;
    }

    cout << "Sending file..." << endl;
    while (inFile.read(buffer, SIZE) || inFile.gcount() > 0) {
        sendto(clientSocket, buffer, inFile.gcount(), 0, (struct sockaddr *)&serverAddr, addrlen);
    }
    inFile.close();
    cout << "File sent successfully." << endl;
}

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    char message[] = "HELLO";
    char buffer[SIZE];
    socklen_t addrlen = sizeof(serverAddr);

    clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket < 0) {
        perror("Socket creation failed");
        return 1;
    }

    memset(&clientAddr, 0, sizeof(clientAddr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = INADDR_ANY;
    clientAddr.sin_port = htons(CLIENT_PORT);

    if (bind(clientSocket, (const struct sockaddr *)&clientAddr, sizeof(clientAddr)) < 0) {
        perror("Bind failed");
        close(clientSocket);
        return 1;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(SERVER_PORT);

    sendto(clientSocket, message, strlen(message), 0, (struct sockaddr *)&serverAddr, addrlen);
    cout << "Sent: " << message << endl;

    ssize_t recvLen = recvfrom(clientSocket, buffer, SIZE, 0, (struct sockaddr *)&serverAddr, &addrlen);
    if (recvLen < 0) {
        perror("Receive failed");
    } else {
        buffer[recvLen] = '\0';
        cout << "Received echo: " << buffer << endl;
    }

    char startMessage[] = "START_FILE_TRANSFER";
    sendto(clientSocket, startMessage, strlen(startMessage), 0, (struct sockaddr *)&serverAddr, addrlen);
    send_file(clientSocket, serverAddr);

    close(clientSocket);
    return 0;
}
