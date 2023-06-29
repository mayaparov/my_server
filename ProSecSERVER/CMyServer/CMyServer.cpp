//
//  CMyServer.cpp
//  ProSecServer
//
//  Created by Матвей Япаров on 11.02.2023.
//
#include "CMyServer.hpp"





void CMyServer::ClientConnectionHandler()
{
    int i = 0;
    while (true) 
    {
        boost::asio::ip::tcp::socket socket(ioContext);
        acceptor.accept(socket);

        bool flag = 0; //следующий код (включая блок if) нужен для того, чтобы избежать добавление
        //клиента в массив подключенных клиентов в случае, если он уже там есть
        for (auto& client : connectedClients)
            if (client.socket.remote_endpoint().address() == socket.remote_endpoint().address() and 
               client.socket.remote_endpoint().port() == socket.remote_endpoint().port()) flag = 1;
        
        if (flag)
        {
            i--;
            continue;
        }

        std::string clientName = "Client " + std::to_string(connectedClients.size() + 1);
        
        connectedClients.push_back({ clientName, std::move(socket), "", ""});

        //clientThreads.emplace_back(&CMyServer::ClientHandler, std::ref(connectedClients[i]), std::ref(connectedClients));
        clientThreads.emplace_back(&CMyServer::ClientHandler, this, std::ref(connectedClients[i]));        
        i++;
        std::cout << clientName << " connected" << std::endl;
    }
    for(unsigned j; j < clientThreads.size(); j++)
    clientThreads[j].join();
}

void CMyServer::joinClientThreads()
{
    std::vector<std::thread*> current_clientThreads;
    while (true)
    {
        for (unsigned i = 0; i < clientThreads.size(); i++)
        {
            
        }
    }
}

void CMyServer::StartServer()
{
    std::cout << "[CMyServer Start]: Server Started! " << std::endl;
    try
    {
        std::cout << "[My Server]: Server is running. Waiting for connections..." << std::endl;
        
        std::thread ConnectionHandler(&CMyServer::ClientConnectionHandler, this);
        ConnectionHandler.join();
    }
    catch (const std::exception& ShitCode)
    {
        setlocale(LC_ALL, "rus");
        std::cout << "[My Server ShitCode]: Error: " << ShitCode.what() << std::endl;
    }
    
    std::cout << "[MyServer Start]: Called Exit. " << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void CMyServer::ClientHandler(Client& client)
{
    try
    {
        while (true)
        {
            std::array<char, 128> buffer;
            std::fill(buffer.begin(), buffer.end(), 0);
            boost::system::error_code error;
            size_t bytesRead = client.socket.read_some(boost::asio::buffer(buffer), error);

            if (error == boost::asio::error::eof)
            {
                std::cout << "[My Server ShitCode]: End of filestream error" << std::endl;
                break;
            }
            else if (error == boost::asio::error::connection_reset and !(client.clientID.empty()))
            {
                std::cout << "Client " << client.name << " disconnected" << std::endl;
                // Отправляем сообщение о выходе клиента остальным клиентам
                std::string message = "[SYSTEM INFO]: Client " + client.name + " has left the chat";
                
                for (auto& otherClient : connectedClients)
                    if (&otherClient != &client) boost::asio::write(otherClient.socket, boost::asio::buffer(message + "\n"));
                
                throw boost::system::system_error(error);
            }
            else if (error)
            {
                throw boost::system::system_error(error);
            }

            std::string msg = "";
            for (char a : buffer)
                if (a != '\n' && a != 0) msg = msg + a;
            msg = msg + '\n';
            std::cout << "Client send " << msg << '\n';
            this->messageHandler(msg, client);
            // Отправляем полученное сообщение остальным клиентам
            //можно будет использовать для определения статусов онлайн-оффлай
            /*for (auto& otherClient : connectedClients)
            {
                if (&otherClient != &client) {
                    boost::asio::write(otherClient.socket, boost::asio::buffer(buffer, bytesRead));
                }
            }*/
        }
    }
    catch (std::exception& ShitCode)
    {
        setlocale(LC_ALL, "rus");
        std::cerr << "[My Server ShitCode]: Error: " << ShitCode.what() << std::endl;
    }
}



std::string CMyServer::reciveMessageFromSocket(boost::asio::ip::tcp::socket& socket)
{
    boost::asio::streambuf buf;
    boost::asio::read_until(socket, buf, "\n");
    std::string data = boost::asio::buffer_cast<const char*>(buf.data());
    std::cout << "success read operation:\t" << data << '\n';
    return data;
}


bool CMyServer::messageHandler(std::string message, Client& client)
{
    if (message.find(m_AuthKey) == 0)
    {
        message.erase(0, m_AuthKey.size());
        std::cout << "authorisation attempt" << '\t' << message << '\n';

        if (this->auth(client.socket, message, client))
        {
            this->sendReplyToClient(client.socket, m_AuthKey + client.name + '\t' +  client.clientID);
            std::cout << "respond: " << m_AuthKey + client.name + '\t' + client.clientID << '\n';
        }
        else
        {
            this->sendReplyToClient(client.socket, "false");
            std::cout << "respond: false";
        }
        return true;

    }
    else if (message.find(m_MessageKey) == 0)
    {
        std::cout << "message: " << '\t' << message;//нет \n, потому что в message по умолчанию стоит этот символ в конце
        message.erase(0, m_MessageKey.size());

        sendMessageToClient(message, client);
        return true;
    }
    std::cout << "recieved garbage\t" << message << '\n';
    return false;
}

bool CMyServer::auth(boost::asio::ip::tcp::socket& socket_, std::string& message, Client& client)
{
    std::ifstream data("Clients\\" + message.substr(0, message.find(' ')) + "\\Data.xls"); //второе слагаемое - логин
    
    if (!(data.is_open())) return false; //если такого логина не существует
    
    //получение данных из файла
    std::string password, UID;
    std::string auth_password = message.substr(message.find(' ') + 1, message.length() - message.find(' '));
    if (auth_password.size() == 0) return false;

    getline(data, UID, '\t');
    getline(data, password, '\t'); 
    
    auth_password.erase(auth_password.find('\n'));
    std::cout << "PASS:" << auth_password << '\n';
    if (std::strcmp(password.c_str(),auth_password.c_str()) == 0)
    {
        client.clientID = UID;
        client.login = message.substr(0, message.find(' '));
        getline(data, client.name, '\n');
        data.close();
        return true;
    }
    else
    {
        if (data.is_open())
        {
            data.close();
            return false;
        }
    }
}

bool CMyServer::sendMessageToClient(std::string& message, Client& client)
{
    std::string UID;
    Client* reciever;
    if (findClient(message.substr(0, message.find('\t'))))   //если клиент найдётся, указатель будет не nullptr => true 
    {
        reciever = findClient(message.substr(0, message.find('\t'))); //нашли 
        message.erase(0, message.find('\t'));
       
        //после того, как из сообщения удалены данные об отправителе и получателе, инициируем его запись в файлы
        //данные отправителя и получателе теперь хранятся в переменных client (sender)
        //и *reciever
        std::thread th(&CMyServer::messageSaver, this, std::ref(message), std::ref(client), std::ref(*reciever)); //сохранение сообщения в файл
        th.detach();

        message.insert(0, m_MessageKey + client.clientID);
        sendReplyToClient(reciever->socket, message);
    }

    else sendReplyToClient(client.socket, "Reciever is offline\n");
    return false;
}

bool CMyServer::messageSaver(std::string & message, Client& client_sender, Client& client_reciever)
{
    std::ofstream dialog;

    //пишем в файл, который лежит в папке, по пути, описанному ниже
    std::cout << "Start writting to the sender file\tClients\\" << 
        client_sender.login << "\\dialogs\\" << client_reciever.clientID << '\n';
    //путь такой, потому что мы не хотим, чтобы можно было найти переписку на сервере, зная логины отправителя и получаетеля. 
    //считаю, что понять, за какого пользователя отвечает uid, сложнее
    dialog.open("Clients\\" + client_sender.login + "\\dialogs\\" + client_reciever.clientID, std::ios_base::app);
    if (dialog.is_open())
    {
        //так как функция вызывается после удаления UID отправиеля из сообщения, мы можем записать 
        //в файлы отправителя и получаетеля сообщения, при чём информацию о получателе
        //не нужно хранить в папке отправителя, потому что история переписке хранится с файлом, 
        //название которого - UID получателя. 
        dialog << client_sender.clientID << '\t' << message;
        dialog.close();
        std::cout << "writting to the sender file is done\n";
    }
    else
    {
        throw std::string{ "Cannot open the file Clients\\" + client_sender.login + "\\dialogs\\" + client_reciever.clientID};
        std::cout << "writting to the sender file is failed\n";
        return false;
    }
    
    //записываем то же самое, только в качестве отправителя в первую ячейку файла запишется UID отправителя
    dialog.open("Clients\\" + client_reciever.login + "\\dialogs\\" + client_sender.clientID);
    std::cout << "Start writting to the reciever file\tClients\\" <<
        client_reciever.login + "\\dialogs\\" + client_sender.clientID << '\n';
    
    if (dialog.is_open())
    {
        dialog << client_sender.clientID << '\t' << message;
        dialog.close();
        std::cout << "writting to the reciever file is done\n";
        return true;
    }
    else
    {
        throw std::string{ "Cannot open the file Clients\\" + client_reciever.login + "\\dialogs\\" + client_sender.clientID};
        std::cout << "writting to the reviever file is failed\n";
        return false;
    }
}


CMyServer::Client* CMyServer::findClient(std::string clientID)
{
    for (int i = 0; i < connectedClients.size(); i++)
        if (connectedClients[i].clientID == clientID) return &connectedClients[i];
    return nullptr;
}

bool CMyServer::createClient(unsigned clientID, std::string login, std::string password, std::string name)
{
    std::ofstream client_file;
    std::cout << "Creating a client " << name << " \n";
    std::string filename = "Clients\\" + login;
    _mkdir(filename.c_str());
    filename = filename + "\\Dialogs";
    _mkdir(filename.c_str());
    filename = "Clients\\" + login;
    filename = filename + "\\Data.xls";
    client_file.open(filename, std::ios_base::app);
    if (client_file.is_open())
    {
        client_file << clientID << '\t' << password << '\t' << name;
        client_file.close();
        std::cout << "writting to the file is done\n";
        return true;
    }
    else
    {
        throw std::string{ "Cannot open the file client.txt !" };
        std::cout << "writting to the file is failed\n";
        return false;
    }
}


void  CMyServer::sendReplyToClient(boost::asio::ip::tcp::socket& socket, const std::string& message)
{
    const std::string msg = message + "\n";
    boost::asio::write(socket, boost::asio::buffer(message));
}

CMyServer::~CMyServer()
{

}
