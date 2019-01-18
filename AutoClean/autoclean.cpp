// CreateService.cpp : ����Ӧ�ó������ڵ㡣
//

#include <tchar.h>  
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <process.h>

//#define  SERVICE_DEBUG

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
	char *CleanPath; // Ҫɾ����·��
	__time64_t NextRunTime; // ��һ������ʱ��
	__time64_t NotifiedTime; // ���û���֪ͨʱ��
	DWORD  DaysBefore;
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
		nRetVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\AutoClean", 0, KEY_READ, &hKey);
		if (ERROR_SUCCESS != nRetVal)
		{
			hKey = NULL;
			printf("RegOpenKeyEx Error Line: %d\n", __LINE__);
			nRetVal = 1;
			break;
		}

		ZeroMemory(pchRegValueBuf, REG_VALUE_BUF_SIZE);
		dwBufSize = REG_VALUE_BUF_SIZE;
		nRetVal = RegQueryValueEx(hKey, "RepeatMode", NULL, &dwType, (BYTE *)pchRegValueBuf, &dwBufSize);
		if (ERROR_SUCCESS != nRetVal)
		{
			printf("RegQueryValueEx Error Line: %d\n", __LINE__);
			nRetVal = 2;
		}
		else
		{
			RegData->RepeatMode = *(REPEATE_MODE_T*)&pchRegValueBuf[0];
			printf("RegQueryValueEx RepeatMode= %d\n", RegData->RepeatMode);
		}

		ZeroMemory(pchRegValueBuf, REG_VALUE_BUF_SIZE);
		dwBufSize = REG_VALUE_BUF_SIZE;
		nRetVal = RegQueryValueEx(hKey, "RunningDay", NULL, &dwType, (BYTE *)pchRegValueBuf, &dwBufSize);
		if (ERROR_SUCCESS != nRetVal)
		{
			printf("RegQueryValueEx Error Line: %d\n", __LINE__);
			nRetVal = 2;
		}
		else
		{
			memcpy(RegData->RunningDay, pchRegValueBuf,sizeof(RegData->RunningDay));
			printf("RegQueryValueEx RunningDay= 0x%x,0x%x,0x%x,0x%x\n", RegData->RunningDay[3], RegData->RunningDay[2], RegData->RunningDay[1], RegData->RunningDay[0]);
		}

		dwBufSize = 0;
		RegData->CleanPath = NULL;
		nRetVal = RegQueryValueEx(hKey, "CleanPath", NULL, &dwType, NULL, &dwBufSize);
		if (ERROR_SUCCESS == nRetVal)
		{
			dwBufSize += 2;
			RegData->CleanPath = (char*)malloc(dwBufSize);
			ZeroMemory(RegData->CleanPath, dwBufSize);

			nRetVal = RegQueryValueEx(hKey, "CleanPath", NULL, &dwType, (BYTE *)RegData->CleanPath, &dwBufSize);
			if (ERROR_SUCCESS != nRetVal)
			{
				printf("RegQueryValueEx Error Line: %d\n", __LINE__);
				free(RegData->CleanPath);
				RegData->CleanPath = NULL;
				nRetVal = 2;
			}
		}

		ZeroMemory(pchRegValueBuf, REG_VALUE_BUF_SIZE);
		dwBufSize = REG_VALUE_BUF_SIZE;
		nRetVal = RegQueryValueEx(hKey, "NextRunTime", NULL, &dwType, (BYTE *)pchRegValueBuf, &dwBufSize);
		if (ERROR_SUCCESS != nRetVal)
		{
			printf("RegQueryValueEx Error Line: %d\n", __LINE__);
			nRetVal = 2;
		}
		else
		{
			RegData->NextRunTime = *(__time64_t*)&pchRegValueBuf[0];
			printf("RegQueryValueEx NextRunTime= %I64u\n", RegData->NextRunTime);
		}

		ZeroMemory(pchRegValueBuf, REG_VALUE_BUF_SIZE);
		dwBufSize = REG_VALUE_BUF_SIZE;
		nRetVal = RegQueryValueEx(hKey, "NotifiedTime", NULL, &dwType, (BYTE *)pchRegValueBuf, &dwBufSize);
		if (ERROR_SUCCESS != nRetVal)
		{
			printf("RegQueryValueEx Error Line: %d\n", __LINE__);
			nRetVal = 2;
			RegData->NotifiedTime = 0;
		}
		else
		{
			RegData->NotifiedTime = *(__time64_t*)&pchRegValueBuf[0];
			printf("RegQueryValueEx NotifiedTime= %I64u\n", RegData->NotifiedTime);
		}

		ZeroMemory(pchRegValueBuf, REG_VALUE_BUF_SIZE);
		dwBufSize = REG_VALUE_BUF_SIZE;
		nRetVal = RegQueryValueEx(hKey, "DaysBefore", NULL, &dwType, (BYTE *)pchRegValueBuf, &dwBufSize);
		if (ERROR_SUCCESS != nRetVal)
		{
			printf("RegQueryValueEx Error Line: %d\n", __LINE__);
			nRetVal = 2;
			RegData->DaysBefore = 0;
		}
		else
		{
			RegData->DaysBefore = *(DWORD*)&pchRegValueBuf[0];
			printf("RegQueryValueEx DaysBefore= %u\n", RegData->DaysBefore);
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

#include <tlhelp32.h>
#include <WtsApi32.h>
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Wtsapi32.lib")

DWORD SendMsessgeAndWaitForRespones(LPSTR pTitle, DWORD TitleLength, LPSTR pMessage, DWORD MessageLength, DWORD Style)
{
	DWORD sessionId = WTSGetActiveConsoleSessionId();
	DWORD     dwRespon = 0xff;
	WTSSendMessageA((HANDLE)0, sessionId, pTitle, TitleLength, pMessage, MessageLength, Style, 0, &dwRespon, true);

	return dwRespon;
}

void DbgMessage(LPSTR pMessage, DWORD MessageLength)
{
	SendMsessgeAndWaitForRespones((LPSTR)"��ʾ", (DWORD)strlen("��ʾ"), pMessage, MessageLength, MB_OK);
}

VOID NotifyUserToBackFiles(time_t DelTime,char *CleanPath,int DaysBefore)
{
	struct tm tmFileTime, tmDelTime;

	memcpy(&tmDelTime, localtime(&DelTime),sizeof(struct tm));
	if (DaysBefore > 0)
	{
		time_t FileTime = DelTime - DaysBefore * 24 * 3600;
		memcpy(&tmFileTime, localtime(&FileTime), sizeof(struct tm));
	}
	else
	{
		memcpy(&tmFileTime, &tmDelTime, sizeof(struct tm));
		if (tmFileTime.tm_mon > 2)
		{
			tmFileTime.tm_mon -= 3;
		}
		else
		{
			tmFileTime.tm_year -= 1;
			tmFileTime.tm_mon = tmFileTime.tm_mon + 12 - 3;
		}
	}

	char *msg = (char*)calloc(1,strlen(CleanPath) + 256);
	int length = sprintf(msg, "ϵͳ���� %d-%d-%d ��� %s ·���� %d-%d-%d ǰ�����ݣ�������Ҫ�����뼰ʱ���ݡ�", tmDelTime.tm_year + 1900, tmDelTime.tm_mon + 1, tmDelTime.tm_mday, CleanPath, tmFileTime.tm_year + 1900, tmFileTime.tm_mon + 1, tmFileTime.tm_mday);
	SendMsessgeAndWaitForRespones((LPSTR)"����",(DWORD)strlen("����"),msg,length,MB_OK);
	free(msg);
}

DWORD WaitUserToConfirm(time_t DelTime, char *CleanPath, int DaysBefore)
{
	struct tm tmFileTime, tmDelTime;

	memcpy(&tmDelTime, localtime(&DelTime), sizeof(struct tm));
	if (DaysBefore > 0)
	{
		time_t FileTime = DelTime - DaysBefore * 24 * 3600;
		memcpy(&tmFileTime, localtime(&FileTime), sizeof(struct tm));
	}
	else
	{
		memcpy(&tmFileTime, &tmDelTime, sizeof(struct tm));
		if (tmFileTime.tm_mon > 2)
		{
			tmFileTime.tm_mon -= 3;
		}
		else
		{
			tmFileTime.tm_year -= 1;
			tmFileTime.tm_mon = tmFileTime.tm_mon + 12 - 3;
		}
	}

	char *msg = (char*)calloc(1,strlen(CleanPath) + 256);
	int length = sprintf(msg, "ϵͳ����ʼ��� %s Ŀ¼�� %d-%d-%d ǰ�����ݣ����ȷ����ʼ������������ȡ����ȡ����������ƻ���", CleanPath, tmFileTime.tm_year + 1900, tmFileTime.tm_mon + 1, tmFileTime.tm_mday);
	DWORD dwRespon = SendMsessgeAndWaitForRespones((LPSTR)"����", (DWORD)strlen("����"), msg, length, MB_OKCANCEL);
	free(msg);
	if (IDOK == dwRespon)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

DWORD SetRegData(const char *key,char *val,int size)
{
#define REG_VALUE_BUF_SIZE		64
	LONG nRetVal = ERROR_SUCCESS;
	HKEY hKey = NULL;
	DWORD dwBufSize = REG_VALUE_BUF_SIZE;

	for (;;)
	{
		nRetVal = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\AutoClean", 0, KEY_WRITE, &hKey);
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


DWORD SetNextRunTime(time_t cur, RegData_t *rd)
{
	time_t NextRunTime;

	if(REPEATE_MODE_DAILY == rd->RepeatMode)
	{
		NextRunTime = cur + (rd->RunningDay[0] + rd->RunningDay[1])*24 * 3600;
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

typedef DWORD(*FILE_PROCESSOR_CB)(LPTSTR szPath, LPTSTR szFileName, DWORD DaysBefore);

/*
* DWORD EnumerateFileInDirectory(LPSTR szPath,LPWSTR filter,BOOL subdir, FILE_PROCESSOR_CB fp_cb)
* ���ܣ�����Ŀ¼�µ��ļ�����Ŀ¼
*
* ������LPSTR szPath��Ϊ�������·��
*
* ����ֵ��0����ִ����ɣ�1�����ʹ���
*/
DWORD EnumerateFileInDirectory(LPTSTR szPath, LPTSTR filter, BOOL recursion, FILE_PROCESSOR_CB fp_cb, DWORD DaysBefore)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hListFile;
	char szFilePath[MAX_PATH] = { 0 };

	// ���������Ŀ¼���ļ���·�����ַ�����ʹ��ͨ���"*"
	strcpy(szFilePath, szPath);

	// ע�͵Ĵ���������ڲ��������ԡ�.txt����β���ļ�
	lstrcat(szFilePath, "\\");
	strcat(szFilePath, filter);

	// ���ҵ�һ���ļ�/Ŀ¼����ò��Ҿ��
	hListFile = FindFirstFile(szFilePath, &FindFileData);
	// �жϾ��
	if (hListFile == INVALID_HANDLE_VALUE)
	{
		printf("����%d\n", GetLastError());
		return 1;
	}
	else
	{
		do
		{
			/* ���������ʾ������Ŀ¼���ϼ�Ŀ¼�ġ�.���͡�..����
			����ʹ��ע�Ͳ��ֵĴ������*/
			if (strcmp(FindFileData.cFileName, ".") == 0 ||
				strcmp(FindFileData.cFileName, "..") == 0)
			{
				continue;
			}

			// �ж��ļ����ԣ��Ƿ�ΪĿ¼
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (recursion)
				{
					char szChildPath[MAX_PATH] = { 0 };
					strcpy(szChildPath, szPath);
					strcat(szChildPath, "\\");
					strcat(szChildPath, FindFileData.cFileName);
					EnumerateFileInDirectory(szChildPath, filter, recursion, fp_cb, DaysBefore);
				}
			}
			else
			{
				//wprintf(L"%ws\n", FindFileData.cFileName);
				if (NULL != fp_cb)
				{
					fp_cb(szPath, FindFileData.cFileName, DaysBefore);
				}
			}
		} while (FindNextFile(hListFile, &FindFileData));
		FindClose(hListFile);
	}
	return 0;
}

DWORD DeleteFileCb(LPTSTR szPath, LPTSTR szFileName,DWORD DaysBefore)
{
	char szFilePath[MAX_PATH] = { 0 };

	strcpy(szFilePath, szPath);
	strcat(szFilePath, "\\");
	strcat(szFilePath, szFileName);

	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(szFilePath, &ffd);
	SYSTEMTIME stUTC, stLocal,stCur;
	FileTimeToSystemTime(&(ffd.ftLastWriteTime), &stUTC);
	SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

	GetLocalTime(&stCur);
	
	if (1 > DaysBefore )
	{
		DWORD DiffMon = (stCur.wYear - stLocal.wYear) * 12 + stCur.wMonth - stLocal.wMonth;
		if (DiffMon  > 3)
		{
			DeleteFile(szFilePath);
		}
		else if (DiffMon == 3)
		{
			if (stCur.wDay > stLocal.wDay)
			{
				DeleteFile(szFilePath);
			}
		}
	}
	else
	{
		FILETIME ftCur;
		SystemTimeToFileTime(&stCur,&ftCur);
		ULARGE_INTEGER ulLWT = { ffd.ftLastWriteTime.dwLowDateTime,ffd.ftLastWriteTime.dwHighDateTime };
		ULARGE_INTEGER ulCur = { ftCur.dwLowDateTime,ftCur.dwHighDateTime };

		if ((ulCur.QuadPart - ulLWT.QuadPart)/10/1000/1000 > DaysBefore * 24 * 3600)
		{
			DeleteFile(szFilePath);
		}
	}
	return 0;
}

DWORD DeleteUserFiles(char *CleanPath,DWORD DaysBefore)
{
	char *pContext;
	char *pPath = (char *)calloc(1,strlen(CleanPath) + 2);
	strcpy(pPath,CleanPath);

	char *pSubPath = strtok_s(pPath,";",&pContext);
	while (NULL != pSubPath)
	{
		EnumerateFileInDirectory(pSubPath, (LPTSTR)"*", TRUE, DeleteFileCb, DaysBefore);
		pSubPath = strtok_s(NULL, ";", &pContext);
	}

	free(pPath);
	return 0;
}

unsigned AutoCleanThread(void * param)
{
	RegData_t rd;
	DWORD ret;

	time_t cur;
	struct tm local, NextRunTime, *plocal, *pNextRunTime;
	double diff;

	// check user logon
	while (1)
	{
		DWORD sessionId = WTSGetActiveConsoleSessionId();
		LPSTR  pbuf;
		DWORD length;
		BOOL ret = WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, sessionId, WTSConnectState,&pbuf,&length);
		if (ret)
		{
			if (0 == *(DWORD*)pbuf)
			{
				WTSFreeMemory(pbuf);
				break;
			}
			WTSFreeMemory(pbuf);
		}

		Sleep(3 * 1000);
	}

	Sleep(5 * 1000);

	GetRegData(&rd);

	cur = time(NULL);
	plocal = localtime(&cur);
	memcpy(&local, plocal, sizeof(struct tm));

	pNextRunTime = localtime(&rd.NextRunTime);
	memcpy(&NextRunTime, pNextRunTime, sizeof(struct tm));

	if (REPEATE_MODE_DAILY != rd.RepeatMode && REPEATE_MODE_WEEKLY != rd.RepeatMode && REPEATE_MODE_MONTHLY != rd.RepeatMode)
	{
		LogEvent(_T("�ظ��������ô���"));
	}
	else
	{
		diff = difftime(rd.NextRunTime, cur);
		if ((0 > diff) || ((local.tm_year == NextRunTime.tm_year) && (local.tm_mon == NextRunTime.tm_mon) && (local.tm_mday == NextRunTime.tm_mday)))
		{
			ret = WaitUserToConfirm(rd.NextRunTime, rd.CleanPath, rd.DaysBefore);
			if (0 == ret)
			{
				SetNextRunTime(cur, &rd);
				DeleteUserFiles(rd.CleanPath, rd.DaysBefore);
				LogEvent(_T("Ŀ¼ %s ������ɡ�ɾ���� %d%s ǰ���ļ���"), rd.CleanPath, rd.DaysBefore > 0?rd.DaysBefore:3, rd.DaysBefore > 0?"��":"����");
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
				NotifyUserToBackFiles(rd.NextRunTime, rd.CleanPath, rd.DaysBefore);
				LogEvent(_T("�ٽ�ɾ�����ڣ���ʾ�û������ļ���"));
			}
		}
	}

	if (NULL != rd.CleanPath)
	{
		free(rd.CleanPath);
	}

	_endthreadex(0);
	return 0;
}

#ifndef SERVICE_DEBUG
DWORD AutoCleanMain()
#else
int main(int argc, char **argv)
#endif
{
	HANDLE hThread = NULL;
	unsigned threadid = 0 ;

	_beginthreadex(NULL, 0, (_beginthreadex_proc_type)AutoCleanThread, NULL, 0, &threadid);

	while (1)
	{
		Sleep(1 * 1000);
	}
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
