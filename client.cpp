#include <arpa/inet.h>
#include <errno.h>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#define EXIT_KEYWORD "exit"

void header()
{
    std::cout << "Welcome to the best chat ever!" << std::endl;
    std::cout << "Type '" EXIT_KEYWORD "' to quit." << std::endl;
    std::cout << std::endl;
    std::cout << "Username must" << std::endl;
    std::cout << " - be 3 min length" << std::endl;
    std::cout << " - be 14 max length" << std::endl;
    std::cout << " - contain only alphanumericals caracters." << std::endl;
    std::cout << std::endl;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Format: client <host> <port>\n");
        return EXIT_FAILURE;
    }

    std::string username;

    header();

    std::cout << "Please, enter your username." << std::endl;
    std::cout << std::endl;

    auto is_username_valid = [&username]() {
        if (username.empty() || username.size() < 3 || username.size() > 14)
        {
            return false;
        }

        for (auto &c : username)
        {
            if (!isalnum(c))
            {
                return false;
            }
        }

        return true;
    };

    while (!is_username_valid())
    {
        std::cout << "Username: ";
        std::cin >> username;

        if (!is_username_valid())
        {
            std::cerr << "Username is invalid." << std::endl;
        }
    }

    auto host = argv[1];
    auto port = atoi(argv[2]);

    auto client = sockaddr_in{
        .sin_family = AF_INET,
        .sin_port = htons(port),
    };

    inet_pton(AF_INET, host, &client.sin_addr);

    auto socked_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (connect(socked_fd, (struct sockaddr *)&client, sizeof(client)) == -1)
    {
        std::cerr << "Error when binding socket: " << errno << std::endl;
        return EXIT_FAILURE;
    }

    send(socked_fd, username.c_str(), username.size(), 0);

    std::string buffer;

    while (1)
    {
        std::cout << username << "> ";
        std::getline(std::cin >> std::ws, buffer);

        if (buffer == EXIT_KEYWORD)
        {
            break;
        }

        send(socked_fd, buffer.c_str(), buffer.size(), 0);
    }

    close(socked_fd);

    return EXIT_SUCCESS;
}
