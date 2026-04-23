#include "pinyin_converter.h"
#include "pinyin_data.h"
#include "tone_handler.h"
#include "polyphone_handler.h"
#include "encoding_utils.h"
#include <sstream>
#include <cctype>

namespace pinyingen {

PinyinConverter& PinyinConverter::instance() {
    static PinyinConverter inst;
    return inst;
}

std::string PinyinConverter::capitalize_first(std::string_view str) {
    if (str.empty()) {
        return {};
    }
    std::string result(str);
    result[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(result[0])));
    return result;
}

std::string PinyinConverter::process_char(
    char32_t c,
    const std::vector<char32_t>& text,
    size_t index,
    const Options& options,
    std::vector<std::string>* alternatives
) const {
    const auto& data = PinyinData::instance();
    const auto& polyphone = PolyphoneHandler::instance();
    
    if (!data.has_pinyin(c)) {
        if (options.keep_non_chinese) {
            std::u32string u32str(1, c);
            return EncodingUtils::utf32_to_utf8(u32str);
        }
        return {};
    }
    
    const auto& candidates = data.get_pinyins(c);
    
    if (alternatives && candidates.size() > 1) {
        *alternatives = ToneHandler::convert_tones(candidates, options.tone_style);
    }
    
    std::string selected;
    
    switch (options.polyphone_strategy) {
        case PolyphoneStrategy::First:
            selected = candidates[0];
            break;
        case PolyphoneStrategy::Smart:
            selected = polyphone.smart_select(text, index, candidates);
            break;
        case PolyphoneStrategy::All:
        default:
            selected = candidates[0];
            break;
    }
    
    std::string result = ToneHandler::convert_tone(selected, options.tone_style);
    
    if (options.capitalize && !result.empty()) {
        result = capitalize_first(result);
    }
    
    return result;
}

PinyinResult PinyinConverter::convert(std::string_view text, const Options& options) const {
    PinyinResult result;
    result.original = std::string(text);
    
    if (text.empty()) {
        return result;
    }
    
    std::vector<char32_t> codepoints;
    try {
        codepoints = EncodingUtils::string_to_codepoints(text);
    } catch (...) {
        return result;
    }
    
    result.pinyins.reserve(codepoints.size());
    result.alternatives.reserve(codepoints.size());
    
    for (size_t i = 0; i < codepoints.size(); ++i) {
        char32_t c = codepoints[i];
        std::vector<std::string> alts;
        
        std::string pinyin = process_char(c, codepoints, i, options, &alts);
        
        result.pinyins.push_back(pinyin);
        result.alternatives.push_back(alts);
        
        if (!alts.empty()) {
            result.has_polyphone = true;
        }
    }
    
    return result;
}

std::string PinyinConverter::to_string(const PinyinResult& result, const Options& options) const {
    if (result.pinyins.empty()) {
        return {};
    }
    
    std::ostringstream oss;
    for (size_t i = 0; i < result.pinyins.size(); ++i) {
        if (i > 0 && !result.pinyins[i-1].empty() && !result.pinyins[i].empty()) {
            char32_t prev_last = 0;
            char32_t curr_first = 0;
            
            if (!result.pinyins[i-1].empty()) {
                auto u32_prev = EncodingUtils::utf8_to_utf32(result.pinyins[i-1]);
                if (!u32_prev.empty()) {
                    prev_last = u32_prev.back();
                }
            }
            
            if (!result.pinyins[i].empty()) {
                auto u32_curr = EncodingUtils::utf8_to_utf32(result.pinyins[i]);
                if (!u32_curr.empty()) {
                    curr_first = u32_curr[0];
                }
            }
            
            bool prev_is_letter = (prev_last >= U'a' && prev_last <= U'z') || 
                                   (prev_last >= U'A' && prev_last <= U'Z') ||
                                   (prev_last >= U'0' && prev_last <= U'9');
            bool curr_is_letter = (curr_first >= U'a' && curr_first <= U'z') || 
                                   (curr_first >= U'A' && curr_first <= U'Z');
            
            if (prev_is_letter && curr_is_letter) {
                oss << options.separator;
            }
        }
        oss << result.pinyins[i];
    }
    
    return oss.str();
}

std::vector<std::string> PinyinConverter::get_pinyins(char32_t codepoint, ToneStyle style) const {
    const auto& pinyins = PinyinData::instance().get_pinyins(codepoint);
    return ToneHandler::convert_tones(pinyins, style);
}

std::string PinyinConverter::get_first_pinyin(char32_t codepoint, ToneStyle style) const {
    const auto& pinyins = PinyinData::instance().get_pinyins(codepoint);
    if (pinyins.empty()) {
        return {};
    }
    return ToneHandler::convert_tone(pinyins[0], style);
}

std::string PinyinConverter::get_initials(std::string_view text, bool keep_non_chinese) const {
    if (text.empty()) {
        return {};
    }
    
    std::vector<char32_t> codepoints;
    try {
        codepoints = EncodingUtils::string_to_codepoints(text);
    } catch (...) {
        return {};
    }
    
    std::ostringstream oss;
    
    for (char32_t c : codepoints) {
        const auto& pinyins = PinyinData::instance().get_pinyins(c);
        
        if (!pinyins.empty()) {
            std::string initial = ToneHandler::convert_tone(pinyins[0], ToneStyle::FirstLetter);
            oss << initial;
        } else if (keep_non_chinese) {
            if (EncodingUtils::is_chinese_char(c)) {
            } else if ((c >= U'A' && c <= U'Z') || (c >= U'a' && c <= U'z')) {
                oss << static_cast<char>(std::toupper(static_cast<unsigned char>(static_cast<char>(c))));
            } else {
                std::u32string u32str(1, c);
                oss << EncodingUtils::utf32_to_utf8(u32str);
            }
        }
    }
    
    return oss.str();
}

}
