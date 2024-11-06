/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/03 13:10:39 by xav               #+#    #+#             */
/*   Updated: 2024/11/06 09:08:29 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>
#include <map>
#include "Client.hpp"

class Client;

class Channel 
{
    private:
        std::string name;
        std::map<std::string, int> clients; // Map nickname -> client_fd
        std::map<std::string, int> operators; // Map nickname -> client_fd
    public:
        Channel(const std::string &channelName);
        ~Channel();

        std::string getName() const;
        bool addClient(const std::string &nickname, int client_fd);
        void removeClient(const std::string &nickname);
        bool isClientInChannel(const std::string &nickname) const;
        bool isOperator(const std::string &nickname) const;
        void promoteToOperator(const std::string &nickname, int client_fd);
		void broadcastMessage(const std::string &message, int sender_fd);
		int count() const;
};

#endif

