#pragma once

#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>
#include <stdexcept>
#include <thread>
#include <fstream>
#include <map>
#include <functional>
#include <string>
#include <iostream>

using json = nlohmann::json;

void clientHandler(const SOCKET clientSocket);