#pragma once

#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <nlohmann/json.hpp>
#include <thread>
#include <string>
#include <iostream>

using json = nlohmann::json;

void clientHandler(SOCKET clientSocket);