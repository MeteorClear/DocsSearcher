
// DocsSearcherDlg.h: 헤더 파일
//

#pragma once


// CDocsSearcherDlg 대화 상자
class CDocsSearcherDlg : public CDialogEx
{
// 생성입니다.
public:
	CDocsSearcherDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DOCSSEARCHER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton2();
	afx_msg void OnLvnItemchangedListResult(NMHDR* pNMHDR, LRESULT* pResult);
	
	// 제어 변수
	CEdit folder_edit_;  // 폴더 선택 제어 변수
	CEdit keyword_edit_;  // 키워드 검색 제어 변수
	CListCtrl result_list_;  // 결과 목록 제어 변수

	// 버튼 클릭 메서드
	afx_msg void OnBnClickedBtnFolder();
	afx_msg void OnBnClickedBtnKeyword();
private:
	void SearchFolder(const CString& folder_path, const CString& target_keyword);  // 폴더 검색 메서드
};
