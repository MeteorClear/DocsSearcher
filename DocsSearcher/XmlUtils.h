#pragma once

#ifndef DOCSSEARCHER_XML_UTILS_H_
#define DOCSSEARCHER_XML_UTILS_H_

#include <string>
#include <vector>
#include <algorithm>
#include <codecvt>
#include <locale>
#include <memory>
#include <afx.h>
#include <zip.h>			// libzip
#include <pugixml.hpp>		// pugixml

std::wstring ExtractDocxText(const std::wstring& filepath);  // .docx 파일에서 데이터 추출 함수
std::wstring ExtractHwpxText(const std::wstring& filepath);  // .hwpx 파일에서 데이터 추출 함수
bool SearchKeywordInDocx(const CString& file_path, const CString& keyword, CString& context);  // .docx 파일에서 키워드 검색 함수
bool SearchKeywordInHwpx(const CString& file_path, const CString& keyword, CString& context);  // .hwpx 파일에서 키워드 검색 함수

#endif  // DOCSSEARCHER_XML_UTILS_H_