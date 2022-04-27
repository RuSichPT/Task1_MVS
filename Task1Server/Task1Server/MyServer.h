#pragma once

#include "includes.h"

class MyServer
{
private:
	unsigned short m_port;
	std::string m_ip_string;
	std::string m_folder;

	SOCKET m_socketTCP;
	SOCKET m_socketTCPAccept;
	SOCKET m_socketUDP;

public:
	MyServer(std::string ip, int port, std::string folder)
		:m_ip_string(ip), m_port(port), m_folder(folder), m_socketTCP(0), m_socketTCPAccept(0), m_socketUDP(0)
	{};
	~MyServer() {};

	void start();

private:
	bool listenTCP();
	void waitConnection();
	void handleUDP(const DataPacket_t *iniPack);
	bool sendACK(DataPacket_t *packet);
};

