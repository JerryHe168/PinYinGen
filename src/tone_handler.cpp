#include "tone_handler.h"
#include "encoding_utils.h"
#include <algorithm>
#include <stdexcept>

namespace pinyingen {

const std::unordered_map<char32_t, std::pair<char, int>> ToneHandler::tone_mark_map_ = {
    {U'ā', {'a', 1}}, {U'á', {'a', 2}}, {U'ǎ', {'a', 3}}, {U'à', {'a', 4}},
    {U'ō', {'o', 1}}, {U'ó', {'o', 2}}, {U'ǒ', {'o', 3}}, {U'ò', {'o', 4}},
    {U'ē', {'e', 1}}, {U'é', {'e', 2}}, {U'ě', {'e', 3}}, {U'è', {'e', 4}},
    {U'ī', {'i', 1}}, {U'í', {'i', 2}}, {U'ǐ', {'i', 3}}, {U'ì', {'i', 4}},
    {U'ū', {'u', 1}}, {U'ú', {'u', 2}}, {U'ǔ', {'u', 3}}, {U'ù', {'u', 4}},
    {U'ǖ', {'v', 1}}, {U'ǘ', {'v', 2}}, {U'ǚ', {'v', 3}}, {U'ǜ', {'v', 4}},
    {U'ü', {'v', 0}},
};

const std::unordered_map<std::string, std::string> ToneHandler::vowel_replace_map_ = {
    {"a1", "ā"}, {"a2", "á"}, {"a3", "ǎ"}, {"a4", "à"},
    {"o1", "ō"}, {"o2", "ó"}, {"o3", "ǒ"}, {"o4", "ò"},
    {"e1", "ē"}, {"e2", "é"}, {"e3", "ě"}, {"e4", "è"},
    {"i1", "ī"}, {"i2", "í"}, {"i3", "ǐ"}, {"i4", "ì"},
    {"u1", "ū"}, {"u2", "ú"}, {"u3", "ǔ"}, {"u4", "ù"},
    {"v1", "ǖ"}, {"v2", "ǘ"}, {"v3", "ǚ"}, {"v4", "ǜ"},
    {"ü1", "ǖ"}, {"ü2", "ǘ"}, {"ü3", "ǚ"}, {"ü4", "ǜ"},
};

int ToneHandler::get_tone_number(char32_t c) {
    auto it = tone_mark_map_.find(c);
    if (it != tone_mark_map_.end()) {
        return it->second.second;
    }
    return 0;
}

bool ToneHandler::is_vowel(char c) {
    return c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' || c == 'v' || c == 'ü';
}

std::string ToneHandler::remove_tone(std::string_view pinyin) {
    std::u32string u32 = EncodingUtils::utf8_to_utf32(pinyin);
    std::u32string result;
    result.reserve(u32.size());
    
    int tone = 0;
    for (char32_t c : u32) {
        auto it = tone_mark_map_.find(c);
        if (it != tone_mark_map_.end()) {
            result.push_back(static_cast<char32_t>(it->second.first));
            if (it->second.second > 0) {
                tone = it->second.second;
            }
        } else {
            result.push_back(c);
        }
    }
    
    if (result.size() >= 2) {
        char32_t last = result.back();
        if (last >= U'1' && last <= U'4') {
            result.pop_back();
        }
    }
    
    return EncodingUtils::utf32_to_utf8(result);
}

std::string ToneHandler::tone_to_number(std::string_view pinyin) {
    std::u32string u32 = EncodingUtils::utf8_to_utf32(pinyin);
    std::u32string result;
    result.reserve(u32.size() + 1);
    
    int tone = 0;
    for (char32_t c : u32) {
        auto it = tone_mark_map_.find(c);
        if (it != tone_mark_map_.end()) {
            result.push_back(static_cast<char32_t>(it->second.first));
            if (it->second.second > 0) {
                tone = it->second.second;
            }
        } else {
            result.push_back(c);
        }
    }
    
    if (result.size() >= 2) {
        char32_t last = result.back();
        if (last >= U'1' && last <= U'4') {
            tone = last - U'0';
            result.pop_back();
        }
    }
    
    if (tone > 0) {
        result.push_back(static_cast<char32_t>('0' + tone));
    }
    
    return EncodingUtils::utf32_to_utf8(result);
}

char32_t ToneHandler::get_tone_mark(char vowel, int tone) {
    std::string key;
    key += vowel;
    key += static_cast<char>('0' + tone);
    
    auto it = vowel_replace_map_.find(key);
    if (it != vowel_replace_map_.end()) {
        std::u32string u32 = EncodingUtils::utf8_to_utf32(it->second);
        if (!u32.empty()) {
            return u32[0];
        }
    }
    return static_cast<char32_t>(vowel);
}

std::string ToneHandler::tone_to_mark(std::string_view pinyin) {
    std::string base = remove_tone(pinyin);
    
    int tone = 0;
    if (!base.empty()) {
        char last = base.back();
        if (last >= '1' && last <= '4') {
            tone = last - '0';
            base.pop_back();
        }
    }
    
    if (tone == 0 || base.empty()) {
        return base;
    }
    
    std::vector<size_t> vowel_positions;
    for (size_t i = 0; i < base.size(); ++i) {
        if (is_vowel(base[i])) {
            vowel_positions.push_back(i);
        }
    }
    
    if (vowel_positions.empty()) {
        return base;
    }
    
    size_t mark_pos = vowel_positions.back();
    if (vowel_positions.size() >= 2) {
        char v1 = base[vowel_positions[0]];
        char v2 = base[vowel_positions[1]];
        
        if ((v1 == 'a' || v1 == 'o' || v1 == 'e') && 
            (v2 == 'i' || v2 == 'u' || v2 == 'v')) {
            mark_pos = vowel_positions[0];
        } else if ((v1 == 'i' || v1 == 'u') && 
                   (v2 == 'a' || v2 == 'o' || v2 == 'e')) {
            mark_pos = vowel_positions[1];
        } else if (base == "iu" || base == "ui") {
            mark_pos = vowel_positions[1];
        } else if (base == "ie" || base == "ue") {
            mark_pos = vowel_positions[1];
        }
    }
    
    char vowel = base[mark_pos];
    char32_t marked = get_tone_mark(vowel, tone);
    
    std::u32string result = EncodingUtils::utf8_to_utf32(base);
    if (mark_pos < result.size()) {
        result[mark_pos] = marked;
    }
    
    return EncodingUtils::utf32_to_utf8(result);
}

std::string ToneHandler::get_first_letter(std::string_view pinyin) {
    if (pinyin.empty()) {
        return {};
    }
    
    std::u32string u32 = EncodingUtils::utf8_to_utf32(pinyin);
    if (u32.empty()) {
        return {};
    }
    
    char32_t first = u32[0];
    
    auto it = tone_mark_map_.find(first);
    if (it != tone_mark_map_.end()) {
        return std::string(1, static_cast<char>(std::toupper(it->second.first)));
    }
    
    if (first >= U'a' && first <= U'z') {
        return std::string(1, static_cast<char>(std::toupper(static_cast<char>(first))));
    }
    
    if (first >= U'A' && first <= U'Z') {
        return std::string(1, static_cast<char>(first));
    }
    
    return EncodingUtils::utf32_to_utf8(std::u32string(1, first));
}

std::string ToneHandler::convert_tone(std::string_view pinyin, ToneStyle style) {
    switch (style) {
        case ToneStyle::None:
            return remove_tone(pinyin);
        case ToneStyle::ToneMark:
            return tone_to_mark(pinyin);
        case ToneStyle::ToneNumber:
            return tone_to_number(pinyin);
        case ToneStyle::FirstLetter:
            return get_first_letter(pinyin);
        default:
            return std::string(pinyin);
    }
}

std::vector<std::string> ToneHandler::convert_tones(const std::vector<std::string>& pinyins, ToneStyle style) {
    std::vector<std::string> result;
    result.reserve(pinyins.size());
    for (const auto& p : pinyins) {
        result.push_back(convert_tone(p, style));
    }
    return result;
}

}
