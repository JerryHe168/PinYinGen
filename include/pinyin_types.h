#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_map>

namespace pinyingen {

enum class ToneStyle {
    None = 0,
    ToneMark = 1,
    ToneNumber = 2,
    FirstLetter = 3
};

enum class PolyphoneStrategy {
    First = 0,
    All = 1,
    Smart = 2
};

struct PinyinResult {
    std::string original;
    std::vector<std::string> pinyins;
    std::vector<std::vector<std::string>> alternatives;
    bool has_polyphone = false;
};

struct NamePinyin {
    std::string surname;
    std::vector<std::string> given_name;
    std::string full_pinyin;
    std::string initials;
};

using PinyinMap = std::unordered_map<char32_t, std::vector<std::string>>;
using SurnameMap = std::unordered_map<char32_t, std::string>;

}
