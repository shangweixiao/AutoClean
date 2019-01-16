
// AutoCleanSettingDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "AutoCleanSetting.h"
#include "AutoCleanSettingDlg.h"
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
	DDX_Control(pDX, IDC_RADIO_DAILY, m_RepeateModeDaily);
	DDX_Control(pDX, IDC_RADIO_WEEKLY, m_RepeateModeWeekly);
	DDX_Control(pDX, IDC_RADIO_MONTHLY, m_RepeateModeMonthly);
	DDX_Control(pDX, IDC_EDIT_DAILY_REPEATE_DAYS, m_RepeateDays);
	DDX_Control(pDX, IDC_CHECK_WEEKLY_SUN, m_RepeateWeeklySun);
	DDX_Control(pDX, IDC_CHECK_WEEKLY_MON, m_RepeateWeeklyMon);
	DDX_Control(pDX, IDC_CHECK_WEEKLY_TUE, m_RepeateWeeklyTue);
	DDX_Control(pDX, IDC_CHECK_WEEKLY_WED, m_RepeateWeeklyWed);
	DDX_Control(pDX, IDC_CHECK_WEEKLY_THU, m_RepeateWeeklyThu);
	DDX_Control(pDX, IDC_CHECK_WEEKLY_FRI, m_RepeateWeeklyFri);
	DDX_Control(pDX, IDC_CHECK_WEEKLY_SAT, m_RepeateWeeklySat);
	DDX_Control(pDX, IDC_CHECK_MONTHLY_1, m_RepeateMonthly1);
	DDX_Control(pDX, IDC_CHECK_MONTHLY_2, m_RepeateMonthly2);
	DDX_Control(pDX, IDC_CHECK_MONTHLY_3, m_RepeateMonthly3);
	DDX_Control(pDX, IDC_CHECK_MONTHLY_4, m_RepeateMonthly4);
	DDX_Control(pDX, IDC_CHECK_MONTHLY_5, m_RepeateMonthly5);
	DDX_Control(pDX, IDC_CHECK_MONTHLY_6, m_RepeateMonthly6);
	DDX_Control(pDX, IDC_CHECK_MONTHLY_7, m_RepeateMonthly7);
	DDX_Control(pDX, IDC_CHECK_MONTHLY_8, m_RepeateMonthly8);
	DDX_Control(pDX, IDC_CHECK_MONTHLY_9, m_RepeateMonthly9);
	DDX_Control(pDX, IDC_CHECK_MONTHLY_10, m_RepeateMonthly10);
	DDX_Control(pDX, IDC_CHECK_MONTHLY_11, m_RepeateMonthly11);
	DDX_Control(pDX, IDC_CHECK_MONTHLY_12, m_RepeateMonthly12);
	DDX_Control(pDX, IDC_CHECK_MONTHLY_13, m_RepeateMonthly13);
	DDX_Control(pDX, IDC_CHECK_MONTHLY_14, m_RepeateMonthly14);
	DDX_Control(pDX, IDC_CHECK_MONTHLY_15, m_RepeateMonthly15);
	DDX_Control(pDX, IDC_CHECK_MONTHLY_16, m_RepeateMonthly16);
	DDX_Control(pDX, IDC_CHECK_MONTHLY_17, m_RepeateMonthly17);
	DDX_Control(pDX, IDC_CHECK_MONTHLY_18, m_RepeateMonthly18);
	DDX_Control(pDX, IDC_CHECK_MONTHLY_19, m_RepeateMonthly19);
	DDX_Control(pDX, IDC_CHECK_MONTHLY_20, m_RepeateMonthly20);
	DDX_Control(pDX, IDC_CHECK_MONTHLY_21, m_RepeateMonthly21);
	DDX_Control(pDX, IDC_CHECK_MONTHLY_22, m_RepeateMonthly22);
	DDX_Control(pDX, IDC_CHECK_MONTHLY_23, m_RepeateMonthly23);
	DDX_Control(pDX, IDC_CHECK_MONTHLY_24, m_RepeateMonthly24);
	DDX_Control(pDX, IDC_CHECK_MONTHLY_25, m_RepeateMonthly25);
	DDX_Control(pDX, IDC_CHECK_MONTHLY_26, m_RepeateMonthly26);
	DDX_Control(pDX, IDC_CHECK_MONTHLY_27, m_RepeateMonthly27);
	DDX_Control(pDX, IDC_CHECK_MONTHLY_28, m_RepeateMonthly28);
	DDX_Control(pDX, IDC_CHECK_MONTHLY_29, m_RepeateMonthly29);
	DDX_Control(pDX, IDC_CHECK_MONTHLY_30, m_RepeateMonthly30);
	DDX_Control(pDX, IDC_CHECK_MONTHLY_31, m_RepeateMonthly31);
	DDX_Control(pDX, IDC_STATIC_DAILY_MSG, m_RepeateDailyMsg);
	DDX_Control(pDX, IDC_STATIC_DAILY_MSG2, m_RepeateDailyMsg1);
}

BEGIN_MESSAGE_MAP(CAutoCleanSettingDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_RADIO_DAILY, &CAutoCleanSettingDlg::OnBnClickedRadioDaily)
	ON_BN_CLICKED(IDC_RADIO_WEEKLY, &CAutoCleanSettingDlg::OnBnClickedRadioWeekly)
	ON_BN_CLICKED(IDC_RADIO_MONTHLY, &CAutoCleanSettingDlg::OnBnClickedRadioMonthly)
	ON_BN_CLICKED(IDOK, &CAutoCleanSettingDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CAutoCleanSettingDlg 消息处理程序
void CAutoCleanSettingDlg::InitVisiable(REPEATE_MODE_T mode)
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
	DWORD RepeateMode = 0;
	BYTE buf[32];
	ULONG length = sizeof(buf);
	CString day;
	

	key.Open(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\AutoClean");
	key.QueryDWORDValue("RepeatMode", RepeateMode);
	key.QueryBinaryValue("RunningDay", buf,&length);

	switch (RepeateMode)
	{
	case REPEATE_MODE_DAILY:
		day.Format("%d", buf[0]);
		m_RepeateModeDaily.SetCheck(1);
		m_RepeateDays.SetWindowText(day);
		break;
	case REPEATE_MODE_WEEKLY:
		m_RepeateModeWeekly.SetCheck(1);
		for (int i = 0; i < 7; i++) // tm_wday: [0 - 6],sun,mon,...,sat
		{
			if (buf[i / 8] & (1UL << (i % 8)))
			{
				CButton *pwndctrl = (CButton*)GetDlgItem(m_RepeateWeeklyDays[i]);
				pwndctrl->SetCheck(1);
			}
		}
		break;
	case REPEATE_MODE_MONTHLY:
		m_RepeateModeMonthly.SetCheck(1);
		for (int i = 0; i < 31; i++) // tm_wday: [0 - 6],sun,mon,...,sat
		{
			if (buf[i / 8] & (1UL << (i % 8)))
			{
				CButton *pwndctrl = (CButton*)GetDlgItem(m_RepeateMonthlyDays[i]);
				pwndctrl->SetCheck(1);
			}
		}
		break;
	default:
		m_RepeateModeDaily.SetCheck(1);
		RepeateMode = REPEATE_MODE_DAILY;
		break;
	}
	
	InitVisiable((REPEATE_MODE_T)RepeateMode);

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
	InitVisiable(REPEATE_MODE_DAILY);
}


void CAutoCleanSettingDlg::OnBnClickedRadioWeekly()
{
	// TODO: 在此添加控件通知处理程序代码
	InitVisiable(REPEATE_MODE_WEEKLY);
}


void CAutoCleanSettingDlg::OnBnClickedRadioMonthly()
{
	// TODO: 在此添加控件通知处理程序代码
	InitVisiable(REPEATE_MODE_MONTHLY);
}


void CAutoCleanSettingDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();
}
