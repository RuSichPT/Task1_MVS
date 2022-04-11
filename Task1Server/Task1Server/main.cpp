#include <iostream>
#include "MyServer.h"

int main(int argc, char* argv[])
{
    // Перебираем каждый аргумент и выводим его порядковый номер и значение
    for (int count = 0; count < argc; ++count)
        std::cout << count << " " << argv[count] << '\n';

    std::cout << "This is server" << std::endl;

    std::string ip = argv[1];
    int port = atoi(argv[2]);
    std::string folder = argv[3];
    
    MyServer server(ip, port, folder);
    server.start();

    return 0;    
}

