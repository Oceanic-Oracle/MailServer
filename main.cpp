#include "lib.h"

int main(int argc, char* argv[])
{
    WSAData wsaData;
    WORD DLLVersion = MAKEWORD(2, 1);

    if (WSAStartup(DLLVersion, &wsaData) != 0)
    {
        std::cout << "Error WSAStartup" << std::endl; 
        return 1;
    }

    SOCKADDR_IN addr;
    int sizeaddr = sizeof(addr);
    inet_pton(AF_INET, "127.0.0.1", &(addr.sin_addr));
    addr.sin_port = htons(1234);
    addr.sin_family = AF_INET;

    SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
    if (sListen == INVALID_SOCKET)
    {
        std::cout << "Error socket creation" << std::endl;
        WSACleanup();
        return 1;
    }

    if (bind(sListen, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        std::cout << "Error bind" << std::endl;
        closesocket(sListen);
        WSACleanup();
        return 1;
    }

    if (listen(sListen, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cout << "Error listen" << std::endl;
        closesocket(sListen);
        WSACleanup();
        return 1;
    }

    SOCKET newConnection;
    SOCKADDR_IN clientAddr;
    int clientAddrSize = sizeof(clientAddr);

    while (true) {
        newConnection = accept(sListen, (SOCKADDR*)&clientAddr, &clientAddrSize);
        if (newConnection == INVALID_SOCKET)
        {
            std::cout << "Error accept" << std::endl;
            closesocket(sListen);
            WSACleanup();
            return 1;
        }
        else
        {
            std::cout << "Client Connected" << std::endl;
            std::thread clientThread(clientHandler, newConnection);
            clientThread.detach();
        }
    }

    closesocket(newConnection);
    closesocket(sListen);
    WSACleanup();

    return 0;
}