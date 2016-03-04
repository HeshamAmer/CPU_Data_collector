#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <iostream>
#include "list_processes.h"
#include <fstream>
#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <cpprest/filestream.h>
#include<ostream>
#include <fstream>
#include <vector>
#pragma comment( lib, "psapi.lib" )
#pragma comment( lib, "pdh.lib" )
int readConfigFile();
//TODO 
#pragma warning (disable : 4996)
void Log(int error_code);
int sendToClient(std::vector<std::string> message);

int collect_Agent_data();