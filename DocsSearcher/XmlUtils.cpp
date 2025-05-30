#include "pch.h"  // MFC ���
#include "XmlUtils.h"


// ������ ���� �Լ� ������
namespace
{
    // ------------
    // ReadZipEntry
    // ------------
    // zip �ڵ��� �̿��� ��Ʈ���� ���� ������ ��ȯ
    std::string ReadZipEntry(zip_t* za, const std::string& entry_name) 
    {
        zip_stat_t st;
        if (zip_stat(za, entry_name.c_str(), 0, &st) != 0) return {};

        std::unique_ptr<zip_file_t, decltype(&zip_fclose)> zf(zip_fopen(za, entry_name.c_str(), 0), &zip_fclose);
        if (!zf) return {};

        std::string buffer(st.size, '\0');          // ����� ���� Ȯ��
        char* writable = &buffer[0];                // non-const ����
        zip_int64_t n = zip_fread(zf.get(), writable, st.size);
        if (n != st.size) return {};
        return buffer;
    }


    // -----------
    // Utf16ToUtf8
    // -----------
    // utf-16 wstring �� utf-8 string ���� ��ȯ
    std::string Utf16ToUtf8(const std::wstring& utf16)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
        return conv.to_bytes(utf16);
    }


    // -----------
    // Utf8ToUtf16
    // -----------
    // utf-8 string �� utf-16 wstring ���� ��ȯ
    std::wstring Utf8ToUtf16(const std::string& utf8) 
    {
        static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
        return conv.from_bytes(utf8);
    }


    // --------------
    // CollectTextRec
    // --------------
    // XML Ʈ���� DFS �ϸ� ��� PCData/CDATA ���� UTF-16�� out�� ����
    void CollectTextRec(const pugi::xml_node& node, std::wstring& out) 
    {
        if (node.type() == pugi::node_pcdata || node.type() == pugi::node_cdata) 
        {
            out += Utf8ToUtf16(node.value());
            out += L' ';    // �ܾ� ����
        }
        for (pugi::xml_node child : node.children())
        {
            CollectTextRec(child, out);
        }
    }


    // --------------
    // CollectAllText
    // --------------
    // xml�� ��Ʈ ��带 �޾� ������ ��ȯ
    std::wstring CollectAllText(const pugi::xml_node& root) 
    {
        std::wstring result;
        CollectTextRec(root, result);
        return result;
    }


    // -------
    // ToLower
    // -------
    // wstring �ҹ��� ��ȯ
    std::wstring ToLower(const std::wstring& src) {
        std::locale loc;
        std::wstring out(src.size(), L'\0');
        std::transform(src.begin(), src.end(), out.begin(), [&loc](wchar_t ch) { return std::tolower(ch, loc); });
        return out;
    }


    // ------------
    // BuildContext
    // ------------
    // ���ۿ��� Ư�� ��ġ �ֺ� ���� ����
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

// ������ �ܺ� �Լ� ������

// ---------------
// ExtractDocxText
// ---------------
// .docx(OOXML) ���Ͽ��� word/document.xml �������� �ؽ�Ʈ ����
std::wstring ExtractDocxText(const std::wstring& filepath) 
{
    // libzip�� UTF-8 ��θ� ���
    const std::string utf8_path = Utf16ToUtf8(filepath);

    int err = 0;
    std::unique_ptr<zip_t, decltype(&zip_close)> za(zip_open(utf8_path.c_str(), ZIP_RDONLY, &err), &zip_close);
    if (!za) return {};

    const std::string xml_buf = ReadZipEntry(za.get(), "word/document.xml");
    if (xml_buf.empty()) return {};

    pugi::xml_document doc;
    if (!doc.load_buffer(xml_buf.data(), xml_buf.size(),pugi::parse_default | pugi::parse_ws_pcdata)) return {};

    return CollectAllText(doc);   // pugi::xml_document �� xml_node �Ϲ� ��ȯ
}


// ---------------
// ExtractHwpxText
// ---------------
// .hwpx ���Ͽ��� section*.xml �������� �ؽ�Ʈ ����
std::wstring ExtractHwpxText(const std::wstring& filepath) 
{
    // libzip�� UTF-8 ��θ� ���
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
// .docx �������� �ؽ�Ʈ ���� �� �˻�
bool SearchKeywordInDocx(const CString& file_path, const CString& keyword, CString& context) 
{
    // ���� ����
    std::wstring w_path(file_path.GetString());
    std::wstring text = ExtractDocxText(w_path);
    if (text.empty()) return false;

    // �˻��� ��ȯ
    std::wstring w_key(keyword.GetString());
    w_key = ToLower(w_key);
    std::wstring lower_text = ToLower(text);

    // �˻�
    const size_t pos = lower_text.find(w_key);
    if (pos == std::wstring::npos) return false;

    // ���� ����
    context = BuildContext(text, pos, w_key.length());

    return true;
}


// -------------------
// SearchKeywordInHwpx
// -------------------
// .hwpx �������� �ؽ�Ʈ ���� �� �˻�
bool SearchKeywordInHwpx(const CString& file_path, const CString& keyword, CString& context)
{
    // ���� ����
    std::wstring w_path(file_path.GetString());
    std::wstring text = ExtractHwpxText(w_path);
    if (text.empty()) return false;

    // �˻��� ��ȯ
    std::wstring w_key(keyword.GetString());
    w_key = ToLower(w_key);
    std::wstring lower_text = ToLower(text);

    // �˻�
    const size_t pos = lower_text.find(w_key);
    if (pos == std::wstring::npos) return false;

    // ���� ����
    context = BuildContext(text, pos, w_key.length());

    return true;
}