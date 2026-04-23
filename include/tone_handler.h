#pragma once

#include "pinyin_types.h"
#include <string>
#include <string_view>
#include <unordered_map>

namespace pinyingen {

class ToneHandler {
public:
    static std::string remove_tone(std::string_view pinyin);
    static std::string tone_to_number(std::string_view pinyin);
    static std::string tone_to_mark(std::string_view pinyin);
    static std::string get_first_letter(std::string_view pinyin);
    
    static std::string convert_tone(std::string_view pinyin, ToneStyle style);
    static std::vector<std::string> convert_tones(const std::vector<std::string>& pinyins, ToneStyle style);
    
private:
    ToneHandler() = delete;
    ~ToneHandler() = delete;
    
    static const std::unordered_map<char32_t, std::pair<char, int>> tone_mark_map_;
    static const std::unordered_map<std::string, std::string> vowel_replace_map_;
    
    static int get_tone_number(char32_t c);
    static char32_t get_tone_mark(char vowel, int tone);
    static bool is_vowel(char c);
};

}
