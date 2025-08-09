#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <cstdlib>     // for getenv
#include <unistd.h>
#include <arpa/inet.h>

std::vector<int> clients;  // store connected client sockets
std::mutex clientsMutex;   // mutex for thread safety

// Broadcast message to all clients
void broadcastMessage(const std::string& message, int senderSocket) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    for (int client : clients) {
        if (client != senderSocket) {
            send(client, message.c_str(), message.size(), 0);
        }
    }
}

// Handle each client connection
void handleClient(int clientSocket) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            std::cout << "Client disconnected.\n";
            close(clientSocket);

            std::lock_guard<std::mutex> lock(clientsMutex);
            clients.erase(std::remove(clients.begin(), clients.end(), clientSocket), clients.end());
            break;
        }
        std::string message(buffer);
        std::cout << "Message received: " << message << std::endl;
        broadcastMessage(message, clientSocket);
    }
}

int main() {
    // Get dynamic port from Railway
    const char* portEnv = std::getenv("PORT");
    int port = portEnv ? std::stoi(portEnv) : 3000; // default for local testing

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating socket\n";
        return -1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error binding to port " << port << "\n";
        return -1;
    }

    if (listen(serverSocket, 5) == -1) {
        std::cerr << "Error listening on socket\n";
        return -1;
    }

    std::cout << "Server running on port " << port << "\n";

    while (true) {
        sockaddr_in clientAddr{};
        socklen_t clientSize = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientSize);
        if (clientSocket == -1) {
            std::cerr << "Error accepting connection\n";
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients.push_back(clientSocket);
        }

        std::thread(handleClient, clientSocket).detach();
    }

    close(serverSocket);
    return 0;
}
