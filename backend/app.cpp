#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[30000] = {0};

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Attach socket to port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;   // 🔥 CRITICAL FIX
    address.sin_port = htons(8080);

    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening
    if (listen(server_fd, 10) < 0) {
        perror("Listen");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on port 8080..." << std::endl;

    while (true) {
        client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (client_socket < 0) {
            perror("Accept");
            continue;
        }

        read(client_socket, buffer, 30000);

        // Get container hostname
        char hostname[1024];
        gethostname(hostname, 1024);

        std::string response_body = "Hello from backend container: ";
        response_body += hostname;

        std::string http_response =
            "HTTP/1.1 200 OK\n"
            "Content-Type: text/plain\n"
            "Content-Length: " + std::to_string(response_body.length()) + "\n"
            "Connection: close\n"
            "\n" +
            response_body;

        send(client_socket, http_response.c_str(), http_response.length(), 0);

        std::cout << "Served by " << hostname << std::endl;

        close(client_socket);
    }

    return 0;
}
