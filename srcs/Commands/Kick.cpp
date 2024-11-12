/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Kick.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/11 17:02:31 by xav               #+#    #+#             */
/*   Updated: 2024/11/11 17:02:57 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

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
        std::string errorMsg = ":" + servername + " NOTICE " + clients[client_fd]->getNickname() + " :You're not a channel operator for " + channelName + "\r\n";
        clients[client_fd]->sendToClient(errorMsg);
        return;
    }

    if (!channel->isClientInChannel(targetNickname)) 
    {
        std::string errorMsg = ":" + servername + " 441 " + nickname + " " + targetNickname + " " + channelName + " :Client is not on that channel\r\n";
        clients[client_fd]->sendToClient(errorMsg);
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