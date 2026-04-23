#pragma once

#include "pinyin_types.h"
#include <string>
#include <vector>

namespace pinyingen {

class PinyinConverter {
public:
    struct Options {
        ToneStyle tone_style = ToneStyle::ToneMark;
        PolyphoneStrategy polyphone_strategy = PolyphoneStrategy::Smart;
        bool keep_non_chinese = true;
        char separator = ' ';
        bool capitalize = false;
    };
    
    static PinyinConverter& instance();
    
    PinyinResult convert(std::string_view text, const Options& options = Options{}) const;
    
    std::string to_string(const PinyinResult& result, const Options& options = Options{}) const;
    
    std::vector<std::string> get_pinyins(char32_t codepoint, ToneStyle style = ToneStyle::ToneMark) const;
    
    std::string get_first_pinyin(char32_t codepoint, ToneStyle style = ToneStyle::ToneMark) const;
    
    std::string get_initials(std::string_view text, bool keep_non_chinese = false) const;
    
private:
    PinyinConverter() = default;
    ~PinyinConverter() = default;
    PinyinConverter(const PinyinConverter&) = delete;
    PinyinConverter& operator=(const PinyinConverter&) = delete;
    
    std::string process_char(char32_t c, 
                              const std::vector<char32_t>& text,
                              size_t index,
                              const Options& options,
                              std::vector<std::string>* alternatives = nullptr) const;
    
    static std::string capitalize_first(std::string_view str);
};

}
