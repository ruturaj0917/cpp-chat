#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>   // for std::remove
#include <cstring>     // for memset
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

std::vector<int> clients;
std::mutex clients_mutex;

void broadcastMessage(const std::string& message, int sender_fd) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (int client_fd : clients) {
        if (client_fd != sender_fd) {
            send(client_fd, message.c_str(), message.size(), 0);
        }
    }
}

void handleClient(int clientSocket) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytes <= 0) {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.erase(std::remove(clients.begin(), clients.end(), clientSocket), clients.end());
            close(clientSocket);
            break;
        }
        std::string message(buffer);
        broadcastMessage(message, clientSocket);
    }
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5000); // Listening on port 5000
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_fd, 5);

    std::cout << "Server listening on port 5000\n";

    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_size = sizeof(client_addr);
        int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_size);

        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.push_back(client_fd);

        std::thread(handleClient, client_fd).detach();
    }

    close(server_fd);
}
