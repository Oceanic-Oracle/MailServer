#include "func.h"

void signal(const json& signal, SOCKET client) 
{
    const std::map<std::string, std::function<void(const json& signal, SOCKET& client)>> actions =
    { 
    {"login",        login},
    {"registration", registration},
    {"message",      message},
    {"refresh",      refresh}
    };

    std::string const temp = signal["0_action"].get<std::string>();

    try {
        auto it = actions.find(temp);
        if (it != actions.end())
        {
            it->second(signal, client);     
        }
        else 
        {
            std::cout << "Unknown action: " << temp << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;
    }
}

void clientHandler(const SOCKET clientSocket)
{
    char msg[4096];
    while (true) {
        int bytesIn = recv(clientSocket, msg, sizeof(msg), 0);

        if (bytesIn <= 0)
        {
            std::cout << "Client Disconnected" << std::endl;
            closesocket(clientSocket);
            break;
        }

        else
        {
            try
            {
                if (bytesIn < sizeof(msg)) 
                {
                    msg[bytesIn] = '\0';
                }

                std::string receivedData(msg, bytesIn);
                json jsonData = json::parse(receivedData);

                std::cout << "Received JSON: " << jsonData.dump() << std::endl;

                std::string response = jsonData.dump();
                
                signal(jsonData, clientSocket);
            }
            catch (json::parse_error& e)
            {
                std::cerr << "JSON parsing error: " << e.what() << std::endl;
            }
            catch (std::exception& e)
            {
                std::cerr << "Unexpected error: " << e.what() << std::endl;
            }
        }
    }
}