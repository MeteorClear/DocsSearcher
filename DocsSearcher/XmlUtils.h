#pragma once

#ifndef DOCSSEARCHER_XML_UTILS_H_
#define DOCSSEARCHER_XML_UTILS_H_

#include <string>
#include <vector>
#include <algorithm>
#include <codecvt>
#include <locale>
#include <memory>

#include <zip.h>			// libzip
#include <pugixml.hpp>		// pugixml

std::wstring ExtractDocxText(const std::wstring& filepath);  // .docx 파일에서 데이터 추출 메서드
std::wstring ExtractHwpxText(const std::wstring& filepath);  // .hwpx 파일에서 데이터 추출 메서드

#endif  // DOCSSEARCHER_XML_UTILS_H_