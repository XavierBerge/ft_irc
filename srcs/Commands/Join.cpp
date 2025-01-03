/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Join.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/11 16:51:14 by xav               #+#    #+#             */
/*   Updated: 2024/11/11 16:53:29 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"


void Server::handleJoin(int client_fd, const std::string& command) 
{
    size_t spacePos = command.find(" ");
    std::string channelName = command.substr(5, spacePos - 5);  
    std::string password;
    
    if (spacePos != std::string::npos)
        password = command.substr(spacePos + 1);  // Get password if password

    // Remove whitespaces at start and end
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

    
    if (channel->getInvited() && !clients[client_fd]->isInvitedToChannel(channelName)) 
	{
        clients[client_fd]->sendToClient("473 " + nickname + " " + channelName + " :Cannot join channel (invite only)\r\n");
        return;
    }

    
    if (channel->getKeyNeeded() && (password.empty() || password != channel->getKey())) 
	{
        clients[client_fd]->sendToClient("475 " + nickname + " " + channelName + " :Cannot join channel (incorrect channel key)\r\n");
        return;
    }

    
    if (channel->addClient(nickname, client_fd)) 
	{
		clients[client_fd]->addChannel(channelName);
        std::string joinMessage = ":" + clients[client_fd]->getNickname() + "!" + clients[client_fd]->getUsername() + "@" + clients[client_fd]->getHostname() + " JOIN " + channelName + "\r\n";
        if (!firstToJoin)
            channel->broadcastMessage(joinMessage, client_fd);

        clients[client_fd]->sendToClient("You joined " + channelName + "\r\n");

        if (firstToJoin) 
		{
            channel->promoteToOperator(nickname, client_fd);
            std::string modemsg = ":" + servername + " MODE " + channelName + " +o " + nickname + "\r\n";
            channel->broadcastMessage(modemsg);
        }

        if (!firstToJoin)
            sendChannelTopic(client_fd, channel);  // Send topic if a topic is set
    } 
    else 
        clients[client_fd]->sendToClient("You already joined " + channelName + "\r\n");
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