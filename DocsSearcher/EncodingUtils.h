#pragma once

#ifndef DOCSSEARCHER_ENCODING_UTILS_H_
#define DOCSSEARCHER_ENCODING_UTILS_H_

#include <afx.h>  // CString, CFile
#include <vector>

enum class TextEncoding 
{
    kUtf8,
    kUtf16LE,
    kUtf16BE,
    kAnsi  // 로컬 코드 페이지(CP949)
};

bool LoadTextFile(const CString& path, CString& out_text, TextEncoding& out_encoding);  // 바이너리로 읽어 UTF-16 CString 으로 변환하는 메서드

#endif  // DOCSSEARCHER_ENCODING_UTILS_H_