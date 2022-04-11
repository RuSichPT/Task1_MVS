#pragma once

#include "includes.h"

class MyClient
{
private:
	unsigned short m_port;
	std::string m_ip_string;

	uint8_t *m_fileBuf;
	int m_fileSize;

	SOCKET m_socketTCP;
	SOCKET m_socketUDP;

public:
	MyClient(std::string ip, int port)
		:m_ip_string(ip), m_port(port), m_fileBuf(nullptr),
		m_fileSize(0), m_socketTCP(0), m_socketUDP(0)
	{};
	~MyClient()
	{
		delete[] m_fileBuf;
	};

	void start(int port, std::string fileName, int timeOutMiliSec);

private:
	bool connectTCP();
	bool loadFile(std::string fileName);
	void sendTCP(int portUDP, std::string fileName);
	void sendUDP(int portUDP, int timeOutMiliSec);
	bool waitACK(DataPacket_t *packet, int timeOutMiliSec, sockaddr_in &sockAddr);


};

