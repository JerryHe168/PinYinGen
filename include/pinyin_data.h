#pragma once

#include "pinyin_types.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <mutex>

namespace pinyingen {

class PinyinData {
public:
    static PinyinData& instance();
    
    const std::vector<std::string>& get_pinyins(char32_t codepoint) const;
    bool has_pinyin(char32_t codepoint) const;
    
    const SurnameMap& get_surnames() const;
    bool is_surname(char32_t codepoint) const;
    std::string get_surname_pinyin(char32_t codepoint) const;
    
    size_t total_chars() const;
    size_t total_surnames() const;
    
private:
    PinyinData();
    ~PinyinData() = default;
    PinyinData(const PinyinData&) = delete;
    PinyinData& operator=(const PinyinData&) = delete;
    
    void init_pinyin_map();
    void init_surname_map();
    
    PinyinMap pinyin_map_;
    SurnameMap surname_map_;
    std::vector<std::string> empty_list_;
    
    static std::once_flag init_flag_;
    static PinyinData* instance_;
};

}
