#pragma once

#include "pinyin_types.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <tuple>

namespace pinyingen {

struct PolyphoneRule {
    char32_t character;
    std::string pinyin;
    std::vector<std::tuple<ptrdiff_t, std::vector<char32_t>>> context;
};

class PolyphoneHandler {
public:
    static PolyphoneHandler& instance();
    
    std::string smart_select(const std::vector<char32_t>& text, 
                              size_t index,
                              const std::vector<std::string>& candidates) const;
    
    std::vector<std::string> get_all_pinyins(char32_t codepoint) const;
    bool is_polyphone(char32_t codepoint) const;
    
    std::string get_name_pinyin(char32_t codepoint, bool is_surname = false) const;
    
private:
    PolyphoneHandler();
    ~PolyphoneHandler() = default;
    PolyphoneHandler(const PolyphoneHandler&) = delete;
    PolyphoneHandler& operator=(const PolyphoneHandler&) = delete;
    
    void init_rules();
    void init_name_rules();
    
    bool match_context(const std::vector<char32_t>& text,
                       size_t index,
                       const std::vector<std::tuple<ptrdiff_t, std::vector<char32_t>>>& contexts) const;
    
    std::unordered_map<char32_t, std::vector<PolyphoneRule>> rules_;
    std::unordered_map<char32_t, std::string> name_defaults_;
    std::unordered_map<char32_t, std::string> surname_defaults_;
};

}
