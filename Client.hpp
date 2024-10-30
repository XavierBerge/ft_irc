#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <unistd.h>

class Client 
{
	private:
		int socket_fd;
		std::string nickname;
		std::string username;
		std::string realname;
		bool authenticated;
		bool irssi;
		bool pass_ok;
		bool nick_ok;
		

	public:
		Client(int fd);
		~Client();

		int getSocketFd() const;
		void setNickname(const std::string& nick);
		std::string getNickname() const;

		void setUsername(const std::string& user);
		std::string getUsername() const;

		void setRealname(const std::string& name);
		std::string getRealname() const;

		bool isAuthenticated() const;
		void authenticate();
		void pass_true();
		void irssi_true();
		void nick_true();

		bool isIrssi() const;
		bool isPass_Ok() const;
		bool isNick_Ok() const;

		ssize_t readFromClient(char *buffer, size_t size);
		ssize_t sendToClient(const std::string& message);

		void handleCommand(const std::string& command);
	};

#endif

