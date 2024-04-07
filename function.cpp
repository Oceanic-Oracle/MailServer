#include "lib.h"

void login(const json& signal, SOCKET& client)
{
    pqxx::connection database("dbname=Mail user=postgres password=241265 hostaddr=127.0.0.1 port=5432");
    if (database.is_open())
    {
        std::cout << "Opened database successfully: " << database.dbname() << std::endl;
    }
    else
    {
        std::cout << "Can't open database" << std::endl;
        throw std::runtime_error("database not opened");
    }

    std::string path("database/authorization.sql");
    std::fstream fs;
    fs.open(path, std::fstream::in);
    if (!fs.is_open())
    {
        std::cout << ".sql is NOT open" << std::endl;
        throw std::runtime_error(".sql file is not open");
    }
    std::string request((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());
    fs.close();

    std::string login = signal["1_username"].get<std::string>();
    std::string password = signal["2_password"].get<std::string>();

    try
    {
        pqxx::work check_work(database);
        pqxx::result check_res = check_work.exec_params(
            request,
            login,
            password
        );
        check_work.commit();

        json response;
        response["0_action"] = "login";
        response["1_result"] = "success";

        std::string temp = response.dump();
        std::cout << temp << std::endl;
        send(client, temp.c_str(), temp.size(), 0);

        std::string path("database/searchmessages.sql");
        std::fstream fs;
        fs.open(path, std::fstream::in);
        if (!fs.is_open())
        {
            std::cout << ".sql is NOT open" << std::endl;
            throw std::runtime_error(".sql file is not open");
        }
        std::string request1((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());
        fs.close();

        try
        {
            pqxx::work send_messages(database);
            pqxx::result send_result = send_messages.exec_params(
                request1,
                login
            );

            for (auto row : send_result) 
            {
                json message;
                message["0_action"]     = "message";
                message["3_theme"]      = row[0].as<std::string>();
                message["4_message"]    = row[1].as<std::string>();
                message["1_sender"]     = row[2].as<std::string>();
                message["2_recipient"]  = row[3].as<std::string>();
                message["5_data"]       = row[4].as<std::string>();
                std::string temp        = message.dump();
                send(client, temp.c_str(), temp.size(), 0);
                std::cout << temp << std::endl;
            }
        }
        catch(const pqxx::sql_error& e) 
        {
            std::cout << "This user has not sent/received messages yet" << std::endl;
        }
    }
    catch (const pqxx::sql_error& e)
    {
        json response;
        response["0_action"] = "login";
        response["1_result"] = "error";

        std::string temp = response.dump();
        std::cout << temp << std::endl;
        send(client, temp.c_str(), temp.size(), 0);
    }
}

void registration(const json& signal, SOCKET& client) 
{
    pqxx::connection database("dbname=Mail user=postgres password=241265 hostaddr=127.0.0.1 port=5432");
    if (!database.is_open()) {
        std::cout << "Can't opened database" << std::endl;
        throw std::runtime_error("database not opened");
    }

    std::string searchpath = "database/newuser.sql";
    std::ifstream fs(searchpath);
    if (!fs.is_open()) 
    {
        std::cout << ".sql is NOT open" << std::endl;
        throw std::runtime_error(".sql file is not open");
    }
    std::string request((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());
    fs.close();

    std::string login = signal["1_username"].get<std::string>();
    std::string password = signal["2_password"].get<std::string>();

    try 
    {
        pqxx::work check_work(database);
        pqxx::result check_res = check_work.exec_params(
            request,
            login,
            password
        );
        check_work.commit();

        json response;
        response["0_action"] = "registration";
        response["1_result"] = "success";

        std::string temp = response.dump();
        std::cout << temp << std::endl;
        send(client, temp.c_str(), temp.size(), 0);
    }
    catch (const pqxx::sql_error& e) 
    {
        json response;
        response["0_action"] = "registration";
        response["1_result"] = "error";

        std::string temp = response.dump();
        std::cout << temp << std::endl;
        send(client, temp.c_str(), temp.size(), 0);
    }  
}

void message(const json& signal, SOCKET& client)
{
    pqxx::connection database("dbname=Mail user=postgres password=241265 hostaddr=127.0.0.1 port=5432");
    if (!database.is_open()) {
        std::cout << "Can't opened database" << std::endl;
        throw std::runtime_error("database not opened");
    }

    std::string searchpath = "database/sendmessage.sql";
    std::ifstream fs(searchpath);
    if (!fs.is_open()) {
        std::cout << ".sql is NOT open" << std::endl;
        throw std::runtime_error(".sql file is not open");
    }

    std::string request((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());
    fs.close();

    std::string sender = signal["1_sender"].get<std::string>();
    std::string recipient = signal["2_recipient"].get<std::string>();
    std::string theme = signal["3_theme"].get<std::string>();
    std::string message = signal["4_message"].get<std::string>();

    try 
    {
        pqxx::work check_work(database);
        pqxx::result check_res = check_work.exec_params(
            request,
            sender,
            recipient,
            theme,
            message
        );
        check_work.commit();
        
        json response;
        response["0_action"] = "message";
        response["1_result"] = "success";

        std::string temp = response.dump();
        std::cout << temp << std::endl;
        send(client, temp.c_str(), temp.size(), 0);
    }
    catch (const pqxx::sql_error& e) 
    {
        json response;
        response["0_action"] = "message";
        response["1_result"] = "error";

        std::string temp = response.dump();
        std::cout << temp << std::endl;
        send(client, temp.c_str(), temp.size(), 0);
    }
}

void signal(const json& signal, SOCKET client) 
{
    const std::map<std::string, std::function<void(const json& signal, SOCKET& client)>> actions =
    { 
    {"login",        login},
    {"registration", registration},
    {"message",      message}
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