#include <winsock2.h>
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <ctime>
#include <sstream>
#include <algorithm>  // For std::remove

#define PORT_1 12345
#define BUFFER_SIZE 1024

std::string adjectives[] = {"Brave", "Swift", "Clever", "Bold", "Lucky", "Fierce", "Gentle", "Witty", "Noble", "Sly", "Mighty", "Happy", "Grumpy", "Sneaky", "Charming", "Eager", "Loyal", "Vivid", "Silent", "Cryptic", "Arcane", "Reckless", "Melancholy", "Electric"};
std::string names[] = {"Lain", "Rei", "Yuno", "Shinji", "Homura", "Misato", "Kamui", "Milly", "Vash", "Priss", "Daisuke", "Lucy", "Saya", "Holo", "Nana", "Akira", "Kaneda", "Tohno", "Rakka", "Makoto", "Kino", "Kaiji", "Guts", "Re-l", "Vincent", "Rin", "Ichise", "Yuki", "Johan", "Tenma"};

std::vector<SOCKET> clients;     // List of connected clients
std::mutex clientsMutex;         // Mutex to protect access to the clients list

// Function to broadcast a message to all connected clients
void broadcastMessage(const std::string& message, SOCKET sender) {
    std::lock_guard<std::mutex> lock(clientsMutex);  // Lock to protect shared clients list
    for (SOCKET client : clients) {
        if (client != sender) {  // Avoid sending the message back to the sender
            send(client, message.c_str(), message.size(), 0);
        }
    }
}

void handle_client(SOCKET clientSocket, sockaddr_in clientAddr) {
    int numAdjectives = sizeof(adjectives) / sizeof(adjectives[0]);
    int numNames = sizeof(names) / sizeof(names[0]);
    std::srand(std::time(nullptr));
    int adjIndex = std::rand() % numAdjectives;
    int nameIndex = std::rand() % numNames;
    std::string username = adjectives[adjIndex] + names[nameIndex];
    char* clientIP = inet_ntoa(clientAddr.sin_addr);
    int clientPort = ntohs(clientAddr.sin_port);
    
    std::cout << username << ": Client connected: IP = " << clientIP << ", Port = " << clientPort << std::endl;

    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients.push_back(clientSocket);
    }

    std::stringstream responseStream;
    responseStream << "\n _    _      _                          \n";
    responseStream << "| |  | |    | |                         \n";
    responseStream << "| |  | | ___| | ___ ___  _ __ ___   ___ \n";
    responseStream << "| |/\\| |/ _ \\ |/ __/ _ \\| '_ ` _ \\ / _ \\\n";
    responseStream << "\\  /\\  /  __/ | (_| (_) | | | | | |  __/\n";
    responseStream << " \\/  \\/ \\___|_|\\___\\___/|_| |_| |_|\\___|\n";
    responseStream << "                                        \n";
    responseStream << "Welcome to the chatroom! Your generated username is: " << username;
    responseStream << "\nEnter message to send (or 'quit' to exit)\n";
    std::string response = responseStream.str();

    int sendResult = send(clientSocket, response.c_str(), response.size(), 0);
    if (sendResult == SOCKET_ERROR) {
        std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
    }

    while (true) {
        char buffer[BUFFER_SIZE] = {0};
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);

        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "Receive failed: " << WSAGetLastError() << std::endl;
            break;
        }

        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0'; // Null-terminate the received data
            std::cout << "[" << username << "]: " << buffer << std::endl;

            std::stringstream messageStream;
            messageStream << "[" << username << "]: " << buffer << std::endl;
            std::string message = messageStream.str();

            // Broadcast the message to all clients
            broadcastMessage(message, clientSocket);
        } else if (bytesReceived == 0) {
            // Client has closed the connection
            std::cout << username << " left. (Client Disconnected)" << std::endl;
            break;
        } else {
            // An error occurred
            std::cerr << username << " Receive failed: " << WSAGetLastError() << std::endl;
            break;
        }
    }

    // Remove the client from the list and close the socket
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients.erase(std::remove(clients.begin(), clients.end(), clientSocket), clients.end());
    }
    closesocket(clientSocket);
}

int main() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("0.0.0.0");
    serverAddr.sin_port = htons(PORT_1);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server is listening on port " << PORT_1 << "..." << std::endl;

    while (true) {
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed." << std::endl;
            continue;
        }

        std::thread clientThread(handle_client, clientSocket, clientAddr);
        clientThread.detach();
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}