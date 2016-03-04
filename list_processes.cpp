
#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <iostream>
#include <Psapi.h>
#include "TCHAR.h"
#include <fstream>
#include "pdh.h"
#include <vector>
#pragma comment( lib, "psapi.lib" )
#pragma comment( lib, "pdh.lib" )
using namespace std;
#pragma warning (disable : 4996)
//  Forward declarations:
BOOL GetProcessList( );
BOOL ListProcessModules( DWORD dwPID );
BOOL ListProcessThreads( DWORD dwOwnerPID );
void printError( TCHAR* msg );




static ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
static int numProcessors;
static HANDLE self;


void init_process(){
	SYSTEM_INFO sysInfo;
	FILETIME ftime, fsys, fuser;


	GetSystemInfo(&sysInfo);
	numProcessors = sysInfo.dwNumberOfProcessors;


	GetSystemTimeAsFileTime(&ftime);
	memcpy(&lastCPU, &ftime, sizeof(FILETIME));


	self = GetCurrentProcess();
	GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
	memcpy(&lastSysCPU, &fsys, sizeof(FILETIME));
	memcpy(&lastUserCPU, &fuser, sizeof(FILETIME));
}


double getCurrentProcessValue(HANDLE hProcess){
	FILETIME ftime, fsys, fuser;
	ULARGE_INTEGER now, sys, user;
	double percent;


	GetSystemTimeAsFileTime(&ftime);
	memcpy(&now, &ftime, sizeof(FILETIME));


	GetProcessTimes(hProcess, &ftime, &ftime, &fsys, &fuser);
	memcpy(&sys, &fsys, sizeof(FILETIME));
	memcpy(&user, &fuser, sizeof(FILETIME));
	percent = (sys.QuadPart - lastSysCPU.QuadPart) +
		(user.QuadPart - lastUserCPU.QuadPart);
	percent /= (now.QuadPart - lastCPU.QuadPart);
	percent /= numProcessors;
	lastCPU = now;
	lastUserCPU = user;
	lastSysCPU = sys;


	return percent * 100;
}
BOOL GetProcessList( )
{
	HANDLE hProcessSnap;
	HANDLE hProcess;
	PROCESSENTRY32 pe32;
	DWORD dwPriorityClass;
	int processCounter=0;
	FILE * pFile;
	int n;
	char name [100];
	pFile = fopen ("process.txt","w");
	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if( hProcessSnap == INVALID_HANDLE_VALUE )
	{
		printError( TEXT("CreateToolhelp32Snapshot (of processes)") );
		return( FALSE );
	}

	// Set the size of the structure before using it.	
	pe32.dwSize = sizeof( PROCESSENTRY32 );

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if( !Process32First( hProcessSnap, &pe32 ) )
	{
		printError( TEXT("Process32First") ); // show cause of failure
		CloseHandle( hProcessSnap );          // clean the snapshot object
		return( FALSE );
	}

	// Now walk the snapshot of processes, and
	// display information about each process in turn
	do
	{

		// _tprintf( TEXT("\nPROCESS NAME:  %s"), pe32.szExeFile );
		fprintf (pFile, "Process Name: %s\n",pe32.szExeFile);
		//P->Name = pe32.szExeFile;

		// Retrieve the priority class.
		dwPriorityClass = 0;
		hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID );
		PROCESS_MEMORY_COUNTERS_EX pmc;
		if( hProcess == NULL )
			printError( TEXT("OpenProcess") );
		else
		{
			dwPriorityClass = GetPriorityClass( hProcess );
			if( !dwPriorityClass )
				printError( TEXT("GetPriorityClass") );
			GetProcessMemoryInfo(hProcess,(PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
			SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;
			std::printf("\n%lu\n", static_cast<unsigned long>(virtualMemUsedByMe));
			fprintf(pFile, "Memory used by process: %lu\n",virtualMemUsedByMe);
			fprintf(pFile, "CPU Usage by process: %lu\n",getCurrentProcessValue(hProcess));


			CloseHandle( hProcess );
		}
		processCounter++;


	} while( Process32Next( hProcessSnap, &pe32 ) );

	fclose (pFile);
	CloseHandle( hProcessSnap );
	return( processCounter );
}

unsigned long getTotalMemory()
{
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);
	DWORDLONG totalPhysMem = memInfo.ullTotalPhys;

	return totalPhysMem;
}
unsigned long getTotalMemoryUsed()
{
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);
	DWORDLONG totalVirtualMem = memInfo.ullTotalPageFile;
	DWORDLONG physMemUsed = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
	return physMemUsed;
}

void printError( TCHAR* msg )
{
	DWORD eNum;
	TCHAR sysMsg[256];
	TCHAR* p;

	eNum = GetLastError( );
	FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, eNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		sysMsg, 256, NULL );

	// Trim the end of the line and terminate it with a null
	p = sysMsg;
	while( ( *p > 31 ) || ( *p == 9 ) )
		++p;
	do { *p-- = 0; } while( ( p >= sysMsg ) &&
		( ( *p == '.' ) || ( *p < 33 ) ) );

	// Display the message
	_tprintf( TEXT("\n  WARNING: %s failed with error %d (%s)"), msg, eNum, sysMsg );
}




static PDH_HQUERY cpuQuery;
static PDH_HCOUNTER cpuTotal;


void init(){
	PdhOpenQuery(NULL, NULL, &cpuQuery);
	PdhAddCounter(cpuQuery, "\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
	PdhCollectQueryData(cpuQuery);
}


double getCurrentValue(){
	PDH_FMT_COUNTERVALUE counterVal;


	PdhCollectQueryData(cpuQuery);
	PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
	return counterVal.doubleValue;
}
int collect_machine_data()
{
	std::vector<int> V;
	init_process();
	init();
	int processCounter=0;
	processCounter= GetProcessList( );
	unsigned long total_physical_memory =getTotalMemory();
	unsigned long total_physical_memory_used =getTotalMemoryUsed();
	double memory_percent = ((double)total_physical_memory_used / (double)total_physical_memory )*100;
	
	double total_CPU_usage=getCurrentValue();
	ofstream cout ("machine_data.txt");
	cout<<"Processes counter="<<processCounter<<endl;
	cout<<"Total physical  memory:"<<total_physical_memory<<endl;
	cout << "Total phyiscal used:" << total_physical_memory_used << endl;
	cout << "Percent used:" << (int)memory_percent << endl;
	cout.close();
	return 0;

}
