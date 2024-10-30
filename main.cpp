#include "Server.hpp"
#include <cstdlib>
#include <iostream>

int main(int argc, char* argv[]) 
{
    if (argc != 3) 
    {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return EXIT_FAILURE;
    }

    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) 
    {
        std::cerr << "Port invalide. Choisissez un port entre 1 et 65535." << std::endl;
        return EXIT_FAILURE;
    }
    std::string password = argv[2];
    Server server(port, password);
    server.run();
    return EXIT_SUCCESS;
}
