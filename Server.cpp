#include "Server.hpp"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio>
#include <cerrno>
#include <sstream>


Server::Server(int port, const std::string &password) : password(password)
{
    // AF_INET = IPv4 , SOCK_STREAM = TCP
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    int opt = 1;
    // Option SO_REUSEADDR qui permet de relancer directement le serveur s'il vient d'etre fermer avant qu'il libere son adresse et son port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    // Socket en mode non bloquant = Programme ne bloque pas sur read, write, send, recv s'ils sont impossibles
    if (fcntl(server_fd, F_SETFL, O_NONBLOCK) < 0)
    {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }
    // Configuration adresse IP 
    struct sockaddr_in server_addr; // struct pour info adresse ip client
    memset(&server_addr, 0, sizeof(server_addr)); // init tout a 0 pour eviter valeur random
    server_addr.sin_family = AF_INET; // IpV4
    server_addr.sin_addr.s_addr = INADDR_ANY; // n'importe quel adresse ip locale 
    server_addr.sin_port = htons(port); // port d'ecoute converti en ordre d'octets reseau

    // Bind du socket a l'adresse ip et au port
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind"); 
        exit(EXIT_FAILURE);
    }
    // Activation du socket en ecoute
    if (listen(server_fd, SOMAXCONN) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    // Ajout du socket a poll_fds en 3(0 = stdin, 1 = stdout, 2 = stderr)
    add_poll_fd(server_fd, POLLIN);
}

Server::~Server()
{
    close(server_fd);
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
        delete it->second;
}
// ajoute fd a poll_fds
void Server::add_poll_fd(int fd, short events)
{
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = events;
    pfd.revents = 0;
    poll_fds.push_back(pfd);
}
// supprime un fd a poll_fds
void Server::remove_poll_fd(int fd)
{
    for (std::vector <struct pollfd>:: iterator it = poll_fds.begin(); it != poll_fds.end(); ++it)
    {
        if (it->fd == fd)
        {
            poll_fds.erase(it);
            break;
        }
    }
}
// Boucle principale du serveur 
void Server::run()
{
    while (true)
    {
        // tableau statique avec &poll_fds[0] au lieu de vector, taille tableau, -1 pour attendre a l'infini
        int poll_count = poll(&poll_fds[0], poll_fds.size(), -1);

        if (poll_count < 0)
        {
            perror("poll");
            exit(EXIT_FAILURE);
        }
        // Verification si un evenement a lieu pour chacun des fds 
        for (size_t i = 0; i < poll_fds.size(); ++i)
        {
            // Verification des evenements de lecture
            if(poll_fds[i].revents & POLLIN)
            {
                if (poll_fds[i].fd == server_fd)
                    handle_new_connection();
                else
                    handle_client_data(poll_fds[i].fd);
            }
        
            // Verification des erreurs 
            if (poll_fds[i].revents & (POLLHUP | POLLERR |POLLNVAL))
            {
                if (poll_fds[i].fd != server_fd)
                {
                    std::cout << "Deconnexion ou erreur sur le client " << poll_fds[i].fd << std::endl;
                    close_connection(poll_fds[i].fd);
                }
            }
        }
    }
}
// Nouvelle connexion d'un client
void Server::handle_new_connection()
{
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);

    if (client_fd < 0)
    {
        if (errno != EWOULDBLOCK && errno != EAGAIN)
            perror("accept");
        return; 
    }

    // Utilisation de inet_ntoa() pour obtenir l'adresse IP du client
   std::cout << "\033[1;32m"
              << "New connection accepted : " << client_fd 
              << " from " << inet_ntoa(client_addr.sin_addr) 
              << ":" << ntohs(client_addr.sin_port)
              << "\033[0m"
              << std::endl;

    // Rendre le socket client non-bloquant
    if (fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0)
    {
        perror("fcntl");
        close(client_fd);
        return; 
    }

    // Ajouter le client dans la liste des clients connectés
    Client *new_client = new Client(client_fd);
    clients[client_fd] = new_client;

    // Ajouter le descripteur du client à poll() pour surveiller les événements d'E/S
    add_poll_fd(client_fd, POLLIN);

    // Envoyer un message de demande de mot de passe au client
    std::string welcome_message = "Please enter the server password to get access : PASS <password>\r\n";
    if (send(client_fd, welcome_message.c_str(), welcome_message.size(), 0) < 0)
    {
        perror("send");
        close_connection(client_fd); // Fermer si l'envoi échoue
        return;
    }
}



void Server::close_connection(int client_fd)
{
    close(client_fd);
    remove_poll_fd(client_fd);
    delete clients[client_fd];
    clients.erase(client_fd);
}

void Server::handle_authentication(int client_fd, const std::string& data) 
{
    Client *client = clients[client_fd];
    std::stringstream ss(data);
    std::string line;
    
    while (std::getline(ss, line)) 
    {
        if (!line.empty() && line[line.size() - 1] == '\r') 
            line.erase(line.size() - 1);

        std::cout << "Received command : " << line << std::endl;

        if (line.find("CAP LS") == 0) 
		{
			client->irssi_true();
            continue;
        }

        if (line.find("PASS ") == 0) 
        {
            std::string client_pass = line.substr(5);
            if (client_pass == password) 
            {
				if (!client->isIrssi())
				{
					std::string response = "Correct password. Please enter your nickname : NICK <nickname>.\r\n";
					send(client_fd, response.c_str(), response.size(), 0);
				}
            } 
            else 
            {
                std::cout << "Wrong password. Try to reconnect with the correct password\r\n";
                close_connection(client_fd);
                return;
            }
			client->pass_true();
            continue;
        }

        if (line.find("NICK ") == 0) 
		{
			if (!client->isPass_Ok())
			{
				std::string error = "You must enter the server password before choosing a nickname : PASS <password>.\r\n";
				send(client_fd, error.c_str(), error.size(), 0);
				continue;
			}
            std::string nickname = line.substr(5);

            // Vérifier l'unicité du nickname
            bool nickname_taken = false;
            for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) 
			{
                if (it->second->getNickname() == nickname) 
				{
                    nickname_taken = true;
                    break;
                }
            }

            if (nickname_taken) 
			{
                std::string response = "433 * " + nickname + " :This nickname is already taken. Please choose a different nickname.\r\n";
                send(client_fd, response.c_str(), response.size(), 0);
                continue;
            }

            // Si le nickname est unique, on le définit
            client->setNickname(nickname);
			if (!client->isIrssi())
			{
				std::string response = nickname + " :NICK accepted. Please enter a username: USER <username> <hostname> <servername> :<realname>\r\n";
				send(client_fd, response.c_str(), response.size(), 0);
			}
			client->nick_true();
            continue;
        }

        if (line.find("USER ") == 0) 
		{
			if (!client->isNick_Ok())
			{
				std::string error = "Please first define your nickname : NICK <nickname>.\r\n";
				send(client_fd, error.c_str(), error.size(), 0);
				continue;
			}
            std::cout << "USER accepted for " << client->getNickname() << std::endl;
            client->authenticate();  
            
            std::string response = "001 " + client->getNickname() + " :Bienvenue sur le serveur IRC!\r\n";
            send(client_fd, response.c_str(), response.size(), 0);
            response = "002 " + client->getNickname() + " :Votre hôte est localhost, version 1.0\r\n";
            send(client_fd, response.c_str(), response.size(), 0);
            response = "003 " + client->getNickname() + " :Ce serveur a été créé aujourd'hui\r\n";
            send(client_fd, response.c_str(), response.size(), 0);
            response = "004 " + client->getNickname() + " localhost 1.0 iowghraAsORTVSx\r\n";
            send(client_fd, response.c_str(), response.size(), 0);
            response = "375 " + client->getNickname() + " :- Message du jour -\r\n";
            send(client_fd, response.c_str(), response.size(), 0);
            response = "372 " + client->getNickname() + " :- Bienvenue sur ce serveur IRC !\r\n";
            send(client_fd, response.c_str(), response.size(), 0);
            response = "376 " + client->getNickname() + " :Fin du message du jour\r\n";
            send(client_fd, response.c_str(), response.size(), 0);

            std::cout << "Client " << client_fd << " authentified with success." << std::endl;
            return;
        }
        std::string response = "In authentification process PASS, NICK and USER must be defined, command : " + line + " is not valid.\r\n";
        send(client_fd, response.c_str(), response.size(), 0);
    }
}

/*
void Server::send_welcome_message(int client_fd, const Client& client)
{
    std::string host = "localhost";  // Ou autre nom d'hôte du serveur
    std::string nick = client.getNickname();
    std::string user = client.getUsername();

    // RPL_WELCOME
    std::string welcomeMessage = ":" + host + " 001 " + nick + " :Welcome to the Internet Relay Network " + nick + "!" + user + "@localhost\r\n";
    send(client_fd, welcomeMessage.c_str(), welcomeMessage.size(), 0);

    // RPL_YOURHOST
    welcomeMessage = ":" + host + " 002 " + nick + " :Your host is localhost, running version 1.0\r\n";
    send(client_fd, welcomeMessage.c_str(), welcomeMessage.size(), 0);

    // RPL_CREATED
    welcomeMessage = ":" + host + " 003 " + nick + " :This server was created on Oct 24, 2024\r\n";
    send(client_fd, welcomeMessage.c_str(), welcomeMessage.size(), 0);

    // RPL_MYINFO
    welcomeMessage = ":" + host + " 004 " + nick + " localhost 1.0 iowghraAsORTVSx NC\r\n";
    send(client_fd, welcomeMessage.c_str(), welcomeMessage.size(), 0);

    // RPL_MOTDSTART, RPL_MOTD, RPL_ENDOFMOTD (Message of the Day)
    welcomeMessage = ":" + host + " 375 " + nick + " :- Message of the day -\r\n";
    send(client_fd, welcomeMessage.c_str(), welcomeMessage.size(), 0);
    welcomeMessage = ":" + host + " 372 " + nick + " :- Welcome to this IRC server!\r\n";
    send(client_fd, welcomeMessage.c_str(), welcomeMessage.size(), 0);
    welcomeMessage = ":" + host + " 376 " + nick + " :End of /MOTD command\r\n";
    send(client_fd, welcomeMessage.c_str(), welcomeMessage.size(), 0);
}
*/


void Server::handle_client_data(int client_fd) 
{
    Client *client = clients[client_fd];
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    // Lire les données envoyées par le client
    ssize_t bytes_received = client->readFromClient(buffer, sizeof(buffer));

    if (bytes_received <= 0) 
    {
        // Si on ne reçoit rien ou si la connexion est fermée
        if (bytes_received == 0) 
		{
            std::cout << "Client " << client_fd << " shut down the connection." << std::endl;
        } 
		else 
            perror("recv");
        close_connection(client_fd);
    } 
    else 
    {
        // Convertir les données en une chaîne de caractères
        std::string command(buffer);
        command.erase(command.find_last_not_of("\r\n") + 1);  // Supprimer les éventuels caractères de nouvelle ligne

        std::cout << "Received from client " << client_fd << ": " << command << std::endl;

        // Si le client n'est pas encore authentifié, gérer l'authentification
        if (!client->isAuthenticated()) 
            handle_authentication(client_fd, command);
        // Sinon, gérer les commandes classiques après authentification
        else 
		{
            client->handleCommand(command);

            if (command.find("/quit") == 0) 
                close_connection(client_fd);
        }

    }
}


