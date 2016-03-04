#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <Psapi.h>
#include "TCHAR.h"
#include "pdh.h"

//  Forward declarations:
BOOL GetProcessList( );
BOOL ListProcessModules( DWORD dwPID );
BOOL ListProcessThreads( DWORD dwOwnerPID );
void printError( TCHAR* msg );

void init_process();


double getCurrentProcessValue(HANDLE hProcess);
BOOL GetProcessList( );

unsigned long getTotalMemory();
unsigned long getTotalMemoryUsed();

void printError( TCHAR* msg );

void init();


double getCurrentValue();

int collect_machine_data();
