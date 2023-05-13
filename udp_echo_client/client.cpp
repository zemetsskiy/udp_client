#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include <fstream>

// указываем компоновщику включить библиотеку winsock в процесс сборки программы
#pragma comment(lib, "ws2_32.lib")

int get_config_data(std::string& ip_address, std::string& server_port, std::string& client_port) {
    std::string filename = "../Debug/udp_client.cfg.TXT";

    // создание объекта класса ifstream для чтения данных из файла с заданным именем filename
    std::ifstream config(filename);
    if (!config.is_open()) {
        std::cerr << "Error: Unable to open config file " << filename << std::endl;
        return 1;
    }

    if (config.peek() == std::ifstream::traits_type::eof()) {
        std::cerr << "Error: Config file " << filename << " is empty" << std::endl;
        return 1;
    }

    std::string line;
    while (std::getline(config, line)) {
        if (line.find("ip_address=") == 0) {
            ip_address = line.substr(11);
            break;
        }
    }

    while (std::getline(config, line)) {
        if (line.find("server_port=") == 0) {
            server_port = line.substr(12);
            break;
        }
    }

    while (std::getline(config, line)) {
        if (line.find("client_port=") == 0) {
            client_port = line.substr(12);
            break;
        }
    }

    config.close();

    std::cout << "Server Ip address: " << ip_address << std::endl;
    std::cout << "Server port: " << server_port << std::endl;
    std::cout << "Client port: " << client_port << std::endl;
    return 0;
}

int main(int argc, char* argv[])
{
    // Validate the parameters
     // getting data from config
    std::string server_ip_address;
    std::string client_port;
    std::string server_port;

    if (argc == 1) {
        // no command line arguments were provided besides program name
        get_config_data(server_ip_address, server_port, client_port);
    }
    else {
        server_ip_address = argv[1];
        server_port = argv[2];
        client_port = argv[3];
    }

    // инициализация библиотеки сокетов

    
    // переменная типа WSADATA - структура, используется для хранения информации о версии и конфигурации библиотеки винсок
    WSADATA wsaData;
    // makeword определяет версию винсок, которую необходимо использовать

    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Can't create socket! Quitting" << std::endl;
        WSACleanup();
        return 1;
    }


    sockaddr_in client;
    client.sin_family = AF_INET;
    // stoi: string -> integer
    // htons: 16 битное число из прямого порядка в обратный порядок
    client.sin_port = htons(std::stoi(client_port));
    // связываем сокет с любым доступным ip адресом на локальной машине
    client.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (sockaddr*)&client, sizeof(client)) == SOCKET_ERROR) {
        std::cerr << "Can't bind socket! Quitting" << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }


    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(std::stoi(server_port));
    //  IPv4 или IPv6 из стандартной нотации в двоичное представление
    inet_pton(AF_INET, server_ip_address.c_str(), &server.sin_addr);

    const int bufferSize = 1024;
    char buffer[bufferSize];
    std::string userInput;

    while (true) {
        std::cout << "Enter a message: ";
        std::getline(std::cin, userInput);

        int sendResult = sendto(sock, userInput.c_str(), userInput.size() + 1, 0, (sockaddr*)&server, sizeof(server));
        if (sendResult == SOCKET_ERROR) {
            std::cerr << "Can't send message! Quitting" << std::endl;
            closesocket(sock);
            WSACleanup();
            return 1;
        }

        sockaddr_in from;
        int fromLength = sizeof(from);
        int bytesReceived = recvfrom(sock, buffer, bufferSize, 0, (sockaddr*)&from, &fromLength);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "Can't receive response! Quitting" << std::endl;
            closesocket(sock);
            WSACleanup();
            return 1;
        }

        std::cout << "Server: " << buffer << std::endl;
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}