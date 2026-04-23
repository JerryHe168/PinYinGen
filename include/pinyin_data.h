#pragma once

#include "pinyin_types.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <mutex>
#include <functional>

namespace pinyingen {

class PinyinData {
public:
    static PinyinData& instance();
    
    static void set_data_directory(const std::string& dir);
    static std::string get_data_directory();
    
    static bool load_pinyin_file(const std::string& filepath);
    static bool load_zdic_file(const std::string& filepath);
    static bool load_all_data_files();
    
    const std::vector<std::string>& get_pinyins(char32_t codepoint) const;
    bool has_pinyin(char32_t codepoint) const;
    
    const SurnameMap& get_surnames() const;
    bool is_surname(char32_t codepoint) const;
    std::string get_surname_pinyin(char32_t codepoint) const;
    
    size_t total_chars() const;
    size_t total_surnames() const;
    
    void clear();
    void reload();
    
    using ParseCallback = std::function<bool(char32_t codepoint, const std::vector<std::string>& pinyins)>;
    static bool parse_pinyin_line(const std::string& line, ParseCallback callback);
    
private:
    PinyinData();
    ~PinyinData() = default;
    PinyinData(const PinyinData&) = delete;
    PinyinData& operator=(const PinyinData&) = delete;
    
    void init_pinyin_map();
    void init_surname_map();
    void init_from_files();
    void init_builtin_data();
    
    bool load_file_internal(const std::string& filepath);
    
    PinyinMap pinyin_map_;
    SurnameMap surname_map_;
    std::vector<std::string> empty_list_;
    bool loaded_from_files_ = false;
    
    static std::string data_dir_;
    static std::once_flag init_flag_;
    static PinyinData* instance_;
};

}
