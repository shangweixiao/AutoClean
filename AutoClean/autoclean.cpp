// CreateService.cpp : 定义应用程序的入口点。
//

#include <tchar.h>  
#include <windows.h>
#include <stdio.h>
#include <time.h>

#define  SERVICE_DEBUG

//定义全局函数变量 

#ifndef SERVICE_DEBUG
void Init();
BOOL IsInstalled();
BOOL Install();
BOOL Uninstall();
void WINAPI ServiceMain();
void WINAPI ServiceStrl(DWORD dwOpcode);
#endif

void LogEvent(LPCTSTR pszFormat, ...);

TCHAR szServiceName[] = _T("AutoClean");
BOOL bInstall;
SERVICE_STATUS_HANDLE hServiceStatus;
SERVICE_STATUS status;
DWORD dwThreadID;

typedef enum {
	REPEATE_MODE_UNDEFINE = 0,
	REPEATE_MODE_WEEKLY,
	REPEATE_MODE_MONTHLY,
	REPEATE_MODE_DAILY,
}REPEATE_MODE_T;

typedef struct {
	REPEATE_MODE_T RepeatMode; // 重复周期
	char RunningDay[4]; // 运行日期,按位记录，一天占用一位，一月最多31位。
	char DelPath[MAX_PATH]; // 要删除的路径
	__time64_t NextRunTime; // 下一次运行时间
	__time64_t NotifiedTime; // 给用户的通知时间
}RegData_t;

DWORD GetRegData(RegData_t *RegData)
{
#define REG_VALUE_BUF_SIZE		64
	LONG nRetVal = ERROR_SUCCESS;
	HKEY hKey = NULL;
	DWORD dwType;
	CHAR pchRegValueBuf[REG_VALUE_BUF_SIZE];
	DWORD dwBufSize = REG_VALUE_BUF_SIZE;

	for (;;)
	{
		nRetVal = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\AutoClean", 0, KEY_READ, &hKey);
		if (ERROR_SUCCESS != nRetVal)
		{
			hKey = NULL;
			wprintf(L"RegOpenKeyEx Error Line: %d\n", __LINE__);
			nRetVal = 1;
			break;
		}

		ZeroMemory(pchRegValueBuf, REG_VALUE_BUF_SIZE);
		dwBufSize = REG_VALUE_BUF_SIZE;
		nRetVal = RegQueryValueExW(hKey, L"RepeatMode", NULL, &dwType, (BYTE *)pchRegValueBuf, &dwBufSize);
		if (ERROR_SUCCESS != nRetVal)
		{
			wprintf(L"RegQueryValueEx Error Line: %d\n", __LINE__);
			nRetVal = 2;
		}
		else
		{
			RegData->RepeatMode = *(REPEATE_MODE_T*)&pchRegValueBuf[0];
			wprintf(L"RegQueryValueEx ReadSysDisk= %d\n", RegData->RepeatMode);
		}

		ZeroMemory(pchRegValueBuf, REG_VALUE_BUF_SIZE);
		dwBufSize = REG_VALUE_BUF_SIZE;
		nRetVal = RegQueryValueExW(hKey, L"RunningDay", NULL, &dwType, (BYTE *)pchRegValueBuf, &dwBufSize);
		if (ERROR_SUCCESS != nRetVal)
		{
			wprintf(L"RegQueryValueEx Error Line: %d\n", __LINE__);
			nRetVal = 2;
		}
		else
		{
			memcpy(RegData->RunningDay, pchRegValueBuf,sizeof(RegData->RunningDay));
			wprintf(L"RegQueryValueEx ReadSysDisk= 0x%x,0x%x,0x%x,0x%x\n", RegData->RunningDay[3], RegData->RunningDay[2], RegData->RunningDay[1], RegData->RunningDay[0]);
		}

		ZeroMemory(pchRegValueBuf, REG_VALUE_BUF_SIZE);
		dwBufSize = REG_VALUE_BUF_SIZE;
		nRetVal = RegQueryValueEx(hKey, "DelPath", NULL, &dwType, (BYTE *)pchRegValueBuf, &dwBufSize);
		if (ERROR_SUCCESS != nRetVal)
		{
			wprintf(L"RegQueryValueEx Error Line: %d\n", __LINE__);
			nRetVal = 2;
		}
		else
		{
			memcpy(RegData->DelPath, pchRegValueBuf, dwBufSize);
			wprintf(L"RegQueryValueEx ReadSysDisk= 0x%x,0x%x,0x%x,0x%x\n", RegData->RunningDay[3], RegData->RunningDay[2], RegData->RunningDay[1], RegData->RunningDay[0]);
		}

		ZeroMemory(pchRegValueBuf, REG_VALUE_BUF_SIZE);
		dwBufSize = REG_VALUE_BUF_SIZE;
		nRetVal = RegQueryValueExW(hKey, L"NextRunTime", NULL, &dwType, (BYTE *)pchRegValueBuf, &dwBufSize);
		if (ERROR_SUCCESS != nRetVal)
		{
			wprintf(L"RegQueryValueEx Error Line: %d\n", __LINE__);
			nRetVal = 2;
		}
		else
		{
			RegData->NextRunTime = *(__time64_t*)&pchRegValueBuf[0];
			wprintf(L"RegQueryValueEx ReadSysDisk= %I64u\n", RegData->NextRunTime);
		}

		ZeroMemory(pchRegValueBuf, REG_VALUE_BUF_SIZE);
		dwBufSize = REG_VALUE_BUF_SIZE;
		nRetVal = RegQueryValueExW(hKey, L"NotifiedTime", NULL, &dwType, (BYTE *)pchRegValueBuf, &dwBufSize);
		if (ERROR_SUCCESS != nRetVal)
		{
			wprintf(L"RegQueryValueEx Error Line: %d\n", __LINE__);
			nRetVal = 2;
		}
		else
		{
			RegData->NotifiedTime = *(__time64_t*)&pchRegValueBuf[0];
			wprintf(L"RegQueryValueEx ReadSysDisk= %I64u\n", RegData->NotifiedTime);
		}
		break;
	}

	if (NULL != hKey)
	{
		RegCloseKey(hKey);
		hKey = NULL;
	}

	return nRetVal;
}

VOID NotifyUserToBackFiles(time_t DelTime)
{

}

DWORD WaitUserToConfirm(char DelPath[MAX_PATH])
{
	return 0;
}

DWORD SetRegData(const char *key,char *val,int size)
{
#define REG_VALUE_BUF_SIZE		64
	LONG nRetVal = ERROR_SUCCESS;
	HKEY hKey = NULL;
	DWORD dwType;
	CHAR pchRegValueBuf[REG_VALUE_BUF_SIZE];
	DWORD dwBufSize = REG_VALUE_BUF_SIZE;

	for (;;)
	{
		nRetVal = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\AutoClean", 0, KEY_READ, &hKey);
		if (ERROR_SUCCESS != nRetVal)
		{
			hKey = NULL;
			wprintf(L"RegOpenKeyEx Error Line: %d\n", __LINE__);
			nRetVal = 1;
			break;
		}

		nRetVal = RegSetValueEx(hKey, key, NULL, REG_BINARY, (BYTE*)val, size);
		if (ERROR_SUCCESS != nRetVal)
		{
			hKey = NULL;
			wprintf(L"RegSetValueEx Error Line: %d\n", __LINE__);
			nRetVal = 1;
			break;
		}
	}

	if (NULL != hKey)
	{
		RegCloseKey(hKey);
		hKey = NULL;
	}

	return nRetVal;
}


DWORD SetNextRunTime(time_t cur, RegData_t *rd)
{
	time_t NextRunTime;

	if(REPEATE_MODE_DAILY == rd->RepeatMode)
	{
		NextRunTime = cur + 24 * 3600;
	} 
	else if(REPEATE_MODE_WEEKLY == rd->RepeatMode)
	{
		struct tm *local;
		int i;
		local = localtime(&cur);
		NextRunTime = cur;
		for(i = (local->tm_wday + 1)%7; ; i = (i + 1) % 7) // tm_wday: [0 - 6]
		{
			NextRunTime += 24 * 3600;
			if(rd->RunningDay[i/8] & (1UL << (i % 8)))
			{
				break;
			}

			if (i == local->tm_wday)
			{
				break;
			}
		}
	}
	else if (REPEATE_MODE_MONTHLY == rd->RepeatMode)
	{
		struct tm *local,*nrt;
		int i;
		local = localtime(&cur);
		NextRunTime = cur;
		for (i = local->tm_mday; ; i = (i + 1) % 31) // tm_mday: [1 - 31]
		{
			NextRunTime += 24 * 3600;
			nrt = localtime(&NextRunTime);
			if((nrt->tm_mday - 1) != i)
			{
				i = nrt->tm_mday - 1;
			}

			if (rd->RunningDay[i/8] & (1UL << (i % 8)))
			{
				break;
			}

			if (i == local->tm_mday)
			{
				break;
			}
		}
	}
	else
	{
		return 1;
	}

	SetRegData("NextRunTime",(char*)&NextRunTime,sizeof(NextRunTime));

	return 0;
}

DWORD DeleteUserFiles(char DelPath[MAX_PATH])
{
	char cmd[MAX_PATH + 32] = { 0 };
	sprintf(cmd,"rd /s /q \"%s\"", DelPath);
	system(cmd);
	return 0;
}

#ifndef SERVICE_DEBUG
DWORD AutoCleanMain()
#else
int main(int argc,char **argv)
#endif
{
	while (1)
	{
		RegData_t rd;
		DWORD ret;

		time_t cur;
		struct tm local, NextRunTime,*plocal, *pNextRunTime;
		double diff;
		GetRegData(&rd);

		cur = time(NULL);
		plocal = localtime(&cur);
		memcpy(&local,plocal,sizeof(struct tm));
		
		pNextRunTime = localtime(&rd.NextRunTime);
		memcpy(&NextRunTime, pNextRunTime, sizeof(struct tm));

		if (REPEATE_MODE_UNDEFINE == rd.RepeatMode)
		{
			LogEvent(_T("重复周期设置错误。"));
			break;
		}

		diff = difftime(rd.NextRunTime, cur);
		if ((0 > diff) || ((local.tm_year == NextRunTime.tm_year) && (local.tm_mon == NextRunTime.tm_mon) && (local.tm_mday == NextRunTime.tm_mday)))
		{
			ret = WaitUserToConfirm(rd.DelPath);
			if (0 == ret)
			{
				SetNextRunTime(cur, &rd);
				DeleteUserFiles(rd.DelPath);
				LogEvent(_T("删除目录:%s"), rd.DelPath);
			}
			else
			{
				SetNextRunTime(cur, &rd);
				LogEvent(_T("用户取消了本次删除计划，已安排下一次运行时间。"));
			}
		}
		else
		{
			if (3 * 24 * 3600 > diff && 0 < diff)
			{
				NotifyUserToBackFiles(rd.NextRunTime);
				LogEvent(_T("删除时间小于3天，提示用户备份文件。"));
			}
		}
		break;
	}

	return 0;
}

#ifndef SERVICE_DEBUG
int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	Init();
	dwThreadID = ::GetCurrentThreadId();
	SERVICE_TABLE_ENTRY st[] =
	{
		{ szServiceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
		{ NULL, NULL }
	};

	if (strcmp(lpCmdLine, "/install") == 0)
	{
		Install();
	}
	else if (strcmp((LPCTSTR)lpCmdLine, "/uninstall") == 0)
	{
		Uninstall();
	}
	else
	{
		if (!::StartServiceCtrlDispatcher(st))
		{
			LogEvent(_T("Register Service Main Function Error!"));
		}
	}

	return 0;
}

//初始化
void Init()
{
	hServiceStatus = NULL;
	status.dwServiceType = SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS;
	status.dwCurrentState = SERVICE_START_PENDING;
	status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	status.dwWin32ExitCode = 0;
	status.dwServiceSpecificExitCode = 0;
	status.dwCheckPoint = 0;
	status.dwWaitHint = 0;
}

//服务主函数，这在里进行控制对服务控制的注册
void WINAPI ServiceMain()
{
	status.dwCurrentState = SERVICE_START_PENDING;
	status.dwControlsAccepted = SERVICE_ACCEPT_STOP;

	//注册服务控制  
	hServiceStatus = RegisterServiceCtrlHandler(szServiceName, ServiceStrl);
	if (hServiceStatus == NULL)
	{
		LogEvent(_T("Handler not installed"));
		return;
	}
	SetServiceStatus(hServiceStatus, &status);

	status.dwWin32ExitCode = S_OK;
	status.dwCheckPoint = 0;
	status.dwWaitHint = 0;
	status.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(hServiceStatus, &status);

	//模拟服务的运行。应用时将主要任务放于此即可  
	//可在此写上服务需要执行的代码，一般为死循环  
	AutoCleanMain();

	status.dwCurrentState = SERVICE_STOPPED;
	SetServiceStatus(hServiceStatus, &status);
}

//Description:          服务控制主函数，这里实现对服务的控制，  
//                      当在服务管理器上停止或其它操作时，将会运行此处代码  
void WINAPI ServiceStrl(DWORD dwOpcode)
{
	switch (dwOpcode)
	{
	case SERVICE_CONTROL_STOP:
		status.dwCheckPoint = 1;
		status.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus(hServiceStatus, &status);
		Sleep(500);
		status.dwCheckPoint = 0;
		status.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(hServiceStatus, &status);

		PostThreadMessage(dwThreadID, WM_CLOSE, 0, 0);
		break;
	case SERVICE_CONTROL_PAUSE:
		break;
	case SERVICE_CONTROL_CONTINUE:
		break;
	case SERVICE_CONTROL_INTERROGATE:
		break;
	case SERVICE_CONTROL_SHUTDOWN:
		exit(0);
		break;
	default:
		LogEvent(_T("Bad service request"));
	}
}

//判断服务是否已经被安装
BOOL IsInstalled()
{
	BOOL bResult = FALSE;

	//打开服务控制管理器  
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hSCM != NULL)
	{
		//打开服务  
		SC_HANDLE hService = ::OpenService(hSCM, szServiceName, SERVICE_QUERY_CONFIG);
		if (hService != NULL)
		{
			bResult = TRUE;
			::CloseServiceHandle(hService);
		}
		::CloseServiceHandle(hSCM);
	}
	return bResult;
}

//安装服务函数
BOOL Install()
{
	//检测是否安装过
	if (IsInstalled())
		return TRUE;

	//打开服务控制管理器  
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSCM == NULL)
	{
		MessageBox(NULL, _T("Couldn't open service manager"), szServiceName, MB_OK);
		return FALSE;
	}

	//获取程序目录
	TCHAR szFilePath[MAX_PATH];
	::GetModuleFileName(NULL, szFilePath, MAX_PATH);

	//创建服务  
	SC_HANDLE hService = ::CreateService(hSCM, szServiceName, szServiceName,
		SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
		szFilePath, NULL, NULL, _T(""), NULL, NULL);

	//检测创建是否成功
	if (hService == NULL)
	{
		::CloseServiceHandle(hSCM);
		MessageBox(NULL, _T("Couldn't create service"), szServiceName, MB_OK);
		return FALSE;
	}

	//释放资源
	::CloseServiceHandle(hService);
	::CloseServiceHandle(hSCM);
	return TRUE;
}

//删除服务函数
BOOL Uninstall()
{
	//检测是否安装过
	if (!IsInstalled())
		return TRUE;

	//打开服务控制管理器
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSCM == NULL)
	{
		MessageBox(NULL, _T("Couldn't open service manager"), szServiceName, MB_OK);
		return FALSE;
	}

	//打开具体服务
	SC_HANDLE hService = ::OpenService(hSCM, szServiceName, SERVICE_STOP | DELETE);
	if (hService == NULL)
	{
		::CloseServiceHandle(hSCM);
		MessageBox(NULL, _T("Couldn't open service"), szServiceName, MB_OK);
		return FALSE;
	}

	//先停止服务
	SERVICE_STATUS status;
	::ControlService(hService, SERVICE_CONTROL_STOP, &status);

	//删除服务  
	BOOL bDelete = ::DeleteService(hService);
	::CloseServiceHandle(hService);
	::CloseServiceHandle(hSCM);

	if (bDelete)  return TRUE;
	LogEvent(_T("Service could not be deleted"));
	return FALSE;
}
#endif

//记录服务事件
void LogEvent(LPCTSTR pFormat, ...)
{
	TCHAR    chMsg[256];
	HANDLE  hEventSource;
	LPTSTR  lpszStrings[1];
	va_list pArg;

	va_start(pArg, pFormat);
	_vstprintf(chMsg, pFormat, pArg);
	va_end(pArg);

	lpszStrings[0] = chMsg;

	hEventSource = RegisterEventSource(NULL, szServiceName);
	if (hEventSource != NULL)
	{
		ReportEvent(hEventSource, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, (LPCTSTR*)&lpszStrings[0], NULL);
		DeregisterEventSource(hEventSource);
	}
}
