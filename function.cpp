#include "lib.h"

void signal(json signal, SOCKET client) 
{
    std::string action = signal["0_action"];
    std::cout << action << std::endl;

    json response;

    if (action == "login")
    {
        response["0_action"] = "login";
        response["1_result"] = "success";

        std::string temp = response.dump();
        send(client, temp.c_str(), temp.size(), 0);
        //Приделать БД и проверку на сходство
        std::cout << "Posted JSON: " << response.dump() << std::endl;
    }

    else if (action == "registration")
    {
        response["0_action"] = "login";
        response["1_result"] = "success";

        std::string temp = response.dump();
        send(client, temp.c_str(), temp.size(), 0);
        //Приделать БД и проверку на сходство
        std::cout << "Posted JSON: " << response.dump() << std::endl;
    }

    else if (action == "message")
    {
        std::string temp = response.dump();
        send(client, temp.c_str(), temp.size(), 0);
        //Приделать БД и проверку на сходство
        std::cout << "Posted JSON: " << response.dump() << std::endl;
    }

    else 
    {
        std::cout << "Wrong format JSON!" << std::endl;
    }
}

void clientHandler(SOCKET clientSocket)
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