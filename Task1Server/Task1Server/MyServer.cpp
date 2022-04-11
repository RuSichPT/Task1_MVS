#include "MyServer.h"
#include <iostream>
#include <fstream>
#include <filesystem>

void MyServer::start()
{
    if (!listenTCP())
    {
        return;
    }

    // Ждем подключающегося абонента
    waitConnection();
}

bool MyServer::listenTCP()
{
    // Инициализация
    WSAData wsadata;
    if (WSAStartup(MAKEWORD(2, 2), &wsadata) == 0)
    {
        std::cout << "WSA Startup succes" << std::endl;
    }
    else
    {
        std::cout << "WSA Startup unsucces" << std::endl;
        WSACleanup();
        return false;
    }

    // Создание сокета
    m_socketTCP = socket(AF_INET, SOCK_STREAM, 0);

    if (m_socketTCP == INVALID_SOCKET)
    {
        std::cout << "Socket TCP not created " << WSAGetLastError() << std::endl;
        WSACleanup();
        return false;
    }

    // Привязка сокета
    sockaddr_in sockAddr;
    ZeroMemory(&sockAddr, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = inet_addr(m_ip_string.c_str());
    sockAddr.sin_port = htons(m_port);

    if (bind(m_socketTCP, (sockaddr*)&sockAddr, sizeof(sockAddr)) != SOCKET_ERROR)
    {
        std::cout << "Socket TCP succed binded" << std::endl;
    }
    else
    {
        std::cout << "Socket TCP not binded " << WSAGetLastError() << std::endl;
        closesocket(m_socketTCP);
        WSACleanup();
        return false;
    }

    // Переходим в "слушающий режим"
    if (listen(m_socketTCP, SOMAXCONN) != SOCKET_ERROR)
    {
        std::cout << "Socket TCP is listening" << std::endl;
    }
    else
    {
        std::cout << "Socket TCP isn't listening" << std::endl;
    }

    return true;
}

void MyServer::waitConnection()
{
    sockaddr sockAddrAccept;
    char buf[100];
    ZeroMemory(buf, 100);

    while (true)
    {
        m_socketTCPAccept = accept(m_socketTCP, nullptr, nullptr);
        if (m_socketTCPAccept != SOCKET_ERROR)
        {
            std::cout << "Client connected" << std::endl;

            if (recv(m_socketTCPAccept, buf, 100, 0) != SOCKET_ERROR)
            {
                PacketType_t type = (PacketType_t)buf[0];

                if (type == PacketType_t::INIT)
                {
                    InitPacket_t* initPacket = (InitPacket_t*)buf;
                    std::cout << "The message recieved: " << initPacket->port << " " << initPacket->fileName << std::endl;
                    handleUDP(initPacket);
                }
            }
            closesocket(m_socketTCPAccept);
        }
    }
}

void MyServer::handleUDP(const InitPacket_t *iniPack)
{
    // Создание сокета
    m_socketUDP = socket(AF_INET, SOCK_DGRAM, 0);

    if (m_socketUDP == INVALID_SOCKET)
    {
        std::cout << "Socket UDP not created " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    // Привязка сокета
    sockaddr_in sockAddr;
    ZeroMemory(&sockAddr, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = inet_addr(m_ip_string.c_str());
    sockAddr.sin_port = htons((unsigned short)iniPack->port);

    if (bind(m_socketUDP, (sockaddr*)&sockAddr, sizeof(sockAddr)) != SOCKET_ERROR)
    {
        std::cout << "Socket UDP succed binded " << std::endl;
    }
    else
    {
        std::cout << "Socket UDP not binded " << WSAGetLastError() << std::endl;
        closesocket(m_socketUDP);
        WSACleanup();
        return;
    }

    // Прием данных UDP
    uint8_t *packetBuf = new uint8_t[MAX_SIZE_PACKET];
    DataPacket_t *packet = (DataPacket_t*)packetBuf;

    if (std::filesystem::create_directory(m_folder))
    {
        std::cout << "folder created" << std::endl;
    }
    else
    {
        std::cout << "folder not created" << std::endl;
    }

    std::string path = m_folder + "\\" + iniPack->fileName;
    std::ofstream file(path, std::ios::binary);
    int fileSize = 0;
    int lastRecievedId = 0xFFFFFFFF;

    while (true)
    {
        if (recv(m_socketUDP, (char*)packetBuf, MAX_SIZE_PACKET, 0) != SOCKET_ERROR)
        {
            std::cout << "UDP packet " << packet->id << " recieved" << std::endl;            
            if (sendACK(packet) && packet->id != lastRecievedId)
            {
                // Запись в файл
                int sizeData = packet->size - sizeof(DataPacket_t);
                file.write((char*)packet->data, sizeData);
                fileSize += sizeData;
                lastRecievedId = packet->id;


                if (packet->type == PacketType_t::END)
                {
                    file.close();
                    std::cout << "File wrote down, Size:" << fileSize << std::endl;
                    break;
                }
            }
        }
    }

    closesocket(m_socketUDP);

    delete[] packetBuf;
}

bool MyServer::sendACK(DataPacket_t *packet)
{
    DataPacket_t ack;
    ack.type = PacketType_t::ACK;
    ack.id = packet->id;
    if (send(m_socketTCPAccept, (char*)&ack, (int)sizeof(ack), 0) != SOCKET_ERROR)
    {
        std::cout << "ACK sent" << std::endl;
        return true;
    }
    else
    {
        std::cout << "ACK not sent " << WSAGetLastError() << std::endl;
        return false;
    }
}


