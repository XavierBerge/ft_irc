/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Invite.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/11 17:03:35 by xav               #+#    #+#             */
/*   Updated: 2024/11/11 17:03:53 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

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
        std::string errorMsg = ":" + servername + " 482 " + clients[client_fd]->getNickname() + " :You're not a channel operator for " + channelName + "\r\n";
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
    std::string inviteMessage = ":" + inviterNickname + " INVITE " + targetNickname + " :" + channelName;
    if (channel->getKeyNeeded()) 
    {
        inviteMessage += " (Password: " + channel->getKey() + ")";
    }
    inviteMessage += "\r\n";
    targetClient->sendToClient(inviteMessage);
}
