#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fstream>

#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

using namespace std;

void receive_file(int sockfd, struct sockaddr_in &clientAddr, socklen_t &addrLen) {
    ofstream outFile("received.bmp", ios::binary);
    char buffer[BUFFER_SIZE];
    ssize_t recvLen;

    if (!outFile) {
        cerr << "Error opening file for writing." << endl;
        return;
    }

    cout << "Receiving file..." << endl;
    while ((recvLen = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&clientAddr, &addrLen)) > 0) {
        outFile.write(buffer, recvLen);
        if (recvLen < BUFFER_SIZE) break; // End of file
    }
    outFile.close();
    cout << "File received successfully." << endl;
}

int main() {
    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Configure server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(SERVER_PORT);

    // Bind socket
    if (bind(sockfd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        return 1;
    }

    cout << "UDP Server listening on port " << SERVER_PORT << endl;

    while (true) {
        // Receive message from client
        ssize_t recvLen = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, 
                                   (struct sockaddr *)&clientAddr, &addrLen);
        if (recvLen < 0) {
            perror("Receive failed");
            break;
        }
        buffer[recvLen] = '\0';
        cout << "Received from client (port " << ntohs(clientAddr.sin_port) << "): " << buffer << endl;

        if (strcmp(buffer, "START_FILE_TRANSFER") == 0) {
            receive_file(sockfd, clientAddr, addrLen);
            continue;
        }

        // Echo message back to client
        sendto(sockfd, buffer, recvLen, 0, (struct sockaddr *)&clientAddr, addrLen);
        cout << "Echoed back to client (port " << ntohs(clientAddr.sin_port) << "): " << buffer << endl;
    }

    close(sockfd);
    return 0;
}
