#pragma once

#include "pinyin_types.h"
#include "encoding_utils.h"
#include "pinyin_data.h"
#include "tone_handler.h"
#include "polyphone_handler.h"
#include "pinyin_converter.h"
#include "name_generator.h"

namespace pinyingen {

inline std::string to_pinyin(std::string_view text,
                               ToneStyle tone = ToneStyle::ToneMark,
                               PolyphoneStrategy strategy = PolyphoneStrategy::Smart,
                               char separator = ' ') {
    PinyinConverter::Options opts;
    opts.tone_style = tone;
    opts.polyphone_strategy = strategy;
    opts.separator = separator;
    
    auto result = PinyinConverter::instance().convert(text, opts);
    return PinyinConverter::instance().to_string(result, opts);
}

inline std::string to_pinyin_without_tone(std::string_view text,
                                            char separator = ' ') {
    return to_pinyin(text, ToneStyle::None, PolyphoneStrategy::Smart, separator);
}

inline std::string to_pinyin_with_number(std::string_view text,
                                          char separator = ' ') {
    return to_pinyin(text, ToneStyle::ToneNumber, PolyphoneStrategy::Smart, separator);
}

inline std::string get_initials(std::string_view text, bool keep_non_chinese = false) {
    return PinyinConverter::instance().get_initials(text, keep_non_chinese);
}

inline NamePinyin name_to_pinyin(std::string_view name,
                                   ToneStyle tone = ToneStyle::ToneMark) {
    PinyinConverter::Options opts;
    opts.tone_style = tone;
    return NameGenerator::instance().convert_name(name, opts);
}

inline std::string generate_name() {
    return NameGenerator::instance().generate().name;
}

inline NameGenerator::GeneratedName generate_name_with_pinyin() {
    return NameGenerator::instance().generate();
}

}
