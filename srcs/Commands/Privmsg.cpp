/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Privmsg.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/11 16:52:02 by xav               #+#    #+#             */
/*   Updated: 2024/11/11 16:52:28 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::handlePrivmsg(int client_fd, const std::string& command) 
{
    std::istringstream iss(command);
    std::string cmd, chOrNick, msg;
    iss >> cmd >> chOrNick;
    std::getline(iss, msg);

    if (!msg.empty() && msg[0] == ' ') 
        msg = msg.substr(1);  
`
    if (msg[0] != ':')
        msg = ": " + msg;

    // channel part privmsg
    if (channels.find(chOrNick) != channels.end()) 
    {
        Channel *channel = channels[chOrNick];

        
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
    // Usert part privmsg
    else 
    {
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