#include <iostream>
#include "MyClient.h"

int main(int argc, char* argv[])
{
    // Перебираем каждый аргумент и выводим его порядковый номер и значение
    for (int count = 0; count < argc; ++count)
        std::cout << count << " " << argv[count] << '\n';

    std::cout << "This is client" << std::endl;

    std::string ip = argv[1];
    int portTCP = atoi(argv[2]);
    int portUDP = atoi(argv[3]);
    std::string fileName = argv[4];
    int timeOut = atoi(argv[5]);

    MyClient client(ip, portTCP);

    client.start(portUDP, fileName, timeOut);

    return 1;
}

