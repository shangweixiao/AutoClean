
// AutoCleanSettingDlg.h: 头文件
//

#pragma once


// CAutoCleanSettingDlg 对话框
class CAutoCleanSettingDlg : public CDialogEx
{
// 构造
public:
	CAutoCleanSettingDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_AUTOCLEANSETTING_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	typedef enum {
		REPEATE_MODE_UNDEFINE = 0,
		REPEATE_MODE_WEEKLY,
		REPEATE_MODE_MONTHLY,
		REPEATE_MODE_DAILY,
	}REPEATE_MODE_T;

	const DWORD m_RepeateDaily[3] = {IDC_STATIC_DAILY_MSG,IDC_EDIT_DAILY_REPEATE_DAYS,IDC_STATIC_DAILY_MSG2};

	const DWORD m_RepeateWeeklyDays[7] = {IDC_CHECK_WEEKLY_SUN,IDC_CHECK_WEEKLY_MON,IDC_CHECK_WEEKLY_TUE,IDC_CHECK_WEEKLY_WED,IDC_CHECK_WEEKLY_THU,IDC_CHECK_WEEKLY_FRI,IDC_CHECK_WEEKLY_SAT};

	const DWORD m_RepeateMonthlyDays[31] = {IDC_CHECK_MONTHLY_1,IDC_CHECK_MONTHLY_2,IDC_CHECK_MONTHLY_3,IDC_CHECK_MONTHLY_4,IDC_CHECK_MONTHLY_5,
											IDC_CHECK_MONTHLY_6,IDC_CHECK_MONTHLY_7,IDC_CHECK_MONTHLY_8,IDC_CHECK_MONTHLY_9,IDC_CHECK_MONTHLY_10,
											IDC_CHECK_MONTHLY_11,IDC_CHECK_MONTHLY_12,IDC_CHECK_MONTHLY_13,IDC_CHECK_MONTHLY_14,IDC_CHECK_MONTHLY_15,
											IDC_CHECK_MONTHLY_16,IDC_CHECK_MONTHLY_17,IDC_CHECK_MONTHLY_18,IDC_CHECK_MONTHLY_19,IDC_CHECK_MONTHLY_20,
											IDC_CHECK_MONTHLY_21,IDC_CHECK_MONTHLY_22,IDC_CHECK_MONTHLY_23,IDC_CHECK_MONTHLY_24,IDC_CHECK_MONTHLY_25,
											IDC_CHECK_MONTHLY_26,IDC_CHECK_MONTHLY_27,IDC_CHECK_MONTHLY_28,IDC_CHECK_MONTHLY_29,IDC_CHECK_MONTHLY_30,
											IDC_CHECK_MONTHLY_31};
// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	void InitVisiable(REPEATE_MODE_T mode);
public:
	CButton m_RepeateModeDaily;
	CButton m_RepeateModeWeekly;
	CButton m_RepeateModeMonthly;
	CEdit m_RepeateDays;
	CButton m_RepeateWeeklySun;
	CButton m_RepeateWeeklyMon;
	CButton m_RepeateWeeklyTue;
	CButton m_RepeateWeeklyWed;
	CButton m_RepeateWeeklyThu;
	CButton m_RepeateWeeklyFri;
	CButton m_RepeateWeeklySat;
	CButton m_RepeateMonthly1;
	CButton m_RepeateMonthly2;
	CButton m_RepeateMonthly3;
	CButton m_RepeateMonthly4;
	CButton m_RepeateMonthly5;
	CButton m_RepeateMonthly6;
	CButton m_RepeateMonthly7;
	CButton m_RepeateMonthly8;
	CButton m_RepeateMonthly9;
	CButton m_RepeateMonthly10;
	CButton m_RepeateMonthly11;
	CButton m_RepeateMonthly12;
	CButton m_RepeateMonthly13;
	CButton m_RepeateMonthly14;
	CButton m_RepeateMonthly15;
	CButton m_RepeateMonthly16;
	CButton m_RepeateMonthly17;
	CButton m_RepeateMonthly18;
	CButton m_RepeateMonthly19;
	CButton m_RepeateMonthly20;
	CButton m_RepeateMonthly21;
	CButton m_RepeateMonthly22;
	CButton m_RepeateMonthly23;
	CButton m_RepeateMonthly24;
	CButton m_RepeateMonthly25;
	CButton m_RepeateMonthly26;
	CButton m_RepeateMonthly27;
	CButton m_RepeateMonthly28;
	CButton m_RepeateMonthly29;
	CButton m_RepeateMonthly30;
	CButton m_RepeateMonthly31;
	CStatic m_RepeateDailyMsg;
	CStatic m_RepeateDailyMsg1;
	afx_msg void OnBnClickedRadioDaily();
	afx_msg void OnBnClickedRadioWeekly();
	afx_msg void OnBnClickedRadioMonthly();
	afx_msg void OnBnClickedOk();
};
