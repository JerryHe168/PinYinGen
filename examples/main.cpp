#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include "pinyingen.h"

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

using namespace pinyingen;
using namespace std;

#ifdef _WIN32
class ConsoleOutput {
public:
    static void init() {
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
        is_console_ = is_console();
    }
    
    static void print(const std::string& utf8_str) {
        if (is_console_) {
            try {
                std::wstring wstr = utf8_to_wide(utf8_str);
                HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
                if (hConsole != INVALID_HANDLE_VALUE) {
                    DWORD written = 0;
                    WriteConsoleW(hConsole, wstr.c_str(), 
                                  static_cast<DWORD>(wstr.size()), 
                                  &written, nullptr);
                }
            } catch (...) {
                std::cout << utf8_str;
            }
        } else {
            std::cout << utf8_str;
        }
    }
    
    static void println(const std::string& utf8_str) {
        print(utf8_str);
        print("\n");
    }
    
private:
    static bool is_console_;
    
    static bool is_console() {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConsole == INVALID_HANDLE_VALUE) {
            return false;
        }
        DWORD mode = 0;
        return GetConsoleMode(hConsole, &mode) != FALSE;
    }
    
    static std::wstring utf8_to_wide(const std::string& utf8_str) {
        if (utf8_str.empty()) return {};
        
        int len = MultiByteToWideChar(CP_UTF8, 0, utf8_str.data(), 
                                       static_cast<int>(utf8_str.size()), 
                                       nullptr, 0);
        if (len <= 0) return {};
        
        std::wstring wstr(len, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, utf8_str.data(), 
                             static_cast<int>(utf8_str.size()), 
                             &wstr[0], len);
        return wstr;
    }
};

bool ConsoleOutput::is_console_ = false;

#define COUT(x) ConsoleOutput::print(x)
#define COUTLN(x) ConsoleOutput::println(x)

#else

class ConsoleOutput {
public:
    static void init() {}
    static void print(const std::string& s) { std::cout << s; }
    static void println(const std::string& s) { std::cout << s << std::endl; }
};

#define COUT(x) std::cout << (x)
#define COUTLN(x) std::cout << (x) << std::endl

#endif

template<typename T>
std::string to_string_fixed(const T& value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

void print_separator() {
    COUTLN("--------------------------------------------------");
}

void demo_basic_conversion() {
    COUTLN("\n【基础拼音转换演示】");
    print_separator();
    
    vector<string> test_names = {
        "张三",
        "李四",
        "王五",
        "赵六",
        "欧阳峰",
        "单田芳",
        "解小东",
        "华春莹"
    };
    
    for (const auto& name : test_names) {
        COUT("\n姓名: ");
        COUTLN(name);
        
        string pinyin_mark = to_pinyin(name, ToneStyle::ToneMark);
        string pinyin_num = to_pinyin(name, ToneStyle::ToneNumber);
        string pinyin_none = to_pinyin(name, ToneStyle::None);
        string initials = get_initials(name);
        
        COUT("  带声调: ");
        COUTLN(pinyin_mark);
        COUT("  数字调: ");
        COUTLN(pinyin_num);
        COUT("  无声调: ");
        COUTLN(pinyin_none);
        COUT("  首字母: ");
        COUTLN(initials);
    }
}

void demo_name_conversion() {
    COUTLN("\n【姓名拼音详情演示】");
    print_separator();
    
    vector<string> names = {
        "诸葛亮",
        "司马懿",
        "长孙无忌",
        "慕容复",
        "任我行",
        "燕南天"
    };
    
    for (const auto& name : names) {
        COUT("\n姓名: ");
        COUTLN(name);
        NamePinyin np = name_to_pinyin(name, ToneStyle::ToneMark);
        
        COUT("  姓氏拼音: ");
        COUTLN(np.surname);
        COUT("  名字拼音: ");
        for (size_t i = 0; i < np.given_name.size(); ++i) {
            if (i > 0) COUT(" ");
            COUT(np.given_name[i]);
        }
        COUTLN("");
        COUT("  完整拼音: ");
        COUTLN(np.full_pinyin);
        COUT("  拼音首写: ");
        COUTLN(np.initials);
    }
}

void demo_polyphone_handling() {
    COUTLN("\n【多音字处理演示】");
    print_separator();
    
    vector<string> test_cases = {
        "快乐",
        "音乐",
        "行走",
        "银行",
        "好人",
        "爱好",
        "都是",
        "首都",
        "长发",
        "长大"
    };
    
    for (const auto& text : test_cases) {
        COUT("\n词语: ");
        COUTLN(text);
        
        PinyinConverter::Options opts;
        opts.tone_style = ToneStyle::ToneMark;
        opts.polyphone_strategy = PolyphoneStrategy::Smart;
        
        PinyinResult result = PinyinConverter::instance().convert(text, opts);
        
        COUT("  智能识别: ");
        COUTLN(PinyinConverter::instance().to_string(result, opts));
        
        if (result.has_polyphone) {
            COUTLN("  多音字候选: ");
            for (size_t i = 0; i < result.alternatives.size(); ++i) {
                if (!result.alternatives[i].empty()) {
                    COUT("    第");
                    COUT(to_string_fixed(i + 1));
                    COUT("字: ");
                    for (size_t j = 0; j < result.alternatives[i].size(); ++j) {
                        if (j > 0) COUT(" / ");
                        COUT(result.alternatives[i][j]);
                    }
                    COUTLN("");
                }
            }
        }
    }
}

void demo_name_generation() {
    COUTLN("\n【随机姓名生成演示】");
    print_separator();
    
    NameGenerator& gen = NameGenerator::instance();
    
    COUTLN("\n生成10个随机姓名（带拼音）:");
    
    std::ostringstream header;
    header << setw(8) << "序号" << setw(12) << "姓名" 
           << setw(25) << "拼音" << setw(10) << "首写";
    COUTLN(header.str());
    COUTLN(string(55, '-'));
    
    for (size_t i = 0; i < 10; ++i) {
        auto name = gen.generate();
        
        std::ostringstream line;
        line << setw(6) << (i + 1) << "." 
             << setw(10) << name.name
             << setw(25) << name.pinyin.full_pinyin
             << setw(8) << name.pinyin.initials;
        COUTLN(line.str());
    }
    
    NameGenerator::GenerateOptions opts;
    opts.given_name_length = 1;
    
    COUTLN("\n生成5个单字名:");
    for (size_t i = 0; i < 5; ++i) {
        auto name = gen.generate(opts);
        COUT("  ");
        COUT(name.name);
        COUT(" (");
        COUT(name.pinyin.full_pinyin);
        COUTLN(")");
    }
}

void demo_tone_styles() {
    COUTLN("\n【声调风格演示】");
    print_separator();
    
    string text = "中文拼音转换测试";
    COUT("原文: ");
    COUTLN(text);
    COUTLN("");
    
    PinyinConverter::Options opts;
    opts.keep_non_chinese = true;
    
    opts.tone_style = ToneStyle::ToneMark;
    COUT("声调符号: ");
    COUTLN(to_pinyin(text, ToneStyle::ToneMark));
    
    opts.tone_style = ToneStyle::ToneNumber;
    COUT("数字标注: ");
    COUTLN(to_pinyin(text, ToneStyle::ToneNumber));
    
    opts.tone_style = ToneStyle::None;
    COUT("无声调:   ");
    COUTLN(to_pinyin(text, ToneStyle::None));
    
    opts.tone_style = ToneStyle::FirstLetter;
    COUT("首字母:   ");
    COUTLN(to_pinyin(text, ToneStyle::FirstLetter));
}

void demo_encoding_utils() {
    COUTLN("\n【编码工具演示】");
    print_separator();
    
    string utf8_str = "你好，世界！";
    
    COUT("UTF-8 字符串: ");
    COUTLN(utf8_str);
    
    try {
        u32string u32 = EncodingUtils::utf8_to_utf32(utf8_str);
        COUT("转换为 UTF-32: ");
        COUT(to_string_fixed(u32.size()));
        COUTLN(" 个码点");
        
        vector<char32_t> codepoints = EncodingUtils::string_to_codepoints(utf8_str);
        COUT("码点列表: ");
        for (char32_t c : codepoints) {
            std::ostringstream oss;
            oss << "U+" << hex << uppercase << setw(4) << setfill('0') 
                << static_cast<uint32_t>(c) << " ";
            COUT(oss.str());
        }
        COUTLN("");
        
        COUTLN("中文字符检查:");
        for (size_t i = 0; i < codepoints.size(); ++i) {
            bool is_cn = EncodingUtils::is_chinese_char(codepoints[i]);
            u32string u32c(1, codepoints[i]);
            string c = EncodingUtils::utf32_to_utf8(u32c);
            COUT("  '");
            COUT(c);
            COUT("' : ");
            COUTLN(is_cn ? "是中文" : "非中文");
        }
    }
    catch (const exception& e) {
        COUT("编码转换错误: ");
        COUTLN(e.what());
    }
}

void demo_safety_check() {
    COUTLN("\n【安全性验证演示】");
    print_separator();
    
    vector<string> test_inputs = {
        "",
        "a",
        "abc",
        "Hello 世界",
        "12345",
        "测试：标点符号！",
    };
    
    COUTLN("边界输入测试:");
    for (const auto& input : test_inputs) {
        try {
            string result = to_pinyin(input);
            COUT("  输入: \"");
            COUT(input);
            COUTLN("\"");
            COUT("    输出: \"");
            COUT(result);
            COUTLN("\"");
        }
        catch (const exception& e) {
            COUT("  输入: \"");
            COUT(input);
            COUTLN("\"");
            COUT("    异常: ");
            COUTLN(e.what());
        }
    }
    
    COUTLN("\n空字符串验证:");
    string empty_pinyin = to_pinyin("");
    COUT("  to_pinyin(\"\") = \"");
    COUT(empty_pinyin);
    COUTLN("\"");
    
    string empty_initials = get_initials("");
    COUT("  get_initials(\"\") = \"");
    COUT(empty_initials);
    COUTLN("\"");
}

int main() {
    ConsoleOutput::init();
    
    COUTLN("==============================================");
    COUTLN("    PinYinGen - 中文姓名转拼音工具库");
    COUTLN("    版本: 1.0.0");
    COUTLN("==============================================");
    
    try {
        demo_basic_conversion();
        demo_name_conversion();
        demo_polyphone_handling();
        demo_name_generation();
        demo_tone_styles();
        demo_encoding_utils();
        demo_safety_check();
        
        COUTLN("\n\n==============================================");
        COUTLN("    演示完毕，感谢使用 PinYinGen！");
        COUTLN("==============================================");
    }
    catch (const exception& e) {
        COUT("\n发生未预期的错误: ");
        COUTLN(e.what());
        return 1;
    }
    
    return 0;
}
