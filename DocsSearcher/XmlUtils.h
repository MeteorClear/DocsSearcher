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

std::wstring ExtractDocxText(const std::wstring& filepath);  // .docx ���Ͽ��� ������ ���� �Լ�
std::wstring ExtractHwpxText(const std::wstring& filepath);  // .hwpx ���Ͽ��� ������ ���� �Լ�
bool SearchKeywordInDocx(const CString& file_path, const CString& keyword, CString& context);  // .docx ���Ͽ��� Ű���� �˻� �Լ�
bool SearchKeywordInHwpx(const CString& file_path, const CString& keyword, CString& context);  // .hwpx ���Ͽ��� Ű���� �˻� �Լ�

#endif  // DOCSSEARCHER_XML_UTILS_H_