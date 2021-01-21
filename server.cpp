#include <arpa/inet.h>
#include <cerrno>
#include <iostream>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Format: server <port>\n");
        return EXIT_FAILURE;
    }

    auto port = atoi(argv[1]);
    auto server_socket = sockaddr_in{
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = INADDR_ANY,
    };

    auto socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (bind(socket_fd, reinterpret_cast<sockaddr *>(&server_socket), sizeof(server_socket)) == -1)
    {
        std::cerr << "Error when binding socket: " << errno << std::endl;
        return EXIT_FAILURE;
    }

    if (listen(socket_fd, 40) == -1)
    {
        std::cerr << "Error: " << errno << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Server listening on port " << port << std::endl;

    sockaddr_in client_socket;
    auto client_socket_size = sizeof(client_socket);

    int client_fd = 0;
    char buffer[256] = {0};

    while ((client_fd = accept(socket_fd, reinterpret_cast<sockaddr *>(&client_socket), reinterpret_cast<socklen_t *>(&client_socket_size))))
    {
        std::string username;

        auto pid = fork();
        if (pid == 0)
        {
            while (recv(client_fd, buffer, 255, 0) > 0)
            {
                if (username.empty())
                {
                    auto is_username_valid = strlen(buffer) >= 3 || strlen(buffer) <= 14;

                    if (!is_username_valid)
                    {
                        for (auto cur = buffer; *cur != '\0'; cur++)
                        {
                            if (!isalnum(*cur))
                            {
                                is_username_valid = false;
                                break;
                            }
                        }
                    }

                    if (!is_username_valid)
                    {
                        break;
                    }

                    username = buffer;
                    std::cout << "[info]> " << username << " has join the chat" << std::endl;
                }
                else
                {
                    std::cout << username << "> " << buffer << std::endl;
                }

                memset(buffer, 0, 256);
            }

            if (!username.empty())
            {
                std::cout << "[info]> " << username << " has quit the chat" << std::endl;
            }

            close(socket_fd);
            close(client_fd);

            return EXIT_SUCCESS;
        }
    }

    close(socket_fd);
    return EXIT_SUCCESS;
}