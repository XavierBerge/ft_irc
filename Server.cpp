/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/03 13:11:00 by xav               #+#    #+#             */
/*   Updated: 2024/11/03 13:11:01 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

 bool Server::Signal = false;

Server::Server(int port, const std::string &password) : password(password), hostname("client.host"), servername("irc.tower.com")
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
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
        close(it->first);
        delete it->second;
    }
    clients.clear();
    if (server_fd >= 0) 
        close(server_fd);
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
    while (!Server::Signal)  // Boucle tant qu'aucun signal d'arrêt n'est capté
    {
        // Appel à poll, vérification de la taille et gestion des signaux
        int poll_count = poll(&poll_fds[0], poll_fds.size(), -1);

        if (poll_count < 0) 
		{
            if (errno == EINTR) 
			{
                // Si poll est interrompu par un signal, sortir de la boucle
                break;
            } else 
			{
                perror("poll");
                exit(EXIT_FAILURE);
            }
        }

        // Parcours des descripteurs pour vérifier les événements
        for (size_t i = 0; i < poll_fds.size(); ++i) 
		{
            // Gestion des événements de lecture
            if (poll_fds[i].revents & POLLIN) 
			{
                if (poll_fds[i].fd == server_fd)
                    handle_new_connection();  // Gère une nouvelle connexion au serveur
                else
                    handle_client_data(poll_fds[i].fd);  // Gère les données d'un client existant
            }

            // Gestion des erreurs et des déconnexions
            if (poll_fds[i].revents & (POLLHUP | POLLERR | POLLNVAL)) 
			{
                if (poll_fds[i].fd != server_fd) 
				{
                    std::cout << "Deconnexion ou erreur sur le client " << poll_fds[i].fd << std::endl;
                    close_connection(poll_fds[i].fd);  // Ferme la connexion du client en cas d'erreur
                }
            }
        }
    }

    // Fermer tous les sockets des clients et libérer la mémoire associée
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) 
	{
        close(it->first); // Ferme le descripteur de fichier du client
        delete it->second; // Libère la mémoire associée à chaque objet Client
    }
    clients.clear(); // Vider la map des clients

    // Fermer le socket du serveur si toujours ouvert
    if (server_fd >= 0) 
	{
        close(server_fd);
    }

    std::cout << "Signal received." << std::endl;
}

// Nouvelle connexion d'un client
void Server::handle_new_connection()
{
    // Vérification si la limite de connexions est atteinte
    if (clients.size() >= MAX_CLIENTS)
    {
        // Accepter temporairement la connexion pour envoyer un message de refus
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int temp_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);

        if (temp_fd >= 0) 
        {
            std::string full_message = "Server is full. Connection refused.\r\n";
            send(temp_fd, full_message.c_str(), full_message.size(), 0);
            close(temp_fd); // Fermer immédiatement le descripteur
        }
        return; // Retourner sans ajouter de client
    }

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

void Server::send_welcome_messages(Client *client)
{
    std::string nickname = client->getNickname();
    std::string username = client->getUsername();
    std::string realname = client->getRealname();
    std::string hostname = this->hostname;
    std::string servername = this->servername;
    
    std::string response = "001 " + nickname + " :Welcome to the Internet Relay Network " +
                           nickname + "!" + username + "@" + hostname + "\r\n";
    client->sendToClient(response);

    response = "002 " + nickname + " :Your host is " + servername + ", running version 1.0\r\n";
    client->sendToClient(response);

    std::time_t now = std::time(NULL);
    std::tm* local_time = std::localtime(&now);

    char date_str[100];
    std::strftime(date_str, sizeof(date_str), "%a %b %d %Y at %H:%M", local_time);

    response = "003 " + nickname + " :This server was created " + date_str + "\r\n";
    client->sendToClient(response);

    response = "004 " + nickname + " " + servername + " 1.0 iowghraAsORTVSx \r\n";
    client->sendToClient(response);

    response = "375 " + nickname + " :- " + servername + " Message of the day -\r\n";
    client->sendToClient(response);

    response = "372 " + nickname + " :- Welcome to the Tower IRC server!\r\n";
    client->sendToClient(response);

    response = "376 " + nickname + " :End of /MOTD command\r\n";
    client->sendToClient(response);
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
    		std::istringstream iss(line);
			std::string command;
			
			iss >> command;
			std::string username, hostname, servername, realname;

			iss >> username >> hostname >> servername;

			std::getline(iss, realname);
			
			realname.erase(0, realname.find_first_not_of(" \t"));
    		realname.erase(realname.find_last_not_of(" \t") + 1);
			if (!realname.empty() && realname[0] == ':')
				realname = realname.substr(1);
			
			if (username.empty() || hostname.empty() || servername.empty() || realname.empty())
			{
				std::string error = "461 " + client->getNickname() + " USER :Not enough parameters\r\n";
				send(client_fd, error.c_str(), error.size(), 0);
				continue;
			}

			client->setUsername(username);
    		client->setRealname(realname);
            client->authenticate();
			send_welcome_messages(client);
            return;
        }
        std::string response = "In authentification process PASS, NICK and USER must be defined, command : " + line + " is not valid.\r\n";
        send(client_fd, response.c_str(), response.size(), 0);
    }
}




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

void Server::SignalHandler(int signum)
{
	(void)signum;
	Server::Signal = true;
}


