#include "pch.h"  // MFC 헤더
#include "EncodingUtils.h"


// ─── 내부 함수 ───
namespace 
{
    // ---------
    // DetectBom
    // ---------
    // 파일 앞부분 몇 바이트를 읽어 BOM 기반 인코딩 추정
    TextEncoding DetectBom(const std::vector<BYTE>& b) 
    {
        if (b.size() >= 3 && b[0] == 0xEF && b[1] == 0xBB && b[2] == 0xBF) return TextEncoding::kUtf8;    // EF BB BF
        if (b.size() >= 2 && b[0] == 0xFF && b[1] == 0xFE)                 return TextEncoding::kUtf16LE; // FF FE
        if (b.size() >= 2 && b[0] == 0xFE && b[1] == 0xFF)                 return TextEncoding::kUtf16BE; // FE FF
        return TextEncoding::kAnsi;  // BOM 없음
    }


    // -----------
    // IsValidUtf8
    // -----------
    // UTF-8 시퀀스가 올바른지 검사
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

            // 멀티바이트 시퀀스 길이 판별
            size_t seq_len = (c & 0xE0) == 0xC0 ? 2 : 
                             (c & 0xF0) == 0xE0 ? 3 : 
                             (c & 0xF8) == 0xF0 ? 4 : 0;
            if (!seq_len || i + seq_len > n) return false;

            // 이후 바이트가 모두 10xxxxxx 형태인지 확인
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
    // 1) 주어진 바이트 배열(s, n)을 UTF-16(CStringW)로 변환
    // 2) e가 UTF-16LE 면 그대로 복사, UTF-8/ANSI 인경우 MultiByteToWideChar()
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

// ─── 외부 함수 ───

// ------------
// LoadTextFile
// ------------
// 1) 파일 읽기
// 2) 파일 BOM 검사
// 3) 필요시 변환
// 4) 결과 저장
bool LoadTextFile(const CString& path, CString& content_out, TextEncoding& enc_out) 
{
    // 파일 바이너리로 읽기
    CFile file;
    if (!file.Open(path, CFile::modeRead | CFile::typeBinary)) return false;

    ULONGLONG size = file.GetLength();
    std::vector<BYTE> buf(static_cast<size_t>(size));
    if (size && file.Read(buf.data(), static_cast<UINT>(size)) != size) return false;

    // BOM 검사
    TextEncoding enc = DetectBom(buf);
    size_t offset = (enc == TextEncoding::kUtf8) ? 3 : 
                    (enc == TextEncoding::kUtf16LE || 
                     enc == TextEncoding::kUtf16BE) ? 2 : 0;

    // BOM 없거나, 8인경우 8로 간주
    if (enc == TextEncoding::kAnsi && IsValidUtf8(buf.data(), buf.size())) enc = TextEncoding::kUtf8;

    // 변환
    CStringW wide;
    if (!ToUtf16(buf.data() + offset, buf.size() - offset, enc, wide)) 
    {
        // 안되면 ANSI로 재시도
        enc = TextEncoding::kAnsi;
        if (!ToUtf16(buf.data(), buf.size(), enc, wide)) return false;
    }

    // 결과 저장
    content_out = wide;
    enc_out = enc;

    return true;
}
