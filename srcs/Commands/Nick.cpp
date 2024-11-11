/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Nick.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/11 16:53:54 by xav               #+#    #+#             */
/*   Updated: 2024/11/11 16:54:29 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

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