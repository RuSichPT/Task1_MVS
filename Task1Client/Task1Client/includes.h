#pragma once

#pragma warning(disable:4996) // пока не понимаю зачем, но исправлет ошибку
#include <winsock2.h>
#include <string>

#define MAX_SIZE_PACKET	(1000)
#define MAX_SIZE_DATA	(MAX_SIZE_PACKET - sizeof(DataPacket_t))	

enum class PacketType_t
{
	INIT,
	DATA,
	ACK,
	END,
};


struct InitPacket_t
{
	PacketType_t	type;
	int				port;
	std::string		fileName;
};

struct DataPacket_t
{
	PacketType_t	type;
	int				id;
	int				size;
	uint8_t			data[];
};
