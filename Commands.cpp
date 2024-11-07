/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 13:26:27 by xav               #+#    #+#             */
/*   Updated: 2024/11/07 16:42:20 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <algorithm> 

void Server::handleJoin(int client_fd, const std::string& command) 
{
    std::string channelName = command.substr(5);

    size_t start = 0;
    while (start < channelName.size() && isspace(channelName[start])) 
        ++start;
    size_t end = channelName.size();
    while (end > start && isspace(channelName[end - 1])) 
        --end;
    channelName = channelName.substr(start, end - start);
    
    if (channelName.empty()) 
    {
        clients[client_fd]->sendToClient("461 " + clients[client_fd]->getNickname() + " JOIN :Not enough parameters\r\n");
        return;
    }

    bool firstToJoin = false;
    if (channels.find(channelName) == channels.end())
    {
        channels[channelName] = new Channel(channelName);
        firstToJoin = true;
    }

    Channel *channel = channels[channelName];
    std::string nickname = clients[client_fd]->getNickname();

    if (channel->addClient(nickname, client_fd))
    {
        std::string joinMessage = ":" + clients[client_fd]->getNickname() + "!" + clients[client_fd]->getUsername() + "@" + clients[client_fd]->getHostname() + " JOIN " + channelName + "\r\n";
        if (!firstToJoin)
			channel->broadcastMessage(joinMessage, client_fd);
        clients[client_fd]->sendToClient("You joined " + channelName + "\r\n");

        if (firstToJoin)
		{
			
            channel->promoteToOperator(nickname, client_fd); // Utilise le nickname pour promouvoir l'opérateur
			std::string modemsg = ":" + servername + " MODE " + channelName + " +o " + nickname + "\r\n";
			channel->broadcastMessage(modemsg);
		}
    } 
    else 
    {
        clients[client_fd]->sendToClient("You already joined " + channelName + "\r\n");
    }
}


void Server::handlePrivmsg(int client_fd, const std::string& command) 
{
    std::istringstream iss(command);
    std::string cmd, chOrNick, msg;
    iss >> cmd >> chOrNick;
    std::getline(iss, msg);

    // Supprime le premier espace de `msg` s'il existe
    if (!msg.empty() && msg[0] == ' ') 
        msg = msg.substr(1);  

    // Vérifie si `msg` commence déjà par `:`
    if (msg[0] != ':')
        msg = ":" + msg;

    // Vérifie si `chOrNick` est un canal
    if (channels.find(chOrNick) != channels.end()) 
    {
        Channel *channel = channels[chOrNick];

        // Vérifie que le client est bien membre du channel
        std::string nickname = clients[client_fd]->getNickname();
        if (!channel->isClientInChannel(nickname)) 
        {
            clients[client_fd]->sendToClient("442 " + nickname + " " + chOrNick + " :You're not on that channel\r\n");
            return;
        }

        if (!msg.empty()) 
        {
            std::string formattedMessage = ":" + nickname + 
                                           " PRIVMSG " + chOrNick + " " + msg + "\r\n";
            channel->broadcastMessage(formattedMessage, client_fd);
        } 
        else 
            clients[client_fd]->sendToClient("412 " + nickname + " :No text to send\r\n");
    } 
    // Sinon, on traite `chOrNick` comme un nickname pour un message privé
    else 
    {
        // Recherche le client avec le nickname spécifié
        Client* targetClient = NULL;
        for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) 
        {
            if (it->second->getNickname() == chOrNick) 
            {
                targetClient = it->second;
                break;
            }
        }

        if (targetClient) 
        {
            if (!msg.empty()) 
            {
                std::string formattedMessage = ":" + clients[client_fd]->getNickname() + 
                                               " PRIVMSG " + chOrNick + " " + msg + "\r\n";

                targetClient->sendToClient(formattedMessage);
            } 
            else 
            {
                clients[client_fd]->sendToClient("412 " + clients[client_fd]->getNickname() + " :No text to send\r\n");
            }
        } 
        else 
            clients[client_fd]->sendToClient("401 " + clients[client_fd]->getNickname() + 
                                            " " + chOrNick + " :No such nick/channel\r\n");
    }
}



void Server::handleKick(int client_fd, const std::string& command) 
{
	(void) client_fd;
    std::cout << "handleKick called with command: " << command << std::endl;
}

void Server::handleInvite(int client_fd, const std::string& command) 
{
	(void) client_fd;
    std::cout << "handleInvite called with command: " << command << std::endl;
}

void Server::handleTopic(int client_fd, const std::string& command) 
{
	(void) client_fd;
    std::cout << "handleTopic called with command: " << command << std::endl;
}

void Server::handlePart(int client_fd, const std::string& command) 
{
    std::istringstream iss(command);
    std::string cmd, channelName, partMessage;

    iss >> cmd >> channelName;

    if (channelName.empty()) 
    {
        clients[client_fd]->sendToClient("461 " + clients[client_fd]->getNickname() + " PART :Not enough parameters\r\n");
        return;
    }

    std::getline(iss, partMessage);
    if (!partMessage.empty() && partMessage[0] == ' ')
        partMessage = partMessage.substr(1);

    if (!partMessage.empty() && partMessage[0] == ':')
        partMessage = partMessage.substr(1);

    if (channels.find(channelName) == channels.end()) 
    {
        clients[client_fd]->sendToClient("403 " + clients[client_fd]->getNickname() + " " + channelName + " :No such channel\r\n");
        return;
    }

    Channel *channel = channels[channelName];
    std::string nickname = clients[client_fd]->getNickname();
    
    if (!channel->isClientInChannel(nickname)) 
    {
        clients[client_fd]->sendToClient("442 " + nickname + " " + channelName + " :You're not on that channel\r\n");
        return;
    }

    if (channel->isOperator(nickname)) 
        channel->removeOperator(nickname);

    // Retirer le client du channel
    channel->removeClient(nickname);

    // Envoi du message PART aux autres utilisateurs du channel
    std::string fullPartMessage = ":" + clients[client_fd]->getNickname() + " PART " + channelName;
    if (!partMessage.empty()) 
    {
        fullPartMessage += " :" + partMessage;
    }
    fullPartMessage += "\r\n";

    // Envoyer ce message à tous les autres membres du channel
    channel->broadcastMessage(fullPartMessage);

    // Envoyer une notification au client qui quitte
    clients[client_fd]->sendToClient(":" + servername + " You have left " + channelName + "\r\n");

    // Vérifier si le channel est vide
    if (channel->isEmpty()) 
    {
        // Si le channel est vide, supprimer le channel
        delete channel;
        channels.erase(channelName);
    }
}


void Server::handlePing(int client_fd, const std::string& command) 
{
    std::string response = "PONG :";
    std::string param = command.substr(5);  // Extraction du paramètre après "PING "
    response += param + "\r\n";
    send(client_fd, response.c_str(), response.size(), 0);
}

void Server::handleQuit(int client_fd, const std::string& command) 
{
	(void) command;
    // Récupérer le client associé au client_fd
    Client* client = clients[client_fd];

    // Vérifier si le client existe et récupérer le nickname
    std::string nickname = (client && client->isNick_Ok()) ? client->getNickname() : "Unknown";

    // Afficher le message de déconnexion avec le nickname
    std::cout << "Client " << nickname << " has quit the server.\n";

    // Supprimer le client de la map et fermer la connexion
    close_connection(client_fd);
}

void Server::handleNick(int client_fd, const std::string& command) 
{
    Client* client = clients[client_fd];


    // Vérifier si le client a passé le mot de passe
    if (!client->isPass_Ok()) 
	{
        std::string error = "You must enter the server password before choosing a nickname : PASS <password>.\r\n";
        send(client_fd, error.c_str(), error.size(), 0);
        return;
    }

    // Récupérer le nickname depuis la commande
    std::string nickname = command.substr(5);

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
        std::string response = ":" + servername + " 433 " + nickname + " " + nickname + " :Nickname is already in use\r\n";
        send(client_fd, response.c_str(), response.size(), 0);
        return;
    }

    // Définir le nickname s'il est unique
    client->setNickname(nickname);
	if (!client->isAuthenticated())
	{
		if (!client->isIrssi()) 
		{
			std::string response = nickname + " :NICK accepted. Please enter a username: USER <username> <hostname> <servername> :<realname>\r\n";
			send(client_fd, response.c_str(), response.size(), 0);
		}
		client->nick_true();
	}
}