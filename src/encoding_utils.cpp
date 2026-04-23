#include "encoding_utils.h"
#include <stdexcept>
#include <cstring>

#if defined(_WIN32)
#include <windows.h>
#endif

namespace pinyingen {

std::u32string EncodingUtils::utf8_to_utf32(std::string_view utf8_str) {
    std::u32string result;
    result.reserve(utf8_str.size());
    
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(utf8_str.data());
    const uint8_t* end = ptr + utf8_str.size();
    
    while (ptr < end) {
        if (*ptr < 0x80) {
            result.push_back(static_cast<char32_t>(*ptr++));
        } else if ((*ptr & 0xE0) == 0xC0) {
            if (ptr + 1 >= end || (ptr[1] & 0xC0) != 0x80) {
                throw std::runtime_error("Invalid UTF-8 sequence");
            }
            char32_t c = ((*ptr++ & 0x1F) << 6);
            c |= (*ptr++ & 0x3F);
            result.push_back(c);
        } else if ((*ptr & 0xF0) == 0xE0) {
            if (ptr + 2 >= end || (ptr[1] & 0xC0) != 0x80 || (ptr[2] & 0xC0) != 0x80) {
                throw std::runtime_error("Invalid UTF-8 sequence");
            }
            char32_t c = ((*ptr++ & 0x0F) << 12);
            c |= ((*ptr++ & 0x3F) << 6);
            c |= (*ptr++ & 0x3F);
            result.push_back(c);
        } else if ((*ptr & 0xF8) == 0xF0) {
            if (ptr + 3 >= end || (ptr[1] & 0xC0) != 0x80 || 
                (ptr[2] & 0xC0) != 0x80 || (ptr[3] & 0xC0) != 0x80) {
                throw std::runtime_error("Invalid UTF-8 sequence");
            }
            char32_t c = ((*ptr++ & 0x07) << 18);
            c |= ((*ptr++ & 0x3F) << 12);
            c |= ((*ptr++ & 0x3F) << 6);
            c |= (*ptr++ & 0x3F);
            result.push_back(c);
        } else {
            throw std::runtime_error("Invalid UTF-8 sequence");
        }
    }
    return result;
}

std::string EncodingUtils::utf32_to_utf8(const std::u32string& utf32_str) {
    std::string result;
    result.reserve(utf32_str.size() * 4);
    
    for (char32_t c : utf32_str) {
        if (c <= 0x7F) {
            result.push_back(static_cast<char>(c));
        } else if (c <= 0x7FF) {
            result.push_back(static_cast<char>(0xC0 | (c >> 6)));
            result.push_back(static_cast<char>(0x80 | (c & 0x3F)));
        } else if (c <= 0xFFFF) {
            result.push_back(static_cast<char>(0xE0 | (c >> 12)));
            result.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (c & 0x3F)));
        } else if (c <= 0x10FFFF) {
            result.push_back(static_cast<char>(0xF0 | (c >> 18)));
            result.push_back(static_cast<char>(0x80 | ((c >> 12) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (c & 0x3F)));
        } else {
            throw std::runtime_error("Invalid UTF-32 character");
        }
    }
    return result;
}

std::string EncodingUtils::gbk_to_utf8(std::string_view gbk_str) {
#if defined(_WIN32)
    if (gbk_str.empty()) return {};
    
    int len = MultiByteToWideChar(CP_ACP, 0, gbk_str.data(), 
                                   static_cast<int>(gbk_str.size()), nullptr, 0);
    if (len <= 0) throw std::runtime_error("GBK to UTF-16 conversion failed");
    
    std::wstring wstr(len, L'\0');
    MultiByteToWideChar(CP_ACP, 0, gbk_str.data(), 
                         static_cast<int>(gbk_str.size()), &wstr[0], len);
    
    len = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), len, nullptr, 0, nullptr, nullptr);
    if (len <= 0) throw std::runtime_error("UTF-16 to UTF-8 conversion failed");
    
    std::string result(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), 
                         &result[0], len, nullptr, nullptr);
    return result;
#else
    throw std::runtime_error("GBK conversion not implemented on this platform");
#endif
}

std::string EncodingUtils::utf8_to_gbk(std::string_view utf8_str) {
#if defined(_WIN32)
    if (utf8_str.empty()) return {};
    
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8_str.data(), 
                                   static_cast<int>(utf8_str.size()), nullptr, 0);
    if (len <= 0) throw std::runtime_error("UTF-8 to UTF-16 conversion failed");
    
    std::wstring wstr(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8_str.data(), 
                         static_cast<int>(utf8_str.size()), &wstr[0], len);
    
    len = WideCharToMultiByte(CP_ACP, 0, wstr.data(), len, nullptr, 0, nullptr, nullptr);
    if (len <= 0) throw std::runtime_error("UTF-16 to GBK conversion failed");
    
    std::string result(len, '\0');
    WideCharToMultiByte(CP_ACP, 0, wstr.data(), static_cast<int>(wstr.size()), 
                         &result[0], len, nullptr, nullptr);
    return result;
#else
    throw std::runtime_error("GBK conversion not implemented on this platform");
#endif
}

bool EncodingUtils::is_chinese_char(char32_t c) {
    return (c >= 0x4E00 && c <= 0x9FFF) ||
           (c >= 0x3400 && c <= 0x4DBF) ||
           (c >= 0x20000 && c <= 0x2A6DF) ||
           (c >= 0x2A700 && c <= 0x2B73F) ||
           (c >= 0x2B740 && c <= 0x2B81F) ||
           (c >= 0x2B820 && c <= 0x2CEAF);
}

bool EncodingUtils::is_chinese_string(std::string_view str) {
    try {
        auto codepoints = utf8_to_utf32(str);
        for (char32_t c : codepoints) {
            if (!is_chinese_char(c)) {
                return false;
            }
        }
        return !codepoints.empty();
    } catch (...) {
        return false;
    }
}

std::vector<char32_t> EncodingUtils::string_to_codepoints(std::string_view str) {
    auto u32str = utf8_to_utf32(str);
    return std::vector<char32_t>(u32str.begin(), u32str.end());
}

std::string EncodingUtils::codepoints_to_string(const std::vector<char32_t>& codepoints) {
    std::u32string u32str(codepoints.begin(), codepoints.end());
    return utf32_to_utf8(u32str);
}

}
