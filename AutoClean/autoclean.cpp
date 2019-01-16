// CreateService.cpp : ����Ӧ�ó������ڵ㡣
//

#include <tchar.h>  
#include <windows.h>
#include <stdio.h>
#include <time.h>

#define  SERVICE_DEBUG

//����ȫ�ֺ������� 

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
	REPEATE_MODE_T RepeatMode; // �ظ�����
	char RunningDay[4]; // ��������,��λ��¼��һ��ռ��һλ��һ�����31λ��
	char DelPath[MAX_PATH]; // Ҫɾ����·��
	__time64_t NextRunTime; // ��һ������ʱ��
	__time64_t NotifiedTime; // ���û���֪ͨʱ��
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
			LogEvent(_T("�ظ��������ô���"));
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
				LogEvent(_T("ɾ��Ŀ¼:%s"), rd.DelPath);
			}
			else
			{
				SetNextRunTime(cur, &rd);
				LogEvent(_T("�û�ȡ���˱���ɾ���ƻ����Ѱ�����һ������ʱ�䡣"));
			}
		}
		else
		{
			if (3 * 24 * 3600 > diff && 0 < diff)
			{
				NotifyUserToBackFiles(rd.NextRunTime);
				LogEvent(_T("ɾ��ʱ��С��3�죬��ʾ�û������ļ���"));
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

//��ʼ��
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

//��������������������п��ƶԷ�����Ƶ�ע��
void WINAPI ServiceMain()
{
	status.dwCurrentState = SERVICE_START_PENDING;
	status.dwControlsAccepted = SERVICE_ACCEPT_STOP;

	//ע��������  
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

	//ģ���������С�Ӧ��ʱ����Ҫ������ڴ˼���  
	//���ڴ�д�Ϸ�����Ҫִ�еĴ��룬һ��Ϊ��ѭ��  
	AutoCleanMain();

	status.dwCurrentState = SERVICE_STOPPED;
	SetServiceStatus(hServiceStatus, &status);
}

//Description:          �������������������ʵ�ֶԷ���Ŀ��ƣ�  
//                      ���ڷ����������ֹͣ����������ʱ���������д˴�����  
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

//�жϷ����Ƿ��Ѿ�����װ
BOOL IsInstalled()
{
	BOOL bResult = FALSE;

	//�򿪷�����ƹ�����  
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hSCM != NULL)
	{
		//�򿪷���  
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

//��װ������
BOOL Install()
{
	//����Ƿ�װ��
	if (IsInstalled())
		return TRUE;

	//�򿪷�����ƹ�����  
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSCM == NULL)
	{
		MessageBox(NULL, _T("Couldn't open service manager"), szServiceName, MB_OK);
		return FALSE;
	}

	//��ȡ����Ŀ¼
	TCHAR szFilePath[MAX_PATH];
	::GetModuleFileName(NULL, szFilePath, MAX_PATH);

	//��������  
	SC_HANDLE hService = ::CreateService(hSCM, szServiceName, szServiceName,
		SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
		szFilePath, NULL, NULL, _T(""), NULL, NULL);

	//��ⴴ���Ƿ�ɹ�
	if (hService == NULL)
	{
		::CloseServiceHandle(hSCM);
		MessageBox(NULL, _T("Couldn't create service"), szServiceName, MB_OK);
		return FALSE;
	}

	//�ͷ���Դ
	::CloseServiceHandle(hService);
	::CloseServiceHandle(hSCM);
	return TRUE;
}

//ɾ��������
BOOL Uninstall()
{
	//����Ƿ�װ��
	if (!IsInstalled())
		return TRUE;

	//�򿪷�����ƹ�����
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSCM == NULL)
	{
		MessageBox(NULL, _T("Couldn't open service manager"), szServiceName, MB_OK);
		return FALSE;
	}

	//�򿪾������
	SC_HANDLE hService = ::OpenService(hSCM, szServiceName, SERVICE_STOP | DELETE);
	if (hService == NULL)
	{
		::CloseServiceHandle(hSCM);
		MessageBox(NULL, _T("Couldn't open service"), szServiceName, MB_OK);
		return FALSE;
	}

	//��ֹͣ����
	SERVICE_STATUS status;
	::ControlService(hService, SERVICE_CONTROL_STOP, &status);

	//ɾ������  
	BOOL bDelete = ::DeleteService(hService);
	::CloseServiceHandle(hService);
	::CloseServiceHandle(hSCM);

	if (bDelete)  return TRUE;
	LogEvent(_T("Service could not be deleted"));
	return FALSE;
}
#endif

//��¼�����¼�
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
