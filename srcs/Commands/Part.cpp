/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Part.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/11 16:55:08 by xav               #+#    #+#             */
/*   Updated: 2024/11/12 21:50:51 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

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

    channel->removeClient(nickname);
    if (channel->isOperator(nickname))
	{
        channel->removeOperator(nickname);
		if (!channel->hasOperators()) 
            channel->promoteNextOperator(client_fd);
	} 

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