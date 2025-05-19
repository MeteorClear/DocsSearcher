#include "pch.h"  // MFC ���
#include "EncodingUtils.h"


// ������ ���� �Լ� ������
namespace 
{
    // ---------
    // DetectBom
    // ---------
    // ���� �պκ� �� ����Ʈ�� �о� BOM ��� ���ڵ� ����
    TextEncoding DetectBom(const std::vector<BYTE>& b) 
    {
        if (b.size() >= 3 && b[0] == 0xEF && b[1] == 0xBB && b[2] == 0xBF) return TextEncoding::kUtf8;    // EF BB BF
        if (b.size() >= 2 && b[0] == 0xFF && b[1] == 0xFE)                 return TextEncoding::kUtf16LE; // FF FE
        if (b.size() >= 2 && b[0] == 0xFE && b[1] == 0xFF)                 return TextEncoding::kUtf16BE; // FE FF
        return TextEncoding::kAnsi;  // BOM ����
    }


    // -----------
    // IsValidUtf8
    // -----------
    // UTF-8 �������� �ùٸ��� �˻�
    bool IsValidUtf8(const BYTE* s, size_t n) 
    {
        for (size_t i = 0; i < n;) 
        {
            BYTE c = s[i];
            if (c < 0x80)  // ASCII
            { 
                ++i; 
                continue; 
            }

            // ��Ƽ����Ʈ ������ ���� �Ǻ�
            size_t seq_len = (c & 0xE0) == 0xC0 ? 2 : 
                             (c & 0xF0) == 0xE0 ? 3 : 
                             (c & 0xF8) == 0xF0 ? 4 : 0;
            if (!seq_len || i + seq_len > n) return false;

            // ���� ����Ʈ�� ��� 10xxxxxx �������� Ȯ��
            for (size_t j = 1; j < seq_len; ++j) 
                if ((s[i + j] & 0xC0) != 0x80) 
                    return false;

            i += seq_len;
        }
        return true;
    }

    // -------
    // ToUtf16
    // -------
    // 1) �־��� ����Ʈ �迭(s, n)�� UTF-16(CStringW)�� ��ȯ
    // 2) e�� UTF-16LE �� �״�� ����, UTF-8/ANSI �ΰ�� MultiByteToWideChar()
    bool ToUtf16(const BYTE* s, size_t n, TextEncoding e, CStringW& out) 
    {
        // 16LE
        if (e == TextEncoding::kUtf16LE) 
        { 
            out.SetString(reinterpret_cast<const wchar_t*>(s), static_cast<int>(n / 2));
            return true;
        }

        // 16BE
        if (e == TextEncoding::kUtf16BE) return false;

        // 8 or ANSI => 16
        UINT cp = (e == TextEncoding::kUtf8) ? CP_UTF8 : CP_ACP;

        int wlen = ::MultiByteToWideChar(cp, 0, reinterpret_cast<LPCSTR>(s), static_cast<int>(n), nullptr, 0);
        if (wlen <= 0) return false;

        std::vector<wchar_t> wbuf(static_cast<size_t>(wlen));
        ::MultiByteToWideChar(cp, 0, reinterpret_cast<LPCSTR>(s), static_cast<int>(n), wbuf.data(), wlen);
        out.SetString(wbuf.data(), wlen);

        return true;
    }
} // namespace

// ������ �ܺ� �Լ� ������

// ------------
// LoadTextFile
// ------------
// 1) ���� �б�
// 2) ���� BOM �˻�
// 3) �ʿ�� ��ȯ
// 4) ��� ����
bool LoadTextFile(const CString& path, CString& content_out, TextEncoding& enc_out) 
{
    // ���� ���̳ʸ��� �б�
    CFile file;
    if (!file.Open(path, CFile::modeRead | CFile::typeBinary)) return false;

    ULONGLONG size = file.GetLength();
    std::vector<BYTE> buf(static_cast<size_t>(size));
    if (size && file.Read(buf.data(), static_cast<UINT>(size)) != size) return false;

    // BOM �˻�
    TextEncoding enc = DetectBom(buf);
    size_t offset = (enc == TextEncoding::kUtf8) ? 3 : 
                    (enc == TextEncoding::kUtf16LE || 
                     enc == TextEncoding::kUtf16BE) ? 2 : 0;

    // BOM ���ų�, 8�ΰ�� 8�� ����
    if (enc == TextEncoding::kAnsi && IsValidUtf8(buf.data(), buf.size())) enc = TextEncoding::kUtf8;

    // ��ȯ
    CStringW wide;
    if (!ToUtf16(buf.data() + offset, buf.size() - offset, enc, wide)) 
    {
        // �ȵǸ� ANSI�� ��õ�
        enc = TextEncoding::kAnsi;
        if (!ToUtf16(buf.data(), buf.size(), enc, wide)) return false;
    }

    // ��� ����
    content_out = wide;
    enc_out = enc;

    return true;
}
