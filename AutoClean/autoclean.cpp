// CreateService.cpp : 定义应用程序的入口点。
//

#include <tchar.h>  
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <process.h>

//#define  SERVICE_DEBUG

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
	char *CleanPath; // 要删除的路径
	__time64_t NextRunTime; // 下一次运行时间
	__time64_t NotifiedTime; // 给用户的通知时间
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
	SendMsessgeAndWaitForRespones((LPSTR)"提示", (DWORD)strlen("提示"), pMessage, MessageLength, MB_OK);
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
	int length = sprintf(msg, "系统将于 %d-%d-%d 清除 %s 路径下 %d-%d-%d 前的数据，如有重要数据请及时备份。", tmDelTime.tm_year + 1900, tmDelTime.tm_mon + 1, tmDelTime.tm_mday, CleanPath, tmFileTime.tm_year + 1900, tmFileTime.tm_mon + 1, tmFileTime.tm_mday);
	SendMsessgeAndWaitForRespones((LPSTR)"警告",(DWORD)strlen("警告"),msg,length,MB_OK);
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
	int length = sprintf(msg, "系统将开始清除 %s 目录下 %d-%d-%d 前的数据，点击确定开始清除操作，点击取消以取消本次清除计划。", CleanPath, tmFileTime.tm_year + 1900, tmFileTime.tm_mon + 1, tmFileTime.tm_mday);
	DWORD dwRespon = SendMsessgeAndWaitForRespones((LPSTR)"警告", (DWORD)strlen("警告"), msg, length, MB_OKCANCEL);
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
* 功能：遍历目录下的文件和子目录
*
* 参数：LPSTR szPath，为需遍历的路径
*
* 返回值：0代表执行完成，1代表发送错误
*/
DWORD EnumerateFileInDirectory(LPTSTR szPath, LPTSTR filter, BOOL recursion, FILE_PROCESSOR_CB fp_cb, DWORD DaysBefore)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hListFile;
	char szFilePath[MAX_PATH] = { 0 };

	// 构造代表子目录和文件夹路径的字符串，使用通配符"*"
	strcpy(szFilePath, szPath);

	// 注释的代码可以用于查找所有以“.txt”结尾的文件
	lstrcat(szFilePath, "\\");
	strcat(szFilePath, filter);

	// 查找第一个文件/目录，获得查找句柄
	hListFile = FindFirstFile(szFilePath, &FindFileData);
	// 判断句柄
	if (hListFile == INVALID_HANDLE_VALUE)
	{
		printf("错误：%d\n", GetLastError());
		return 1;
	}
	else
	{
		do
		{
			/* 如果不想显示代表本级目录和上级目录的“.”和“..”，
			可以使用注释部分的代码过滤*/
			if (strcmp(FindFileData.cFileName, ".") == 0 ||
				strcmp(FindFileData.cFileName, "..") == 0)
			{
				continue;
			}

			// 判断文件属性，是否为目录
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
		LogEvent(_T("重复周期设置错误。"));
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
				LogEvent(_T("目录 %s 清理完成。删除了 %d%s 前的文件。"), rd.CleanPath, rd.DaysBefore > 0?rd.DaysBefore:3, rd.DaysBefore > 0?"天":"个月");
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
				NotifyUserToBackFiles(rd.NextRunTime, rd.CleanPath, rd.DaysBefore);
				LogEvent(_T("临近删除日期，提示用户备份文件。"));
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
