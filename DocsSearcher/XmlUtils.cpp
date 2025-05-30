#include "pch.h"  // MFC 헤더
#include "XmlUtils.h"


// ─── 내부 함수 ───
namespace
{
    // ------------
    // ReadZipEntry
    // ------------
    // zip 핸들을 이용해 엔트리를 열어 내용을 반환
    std::string ReadZipEntry(zip_t* za, const std::string& entry_name) 
    {
        zip_stat_t st;
        if (zip_stat(za, entry_name.c_str(), 0, &st) != 0) return {};

        std::unique_ptr<zip_file_t, decltype(&zip_fclose)> zf(zip_fopen(za, entry_name.c_str(), 0), &zip_fclose);
        if (!zf) return {};

        std::string buffer(st.size, '\0');          // 충분한 공간 확보
        char* writable = &buffer[0];                // non-const 버퍼
        zip_int64_t n = zip_fread(zf.get(), writable, st.size);
        if (n != st.size) return {};
        return buffer;
    }


    // -----------
    // Utf16ToUtf8
    // -----------
    // utf-16 wstring 을 utf-8 string 으로 변환
    std::string Utf16ToUtf8(const std::wstring& utf16)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
        return conv.to_bytes(utf16);
    }


    // -----------
    // Utf8ToUtf16
    // -----------
    // utf-8 string 을 utf-16 wstring 으로 변환
    std::wstring Utf8ToUtf16(const std::string& utf8) 
    {
        static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
        return conv.from_bytes(utf8);
    }


    // --------------
    // CollectTextRec
    // --------------
    // XML 트리를 DFS 하며 모든 PCData/CDATA 값을 UTF-16로 out에 누적
    void CollectTextRec(const pugi::xml_node& node, std::wstring& out) 
    {
        if (node.type() == pugi::node_pcdata || node.type() == pugi::node_cdata) 
        {
            out += Utf8ToUtf16(node.value());
            out += L' ';    // 단어 구분
        }
        for (pugi::xml_node child : node.children())
        {
            CollectTextRec(child, out);
        }
    }


    // --------------
    // CollectAllText
    // --------------
    // xml의 루트 노드를 받아 내용을 반환
    std::wstring CollectAllText(const pugi::xml_node& root) 
    {
        std::wstring result;
        CollectTextRec(root, result);
        return result;
    }


    // -------
    // ToLower
    // -------
    // wstring 소문자 변환
    std::wstring ToLower(const std::wstring& src) {
        std::locale loc;
        std::wstring out(src.size(), L'\0');
        std::transform(src.begin(), src.end(), out.begin(), [&loc](wchar_t ch) { return std::tolower(ch, loc); });
        return out;
    }


    // ------------
    // BuildContext
    // ------------
    // 버퍼에서 특정 위치 주변 문맥 추출
    CString BuildContext(const std::wstring& buffer, size_t hit_pos, size_t hit_len, size_t span = 20) {
        const size_t total_len = buffer.length();
        const size_t start = (hit_pos > span) ? hit_pos - span : 0;
        const size_t end = min(total_len, hit_pos + hit_len + span);

        CString context(buffer.substr(start, end - start).c_str());
        if (start > 0) context.Insert(0, _T("..."));
        if (end < total_len) context += _T("...");
        return context;
    }
} // namespace

// ─── 외부 함수 ───

// ---------------
// ExtractDocxText
// ---------------
// .docx(OOXML) 파일에서 word/document.xml 본문에서 텍스트 추출
std::wstring ExtractDocxText(const std::wstring& filepath) 
{
    // libzip는 UTF-8 경로만 허용
    const std::string utf8_path = Utf16ToUtf8(filepath);

    int err = 0;
    std::unique_ptr<zip_t, decltype(&zip_close)> za(zip_open(utf8_path.c_str(), ZIP_RDONLY, &err), &zip_close);
    if (!za) return {};

    const std::string xml_buf = ReadZipEntry(za.get(), "word/document.xml");
    if (xml_buf.empty()) return {};

    pugi::xml_document doc;
    if (!doc.load_buffer(xml_buf.data(), xml_buf.size(),pugi::parse_default | pugi::parse_ws_pcdata)) return {};

    return CollectAllText(doc);   // pugi::xml_document → xml_node 암묵 변환
}


// ---------------
// ExtractHwpxText
// ---------------
// .hwpx 파일에서 section*.xml 본문에서 텍스트 추출
std::wstring ExtractHwpxText(const std::wstring& filepath) 
{
    // libzip는 UTF-8 경로만 허용
    const std::string utf8_path = Utf16ToUtf8(filepath);

    int err = 0;
    std::unique_ptr<zip_t, decltype(&zip_close)> za(zip_open(utf8_path.c_str(), ZIP_RDONLY, &err), &zip_close);
    if (!za) return {};

    std::wstring total;
    const zip_int64_t entry_count = zip_get_num_entries(za.get(), 0);

    for (zip_uint64_t i = 0; i < entry_count; ++i) 
    {
        const char* name = zip_get_name(za.get(), i, 0);
        if (!name) continue;

        std::string entry(name);
        const bool is_section = entry.rfind("Contents/section", 0) == 0 && entry.find(".xml") != std::string::npos;
        if (!is_section) continue;

        const std::string xml_buf = ReadZipEntry(za.get(), entry);
        if (xml_buf.empty()) continue;

        pugi::xml_document doc;
        if (doc.load_buffer(xml_buf.data(), xml_buf.size(), pugi::parse_default | pugi::parse_ws_pcdata))
        {
            CollectTextRec(doc, total);
        }
    }

    return total;
}


// -------------------
// SearchKeywordInDocx
// -------------------
// .docx 문서에서 텍스트 추출 후 검색
bool SearchKeywordInDocx(const CString& file_path, const CString& keyword, CString& context) 
{
    // 본문 추출
    std::wstring w_path(file_path.GetString());
    std::wstring text = ExtractDocxText(w_path);
    if (text.empty()) return false;

    // 검색어 변환
    std::wstring w_key(keyword.GetString());
    w_key = ToLower(w_key);
    std::wstring lower_text = ToLower(text);

    // 검색
    const size_t pos = lower_text.find(w_key);
    if (pos == std::wstring::npos) return false;

    // 문맥 추출
    context = BuildContext(text, pos, w_key.length());

    return true;
}


// -------------------
// SearchKeywordInHwpx
// -------------------
// .hwpx 문서에서 텍스트 추출 후 검색
bool SearchKeywordInHwpx(const CString& file_path, const CString& keyword, CString& context)
{
    // 본문 추출
    std::wstring w_path(file_path.GetString());
    std::wstring text = ExtractHwpxText(w_path);
    if (text.empty()) return false;

    // 검색어 변환
    std::wstring w_key(keyword.GetString());
    w_key = ToLower(w_key);
    std::wstring lower_text = ToLower(text);

    // 검색
    const size_t pos = lower_text.find(w_key);
    if (pos == std::wstring::npos) return false;

    // 문맥 추출
    context = BuildContext(text, pos, w_key.length());

    return true;
}