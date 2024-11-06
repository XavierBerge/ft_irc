/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Mode.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 21:12:32 by xav               #+#    #+#             */
/*   Updated: 2024/11/05 21:13:24 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::handleModePlusO(int client_fd, const std::string& command) {
    std::cout << "handleModePlusO called with command: " << command << " for client_fd: " << client_fd << std::endl;
}

void Server::handleModeMinusO(int client_fd, const std::string& command) {
    std::cout << "handleModeMinusO called with command: " << command << " for client_fd: " << client_fd << std::endl;
}

void Server::handleModePlusI(int client_fd, const std::string& command) {
    std::cout << "handleModePlusI called with command: " << command << " for client_fd: " << client_fd << std::endl;
}

void Server::handleModeMinusI(int client_fd, const std::string& command) {
    std::cout << "handleModeMinusI called with command: " << command << " for client_fd: " << client_fd << std::endl;
}

void Server::handleModePlusL(int client_fd, const std::string& command) {
    std::cout << "handleModePlusL called with command: " << command << " for client_fd: " << client_fd << std::endl;
}

void Server::handleModeMinusL(int client_fd, const std::string& command) {
    std::cout << "handleModeMinusL called with command: " << command << " for client_fd: " << client_fd << std::endl;
}

void Server::handleModePlusK(int client_fd, const std::string& command) {
    std::cout << "handleModePlusK called with command: " << command << " for client_fd: " << client_fd << std::endl;
}

void Server::handleModeMinusK(int client_fd, const std::string& command) {
    std::cout << "handleModeMinusK called with command: " << command << " for client_fd: " << client_fd << std::endl;
}

void Server::handleModePlusT(int client_fd, const std::string& command) {
    std::cout << "handleModePlusT called with command: " << command << " for client_fd: " << client_fd << std::endl;
}

void Server::handleModeMinusT(int client_fd, const std::string& command) {
    std::cout << "handleModeMinusT called with command: " << command << " for client_fd: " << client_fd << std::endl;
}
