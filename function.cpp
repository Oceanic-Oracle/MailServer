#include "lib.h"

std::string login(const json& signal)
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

    pqxx::work check_work(database);
    pqxx::result check_res = check_work.exec_params(
        request,
        login,
        password
    );

    std::string result;
    for (auto row : check_res)
    {
        for (auto field : row) 
        {
            result += field.c_str();
        }
    }

    json response;
    response["0_action"] = "login";
    response["1_result"] = result.empty() ? "error" : "success";

    check_work.commit();

    return response.dump();
}

std::string registration(const json& signal) {
    pqxx::connection database("dbname=Mail user=postgres password=241265 hostaddr=127.0.0.1 port=5432");
    if (!database.is_open()) {
        std::cout << "Can't opened database" << std::endl;
        throw std::runtime_error("database not opened");
    }

    std::string searchpath = "database/searchusers.sql";
    std::ifstream fs(searchpath);
    if (!fs.is_open()) {
        std::cout << ".sql is NOT open" << std::endl;
        throw std::runtime_error(".sql file is not open");
    }
    std::string request((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());
    fs.close();

    std::string login = signal["1_username"].get<std::string>();
    std::string password = signal["2_password"].get<std::string>();

    pqxx::work check_work(database);
    pqxx::result check_res = check_work.exec_params(
        request,
        login
    );
    check_work.commit();

    json response;
    if (check_res.empty()) {
        pqxx::work insert_work(database);
        std::string insertpath = "database/newuser.sql";
        std::ifstream insert_fs(insertpath);
        if (!insert_fs.is_open()) {
            std::cout << ".sql is NOT open" << std::endl;
            throw std::runtime_error(".sql file is not open");
        }

        std::string insert_request((std::istreambuf_iterator<char>(insert_fs)), std::istreambuf_iterator<char>());
        insert_fs.close();

        pqxx::result insert_res = insert_work.exec_params(
            insert_request,
            login, password
        );
        insert_work.commit(); 

        response["0_action"] = "registration";
        response["1_result"] = (insert_res.affected_rows() == 1) ? "success" : "error";
    }
    else {
        response["0_action"] = "registration";
        response["1_result"] = "error";
    }

    return response.dump();
}

std::string message(const json& signal)
{
    json response;
    response["0_action"] = "login";
    response["1_result"] = "success";

    std::string temp = response.dump();
    return temp;
}

void signal(const json& signal, SOCKET client) 
{
    const std::map<std::string, std::function<std::string(const json& signal)>> actions = 
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
            std::string response = it->second(signal);
            std::cout << response << std::endl;
            send(client, response.c_str(), response.size(), 0);
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