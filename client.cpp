#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fstream>

#define SERVER_PORT 8080
#define CLIENT_PORT 8081
#define BUFFER_SIZE 1024

using namespace std;

void send_file(int sockfd, struct sockaddr_in &serverAddr) {
    ifstream inFile("input.bmp", ios::binary);
    char buffer[BUFFER_SIZE];
    socklen_t addrLen = sizeof(serverAddr);

    if (!inFile) {
        cerr << "Error opening file for reading." << endl;
        return;
    }

    cout << "Sending file..." << endl;
    while (inFile.read(buffer, BUFFER_SIZE) || inFile.gcount() > 0) {
        sendto(sockfd, buffer, inFile.gcount(), 0, (struct sockaddr *)&serverAddr, addrLen);
    }
    inFile.close();
    cout << "File sent successfully." << endl;
}

int main() {
    int sockfd;
    struct sockaddr_in serverAddr, clientAddr;
    char message[] = "HELLO";
    char buffer[BUFFER_SIZE];
    socklen_t addrLen = sizeof(serverAddr);

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Bind client to its own port
    memset(&clientAddr, 0, sizeof(clientAddr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = INADDR_ANY;
    clientAddr.sin_port = htons(CLIENT_PORT);

    if (bind(sockfd, (const struct sockaddr *)&clientAddr, sizeof(clientAddr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        return 1;
    }

    // Configure server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(SERVER_PORT);

    // Send message to server
    sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&serverAddr, addrLen);
    cout << "Sent: " << message << endl;

    // Receive echo from server
    ssize_t recvLen = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&serverAddr, &addrLen);
    if (recvLen < 0) {
        perror("Receive failed");
    } else {
        buffer[recvLen] = '\0';
        cout << "Received echo: " << buffer << endl;
    }

    // Initiate file transfer
    char startMessage[] = "START_FILE_TRANSFER";
    sendto(sockfd, startMessage, strlen(startMessage), 0, (struct sockaddr *)&serverAddr, addrLen);
    send_file(sockfd, serverAddr);

    close(sockfd);
    return 0;
}
