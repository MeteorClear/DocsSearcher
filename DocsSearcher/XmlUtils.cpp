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
} // namespace

// ������ �ܺ� �Լ� ������

// ---------------
// ExtractDocxText
// ---------------
// .docx(OOXML) ���Ͽ��� word/document.xml �������� �ؽ�Ʈ ����
std::wstring ExtractDocxText(const std::wstring& filepath) 
{
    // libzip�� utf-8 ��θ� ���
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    const std::string utf8_path = conv.to_bytes(filepath);

    int err = 0;
    std::unique_ptr<zip_t, decltype(&zip_close)> za(zip_open(utf8_path.c_str(), ZIP_RDONLY, &err), &zip_close);
    if (!za) return {};

    const std::string xml_buf = ReadZipEntry(za.get(), "word/document.xml");
    if (xml_buf.empty()) return {};

    pugi::xml_document doc;
    if (!doc.load_buffer(xml_buf.data(), xml_buf.size(),pugi::parse_default | pugi::parse_ws_pcdata)) return {};

    return CollectAllText(doc);   // pugi::xml_document �� xml_node �Ϲ� ��ȯ
}