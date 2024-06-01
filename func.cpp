#include "func.h"

pqxx::connection database("dbname=Mail user=postgres password=241265 hostaddr=127.0.0.1 port=5432");

void login(const json& signal, SOCKET& client)
{
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

        std::string result = "";
        for (auto row : check_res)
        {
            for (auto field : row)
            {
                result += field.c_str();
            }
        }

        json response;

        if (result != "")
        {
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
                    message["0_action"] = "message";
                    message["3_theme"] = row[0].as<std::string>();
                    message["4_message"] = row[1].as<std::string>();
                    message["1_sender"] = row[2].as<std::string>();
                    message["2_recipient"] = row[3].as<std::string>();
                    message["5_data"] = row[4].as<std::string>();
                    std::string temp = message.dump();
                    send(client, temp.c_str(), temp.size(), 0);
                    std::cout << temp << std::endl;
                }
            }
            catch (const pqxx::sql_error& e)
            {
                std::cout << "This user has not sent/received messages yet" << std::endl;
            }
        }
        else
        {
            response["0_action"] = "login";
            response["1_result"] = "error";

            std::string temp = response.dump();
            std::cout << temp << std::endl;
            send(client, temp.c_str(), temp.size(), 0);
        }
    }
    catch (pqxx::sql_error& e)
    {
        std::cout << "error" << std::endl;
    }
}

void registration(const json& signal, SOCKET& client)
{
    if (!database.is_open())
    {
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
    if (!database.is_open())
    {
        std::cout << "Can't opened database" << std::endl;
        throw std::runtime_error("database not opened");
    }

    std::string searchpath = "database/sendmessage.sql";
    std::ifstream fs(searchpath);
    if (!fs.is_open())
    {
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
        response["0_action"] = "send_message";
        response["1_result"] = "success";

        std::string temp = response.dump();
        std::cout << temp << std::endl;
        send(client, temp.c_str(), temp.size(), 0);
    }
    catch (const pqxx::sql_error& e)
    {
        json response;
        response["0_action"] = "send_message";
        response["1_result"] = "error";

        std::string temp = response.dump();
        std::cout << temp << std::endl;
        send(client, temp.c_str(), temp.size(), 0);
    }
}

void refresh(const json& signal, SOCKET& client)
{
    if (!database.is_open())
    {
        std::cout << "Can't opened database" << std::endl;
        throw std::runtime_error("database not opened");
    }

    std::string searchpath;
    if (signal["1_refresh"].get<std::string>() == "incoming")
    {
        searchpath = "database/refreshincoming.sql";
    }
    else
    {
        searchpath = "database/refreshoutgoing.sql";
    }

    std::ifstream fs(searchpath);
    if (!fs.is_open())
    {
        std::cout << ".sql is NOT open" << std::endl;
        throw std::runtime_error(".sql file is not open");
    }

    std::string request((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());
    fs.close();

    std::string refresh = signal["1_refresh"].get<std::string>();
    std::string sender = signal["2_sender"].get<std::string>();

    pqxx::work check_work(database);
    pqxx::result check_res = check_work.exec_params(
        request,
        sender
    );
    check_work.commit();

    for (auto row : check_res)
    {
        json message;
        message["0_action"] = "message";
        message["3_theme"] = row[0].as<std::string>();
        message["4_message"] = row[1].as<std::string>();
        message["1_sender"] = row[2].as<std::string>();
        message["2_recipient"] = row[3].as<std::string>();
        message["5_data"] = row[4].as<std::string>();
        std::string temp = message.dump();
        send(client, temp.c_str(), temp.size(), 0);
        std::cout << temp << std::endl;
    }
}