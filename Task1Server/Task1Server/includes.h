#pragma once

#pragma warning(disable:4996) // пока не понимаю зачем, но исправлет ошибку
#pragma comment(lib, "ws2_32.lib")
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

union Id_port_t
{
	int		id;
	int		port;
};

struct DataPacket_t
{
	PacketType_t	type;
	Id_port_t		id_port;
	int				size;
	uint8_t			data[];
};
