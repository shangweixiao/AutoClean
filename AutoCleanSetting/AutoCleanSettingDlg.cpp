
// AutoCleanSettingDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "AutoCleanSetting.h"
#include "AutoCleanSettingDlg.h"
#include "SelectFolderDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAutoCleanSettingDlg 对话框



CAutoCleanSettingDlg::CAutoCleanSettingDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_AUTOCLEANSETTING_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAutoCleanSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RADIO_DAILY, m_RepeatModeDaily);
	DDX_Control(pDX, IDC_RADIO_WEEKLY, m_RepeatModeWeekly);
	DDX_Control(pDX, IDC_RADIO_MONTHLY, m_RepeatModeMonthly);
}

BEGIN_MESSAGE_MAP(CAutoCleanSettingDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_RADIO_DAILY, &CAutoCleanSettingDlg::OnBnClickedRadioDaily)
	ON_BN_CLICKED(IDC_RADIO_WEEKLY, &CAutoCleanSettingDlg::OnBnClickedRadioWeekly)
	ON_BN_CLICKED(IDC_RADIO_MONTHLY, &CAutoCleanSettingDlg::OnBnClickedRadioMonthly)
	ON_BN_CLICKED(IDOK, &CAutoCleanSettingDlg::OnBnClickedOk)
	ON_EN_CHANGE(IDC_EDIT_DAILY_REPEATE_DAYS, &CAutoCleanSettingDlg::OnEnChangeEditDailyRepeateDays)
	ON_BN_CLICKED(IDC_BUTTON_SELECT_FOLDER, &CAutoCleanSettingDlg::OnBnClickedButtonSelectFolder)
	ON_EN_CHANGE(IDC_EDIT_DAYS_BEFORE, &CAutoCleanSettingDlg::OnEnChangeEditDaysBefore)
END_MESSAGE_MAP()


// CAutoCleanSettingDlg 消息处理程序
void CAutoCleanSettingDlg::SetVisiable(REPEATE_MODE_T mode)
{
	int DailyShow = 0, WeeklyShow = 0, MonthlyShow = 0;
	if(REPEATE_MODE_DAILY == mode)
	{
		DailyShow = 1;
	}
	else if (REPEATE_MODE_WEEKLY == mode)
	{
		WeeklyShow = 1;
	}
	else if (REPEATE_MODE_MONTHLY == mode)
	{
		MonthlyShow = 1;
	}

	for (int i = 0; i < 3; i++)
	{
		CWnd *pwndctrl = GetDlgItem(m_RepeateDaily[i]);
		pwndctrl->ShowWindow(DailyShow);
	}

	for (int i = 0; i < 7; i++)
	{
		CWnd *pwndctrl = GetDlgItem(m_RepeateWeeklyDays[i]);
		pwndctrl->ShowWindow(WeeklyShow);
	}

	for (int i = 0; i < 31; i++)
	{
		CWnd *pwndctrl = GetDlgItem(m_RepeateMonthlyDays[i]);
		pwndctrl->ShowWindow(MonthlyShow);
	}
}

void CAutoCleanSettingDlg::InitChildPos()
{
	CWnd *pwndGroupBox = GetDlgItem(IDC_STATIC_GB_DAYS);
	CRect rectGb;
	pwndGroupBox->GetWindowRect(&rectGb);
	ScreenToClient(&rectGb);
	rectGb.bottom = rectGb.top + 160;
	rectGb.right = rectGb.left + 407;
	pwndGroupBox->MoveWindow(&rectGb);

	for (int i = 0; i < 3; i++)
	{
		CWnd *pwndctrl = GetDlgItem(m_RepeateDaily[i]);
		
		CRect rectWnd;
		pwndctrl->GetWindowRect(&rectWnd);
		ScreenToClient(&rectWnd);

		rectWnd.bottom = rectGb.top + (rectWnd.bottom - rectWnd.top) + 32;
		rectWnd.top = rectGb.top + 32;
		pwndctrl->MoveWindow(&rectWnd);
	}

	for (int i = 0; i < 7; i++)
	{
		CWnd *pwndctrl = GetDlgItem(m_RepeateWeeklyDays[i]);

		CRect rectWnd;
		int line = 0;

		line += i / 4;

		pwndctrl->GetWindowRect(&rectWnd);
		ScreenToClient(&rectWnd);

		rectWnd.bottom = rectGb.top + (rectWnd.bottom - rectWnd.top) + 50 + line * 30;
		rectWnd.top = rectGb.top + 50 + line * 30;
		pwndctrl->MoveWindow(&rectWnd);
	}

	for (int i = 0; i < 31; i++)
	{
		CWnd *pwndctrl = GetDlgItem(m_RepeateMonthlyDays[i]);

		CRect rectWnd;
		int line = 0;

		line += i / 10;

		pwndctrl->GetWindowRect(&rectWnd);
		ScreenToClient(&rectWnd);

		rectWnd.bottom = rectGb.top + (rectWnd.bottom - rectWnd.top) + 25 + line * 30;
		rectWnd.top = rectGb.top + 25 + line * 30;
		pwndctrl->MoveWindow(&rectWnd);
	}
}
BOOL CAutoCleanSettingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	ShowWindow(SW_MAXIMIZE);

	ShowWindow(SW_MINIMIZE);

	// TODO: 在此添加额外的初始化代码
	CRegKey key;
	DWORD RepeatMode = 0;
	DWORD DaysBefore = 0;
	BYTE RunningDay[4] = { 0 };
	BYTE bNextRunTime[8] = { 0 };
	ULONG length;
	CString day;
	char *CleanPath = NULL;
	LSTATUS status;
	time_t NextRunTime;

	status = key.Open(HKEY_LOCAL_MACHINE, RegKeyPath);
	if (ERROR_SUCCESS != status)
	{
		status = key.Create(HKEY_LOCAL_MACHINE, RegKeyPath);
		if (ERROR_SUCCESS != status)
		{
			MessageBox("注册表创建错误，请以管理员权限运行本程序。","警告",MB_OK);
		}
	}

	key.QueryDWORDValue("RepeatMode", RepeatMode);
	key.QueryDWORDValue("DaysBefore", DaysBefore);
	
	length = sizeof(RunningDay);
	key.QueryBinaryValue("RunningDay", RunningDay,&length);

	key.QueryStringValue("CleanPath", NULL, &length);
	CleanPath = (char *)calloc(1,length + 2);
	key.QueryStringValue("CleanPath", CleanPath, &length);

	length = sizeof(bNextRunTime);
	key.QueryBinaryValue("NextRunTime", bNextRunTime, &length);
	NextRunTime = *(time_t*)bNextRunTime;

	key.Close();

	if (NextRunTime > 0)
	{
		struct tm *local;
		local = localtime(&NextRunTime);

		CStatic *pwndNextRunTime = (CStatic*)GetDlgItem(IDC_STATIC_NEXT_RUNTIME);
		day.Format("下次计划运行时间为：%d-%d-%d 。",local->tm_year + 1900,local->tm_mon + 1,local->tm_mday);
		pwndNextRunTime->SetWindowText(day);
		pwndNextRunTime->ShowWindow(1);
	}

	CEdit *pwndCleanPath = (CEdit*)GetDlgItem(IDC_EDIT_CLEAN_PATH);
	pwndCleanPath->SetWindowText(CleanPath);

	CEdit *pwndDaysBefore = (CEdit*)GetDlgItem(IDC_EDIT_DAYS_BEFORE);
	day.Format("%d", DaysBefore);
	pwndDaysBefore->SetWindowText(day);

	switch (RepeatMode)
	{
	case REPEATE_MODE_DAILY:
		day.Format("%d", RunningDay[0] + RunningDay[1]);
		m_RepeatModeDaily.SetCheck(1);
		{
			CEdit *pwndctrl = (CEdit*)GetDlgItem(IDC_EDIT_DAILY_REPEATE_DAYS);
			pwndctrl->SetWindowText(day);
		}
		break;
	case REPEATE_MODE_WEEKLY:
		m_RepeatModeWeekly.SetCheck(1);
		for (int i = 0; i < 7; i++) // tm_wday: [0 - 6],sun,mon,...,sat
		{
			if (RunningDay[i / 8] & (1UL << (i % 8)))
			{
				CButton *pwndctrl = (CButton*)GetDlgItem(m_RepeateWeeklyDays[i]);
				pwndctrl->SetCheck(1);
			}
		}
		break;
	case REPEATE_MODE_MONTHLY:
		m_RepeatModeMonthly.SetCheck(1);
		for (int i = 0; i < 31; i++) // tm_wday: [0 - 6],sun,mon,...,sat
		{
			if (RunningDay[i / 8] & (1UL << (i % 8)))
			{
				CButton *pwndctrl = (CButton*)GetDlgItem(m_RepeateMonthlyDays[i]);
				pwndctrl->SetCheck(1);
			}
		}
		break;
	default:
		m_RepeatModeDaily.SetCheck(1);
		RepeatMode = REPEATE_MODE_DAILY;
		break;
	}
	
	InitChildPos();
	SetVisiable((REPEATE_MODE_T)RepeatMode);

	free(CleanPath);
	ShowWindow(SW_NORMAL);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CAutoCleanSettingDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CAutoCleanSettingDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CAutoCleanSettingDlg::OnBnClickedRadioDaily()
{
	// TODO: 在此添加控件通知处理程序代码
	SetVisiable(REPEATE_MODE_DAILY);
}


void CAutoCleanSettingDlg::OnBnClickedRadioWeekly()
{
	// TODO: 在此添加控件通知处理程序代码
	SetVisiable(REPEATE_MODE_WEEKLY);
}


void CAutoCleanSettingDlg::OnBnClickedRadioMonthly()
{
	// TODO: 在此添加控件通知处理程序代码
	SetVisiable(REPEATE_MODE_MONTHLY);
}


DWORD CAutoCleanSettingDlg::SetNextRunTime(time_t cur, REPEATE_MODE_T RepeatMode,char RunningDay[4])
{
	time_t NextRunTime;

	if (REPEATE_MODE_DAILY == RepeatMode)
	{
		NextRunTime = cur + 24 * 3600 * (RunningDay[0] + RunningDay[1]);
	}
	else if (REPEATE_MODE_WEEKLY == RepeatMode)
	{
		struct tm *local;
		int i;
		local = localtime(&cur);
		NextRunTime = cur;
		for (i = (local->tm_wday + 1) % 7; ; i = (i + 1) % 7) // tm_wday: [0 - 6]
		{
			NextRunTime += 24 * 3600;
			if (RunningDay[i / 8] & (1UL << (i % 8)))
			{
				break;
			}

			if (i == local->tm_wday)
			{
				break;
			}
		}
	}
	else if (REPEATE_MODE_MONTHLY == RepeatMode)
	{
		struct tm *local, *nrt;
		int i;
		local = localtime(&cur);
		NextRunTime = cur;
		for (i = local->tm_mday; ; i = (i + 1) % 31) // tm_mday: [1 - 31]
		{
			NextRunTime += 24 * 3600;
			nrt = localtime(&NextRunTime);
			if ((nrt->tm_mday - 1) != i)
			{
				i = nrt->tm_mday - 1;
			}

			if (RunningDay[i / 8] & (1UL << (i % 8)))
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

	CRegKey key = NULL;
	ULONG length = sizeof(NextRunTime);

	key.Open(HKEY_LOCAL_MACHINE, RegKeyPath);
	key.SetBinaryValue("NextRunTime", (char*)&NextRunTime, length);
	key.Close();

	return 0;
}

void CAutoCleanSettingDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	char buf[4] = { 0 };
	REPEATE_MODE_T RepeatMode = REPEATE_MODE_UNDEFINE;
	CEdit *pwndCleanPath = (CEdit*)GetDlgItem(IDC_EDIT_CLEAN_PATH);
	CString CleanPath;
	pwndCleanPath->GetWindowText(CleanPath);

	if(CleanPath.GetLength() < 2)
	{
		MessageBox("请选择要自动清理的文件夹。","警告", MB_OK);
		return;
	}

	if(m_RepeatModeDaily.GetCheck())
	{
		CString days;
		CEdit* pwndctrl = (CEdit*)GetDlgItem(IDC_EDIT_DAILY_REPEATE_DAYS);
		pwndctrl->GetWindowText(days);

		buf[0] = atoi(days) > 255 ? 255 : atoi(days);
		buf[1] = atoi(days) > 255 ? atoi(days) - 255 : 0;
		RepeatMode = REPEATE_MODE_DAILY;
	} 
	else if (m_RepeatModeWeekly.GetCheck())
	{
		for (int i = 0; i < 7; i++) // tm_wday: [0 - 6],sun,mon,...,sat
		{
			CButton *pwndctrl = (CButton*)GetDlgItem(m_RepeateWeeklyDays[i]);
			if (pwndctrl->GetCheck())
			{
				buf[i / 8] |= (1UL << (i % 8));
			}
		}

		RepeatMode = REPEATE_MODE_WEEKLY;
	} 
	else if (m_RepeatModeMonthly.GetCheck())
	{
		for (int i = 0; i < 31; i++)
		{
			CButton *pwndctrl = (CButton*)GetDlgItem(m_RepeateMonthlyDays[i]);
			if (pwndctrl->GetCheck())
			{
				buf[i / 8] |= (1UL << (i % 8));
			}
		}

		RepeatMode = REPEATE_MODE_MONTHLY;
	}

	if(0 == buf[0] + buf[1] + buf[2] + buf[3])
	{
		MessageBox("时间设置错误，请选择要自动清理的时间。", "警告", MB_OK);
		if (REPEATE_MODE_DAILY == RepeatMode)
		{
			CEdit *pwndctrl = (CEdit*)GetDlgItem(IDC_EDIT_DAILY_REPEATE_DAYS);
			pwndctrl->SetSel(0,-1);
			pwndctrl->SetFocus();
		}
		return;
	}

	CString DaysBefore;
	CEdit *pwndDaysBefore = (CEdit*)GetDlgItem(IDC_EDIT_DAYS_BEFORE);
	pwndDaysBefore->GetWindowText(DaysBefore);

	CRegKey key = NULL;
	ULONG length = sizeof(buf);

	key.Open(HKEY_LOCAL_MACHINE, RegKeyPath);
	key.SetDWORDValue("RepeatMode", RepeatMode);
	key.SetBinaryValue("RunningDay", buf, length);
	key.SetStringValue("CleanPath", CleanPath);
	key.SetDWORDValue("DaysBefore", atoi(DaysBefore));
	key.Close();

	SetNextRunTime(time(NULL), RepeatMode,buf);
	CDialogEx::OnOK();
}


void CAutoCleanSettingDlg::OnEnChangeEditDailyRepeateDays()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	CString days;
	CEdit *pwndctrl = (CEdit*)GetDlgItem(IDC_EDIT_DAILY_REPEATE_DAYS);
	pwndctrl->GetWindowText(days);

	if(days.GetLength() > 3)
	{
		days = days.Left(3);
		pwndctrl->SetWindowText(days);
		pwndctrl->SetSel(3, 3, FALSE);
	}

	if (atoi(days) > 365)
	{
		days = days.Left(2);
		pwndctrl->SetWindowText(days);
		pwndctrl->SetSel(2,2, FALSE);
	}
}


void CAutoCleanSettingDlg::OnBnClickedButtonSelectFolder()
{
	// TODO: 在此添加控件通知处理程序代码
	CString CleanPath,SelectPath;
	
	CEdit *pwndctrl = (CEdit*)GetDlgItem(IDC_EDIT_CLEAN_PATH);
	pwndctrl->GetWindowText(CleanPath);
	SelectPath = CSelectFolderDlg::Show();
	if (-1 == CleanPath.Find(SelectPath))
	{
		if (CleanPath.GetLength() + SelectPath.GetLength() < 64 * 1024)
		{
			if (CleanPath.GetLength() > 2)
			{
				CleanPath += ";";
			}
			CleanPath += SelectPath;
		}
	}
	pwndctrl->SetWindowText(CleanPath);
}


void CAutoCleanSettingDlg::OnEnChangeEditDaysBefore()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	CString days;
	CEdit *pwndctrl = (CEdit*)GetDlgItem(IDC_EDIT_DAYS_BEFORE);
	pwndctrl->GetWindowText(days);

	if (days.GetLength() > 3)
	{
		days = days.Left(3);
		pwndctrl->SetWindowText(days);
		pwndctrl->SetSel(3, 3, FALSE);
	}

	if (atoi(days) > 365)
	{
		days = days.Left(2);
		pwndctrl->SetWindowText(days);
		pwndctrl->SetSel(2, 2, FALSE);
	}
}
