
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
	const CString RegKeyPath = "SOFTWARE\\AutoClean";

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	void SetVisiable(REPEATE_MODE_T mode);
	void InitChildPos();
	DWORD SetNextRunTime(time_t cur, REPEATE_MODE_T RepeatMode, char RunningDay[4]);
public:
	CButton m_RepeatModeDaily;
	CButton m_RepeatModeWeekly;
	CButton m_RepeatModeMonthly;
	afx_msg void OnBnClickedRadioDaily();
	afx_msg void OnBnClickedRadioWeekly();
	afx_msg void OnBnClickedRadioMonthly();
	afx_msg void OnBnClickedOk();
	afx_msg void OnEnChangeEditDailyRepeateDays();
	afx_msg void OnBnClickedButtonSelectFolder();
	afx_msg void OnEnChangeEditDaysBefore();
};
