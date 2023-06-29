

#ifndef CMyServer_hpp
#define CMyServer_hpp

#include <algorithm>
#include <stdio.h>
#include <boost/asio.hpp>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <vector>
#include <direct.h>

#define _WIN32_WINNT_WIN10  0x0A00


class CMyServer
{

    struct Client
    {
        std::string name;
        boost::asio::ip::tcp::socket socket;
        std::string clientID;
        std::string login;
    };
    
    std::vector<Client> connectedClients;
    std::vector<std::thread> clientThreads;

    void joinClientThreads();

    //message type identifiers
    //const std::string   m_AuthKey = "ЂЃ†‰ЋЊ";
    //const std::string   m_MessageKey = "ЌЏ…§©¶";
    //const std::string   RECIEVED = "Ђ¤ЏҐ‰Є";
    //const std::string   REG = "◙○▬‼☻♠";

    const std::string   m_AuthKey = "AUTHKEY";
    const std::string   m_MessageKey = "MESSAGEKEY";
    const std::string   RECIEVED = "RECEIVEDKEY";
    const std::string   REG = "REGKEY";

    const std::string   m_ServerIP = "193.105.114.182";

    void            ClientHandler(Client& client);
    std::string     reciveMessageFromSocket(boost::asio::ip::tcp::socket & socket);
    void            sendReplyToClient(boost::asio::ip::tcp::socket & socket, const std::string& message);
    bool            auth(boost::asio::ip::tcp::socket& socket_, std::string& message, Client& client);
    bool            messageHandler(std::string message, Client &client);
    bool            messageSaver(std::string & message, Client &client_sender, Client &client_reciever);
    bool            sendMessageToClient(std::string& message, Client& client);
    void            ClientConnectionHandler();
    Client*         findClient(std::string clientID);

    boost::asio::io_context ioContext;
    boost::asio::ip::tcp::acceptor acceptor;


    

public:
    bool            createClient(unsigned clientID, std::string login, std::string password, std::string name);
    CMyServer() : ioContext(), acceptor(ioContext, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 21221)) { }
    ~CMyServer();

    void StartServer();
};

#endif /* CMyServer_hpp */
