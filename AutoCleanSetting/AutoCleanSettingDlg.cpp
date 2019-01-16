
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
		{
			CEdit *pDays = (CEdit*)GetDlgItem(IDC_EDIT_DAILY_REPEATE_DAYS);
			pDays->SetWindowText(day);
		}
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
