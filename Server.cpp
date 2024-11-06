/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/03 13:11:00 by xav               #+#    #+#             */
/*   Updated: 2024/11/06 10:28:50 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

 bool Server::Signal = false;

void Server::initializeCommandMap() 
{
    commandMap["JOIN"] = &Server::handleJoin;
    commandMap["PRIVMSG"] = &Server::handlePrivmsg;
    commandMap["KICK"] = &Server::handleKick;
    commandMap["INVITE"] = &Server::handleInvite;
    commandMap["TOPIC"] = &Server::handleTopic;
    commandMap["NICK"] = &Server::handleNick;
    commandMap["MODE"] = &Server::handleMode;
    commandMap["PART"] = &Server::handlePart;
    commandMap["QUIT"] = &Server::handleQuit;
    commandMap["PING"] = &Server::handlePing;
}

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
	initializeCommandMap();
}

Server::~Server() 
{
	for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) 
	{
    	delete it->second;
	}
	channels.clear();

    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) 
	{
        close(it->first);
        delete it->second;
    }
    clients.clear();
    if (server_fd >= 0) 
        close(server_fd);
	
}

std::string Server::getHostname() const
{
	return hostname;
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
	
	for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) 
	{
    	delete it->second;
	}
	channels.clear();

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

    std::cout << "\nSignal received." << std::endl;
}
/*
void Server::handleJoinCommand(int client_fd, const std::string &channelName) 
{
    if (channelName.empty()) 
    {
        clients[client_fd]->sendToClient("461 " + clients[client_fd]->getNickname() + " JOIN :Not enough parameters\r\n");
        return;
    }
	bool first_to_join = false;
    if (channels.find(channelName) == channels.end()) 
    {
        // Créer un nouveau channel s'il n'existe pas
        channels[channelName] = new Channel(channelName, *this);
        std::cout << "Channel " << channelName << " créé." << std::endl;
		first_to_join = true;
    }

    Channel *channel = channels[channelName];
    if (channel->addClient(client_fd)) 
    {
        std::string joinMessage = ":" + clients[client_fd]->getNickname() + " JOIN " + channelName + "\r\n";
        channel->broadcastMessage(joinMessage, client_fd);
        clients[client_fd]->sendToClient("Vous avez rejoint " + channelName + "\r\n");
		if (first_to_join)
			channel->promoteToOperator(client_fd);
    } 
    else 
    {
        clients[client_fd]->sendToClient("Vous êtes déjà dans " + channelName + "\r\n");
    }
}
*/

void Server::close_connection(int client_fd)
{
    close(client_fd);
    remove_poll_fd(client_fd);
    delete clients[client_fd];
    clients.erase(client_fd);
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

        // Si le client n'est pas encore authentifié, gérer l'authentification
        if (!client->isAuthenticated()) 
            handle_authentication(client_fd, command);
        // Sinon, gérer les commandes classiques après authentification
        else 
		{
            handleCommand(client_fd, command);

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


