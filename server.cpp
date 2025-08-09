#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>

#pragma comment(lib, "ws2_32.lib")

std::vector<SOCKET> clients;
std::mutex clients_mutex;

void broadcastMessage(const std::string& message, SOCKET sender) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (SOCKET client : clients) {
        if (client != sender) {
            send(client, message.c_str(), message.size(), 0);
        }
    }
}

void handleClient(SOCKET client) {
    char buffer[1024];
    while (true) {
        int bytes = recv(client, buffer, sizeof(buffer), 0);
        if (bytes <= 0) {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());
            closesocket(client);
            break;
        }
        buffer[bytes] = '\0';
        broadcastMessage(buffer, client);
    }
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5000);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(server, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(server, SOMAXCONN);

    std::cout << "Server listening on port 5000\n";

    while (true) {
        SOCKET client = accept(server, nullptr, nullptr);
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.push_back(client);
        }
        std::thread(handleClient, client).detach();
    }

    closesocket(server);
    WSACleanup();
}
