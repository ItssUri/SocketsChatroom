#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "172.17.20.178"
#define PORT 12345
#define BUFFER_SIZE 1024

void receiveMessages(SOCKET clientSocket) {
    char buffer[BUFFER_SIZE];
    while (true) {
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::cout << buffer;
        } else {
            std::cerr << "Disconnected from server." << std::endl;
            break;
        }
    }
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) != 0) {
        std::cerr << "Connection failed." << std::endl;
        return 1;
    }

    std::cout << "Connected to server!" << std::endl;

    std::thread receiveThread(receiveMessages, clientSocket);

    std::string userInput;
    while (true) {
        // std::cout << "> ";
        std::getline(std::cin, userInput);
        if (userInput == "quit") break;
        send(clientSocket, userInput.c_str(), userInput.size(), 0);
    }

    closesocket(clientSocket);
    WSACleanup();
    receiveThread.join();

    return 0;
}
