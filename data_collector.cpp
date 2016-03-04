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
#include "data_collector.h"
#pragma comment( lib, "psapi.lib" )
#pragma comment( lib, "pdh.lib" )
int readConfigFile(){

	char name[80],password[80],mail[80];
	std::vector<std::string> data;
	try{

		std::ifstream fin ("client_data.txt");
		fin>>name>>password>>mail;
		fin.close();
		
		data.push_back(name);
		data.push_back(password);
		data.push_back(mail);
	}
	catch (int n)
	{
		return n;
	}

	//return client_data;
	return 0;
}
//TODO 
#pragma warning (disable : 4996)
void Log(int error_code)
{
	FILE * pFile;
	int n;
	char name [100];
	pFile = fopen ("client_log.log","w");

	if (error_code == 1) {
		fprintf (pFile, "Process returned with error code %d, config file not found\n",error_code);
	}
	else if (error_code==3){
		fprintf (pFile, "Unable to contact host.\n",error_code);
	}
	else if (error_code ==0){
		fprintf (pFile, "Process returned with error code %d, client successfully terminated and will reschedule\n",error_code);
	}
	fclose (pFile);
}
int sendToClient(std::vector<std::string> message){
	//TODO: Implement this function correctly.
	using namespace std;	
	ifstream cin("machine_data.txt");
	vector<string> V;
	string x;
	while (cin >> x){
		V.push_back(x);
	}
	using namespace web;
	using namespace web::http;
	using namespace web::http::client;

	try{
		http_client client(L"http://localhost:8080");
		client.request(methods::GET, L"?q=S.Somasegar")
			.then([](http_response response) {
			printf("%s\n",response.to_string()); 
		});
			throw 3;
		//system("pause");
	}
	catch (int n){
		return n;
	}
	return 0;
}

int collect_Agent_data(){
	std::vector<std::string> client_data;
	int returnCode = 0;
	returnCode = collect_machine_data();
	returnCode = readConfigFile();
	returnCode = sendToClient(client_data);
	Log(returnCode);
	return 0;
}


