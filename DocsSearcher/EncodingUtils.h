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
    kAnsi  // ���� �ڵ� ������(CP949)
};

bool LoadTextFile(const CString& path, CString& out_text, TextEncoding& out_encoding);  // ���̳ʸ��� �о� UTF-16 CString ���� ��ȯ�ϴ� �޼���

#endif  // DOCSSEARCHER_ENCODING_UTILS_H_