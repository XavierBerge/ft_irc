/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 13:26:27 by xav               #+#    #+#             */
/*   Updated: 2024/11/11 10:14:31 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <algorithm> 

void Server::handleJoin(int client_fd, const std::string& command) 
{
    // Extraire le nom du canal et le mot de passe si fourni
    size_t spacePos = command.find(" ");
    std::string channelName = command.substr(5, spacePos - 5);  // Extrait le nom du canal après "JOIN "
    std::string password;
    
    if (spacePos != std::string::npos)
        password = command.substr(spacePos + 1);  // Mot de passe facultatif

    // Trim des espaces du début et de la fin
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

    // Vérification du mode invitation
    if (channel->getInvited() && !clients[client_fd]->isInvitedToChannel(channelName)) 
	{
        clients[client_fd]->sendToClient("473 " + nickname + " " + channelName + " :Cannot join channel (invite only)\r\n");
        return;
    }

    // Vérification du mot de passe
    if (channel->getKeyNeeded() && (password.empty() || password != channel->getKey())) 
	{
        clients[client_fd]->sendToClient("475 " + nickname + " " + channelName + " :Cannot join channel (incorrect channel key)\r\n");
        return;
    }

    // Ajouter le client au canal
    if (channel->addClient(nickname, client_fd)) 
	{
		clients[client_fd]->addChannel(channelName);
        std::string joinMessage = ":" + clients[client_fd]->getNickname() + "!" + clients[client_fd]->getUsername() + "@" + clients[client_fd]->getHostname() + " JOIN " + channelName + "\r\n";
        if (!firstToJoin)
            channel->broadcastMessage(joinMessage, client_fd);

        clients[client_fd]->sendToClient("You joined " + channelName + "\r\n");

        if (firstToJoin) 
		{
            // Si le client est le premier à rejoindre, il devient opérateur
            channel->promoteToOperator(nickname, client_fd);
            std::string modemsg = ":" + servername + " MODE " + channelName + " +o " + nickname + "\r\n";
            channel->broadcastMessage(modemsg);
        }

        if (!firstToJoin)
            sendChannelTopic(client_fd, channel);  // Envoie le sujet du canal si existant
    } 
    else 
        clients[client_fd]->sendToClient("You already joined " + channelName + "\r\n");
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
    std::istringstream iss(command);
    std::string cmd, channelName, targetNickname;
    iss >> cmd >> channelName >> targetNickname;

    if (channels.find(channelName) == channels.end()) 
	{
        clients[client_fd]->sendToClient("403 " + clients[client_fd]->getNickname() + " " + channelName + " :No such channel\r\n");
        return;
    }

    Channel *channel = channels[channelName];
    std::string nickname = clients[client_fd]->getNickname();


    if (!channel->isOperator(nickname)) 
	{
        clients[client_fd]->sendToClient("482 " + nickname + " " + channelName + " :You're not channel operator\r\n");
        return;
    }

    if (!channel->isClientInChannel(targetNickname)) 
	{
        clients[client_fd]->sendToClient("441 " + nickname + " " + channelName + " " + targetNickname + " :They aren't on that channel\r\n");
        return;
    }

    Client* targetClient = NULL;
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) 
    {
        if (it->second->getNickname() == targetNickname) 
        {
            targetClient = it->second;
            break;
        }
    }

    if (!targetClient) 
	{
        clients[client_fd]->sendToClient("401 " + nickname + " " + targetNickname + " :No such nick/channel\r\n");
        return;
    }

    std::string kickMessage = ":" + nickname + "!" + clients[client_fd]->getUsername() + "@" + clients[client_fd]->getHostname() + " KICK " + channelName + " " + targetNickname + "\r\n";
    channel->broadcastMessage(kickMessage);

    channel->removeClient(targetNickname);

    if (channel->isOperator(targetNickname)) 
        channel->removeOperator(targetNickname);

    targetClient->sendToClient(":" + servername + " 404 " + targetNickname + " " + channelName + " :You have been kicked from the channel\r\n");
}


void Server::handleInvite(int client_fd, const std::string& command) 
{
    std::istringstream iss(command);
    std::string cmd, targetNickname, channelName;

    iss >> cmd >> targetNickname >> channelName;

    if (targetNickname.empty() || channelName.empty()) 
    {
        clients[client_fd]->sendToClient("461 " + clients[client_fd]->getNickname() + " INVITE :Not enough parameters\r\n");
        return;
    }

    if (channels.find(channelName) == channels.end()) 
    {
        clients[client_fd]->sendToClient("403 " + clients[client_fd]->getNickname() + " " + channelName + " :No such channel\r\n");
        return;
    }

    Channel* channel = channels[channelName];
    std::string inviterNickname = clients[client_fd]->getNickname();

    if (!channel->isOperator(inviterNickname)) 
    {
        clients[client_fd]->sendToClient("482 " + inviterNickname + " " + channelName + " :You're not channel operator\r\n");
        return;
    }

    Client* targetClient = NULL;
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) 
    {
        if (it->second->getNickname() == targetNickname) 
        {
            targetClient = it->second;
            break;
        }
    }

    if (targetClient == NULL) 
    {
        clients[client_fd]->sendToClient("401 " + inviterNickname + " " + targetNickname + " :No such nick\r\n");
        return;
    }

    if (channel->isClientInChannel(targetNickname)) 
    {
        clients[client_fd]->sendToClient("443 " + inviterNickname + " " + targetNickname + " " + channelName + " :is already on channel\r\n");
        return;
    }

    targetClient->addChannelInvitation(channelName);

    clients[client_fd]->sendToClient("341 " + inviterNickname + " " + targetNickname + " " + channelName + "\r\n");
    targetClient->sendToClient(":" + inviterNickname + " INVITE " + targetNickname + " :" + channelName + "\r\n");
}


void Server::handleTopic(int client_fd, const std::string& command) 
{
    std::istringstream iss(command);
    std::string mode, channelName, newTopic;

    iss >> mode >> channelName;

    if (channels.find(channelName) == channels.end()) 
	{
        clients[client_fd]->sendToClient(":" + servername + " 403 " + clients[client_fd]->getNickname() + " " + channelName + " :No such channel\r\n");
        return;
    }

    Channel* channel = channels[channelName];
    std::getline(iss, newTopic);

    // Supprimer tous les caractères `:` au début, puis les espaces blancs
    size_t pos = newTopic.find_first_of(":"); 
    if (pos != std::string::npos) {
        newTopic = newTopic.substr(pos + 1);  // Ignorer tout avant (et y compris) le premier ':'
    }

    // Supprimer les `:` restants au début de la chaîne
    while (!newTopic.empty() && newTopic[0] == ':') 
	{
        newTopic = newTopic.substr(1);
    }

    // Supprimer les espaces blancs au début et à la fin
    size_t start = newTopic.find_first_not_of(" \t");
    size_t end = newTopic.find_last_not_of(" \t");
    newTopic = (start == std::string::npos) ? "" : newTopic.substr(start, end - start + 1);


    if (newTopic.empty()) 
    {
        if (!channel->getTopic().empty()) 
		{
            clients[client_fd]->sendToClient(":" + servername + " 332 " + clients[client_fd]->getNickname() + " " + channelName + " :" + channel->getTopic() + "\r\n");
            
            std::ostringstream oss;
            oss << channel->getTopicTime();  // Convertit le timestamp en string avec un ostringstream
            clients[client_fd]->sendToClient(":" + servername + " 333 " + clients[client_fd]->getNickname() + " " + channelName + " " + channel->getTopicOpe() + " " + oss.str() + "\r\n");
        } 
		else 
            clients[client_fd]->sendToClient(":" + servername + " 331 " + clients[client_fd]->getNickname() + " " + channelName + " :No topic is set\r\n");
        return;
    }


    if (channel->getTopicRight() && !channel->isOperator(clients[client_fd]->getNickname())) 
	{
        clients[client_fd]->sendToClient(":" + servername + " 482 " + clients[client_fd]->getNickname() + " " + channelName + " :You're not channel operator\r\n");
        return;
    }

    channel->setTopic(newTopic);
    channel->setTopicOpe(clients[client_fd]->getNickname());
    channel->setTopicTime(time(0));


    std::string response = ":" + clients[client_fd]->getNickname() + " TOPIC " + channelName + " :" + newTopic + "\r\n";
    channel->broadcastMessage(response);
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

    channel->removeClient(nickname);
    std::string fullPartMessage = ":" + clients[client_fd]->getNickname() + " PART " + channelName;
    if (!partMessage.empty()) 
    {
        fullPartMessage += " :" + partMessage;
    }
    fullPartMessage += "\r\n";

    channel->broadcastMessage(fullPartMessage);

	clients[client_fd]->removeChannel(channelName);
    clients[client_fd]->sendToClient(":" + servername + " You have left " + channelName + "\r\n");

    if (channel->isEmpty()) 
    {
        delete channel;
        channels.erase(channelName);
    }
}


void Server::handlePing(int client_fd, const std::string& command) 
{
    std::string response = "PONG :";
    std::string param = command.substr(5);
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

	removeClientFromAllChannels(client_fd);
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

void Server::sendChannelTopic(int client_fd, Channel* channel) 
{
    std::string nickname = clients[client_fd]->getNickname();
    std::string channelName = channel->getName();
    
    if (!channel->getTopic().empty()) 
	{
        clients[client_fd]->sendToClient(":" + servername + " 332 " + nickname + " " + channelName + " :" + channel->getTopic() + "\r\n");
        std::ostringstream oss;
        oss << channel->getTopicTime();
        clients[client_fd]->sendToClient(":" + servername + " 333 " + nickname + " " + channelName + " " + channel->getTopicOpe() + " " + oss.str() + "\r\n");
    } 
}