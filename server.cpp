#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>//sleep func
#include <future>
#include <mutex>
#include <thread>
#include <arpa/inet.h>
#include <iostream>
#include <strings.h>
#include <nlohmann/json.hpp>

#include "ip.h"

using std::mutex, std::string, std::lock_guard, std::cerr, std::endl, std::ifstream, std::thread, std::cout, std::ifstream,
std::exception, std::runtime_error;
using json = nlohmann::json;


void requestProcessing(const int clientSocket, const sockaddr_in& clientAddress) {
    mutex userMutex;
    char receive[1024] = {};
    string sending;
    bool isExit = false;
    while (!isExit) {
        lock_guard<mutex> guard(userMutex);
        bzero(receive, 1024);
        const ssize_t userRead = read(clientSocket, receive, 1024);
        if (userRead <= 0) {
            // cerr << "client[" << clientAddress.sin_addr.s_addr << "] disconnected\n";
            isExit = true;
            continue;
        }
        if constexpr (receive == "disconnect") {
            isExit = true;
            continue;
        }
        send(clientSocket, receive, userRead, 0);
    }
    close(clientSocket);
}


void startServer() {
    const int server = socket(AF_INET, SOCK_STREAM, 0);//file descriptor
    if (server == -1) {
        cerr << "Socket creation error" << endl;
        return;
    }
    sockaddr_in address{}; //IPV4 protocol structure
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("0.0.0.0"); //any = 0.0.0.0
    address.sin_port = htons(PORT);//host to net short
    //close(server);
    if (bind(server, reinterpret_cast<struct sockaddr *>(&address), sizeof(address)) < 0) {
        cerr << "Binding error" << endl;
        return;
    }

    if (listen(server, 10) == -1) {
        cerr << "Socket listening error" << endl;
        return;
    }

    cout << "Server started" << endl;

    sockaddr_in clientAddress{};
    socklen_t clientAddrLen = sizeof(clientAddress);
    while (true){
        int clientSocket = accept(server, reinterpret_cast<struct sockaddr *>(&clientAddress), &clientAddrLen);
        if(clientSocket == -1){
            // cout << "connection fail" << endl;
            continue;
        }
        // cout << "Client[" << clientAddress.sin_addr.s_addr << "] was connected" << endl;
        thread( requestProcessing, clientSocket, clientAddress).detach();
    }
    close(server);
}


int main() {
    startServer();
    return 0;
}