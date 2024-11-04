/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command_parsing.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 15:24:34 by xav               #+#    #+#             */
/*   Updated: 2024/11/04 22:25:54 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <iostream>
#include <sstream>

void Server::handleCommand(int client_fd, const std::string& command) 
{
    // Extraire la commande principale
    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd;

    // Rechercher la commande dans le tableau
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