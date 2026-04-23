#pragma once

#include <string>
#include <vector>
#include <string_view>

namespace pinyingen {

class EncodingUtils {
public:
    static std::u32string utf8_to_utf32(std::string_view utf8_str);
    static std::string utf32_to_utf8(const std::u32string& utf32_str);
    
    static std::string gbk_to_utf8(std::string_view gbk_str);
    static std::string utf8_to_gbk(std::string_view utf8_str);
    
    static bool is_chinese_char(char32_t c);
    static bool is_chinese_string(std::string_view str);
    
    static std::vector<char32_t> string_to_codepoints(std::string_view str);
    static std::string codepoints_to_string(const std::vector<char32_t>& codepoints);
    
private:
    EncodingUtils() = delete;
    ~EncodingUtils() = delete;
};

}
