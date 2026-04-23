#pragma once

#include "pinyin_types.h"
#include "pinyin_converter.h"
#include <string>
#include <vector>
#include <random>

namespace pinyingen {

class NameGenerator {
public:
    struct GenerateOptions {
        size_t surname_count = 1;
        size_t given_name_length = 2;
        PinyinConverter::Options pinyin_options = PinyinConverter::Options{};
        bool include_pinyin = true;
    };
    
    struct GeneratedName {
        std::string name;
        std::string surname;
        std::string given_name;
        NamePinyin pinyin;
    };
    
    static NameGenerator& instance();
    
    GeneratedName generate(const GenerateOptions& options = GenerateOptions{}) const;
    
    std::vector<GeneratedName> generate_batch(size_t count, 
                                                const GenerateOptions& options = GenerateOptions{}) const;
    
    NamePinyin convert_name(std::string_view name,
                          const PinyinConverter::Options& options = PinyinConverter::Options{}) const;
    
    std::string get_random_surname(bool allow_multiple = false) const;
    
    std::string get_random_given_name(size_t length = 2) const;
    
    const std::vector<std::string>& get_common_surnames() const;
    const std::vector<std::string>& get_given_name_chars() const;
    
    void set_seed(unsigned int seed);
    
private:
    NameGenerator();
    ~NameGenerator() = default;
    NameGenerator(const NameGenerator&) = delete;
    NameGenerator& operator=(const NameGenerator&) = delete;
    
    void init_common_surnames();
    void init_given_name_chars();
    
    std::vector<std::string> common_surnames_;
    std::vector<std::string> given_name_chars_;
    
    mutable std::mt19937 rng_;
    mutable std::uniform_int_distribution<size_t> surname_dist_;
    mutable std::uniform_int_distribution<size_t> given_name_dist_;
};

}
