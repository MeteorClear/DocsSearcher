﻿
// DocsSearcherDlg.h: 헤더 파일
//

#pragma once

#define WM_ADD_RESULT (WM_USER + 1)

// 구조체
struct FileTask
{
	CString ext;
	CString file_path;
	CString keyword;
};  // 스레드 파라미터로 넘기는 용도의 데이터 구조체
struct SearchResult
{
	CString file_name;
	CString file_path;
	CString context;
};  // 스레드에서 사용할 검색 결과 데이터 전달용 메시지 구조체

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
	// 제어 변수
	CEdit folder_edit_;  // 폴더 선택 제어 변수
	CEdit keyword_edit_;  // 키워드 검색 제어 변수
	CListCtrl result_list_;  // 결과 목록 제어 변수

	// 버튼 클릭 메서드
	afx_msg void OnBnClickedBtnFolder();
	afx_msg void OnBnClickedBtnKeyword();

	// 리스트 클릭 메서드
	afx_msg void OnNMDblclkResultList(NMHDR* pNMHDR, LRESULT* pResult); // 리스트 컨트롤 항목 더블 클릭시 해당 파일을 여는 메서드
	afx_msg void OnListCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);  // 리스트 컨트롤 항목을 직접 그리는 메서드

	// 멀티 스레드용 메시지 핸들러
	afx_msg LRESULT CDocsSearcherDlg::OnAddResult(WPARAM wParam, LPARAM lParam);  // 리스트 컨트롤에 데이터 등록
private:
	void SearchFolder(const CString& folder_path, const CString& target_keyword);  // 폴더 검색 메서드
	void CollectTargetFiles(const CString& folder_path, const CString& keyword, std::vector<FileTask>& out_tasks);  // 특정 파일 경로만 필터링해 수집하는 메서드
	void AddResultToList(const CString& file_name, const CString& file_path, const CString& context);  // 리스트 컨트롤에 데이터 추가 메서드
	bool SearchKeywordHandler(const CString& ext, const CString& file_path, const CString& keyword, CString& context);  // 파일 확장자별 파일 읽기 메서드 호출 메서드
	bool SearchKeywordInTXT(const CString& file_path, CString keyword, CString& context);  // txt 확장자 파일 읽고 검색하는 메서드
	void DrawHighlightedText(CDC* pDC, const CRect& rc, const CString& text, const CString& keyword);  // 리스트 컨트롤에서 특정 키워드를 강조하는 메서드
};

