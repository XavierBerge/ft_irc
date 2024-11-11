/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Ping.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/11 17:06:54 by xav               #+#    #+#             */
/*   Updated: 2024/11/11 17:07:07 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::handlePing(int client_fd, const std::string& command) 
{
    std::string response = "PONG :";
    std::string param = command.substr(5);
    response += param + "\r\n";
    send(client_fd, response.c_str(), response.size(), 0);
}