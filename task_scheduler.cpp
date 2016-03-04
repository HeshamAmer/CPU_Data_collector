#include <windows.h>
#include <initguid.h>
#include <ole2.h>
#include <mstask.h>
#include <msterr.h>
#include <objidl.h>
#include <wchar.h>
#include <stdio.h>
#include "data_collector.h"
int main(int argc, char **argv)
{
  HRESULT hr = S_OK;
  ITaskScheduler *pITS;


  /////////////////////////////////////////////////////////////////
  // Call CoInitialize to initialize the COM library and then 
  // call CoCreateInstance to get the Task Scheduler object. 
  /////////////////////////////////////////////////////////////////
  hr = CoInitialize(NULL);
  if (SUCCEEDED(hr))
  {
     hr = CoCreateInstance(CLSID_CTaskScheduler,
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           IID_ITaskScheduler,
                           (void **) &pITS);
     if (FAILED(hr))
     {
        CoUninitialize();
        return 1;
     }
  }
  else
  {
     return 1;
  }


  /////////////////////////////////////////////////////////////////
  // Call ITaskScheduler::NewWorkItem to create new task.
  /////////////////////////////////////////////////////////////////
  LPCWSTR pwszTaskName;
  ITask *pITask;
  IPersistFile *pIPersistFile;
  pwszTaskName = L"Collect Data from host";

  hr = pITS->NewWorkItem(pwszTaskName,         // Name of task
                         CLSID_CTask,          // Class identifier 
                         IID_ITask,            // Interface identifier
                         (IUnknown**)&pITask); // Address of task 
                                                                                                                                                                                            //  interface


  pITS->Release();                               // Release object
 
  if (FAILED(hr))
  {
    wprintf(L"Failed calling ITask::SetAccountInformation: ");
    wprintf(L"error = 0x%x\n",hr);
    pITask->Release();
    CoUninitialize();
    return 1;
  }

  /////////////////////////////////////////////////////////////////
  //Set Comment, Name, Working dir, Params
  /////////////////////////////////////////////////////////////////
  pITask->SetComment(L"This is a comment");
  pITask->SetApplicationName(L"..\\Debug\\WindowsAgent.exe");
  //pITask->SetApplicationName(L"C:\\Windows\\notepad.exe");
  pITask->SetWorkingDirectory(L"..\\Debug\\");
  pITask->SetParameters(L"");

  ///////////////////////////////////////////////////////////////////
  // Call ITask::CreateTrigger to create new trigger.
  ///////////////////////////////////////////////////////////////////

  ITaskTrigger *pITaskTrigger;
  WORD piNewTrigger;
  hr = pITask->CreateTrigger(&piNewTrigger,
                             &pITaskTrigger);
  if (FAILED(hr))
  {
    wprintf(L"Failed calling ITask::CreatTrigger: ");
    wprintf(L"error = 0x%x\n",hr);
    pITask->Release();
    CoUninitialize();
    return 1;
  }

//////////////////////////////////////////////////////
  // Define TASK_TRIGGER structure. Note that wBeginDay,
  // wBeginMonth, and wBeginYear must be set to a valid 
  // day, month, and year respectively.
  //////////////////////////////////////////////////////

  TASK_TRIGGER pTrigger;
  ZeroMemory(&pTrigger, sizeof (TASK_TRIGGER));
  
  SYSTEMTIME right_now;
  LPSYSTEMTIME lpSystemTime=&right_now;
  GetLocalTime(lpSystemTime);


  // Add code to set trigger structure?
  pTrigger.wBeginDay = lpSystemTime->wDay;                  // Required
  pTrigger.wBeginMonth = lpSystemTime->wMonth;                // Required
  pTrigger.wBeginYear =lpSystemTime->wYear;              // Required
  pTrigger.cbTriggerSize = sizeof (TASK_TRIGGER); 
  pTrigger.wStartHour = lpSystemTime->wHour;
  pTrigger.wStartMinute = lpSystemTime->wMinute + 2;
  pTrigger.MinutesDuration=1440;
  pTrigger.MinutesInterval=5;
  pTrigger.TriggerType = TASK_TIME_TRIGGER_DAILY;
  pTrigger.Type.Daily.DaysInterval = 1;


  ///////////////////////////////////////////////////////////////////
  // Call ITaskTrigger::SetTrigger to set trigger criteria.
  ///////////////////////////////////////////////////////////////////
  pITask->SetFlags(TASK_FLAG_RUN_ONLY_IF_LOGGED_ON);
  hr = pITask->SetAccountInformation(L"Gedy", 
            NULL);
  hr = pITaskTrigger->SetTrigger (&pTrigger);
  if (FAILED(hr))
  {
	  try{
    wprintf(L"Failed calling ITaskTrigger::SetTrigger: ");
    wprintf(L"error = 0x%x\n",hr);
	throw 2;
    pITask->Release();
    pITaskTrigger->Release();
    CoUninitialize();
	  }
	  catch (int n) {
		  
	  }
    return 1;
  }


   /////////////////////////////////////////////////////////////////
  //Set Flags
  /////////////////////////////////////////////////////////////////

  pITask->SetFlags(TASK_FLAG_RUN_ONLY_IF_LOGGED_ON);

  /////////////////////////////////////////////////////////////////
  // Call IUnknown::QueryInterface to get a pointer to 
  // IPersistFile and IPersistFile::Save to save 
  // the new task to disk.
  /////////////////////////////////////////////////////////////////

  hr = pITask->QueryInterface(IID_IPersistFile,
                              (void **)&pIPersistFile);

  pITask->Release();
  if (FAILED(hr))
  {
     CoUninitialize();
     fprintf(stderr, "Failed calling QueryInterface, error = 0x%x\n",hr);
     return 1;
  }


  hr = pIPersistFile->Save(NULL,
                           TRUE);
  pIPersistFile->Release();
  if (FAILED(hr))
  {
     CoUninitialize();
     fprintf(stderr, "Failed calling Save, error = 0x%x\n",hr);
     return 1;
  }


  CoUninitialize();
  printf("Created task.\n");
  collect_Agent_data();
  return 0;
}