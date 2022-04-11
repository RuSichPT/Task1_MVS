#include "MyClient.h"
#include "Timer.h"
#include <iostream>
#include <fstream> 
#include <ctime>

#define MAX_NUMBER_REPEAT    10

void MyClient::start(int portUDP, std::string fileName, int timeOutMiliSec)
{
    if (!connectTCP())
    {
        return;
    }

    if (loadFile(fileName))
    {
        std::cout << "File load" << std::endl;
    }
    else
    {
        std::cout << "File not load" << std::endl;
        return;
    }
     
    sendTCP(portUDP, fileName);
    sendUDP(portUDP, timeOutMiliSec);

    closesocket(m_socketUDP);
    closesocket(m_socketTCP);
    WSACleanup();
}

bool MyClient::connectTCP()
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

    // Установка соединения
    sockaddr_in sockAddrConnect;
    ZeroMemory(&sockAddrConnect, sizeof(sockAddrConnect));
    sockAddrConnect.sin_family = AF_INET;
    sockAddrConnect.sin_addr.s_addr = inet_addr(m_ip_string.c_str());
    sockAddrConnect.sin_port = htons(m_port);

    if (connect(m_socketTCP, (sockaddr*)&sockAddrConnect, sizeof(sockAddrConnect)) != SOCKET_ERROR)
    {
        std::cout << "Socket TCP succed connected" << std::endl;
    }
    else
    {
        std::cout << "Socket TCP not connected " << WSAGetLastError() << std::endl;
        closesocket(m_socketTCP);
        WSACleanup();
        return false;
    }

    return true;
}

bool MyClient::loadFile(std::string fileName)
{
    // Загрузка файла в память
    std::ifstream file(fileName, std::ios::binary);
    if (!file.is_open())
    {
        return false;
    }

    // Размер файла
    file.seekg(0, std::ios::end); // перемещаемся в конец файла
    m_fileSize = file.tellg();
    file.seekg(0, std::ios::beg); // перемещаемся в начало файла

    // Чтение
    m_fileBuf = new uint8_t[m_fileSize];

    file.read((char*)m_fileBuf, m_fileSize);

    return true;
}

void MyClient::sendTCP(int portUDP, std::string fileName)
{
    // Отправка порта UDP и имени файла
    InitPacket_t initPacket;
    initPacket.type = PacketType_t::INIT;
    initPacket.port = portUDP;
    initPacket.fileName = fileName;

    if (send(m_socketTCP, (char*)&initPacket, (int)sizeof(initPacket), 0) != SOCKET_ERROR)
    {
        std::cout << "The message sended: " << initPacket.port << " " << initPacket.fileName << std::endl;
    }
}

void MyClient::sendUDP(int portUDP, int timeOutMiliSec)
{
    // Создание сокета
    m_socketUDP = socket(AF_INET, SOCK_DGRAM, 0);
    int error;

    if (m_socketUDP == INVALID_SOCKET)
    {
        error = WSAGetLastError();
        std::cout << "Socket UDP not created " << error << std::endl;
        WSACleanup();
        return;
    }

    // Неблок режим сокета TCP для прослушки ACK
    bool nonBlocking = true;
    if (ioctlsocket(m_socketTCP, FIONBIO, (unsigned long*)&nonBlocking) != SOCKET_ERROR)
    {
        std::cout << "Socket TCP set nonBlocking" << std::endl;
    }
    else
    {
        std::cout << "Socket TCP not set nonBlocking " << WSAGetLastError() << std::endl;
    }

    // Отправка данных
    sockaddr_in sockAddr;
    ZeroMemory(&sockAddr, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = inet_addr(m_ip_string.c_str());
    sockAddr.sin_port = htons(portUDP);

    uint8_t *packetBuf = new uint8_t[MAX_SIZE_PACKET];
    DataPacket_t *packet = (DataPacket_t*)packetBuf;


    int numPacket = ceil((float)m_fileSize / MAX_SIZE_DATA);
    int indxData = 0;
    int sizeData = 0;

    for (int i = 0; i < numPacket; i++)
    {
        if (i < numPacket - 1)
        {
            packet->size = MAX_SIZE_PACKET;
            packet->type = PacketType_t::DATA;
            sizeData = MAX_SIZE_DATA;
        }
        else if (i == numPacket - 1) // Хвост
        {
            sizeData = m_fileSize - indxData;
            packet->size = sizeof(DataPacket_t) + sizeData;
            packet->type = PacketType_t::END;
        }
        packet->id = i;
        CopyMemory(packet->data, &m_fileBuf[indxData], sizeData);

        if (sendto(m_socketUDP, (const char*)packet, packet->size, 0, (sockaddr*)&sockAddr, sizeof(sockaddr_in)) != SOCKET_ERROR)
        {
            if (packet->type == PacketType_t::END)
            {
                std::cout << "Last UDP packet " << packet->id << " sended" << std::endl;
            }
            else
            {
                std::cout << "UDP packet " << packet->id << " sended" << std::endl;
            }
        }

        // Не дождались, рвем связь
        if (!waitACK(packet, timeOutMiliSec, sockAddr))
        {
            std::cout << "File transfer failed " << std::endl;
            break;
        }

        indxData += sizeData;
    }

    delete[] packetBuf;
}

bool MyClient::waitACK(DataPacket_t *packet, int timeOutMiliSec, sockaddr_in &sockAddr)
{
    char buf[100];
    ZeroMemory(buf, 100);
    int numRepeat = 0;

    Timer t;
    while (true)
    {
        if (recv(m_socketTCP, buf, 100, 0) != SOCKET_ERROR)
        {
            PacketType_t type = (PacketType_t)buf[0];

            if (type == PacketType_t::ACK)
            {
                DataPacket_t* packetRcv = (DataPacket_t*)buf;
                if (packetRcv->id == packet->id)
                {
                    std::cout << "ACK recieved" << std::endl;
                    return true;
                }
            }
        }

        if (t.elapsed() > (float)timeOutMiliSec / 1000)
        {
            if (numRepeat > MAX_NUMBER_REPEAT)
            {
                return false;
            }

            if (sendto(m_socketUDP, (const char*)packet, packet->size, 0, (sockaddr*)&sockAddr, sizeof(sockaddr_in)) != SOCKET_ERROR)
            {
                std::cout << "repeated UDP packet " << packet->id << " sended" << std::endl;
                numRepeat++;
            }
        }
    }

}
