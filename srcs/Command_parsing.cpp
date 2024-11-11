/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command_parsing.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 15:24:34 by xav               #+#    #+#             */
/*   Updated: 2024/11/11 20:37:31 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <iostream>
#include <sstream>

void Server::handleCommand(int client_fd, const std::string& command) 
{
    Client* client = clients[client_fd];
    
    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd;

    // Only display the command if it is not "PING"
    if (client && cmd != "PING")
	{
		std::cout << "\n\033[1;94m"
          << "Received command from " 
          << "\033[1;32m"
          << client->getNickname() 
          << "\033[1;36m"
          << " :\n" << command 
          << "\033[0m\n" << std::endl;
	}

    std::map<std::string, CommandHandler>::iterator it = commandMap.find(cmd);
    if (it != commandMap.end()) 
    {
        CommandHandler handler = it->second;
        (this->*handler)(client_fd, command);
    } 
    else 
    {
        std::cout << "Unknown command: " << cmd << std::endl;
    }
}



void Server::handleMode(int client_fd, const std::string& command) 
{
    std::map<std::string, void (Server::*)(int, const std::string&)> modeMap;

    modeMap["+o"] = &Server::handleModePlusO;
    modeMap["-o"] = &Server::handleModeMinusO;
    modeMap["+i"] = &Server::handleModePlusI;
    modeMap["-i"] = &Server::handleModeMinusI;
    modeMap["+l"] = &Server::handleModePlusL;
    modeMap["-l"] = &Server::handleModeMinusL;
    modeMap["+k"] = &Server::handleModePlusK;
    modeMap["-k"] = &Server::handleModeMinusK;
    modeMap["+t"] = &Server::handleModePlusT;
    modeMap["-t"] = &Server::handleModeMinusT;

    std::istringstream iss(command);
    std::string commandType, channel, modeOption;
    iss >> commandType >> channel >> modeOption;

    bool isNickname = false;

    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) 
    {
        if (it->second->getNickname() == channel) 
        {
            isNickname = true;
            break;
        }
    }


    if (isNickname && modeOption == "+i") 
    {
        handleNicknameModeI(client_fd, channel);
        return;
    }

    if (channels.find(channel) == channels.end()) 
    {
        clients[client_fd]->sendToClient("403 " + clients[client_fd]->getNickname() + " " + channel + " :No such channel\r\n");
        return;
    }

    Channel* channelObj = channels[channel];

    if (!channelObj->isOperator(clients[client_fd]->getNickname())) 
    {
        clients[client_fd]->sendToClient("481 " + clients[client_fd]->getNickname() + " :Permission denied\r\n");
        return;
    }

    if (!modeOption.empty() && modeMap.find(modeOption) != modeMap.end()) 
    {
        (this->*modeMap[modeOption])(client_fd, command);
    } 
    else 
    {
        clients[client_fd]->sendToClient("472 " + clients[client_fd]->getNickname() + " :Unknown mode\r\n");
    }
}