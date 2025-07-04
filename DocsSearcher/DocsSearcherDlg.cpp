﻿
// DocsSearcherDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "DocsSearcher.h"
#include "DocsSearcherDlg.h"
#include "afxdialogex.h"
#include "EncodingUtils.h"
#include "XmlUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CDocsSearcherDlg 대화 상자



CDocsSearcherDlg::CDocsSearcherDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DOCSSEARCHER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDocsSearcherDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_FOLDER, folder_edit_);
	DDX_Control(pDX, IDC_EDIT_KEYWORD, keyword_edit_);
	DDX_Control(pDX, IDC_LIST_RESULT, result_list_);
}

BEGIN_MESSAGE_MAP(CDocsSearcherDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_FOLDER, &CDocsSearcherDlg::OnBnClickedBtnFolder)
	ON_BN_CLICKED(IDC_BTN_KEYWORD, &CDocsSearcherDlg::OnBnClickedBtnKeyword)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_RESULT, &CDocsSearcherDlg::OnNMDblclkResultList)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_RESULT, &CDocsSearcherDlg::OnListCustomDraw)
	ON_MESSAGE(WM_ADD_RESULT, &CDocsSearcherDlg::OnAddResult)
END_MESSAGE_MAP()


// CDocsSearcherDlg 메시지 처리기

BOOL CDocsSearcherDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// 리스트 컨트롤: 전체 행 선택 + 그리드
	result_list_.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	// "파일명 | 경로 | 문맥" 컬럼 추가
	constexpr int kColWidthName = 100;
	constexpr int kColWidthPath = 300;
	constexpr int kColWidthCtx = 300;
	result_list_.InsertColumn(0, _T("파일명"), LVCFMT_LEFT, kColWidthName);
	result_list_.InsertColumn(1, _T("파일 경로"), LVCFMT_LEFT, kColWidthPath);
	result_list_.InsertColumn(2, _T("검색어 주변 내용"), LVCFMT_LEFT, kColWidthCtx);

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CDocsSearcherDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CDocsSearcherDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CDocsSearcherDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


// --------------------------------------
// CDocsSearcherDlg::OnBnClickedBtnFolder
// --------------------------------------
// 1) 버튼 클릭시, 폴더 선택 다이얼로그 생성
// 2) 폴더 선택시 경로 저장
// 3) 에딧 컨트롤에 경로 출력
void CDocsSearcherDlg::OnBnClickedBtnFolder()
{
	// 1. 다이얼로그 생성
	CFolderPickerDialog dlg(
		nullptr,  // pszPath: 기본 경로
		OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,  // dwFlags: 존재하는경로만 | 올바른경로만
		this,
		0
	);

	// 2. 다이얼로그 확인시
	if (dlg.DoModal() == IDOK) 
	{
		// 선택한 폴더 절대경로 저장
		CString folder_path = dlg.GetPathName();

		// 3. 에디트 컨트롤에 표시
		folder_edit_.SetWindowTextW(folder_path);
	}
}


// ---------------------------------------
// CDocsSearcherDlg::OnBnClickedBtnKeyword
// ---------------------------------------
// 1) 버튼 클릭시, 폴더 경로, 키워드 읽기
// 2) 폴더 경로의 하위 파일 탐색
// 3) 적합한 파일을 읽어 키워드 검색
// 4) 적합한 결과 저장
// 5) 결과를 리스트 컨트롤에 표시
void CDocsSearcherDlg::OnBnClickedBtnKeyword()
{
	CString folder_path, target_keyword;

	// 1. 경로, 키워드 읽기
	folder_edit_.GetWindowText(folder_path);
	keyword_edit_.GetWindowText(target_keyword);

	if (folder_path.IsEmpty() || target_keyword.IsEmpty())
	{
		AfxMessageBox(_T("폴더 또는 검색어가 지정되지 않았습니다."));
		return;
	}

	// 리스트 초기화
	result_list_.DeleteAllItems();

	// Debug code
	/*
	{
		CString msg;
		msg.Format(_T("선택한 폴더 경로:\n%s\n선택한 키워드:\n%s"), static_cast<LPCTSTR>(folder_path), static_cast<LPCTSTR>(target_keyword));
		AfxMessageBox(msg, MB_ICONINFORMATION);
	}
	*/

	// 폴더 탐색
	SearchFolder(folder_path, target_keyword);
	
}


// ------------------------------
// CDocsSearcherDlg::SearchFolder
// ------------------------------
// 
void CDocsSearcherDlg::SearchFolder(const CString& folder_path, const CString& target_keyword)
{
	std::vector<FileTask> file_tasks;
	CollectTargetFiles(folder_path, target_keyword, file_tasks);
	for (const auto& task : file_tasks)
	{
		std::thread t([this, task]() {
			CString context;
			bool is_found = SearchKeywordHandler(task.ext, task.file_path, task.keyword, context);
			if (is_found)
			{
				// UI 스레드에 결과 전달
				auto result = new SearchResult{
					::PathFindFileName(task.file_path),  // 파일명만 추출
					task.file_path,
					context
				};
				PostMessage(WM_ADD_RESULT, reinterpret_cast<WPARAM>(result), 0);
			}
		});
		t.detach();
	}
}


// ------------------------------------
// CDocsSearcherDlg::CollectTargetFiles
// ------------------------------------
// 
void CDocsSearcherDlg::CollectTargetFiles(const CString& folder_path, const CString& keyword, std::vector<FileTask>& out_tasks)
{
	CFileFind finder;
	CString search_spec = folder_path + _T("\\*.*");
	BOOL is_working = finder.FindFile(search_spec);

	const std::vector<CString> kAllowed = {  // 필터링할 확장자
		_T("txt"),
		_T("docx"),
		_T("hwpx"),
	};

	while (is_working)
	{
		is_working = finder.FindNextFile();

		// '.', ".." 무시
		if (finder.IsDots()) continue;

		// 폴더인 경우 재귀
		if (finder.IsDirectory())
		{
			CollectTargetFiles(finder.GetFilePath(), keyword, out_tasks);
			continue;
		}

		// 확장자 필터링
		CString ext = finder.GetFilePath();
		ext = ext.Right(ext.GetLength() - ext.ReverseFind('.') - 1).MakeLower();
		
		if (std::find(kAllowed.begin(), kAllowed.end(), ext) == kAllowed.end()) continue;

		// 필터링한 결과 수집
		FileTask task = { ext, finder.GetFilePath(), keyword };
		out_tasks.push_back(task);
	}
}


// ---------------------------------
// CDocsSearcherDlg::AddResultToList
// ---------------------------------
// 1) 전달받은 데이터를 리스트 컨트롤에 표시
void CDocsSearcherDlg::AddResultToList(const CString& file_name, const CString& file_path, const CString& context)
{
	int idx = result_list_.InsertItem(result_list_.GetItemCount(), file_name);
	result_list_.SetItemText(idx, 1, file_path);
	result_list_.SetItemText(idx, 2, context);
}


// --------------------------------------
// CDocsSearcherDlg::SearchKeywordHandler
// --------------------------------------
// 1) 확장자에 적합한 파일 읽기 메서드 호출
bool CDocsSearcherDlg::SearchKeywordHandler(const CString& ext, const CString& file_path, const CString& keyword, CString& context)
{
	bool is_found = false;

	if (ext.CompareNoCase(_T("txt")) == 0)
	{
		// txt
		is_found = SearchKeywordInTXT(file_path, keyword, context);
	}
	else if (ext.CompareNoCase(_T("docx")) == 0) 
	{
		// docx
		is_found = SearchKeywordInDocx(file_path, keyword, context);
	}
	else if (ext.CompareNoCase(_T("hwpx")) == 0) 
	{
		// hwpx
		is_found = SearchKeywordInHwpx(file_path, keyword, context);
	}
	return is_found;
}


// ------------------------------------
// CDocsSearcherDlg::SearchKeywordInTXT
// ------------------------------------
// 1) 파일 읽기
// 2) 파일 내용에서 키워드 찾기
// 3) 키워드를 발견하면 주변 문맥 저장
// 4) 발견 여부 반환
bool CDocsSearcherDlg::SearchKeywordInTXT(const CString& file_path, CString target_keyword, CString& context) 
{
	CString buffer;
	TextEncoding enc;

	// 파일 읽기
	if (!LoadTextFile(file_path, buffer, enc)) return false;

	// 2. 키워드 검색
	CString keyword = target_keyword;
	keyword.Trim();
	keyword.MakeLower();

	CString lower_buf = buffer;
	lower_buf.MakeLower();

	int position = lower_buf.Find(keyword);

	// 미발견
	if (position == -1) {
		/*
		{  // debug code
			CString msg = _T("미발견");
			AfxMessageBox(lower_buf, MB_ICONINFORMATION);
		}
		*/
		return false;
	}

	// 3. 주변 문맥 추출
	int start = max(0, position - 20);
	int end = min(buffer.GetLength(), position + keyword.GetLength() + 20);
	context = buffer.Mid(start, end - start);

	if (start > 0) context.Insert(0, _T("..."));
	if (end < buffer.GetLength()) context += _T("...");

	return true;
}


// ------------------------------------
// CDocsSearcherDlg::OnNMDblclkResultList
// ------------------------------------
// 1) 리스트 컨트롤 더블 클릭시 호출
// 2) 리스트 컨트롤에서 파일경로 추출
// 3) 윈도우 쉘로 파일 경로의 파일 오픈
void CDocsSearcherDlg::OnNMDblclkResultList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pItem = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;
	int row = pItem->iItem;

	{
		CString msg = _T("호출 테스트");
		AfxMessageBox(msg, MB_ICONINFORMATION);
	}
	
	// 빈공간의 경우
	if (row < 0) return;

	// 파일 경로 추출 (파일명 | 파일경로 | 문맥)
	CString file_path = result_list_.GetItemText(row, 1);

	// 경로가 빈 경우
	if (file_path.IsEmpty()) return;

	// 쉘로 파일 오픈
	HINSTANCE hInst = ::ShellExecute(
		nullptr,			// HWND
		_T("open"),			// lpOperation
		file_path,			// lpFile
		nullptr,			// lpParameters
		nullptr,			// lpDirectory
		SW_SHOWNORMAL		// nShowCmd
	);

	// 호출 실패(오류코드 0~32)
	if (reinterpret_cast<INT_PTR>(hInst) <= 32) 
	{
		CString msg;
		msg.Format(_T("파일을 열 수 없습니다:\n%s"), static_cast<LPCTSTR>(file_path));
		AfxMessageBox(msg, MB_ICONERROR);
	}
}


// ----------------------------------
// CDocsSearcherDlg::OnListCustomDraw
// ----------------------------------
//
void CDocsSearcherDlg::OnListCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	auto* cd = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);
	*pResult = CDRF_DODEFAULT;					// 기본값

	switch (cd->nmcd.dwDrawStage) 
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;			// ITEM 단계 요청
		break;

	case CDDS_ITEMPREPAINT:
		*pResult = CDRF_NOTIFYSUBITEMDRAW;		// SUBITEM 단계 요청
		break;

	case CDDS_SUBITEM | CDDS_ITEMPREPAINT: 
	{
		int row = static_cast<int>(cd->nmcd.dwItemSpec);
		int col = cd->iSubItem;

		CString current_keyword;
		keyword_edit_.GetWindowText(current_keyword);

		// 리스트 2열만 그리기
		if (col == 2 && !current_keyword.IsEmpty()) 
		{
			CDC* pDC = CDC::FromHandle(cd->nmcd.hdc);
			CRect rc;
			result_list_.GetSubItemRect(row, col, LVIR_LABEL, rc);

			CString text = result_list_.GetItemText(row, col);

			DrawHighlightedText(pDC, rc, text, current_keyword);

			*pResult = CDRF_SKIPDEFAULT;		// ListCtrl 기본 그리기 스킵
		}
		break;
	} // case
	} // switch
}


// -------------------------------------
// CDocsSearcherDlg::DrawHighlightedText
// -------------------------------------
//
void CDocsSearcherDlg::DrawHighlightedText(CDC* pDC, const CRect& rc, const CString& text, const CString& keyword)
{
	// 배경
	pDC->FillSolidRect(rc, ::GetSysColor(COLOR_WINDOW));

	// 대소문자 무시 검색
	CString lower = text;     lower.MakeLower();
	CString lower_kw = keyword; lower_kw.MakeLower();
	int pos = lower.Find(lower_kw);

	// 좌측 여백
	int x = rc.left + 4;
	int y = rc.top + (rc.Height() - pDC->GetTextExtent(_T("A")).cy) / 2;

	if (pos == -1) 
	{
		// 키워드가 없으면 한 번에
		pDC->SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
		pDC->ExtTextOut(x, y, ETO_CLIPPED, rc, text, nullptr);
		return;
	}

	CString left = text.Left(pos);
	CString mid = text.Mid(pos, keyword.GetLength());
	CString right = text.Mid(pos + keyword.GetLength());

	SIZE sz{};

	// 앞부분
	pDC->SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
	pDC->ExtTextOut(x, y, 0, nullptr, left, nullptr);
	x += pDC->GetTextExtent(left).cx;

	// 키워드(강조)
	pDC->SetTextColor(RGB(220, 20, 60));		// 빨강
	pDC->ExtTextOut(x, y, 0, nullptr, mid, nullptr);
	x += pDC->GetTextExtent(mid).cx;

	// 뒷부분
	pDC->SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
	pDC->ExtTextOut(x, y, 0, nullptr, right, nullptr);
}


// -----------------------------
// CDocsSearcherDlg::OnAddResult
// -----------------------------
//
LRESULT CDocsSearcherDlg::OnAddResult(WPARAM wParam, LPARAM lParam)
{
	std::unique_ptr<SearchResult> result(reinterpret_cast<SearchResult*>(wParam));
	AddResultToList(result->file_name, result->file_path, result->context);
	return 0;
}