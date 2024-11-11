/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Quit.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/11 17:05:28 by xav               #+#    #+#             */
/*   Updated: 2024/11/11 20:48:43 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::handleQuit(int client_fd, const std::string& command) 
{
	(void) command;
    
	Client* client = clients[client_fd];
    std::string nickname = (client && client->isNick_Ok()) ? client->getNickname() : "Unknown";

    std::cout << RED << "Client " << nickname << " has quit the server.\n" << RESET;
	removeClientFromAllChannels(client_fd);
    close_connection(client_fd);
}