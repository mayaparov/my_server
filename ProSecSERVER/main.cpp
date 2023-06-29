//
//  main.cpp
//  ProSecServer
//
//  Created by Егор Хохлов on 11.02.2023.
//  Modified by Матвей Япаров on 10.03.2023
//



#include "CMyServer/CMyServer.hpp"

int main(int argc, const char* argv[])
{
    CMyServer g_MyServer;
    try
    {
        g_MyServer.StartServer();
    }
    catch (const std::exception& s)
    {
        std::cout << "[CMyServer ShitCode]: " << s.what() << std::endl;
    }



    return 0;
}
