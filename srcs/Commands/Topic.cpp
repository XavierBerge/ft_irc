/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Topic.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/11 16:56:32 by xav               #+#    #+#             */
/*   Updated: 2024/11/12 21:34:08 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

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
	
    // Get rid all of : and whitespaces
    size_t pos = newTopic.find_first_of(":"); 
    if (pos != std::string::npos)
        newTopic = newTopic.substr(pos + 1);  // Ignore all whitespaces before first :

    // Get rid all of :
    while (!newTopic.empty() && newTopic[0] == ':') 
	{
        newTopic = newTopic.substr(1);
    }

    // Remove all whitespaces at the start and the end
    size_t start = newTopic.find_first_not_of(" \t");
    size_t end = newTopic.find_last_not_of(" \t");
    newTopic = (start == std::string::npos) ? "" : newTopic.substr(start, end - start + 1);

    if (newTopic.empty()) 
    {
        if (!channel->getTopic().empty()) 
		{
            clients[client_fd]->sendToClient(":" + servername + " 332 " + clients[client_fd]->getNickname() + " " + channelName + " :" + channel->getTopic() + "\r\n");
            
            std::ostringstream oss;
            oss << channel->getTopicTime(); 
            clients[client_fd]->sendToClient(":" + servername + " 333 " + clients[client_fd]->getNickname() + " " + channelName + " " + channel->getTopicOpe() + " " + oss.str() + "\r\n");
        } 
		else 
            clients[client_fd]->sendToClient(":" + servername + " 331 " + clients[client_fd]->getNickname() + " " + channelName + " :No topic is set\r\n");
        return;
    }

    if (channel->getTopicRight() && !channel->isOperator(clients[client_fd]->getNickname())) 
	{
		std::string errorMsg = ":" + servername + " 482 " + clients[client_fd]->getNickname() + " :You're not a channel operator for " + channelName + "\r\n";
        clients[client_fd]->sendToClient(errorMsg);
        return;
    }
    channel->setTopic(newTopic);
    channel->setTopicOpe(clients[client_fd]->getNickname());
    channel->setTopicTime(time(0));
    std::string response = ":" + clients[client_fd]->getNickname() + " TOPIC " + channelName + " :" + newTopic + "\r\n";
    channel->broadcastMessage(response);
}