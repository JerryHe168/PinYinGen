#include "pinyin_data.h"
#include "tone_handler.h"
#include <mutex>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <cctype>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#endif

namespace pinyingen {

std::once_flag PinyinData::init_flag_;
PinyinData* PinyinData::instance_ = nullptr;
std::string PinyinData::data_dir_;

namespace {

bool starts_with(const std::string& str, const std::string& prefix) {
    if (str.length() < prefix.length()) return false;
    return str.substr(0, prefix.length()) == prefix;
}

std::string trim(const std::string& str) {
    size_t start = 0;
    while (start < str.size() && std::isspace(static_cast<unsigned char>(str[start]))) {
        ++start;
    }
    size_t end = str.size();
    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1]))) {
        --end;
    }
    return str.substr(start, end - start);
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    
    while (std::getline(tokenStream, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

char32_t parse_codepoint(const std::string& str) {
    if (str.length() < 3 || !starts_with(str, "U+")) {
        return 0;
    }
    
    std::string hex_part = str.substr(2);
    std::istringstream iss(hex_part);
    uint32_t codepoint;
    
    iss >> std::hex >> codepoint;
    if (iss.fail()) {
        return 0;
    }
    
    return static_cast<char32_t>(codepoint);
}

#ifdef _WIN32
std::string get_executable_directory() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    std::string::size_type pos = std::string(buffer).find_last_of("\\/");
    return std::string(buffer).substr(0, pos);
}
#endif

std::string find_data_directory() {
    std::vector<std::string> candidates;
    
#ifdef _WIN32
    std::string exe_dir = get_executable_directory();
    candidates.push_back(exe_dir + "\\data");
    candidates.push_back(exe_dir + "\\..\\data");
    candidates.push_back(exe_dir + "\\..\\..\\data");
#endif
    
    candidates.push_back("./data");
    candidates.push_back("../data");
    candidates.push_back("../../data");
    
    for (const auto& dir : candidates) {
        std::string test_file = dir + "/pinyin.txt";
        std::ifstream f(test_file);
        if (f.is_open()) {
            return dir;
        }
    }
    
    return "";
}

}

PinyinData& PinyinData::instance() {
    std::call_once(init_flag_, []() {
        instance_ = new PinyinData();
    });
    return *instance_;
}

void PinyinData::set_data_directory(const std::string& dir) {
    data_dir_ = dir;
}

std::string PinyinData::get_data_directory() {
    if (data_dir_.empty()) {
        data_dir_ = find_data_directory();
    }
    return data_dir_;
}

bool PinyinData::load_pinyin_file(const std::string& filepath) {
    return instance().load_file_internal(filepath);
}

bool PinyinData::load_zdic_file(const std::string& filepath) {
    return instance().load_file_internal(filepath);
}

bool PinyinData::load_all_data_files() {
    std::string data_dir = get_data_directory();
    if (data_dir.empty()) {
        return false;
    }
    
    bool loaded = false;
    
    std::string pinyin_file = data_dir + "/pinyin.txt";
    if (load_pinyin_file(pinyin_file)) {
        loaded = true;
    }
    
    std::string zdic_file = data_dir + "/zdic.txt";
    if (load_zdic_file(zdic_file)) {
        loaded = true;
    }
    
    return loaded;
}

bool PinyinData::parse_pinyin_line(const std::string& line, ParseCallback callback) {
    if (line.empty() || line[0] == '#') {
        return true;
    }
    
    size_t colon_pos = line.find(':');
    if (colon_pos == std::string::npos) {
        return true;
    }
    
    std::string codepoint_str = trim(line.substr(0, colon_pos));
    char32_t codepoint = parse_codepoint(codepoint_str);
    
    if (codepoint == 0) {
        return true;
    }
    
    size_t comment_pos = line.find('#', colon_pos);
    std::string pinyin_part;
    
    if (comment_pos != std::string::npos) {
        pinyin_part = trim(line.substr(colon_pos + 1, comment_pos - colon_pos - 1));
    } else {
        pinyin_part = trim(line.substr(colon_pos + 1));
    }
    
    if (pinyin_part.empty() || pinyin_part == "\"\"") {
        return true;
    }
    
    if (pinyin_part.length() >= 2 && 
        pinyin_part.front() == '"' && 
        pinyin_part.back() == '"') {
        pinyin_part = pinyin_part.substr(1, pinyin_part.length() - 2);
        if (pinyin_part.empty()) {
            return true;
        }
    }
    
    std::vector<std::string> pinyins = split(pinyin_part, ',');
    
    if (pinyins.empty()) {
        return true;
    }
    
    std::vector<std::string> normalized_pinyins;
    for (const auto& p : pinyins) {
        std::string normalized = ToneHandler::tone_to_number(p);
        if (!normalized.empty()) {
            normalized_pinyins.push_back(normalized);
        }
    }
    
    if (normalized_pinyins.empty()) {
        return true;
    }
    
    return callback(codepoint, normalized_pinyins);
}

PinyinData::PinyinData() {
    init_pinyin_map();
    init_surname_map();
}

void PinyinData::init_pinyin_map() {
    std::string data_dir = get_data_directory();
    if (!data_dir.empty()) {
        bool loaded = false;
        
        std::string pinyin_file = data_dir + "/pinyin.txt";
        if (load_file_internal(pinyin_file)) {
            loaded = true;
        }
        
        std::string zdic_file = data_dir + "/zdic.txt";
        if (load_file_internal(zdic_file)) {
            loaded = true;
        }
        
        if (loaded) {
            loaded_from_files_ = true;
            return;
        }
    }
    
    init_builtin_data();
}

void PinyinData::init_from_files() {
    std::string data_dir = get_data_directory();
    if (data_dir.empty()) {
        return;
    }
    
    load_file_internal(data_dir + "/pinyin.txt");
    load_file_internal(data_dir + "/zdic.txt");
}

bool PinyinData::load_file_internal(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    size_t loaded_count = 0;
    
    while (std::getline(file, line)) {
        parse_pinyin_line(line, [this, &loaded_count](char32_t codepoint, const std::vector<std::string>& pinyins) {
            if (codepoint > 0 && !pinyins.empty()) {
                if (pinyin_map_.find(codepoint) == pinyin_map_.end()) {
                    pinyin_map_[codepoint] = pinyins;
                    ++loaded_count;
                }
            }
            return true;
        });
    }
    
    file.close();
    return loaded_count > 0;
}

void PinyinData::init_builtin_data() {
    pinyin_map_ = {
        {U'一', {"yī"}},
        {U'丁', {"dīng", "zhēng"}},
        {U'七', {"qī"}},
        {U'万', {"wàn", "mò"}},
        {U'三', {"sān"}},
        {U'上', {"shàng", "shǎng"}},
        {U'下', {"xià"}},
        {U'不', {"bù", "fǒu"}},
        {U'与', {"yǔ", "yù", "yú"}},
        {U'专', {"zhuān"}},
        {U'世', {"shì"}},
        {U'东', {"dōng"}},
        {U'中', {"zhōng", "zhòng"}},
        {U'丰', {"fēng"}},
        {U'临', {"lín"}},
        {U'丽', {"lì", "lí"}},
        {U'举', {"jǔ"}},
        {U'义', {"yì"}},
        {U'之', {"zhī"}},
        {U'乐', {"lè", "yuè", "yào", "lào"}},
        {U'习', {"xí"}},
        {U'书', {"shū"}},
        {U'事', {"shì"}},
        {U'二', {"èr"}},
        {U'云', {"yún"}},
        {U'互', {"hù"}},
        {U'五', {"wǔ"}},
        {U'井', {"jǐng"}},
        {U'交', {"jiāo"}},
        {U'京', {"jīng"}},
        {U'人', {"rén"}},
        {U'仁', {"rén"}},
        {U'今', {"jīn"}},
        {U'介', {"jiè"}},
        {U'从', {"cóng", "zòng"}},
        {U'他', {"tā"}},
        {U'们', {"men"}},
        {U'代', {"dài"}},
        {U'以', {"yǐ"}},
        {U'任', {"rén", "rèn"}},
        {U'会', {"huì", "kuài"}},
        {U'伟', {"wěi"}},
        {U'传', {"chuán", "zhuàn"}},
        {U'何', {"hé", "hè", "hē"}},
        {U'作', {"zuò", "zuō"}},
        {U'佳', {"jiā"}},
        {U'使', {"shǐ"}},
        {U'例', {"lì"}},
        {U'供', {"gōng", "gòng"}},
        {U'信', {"xìn", "shēn"}},
        {U'候', {"hòu"}},
        {U'值', {"zhí"}},
        {U'假', {"jiǎ", "jià"}},
        {U'健', {"jiàn"}},
        {U'元', {"yuán"}},
        {U'先', {"xiān"}},
        {U'光', {"guāng"}},
        {U'克', {"kè"}},
        {U'入', {"rù"}},
        {U'全', {"quán"}},
        {U'六', {"liù", "lù"}},
        {U'共', {"gòng", "gōng"}},
        {U'关', {"guān"}},
        {U'兴', {"xīng", "xìng"}},
        {U'其', {"qí", "jī"}},
        {U'典', {"diǎn"}},
        {U'兼', {"jiān"}},
        {U'内', {"nèi", "nà"}},
        {U'军', {"jūn"}},
        {U'农', {"nóng"}},
        {U'冠', {"guān", "guàn"}},
        {U'冬', {"dōng"}},
        {U'冷', {"lěng"}},
        {U'凌', {"líng"}},
        {U'凤', {"fèng"}},
        {U'刚', {"gāng"}},
        {U'刘', {"liú"}},
        {U'利', {"lì"}},
        {U'别', {"bié", "biè"}},
        {U'到', {"dào"}},
        {U'制', {"zhì"}},
        {U'前', {"qián"}},
        {U'力', {"lì"}},
        {U'功', {"gōng"}},
        {U'加', {"jiā"}},
        {U'动', {"dòng"}},
        {U'助', {"zhù"}},
        {U'勇', {"yǒng"}},
        {U'勉', {"miǎn"}},
        {U'勤', {"qín"}},
        {U'华', {"huá", "huà", "huā"}},
        {U'协', {"xié"}},
        {U'单', {"dān", "shàn", "chán"}},
        {U'博', {"bó"}},
        {U'卫', {"wèi"}},
        {U'即', {"jí"}},
        {U'却', {"què"}},
        {U'历', {"lì"}},
        {U'参', {"cān", "shēn", "cēn", "sān"}},
        {U'又', {"yòu"}},
        {U'发', {"fā", "fà"}},
        {U'取', {"qǔ"}},
        {U'口', {"kǒu"}},
        {U'古', {"gǔ"}},
        {U'可', {"kě", "kè"}},
        {U'右', {"yòu"}},
        {U'各', {"gè", "gě"}},
        {U'合', {"hé", "gě"}},
        {U'吉', {"jí"}},
        {U'同', {"tóng", "tòng"}},
        {U'名', {"míng"}},
        {U'后', {"hòu"}},
        {U'向', {"xiàng"}},
        {U'君', {"jūn"}},
        {U'吴', {"wú"}},
        {U'周', {"zhōu"}},
        {U'和', {"hé", "hè", "huó", "huò", "hú"}},
        {U'咏', {"yǒng"}},
        {U'品', {"pǐn"}},
        {U'哲', {"zhé"}},
        {U'唐', {"táng"}},
        {U'商', {"shāng"}},
        {U'善', {"shàn"}},
        {U'喜', {"xǐ"}},
        {U'器', {"qì"}},
        {U'四', {"sì"}},
        {U'国', {"guó"}},
        {U'图', {"tú"}},
        {U'地', {"dì", "de"}},
        {U'坚', {"jiān"}},
        {U'在', {"zài"}},
        {U'场', {"chǎng", "cháng"}},
        {U'均', {"jūn"}},
        {U'声', {"shēng"}},
        {U'夏', {"xià"}},
        {U'外', {"wài"}},
        {U'多', {"duō"}},
        {U'夜', {"yè"}},
        {U'天', {"tiān"}},
        {U'奇', {"qí", "jī"}},
        {U'好', {"hǎo", "hào"}},
        {U'如', {"rú"}},
        {U'始', {"shǐ"}},
        {U'姓', {"xìng"}},
        {U'委', {"wěi", "wēi"}},
        {U'孟', {"mèng"}},
        {U'孤', {"gū"}},
        {U'学', {"xué"}},
        {U'孙', {"sūn", "xùn"}},
        {U'宇', {"yǔ"}},
        {U'守', {"shǒu"}},
        {U'安', {"ān"}},
        {U'宋', {"sòng"}},
        {U'完', {"wán"}},
        {U'宏', {"hóng"}},
        {U'官', {"guān"}},
        {U'定', {"dìng"}},
        {U'宜', {"yí"}},
        {U'家', {"jiā", "jia", "jie"}},
        {U'容', {"róng"}},
        {U'宾', {"bīn"}},
        {U'富', {"fù"}},
        {U'寒', {"hán"}},
        {U'察', {"chá"}},
        {U'寸', {"cùn"}},
        {U'导', {"dǎo"}},
        {U'小', {"xiǎo"}},
        {U'少', {"shǎo", "shào"}},
        {U'尔', {"ěr"}},
        {U'尝', {"cháng"}},
        {U'尽', {"jìn", "jǐn"}},
        {U'居', {"jū"}},
        {U'展', {"zhǎn"}},
        {U'山', {"shān"}},
        {U'岐', {"qí"}},
        {U'峰', {"fēng"}},
        {U'崇', {"chóng"}},
        {U'崔', {"cuī"}},
        {U'巖', {"yán"}},
        {U'巩', {"gǒng"}},
        {U'工', {"gōng"}},
        {U'巨', {"jù"}},
        {U'差', {"chā", "chà", "chāi", "cī"}},
        {U'己', {"jǐ"}},
        {U'已', {"yǐ"}},
        {U'巴', {"bā"}},
        {U'巷', {"xiàng", "hàng"}},
        {U'市', {"shì"}},
        {U'布', {"bù"}},
        {U'师', {"shī"}},
        {U'常', {"cháng"}},
        {U'平', {"píng"}},
        {U'年', {"nián"}},
        {U'并', {"bìng", "bīng"}},
        {U'幸', {"xìng"}},
        {U'广', {"guǎng", "ān"}},
        {U'应', {"yīng", "yìng"}},
        {U'序', {"xù"}},
        {U'库', {"kù"}},
        {U'康', {"kāng"}},
        {U'庸', {"yōng"}},
        {U'庚', {"gēng"}},
        {U'廖', {"liào"}},
        {U'张', {"zhāng"}},
        {U'强', {"qiáng", "qiǎng", "jiàng"}},
        {U'彭', {"péng", "bāng"}},
        {U'律', {"lǜ"}},
        {U'徐', {"xú"}},
        {U'德', {"dé"}},
        {U'心', {"xīn"}},
        {U'必', {"bì"}},
        {U'志', {"zhì"}},
        {U'念', {"niàn"}},
        {U'忠', {"zhōng"}},
        {U'性', {"xìng"}},
        {U'恒', {"héng"}},
        {U'恩', {"ēn"}},
        {U'恶', {"è", "wù", "ě", "wū"}},
        {U'悦', {"yuè"}},
        {U'悬', {"xuán"}},
        {U'意', {"yì"}},
        {U'愿', {"yuàn"}},
        {U'慕', {"mù"}},
        {U'慧', {"huì"}},
        {U'戎', {"róng"}},
        {U'成', {"chéng"}},
        {U'我', {"wǒ"}},
        {U'战', {"zhàn"}},
        {U'所', {"suǒ"}},
        {U'才', {"cái"}},
        {U'扬', {"yáng"}},
        {U'承', {"chéng"}},
        {U'抱', {"bào"}},
        {U'招', {"zhāo"}},
        {U'拜', {"bài"}},
        {U'持', {"chí"}},
        {U'振', {"zhèn"}},
        {U'捷', {"jié"}},
        {U'排', {"pái", "pǎi"}},
        {U'授', {"shòu"}},
        {U'接', {"jiē"}},
        {U'推', {"tuī"}},
        {U'提', {"tí", "dī", "dǐ"}},
        {U'操', {"cāo"}},
        {U'政', {"zhèng"}},
        {U'敏', {"mǐn"}},
        {U'救', {"jiù"}},
        {U'教', {"jiào", "jiāo"}},
        {U'敢', {"gǎn"}},
        {U'散', {"sàn", "sǎn"}},
        {U'敦', {"dūn", "duì"}},
        {U'文', {"wén"}},
        {U'斗', {"dòu", "dǒu"}},
        {U'新', {"xīn"}},
        {U'方', {"fāng"}},
        {U'旁', {"páng", "bàng"}},
        {U'施', {"shī"}},
        {U'明', {"míng"}},
        {U'易', {"yì"}},
        {U'星', {"xīng"}},
        {U'春', {"chūn"}},
        {U'晏', {"yàn"}},
        {U'晓', {"xiǎo"}},
        {U'晃', {"huǎng", "huàng"}},
        {U'景', {"jǐng", "yǐng"}},
        {U'暇', {"xiá"}},
        {U'曾', {"zēng", "céng"}},
        {U'朗', {"lǎng"}},
        {U'望', {"wàng"}},
        {U'朝', {"zhāo", "cháo"}},
        {U'期', {"qī", "jī"}},
        {U'木', {"mù"}},
        {U'本', {"běn"}},
        {U'朱', {"zhū"}},
        {U'权', {"quán"}},
        {U'来', {"lái", "lài"}},
        {U'林', {"lín"}},
        {U'果', {"guǒ"}},
        {U'梁', {"liáng"}},
        {U'梅', {"méi"}},
        {U'梦', {"mèng"}},
        {U'楚', {"chǔ"}},
        {U'楼', {"lóu"}},
        {U'杨', {"yáng"}},
        {U'李', {"lǐ"}},
        {U'杜', {"dù"}},
        {U'松', {"sōng"}},
        {U'柳', {"liǔ"}},
        {U'武', {"wǔ"}},
        {U'毅', {"yì"}},
        {U'比', {"bǐ"}},
        {U'毕', {"bì"}},
        {U'毛', {"máo"}},
        {U'氏', {"shì", "zhī"}},
        {U'民', {"mín"}},
        {U'气', {"qì"}},
        {U'水', {"shuǐ"}},
        {U'永', {"yǒng"}},
        {U'江', {"jiāng"}},
        {U'沈', {"shěn", "chén"}},
        {U'沙', {"shā"}},
        {U'河', {"hé"}},
        {U'法', {"fǎ"}},
        {U'波', {"bō"}},
        {U'泰', {"tài"}},
        {U'洋', {"yáng"}},
        {U'洪', {"hóng"}},
        {U'洁', {"jié"}},
        {U'洛', {"luò"}},
        {U'浩', {"hào"}},
        {U'海', {"hǎi"}},
        {U'涛', {"tāo"}},
        {U'清', {"qīng"}},
        {U'源', {"yuán"}},
        {U'溥', {"pǔ"}},
        {U'满', {"mǎn"}},
        {U'汉', {"hàn"}},
        {U'潘', {"pān"}},
        {U'潜', {"qián"}},
        {U'燕', {"yàn", "yān"}},
        {U'玄', {"xuán"}},
        {U'玉', {"yù"}},
        {U'珉', {"mín"}},
        {U'珍', {"zhēn"}},
        {U'珠', {"zhū"}},
        {U'琳', {"lín"}},
        {U'瑜', {"yú"}},
        {U'瑞', {"ruì"}},
        {U'瑾', {"jǐn"}},
        {U'环', {"huán"}},
        {U'用', {"yòng"}},
        {U'申', {"shēn"}},
        {U'由', {"yóu"}},
        {U'畅', {"chàng"}},
        {U'甲', {"jiǎ"}},
        {U'甫', {"fǔ"}},
        {U'男', {"nán"}},
        {U'画', {"huà"}},
        {U'甸', {"diàn", "tián", "shèng"}},
        {U'界', {"jiè"}},
        {U'略', {"lüè"}},
        {U'白', {"bái"}},
        {U'百', {"bǎi", "bó"}},
        {U'盛', {"shèng", "chéng"}},
        {U'目', {"mù"}},
        {U'直', {"zhí"}},
        {U'相', {"xiāng", "xiàng"}},
        {U'省', {"shěng", "xǐng"}},
        {U'真', {"zhēn"}},
        {U'睿', {"ruì"}},
        {U'石', {"shí", "dàn"}},
        {U'碧', {"bì"}},
        {U'磊', {"lěi"}},
        {U'示', {"shì"}},
        {U'祖', {"zǔ"}},
        {U'祝', {"zhù"}},
        {U'神', {"shén"}},
        {U'祭', {"jì", "zhài"}},
        {U'禅', {"chán", "shàn"}},
        {U'秋', {"qiū"}},
        {U'程', {"chéng"}},
        {U'章', {"zhāng"}},
        {U'端', {"duān"}},
        {U'童', {"tóng"}},
        {U'竺', {"zhú"}},
        {U'笔', {"bǐ"}},
        {U'篇', {"piān"}},
        {U'素', {"sù"}},
        {U'紫', {"zǐ"}},
        {U'红', {"hóng", "gōng"}},
        {U'纯', {"chún"}},
        {U'经', {"jīng"}},
        {U'纪', {"jì", "jǐ"}},
        {U'纶', {"lún", "guān"}},
        {U'绍', {"shào"}},
        {U'继', {"jì"}},
        {U'绩', {"jì"}},
        {U'续', {"xù"}},
        {U'绮', {"qǐ"}},
        {U'维', {"wéi"}},
        {U'绵', {"mián"}},
        {U'绶', {"shòu"}},
        {U'练', {"liàn"}},
        {U'组', {"zǔ"}},
        {U'细', {"xì"}},
        {U'终', {"zhōng"}},
        {U'绛', {"jiàng"}},
        {U'给', {"gěi", "jǐ"}},
        {U'缘', {"yuán"}},
        {U'缓', {"huǎn"}},
        {U'编', {"biān"}},
        {U'缦', {"màn"}},
        {U'缯', {"zēng", "zèng"}},
        {U'聂', {"niè"}},
        {U'职', {"zhí"}},
        {U'联', {"lián"}},
        {U'聚', {"jù"}},
        {U'聪', {"cōng"}},
        {U'肃', {"sù"}},
        {U'脉', {"mài", "mò"}},
        {U'能', {"néng", "nài"}},
        {U'臧', {"zāng"}},
        {U'臣', {"chén"}},
        {U'自', {"zì"}},
        {U'致', {"zhì"}},
        {U'舆', {"yú"}},
        {U'舒', {"shū"}},
        {U'舞', {"wǔ"}},
        {U'舟', {"zhōu"}},
        {U'航', {"háng"}},
        {U'良', {"liáng"}},
        {U'色', {"sè", "shǎi"}},
        {U'芝', {"zhī"}},
        {U'芳', {"fāng"}},
        {U'花', {"huā"}},
        {U'芬', {"fēn"}},
        {U'芸', {"yún"}},
        {U'若', {"ruò", "rě"}},
        {U'苗', {"miáo"}},
        {U'英', {"yīng"}},
        {U'茂', {"mào"}},
        {U'范', {"fàn"}},
        {U'荣', {"róng"}},
        {U'莹', {"yíng"}},
        {U'萧', {"xiāo"}},
        {U'葛', {"gé", "gě"}},
        {U'董', {"dǒng"}},
        {U'蒋', {"jiǎng"}},
        {U'蔡', {"cài"}},
        {U'蔺', {"lìn"}},
        {U'薛', {"xuē"}},
        {U'蔚', {"wèi", "yù"}},
        {U'藏', {"cáng", "zàng"}},
        {U'虎', {"hǔ"}},
        {U'虚', {"xū"}},
        {U'虹', {"hóng"}},
        {U'裴', {"péi"}},
        {U'行', {"xíng", "háng", "héng", "hàng"}},
        {U'衡', {"héng"}},
        {U'袁', {"yuán"}},
        {U'补', {"bǔ"}},
        {U'裁', {"cái"}},
        {U'褚', {"chǔ", "zhě", "zhǔ"}},
        {U'见', {"jiàn", "xiàn"}},
        {U'观', {"guān", "guàn"}},
        {U'视', {"shì"}},
        {U'览', {"lǎn"}},
        {U'规', {"guī"}},
        {U'觉', {"jué", "jiào"}},
        {U'解', {"jiě", "jiè", "xiè"}},
        {U'言', {"yán"}},
        {U'誉', {"yù"}},
        {U'詹', {"zhān"}},
        {U'誓', {"shì"}},
        {U'许', {"xǔ", "hǔ"}},
        {U'论', {"lùn", "lún"}},
        {U'识', {"shí", "zhì"}},
        {U'词', {"cí"}},
        {U'试', {"shì"}},
        {U'诗', {"shī"}},
        {U'诚', {"chéng"}},
        {U'语', {"yǔ", "yù"}},
        {U'诵', {"sòng"}},
        {U'诸', {"zhū"}},
        {U'谈', {"tán"}},
        {U'谅', {"liàng"}},
        {U'课', {"kè"}},
        {U'谢', {"xiè"}},
        {U'谭', {"tán"}},
        {U'谯', {"qiáo"}},
        {U'谱', {"pǔ"}},
        {U'记', {"jì"}},
        {U'讳', {"huì"}},
        {U'调', {"diào", "tiáo", "zhōu"}},
        {U'谦', {"qiān"}},
        {U'谨', {"jǐn"}},
        {U'谘', {"zī"}},
        {U'谧', {"mì"}},
        {U'赵', {"zhào"}},
        {U'钱', {"qián"}},
        {U'孙', {"sūn", "xùn"}},
        {U'王', {"wáng", "wàng"}},
        {U'冯', {"féng", "píng"}},
        {U'陈', {"chén"}},
        {U'褚', {"chǔ", "zhě", "zhǔ"}},
        {U'卫', {"wèi"}},
        {U'蒋', {"jiǎng"}},
        {U'沈', {"shěn", "chén"}},
        {U'韩', {"hán"}},
        {U'杨', {"yáng"}},
        {U'朱', {"zhū"}},
        {U'秦', {"qín"}},
        {U'尤', {"yóu"}},
        {U'许', {"xǔ", "hǔ"}},
        {U'何', {"hé", "hè", "hē"}},
        {U'吕', {"lǚ"}},
        {U'施', {"shī"}},
        {U'魏', {"wèi"}},
        {U'陶', {"táo", "yáo"}},
        {U'姜', {"jiāng"}},
        {U'戚', {"qī"}},
        {U'谢', {"xiè"}},
        {U'邹', {"zōu", "jū"}},
        {U'喻', {"yù"}},
        {U'柏', {"bǎi", "bó", "bò"}},
        {U'水', {"shuǐ"}},
        {U'窦', {"dòu"}},
        {U'章', {"zhāng"}},
        {U'云', {"yún"}},
        {U'苏', {"sū"}},
        {U'潘', {"pān"}},
        {U'葛', {"gé", "gě"}},
        {U'奚', {"xī"}},
        {U'范', {"fàn"}},
        {U'彭', {"péng", "bāng"}},
        {U'郎', {"láng", "làng"}},
        {U'鲁', {"lǔ"}},
        {U'韦', {"wéi"}},
        {U'昌', {"chāng"}},
        {U'马', {"mǎ"}},
        {U'苗', {"miáo"}},
        {U'凤', {"fèng"}},
        {U'花', {"huā"}},
        {U'方', {"fāng"}},
        {U'俞', {"yú", "shù"}},
        {U'任', {"rén", "rèn"}},
        {U'袁', {"yuán"}},
        {U'柳', {"liǔ"}},
        {U'酆', {"fēng"}},
        {U'鲍', {"bào"}},
        {U'史', {"shǐ"}},
        {U'唐', {"táng"}},
        {U'费', {"fèi", "bì"}},
        {U'廉', {"lián"}},
        {U'岑', {"cén"}},
        {U'薛', {"xuē"}},
        {U'雷', {"léi", "lèi"}},
        {U'贺', {"hè"}},
        {U'倪', {"ní"}},
        {U'汤', {"tāng", "shāng"}},
        {U'滕', {"téng"}},
        {U'殷', {"yīn", "yān", "yǐn"}},
        {U'毕', {"bì"}},
        {U'郝', {"hǎo"}},
        {U'邬', {"wū"}},
        {U'安', {"ān"}},
        {U'常', {"cháng"}},
        {U'乐', {"lè", "yuè", "yào", "lào"}},
        {U'于', {"yú", "wū"}},
        {U'时', {"shí"}},
        {U'傅', {"fù"}},
        {U'皮', {"pí"}},
        {U'卞', {"biàn"}},
        {U'齐', {"qí", "jì", "zī", "zhāi"}},
        {U'康', {"kāng"}},
        {U'伍', {"wǔ"}},
        {U'余', {"yú"}},
        {U'元', {"yuán"}},
        {U'卜', {"bǔ", "bo"}},
        {U'顾', {"gù"}},
        {U'孟', {"mèng"}},
        {U'平', {"píng"}},
        {U'黄', {"huáng"}},
        {U'和', {"hé", "hè", "huó", "huò", "hú"}},
        {U'穆', {"mù"}},
        {U'萧', {"xiāo"}},
        {U'尹', {"yǐn"}},
        {U'姚', {"yáo"}},
        {U'邵', {"shào"}},
        {U'湛', {"zhàn"}},
        {U'汪', {"wāng"}},
        {U'祁', {"qí"}},
        {U'毛', {"máo"}},
        {U'禹', {"yǔ"}},
        {U'狄', {"dí"}},
        {U'米', {"mǐ"}},
        {U'贝', {"bèi"}},
        {U'明', {"míng"}},
        {U'臧', {"zāng"}},
        {U'计', {"jì"}},
        {U'伏', {"fú"}},
        {U'成', {"chéng"}},
        {U'戴', {"dài"}},
        {U'谈', {"tán"}},
        {U'宋', {"sòng"}},
        {U'茅', {"máo"}},
        {U'庞', {"páng", "lóng"}},
        {U'熊', {"xióng"}},
        {U'纪', {"jì", "jǐ"}},
        {U'舒', {"shū"}},
        {U'屈', {"qū"}},
        {U'项', {"xiàng"}},
        {U'祝', {"zhù"}},
        {U'董', {"dǒng"}},
        {U'梁', {"liáng"}},
        {U'杜', {"dù"}},
        {U'阮', {"ruǎn"}},
        {U'蓝', {"lán"}},
        {U'闵', {"mǐn"}},
        {U'席', {"xí"}},
        {U'季', {"jì"}},
        {U'麻', {"má"}},
        {U'强', {"qiáng", "qiǎng", "jiàng"}},
        {U'贾', {"jiǎ", "gǔ"}},
        {U'路', {"lù"}},
        {U'娄', {"lóu", "lǚ"}},
        {U'危', {"wēi"}},
        {U'江', {"jiāng"}},
        {U'童', {"tóng"}},
        {U'颜', {"yán"}},
        {U'郭', {"guō"}},
        {U'梅', {"méi"}},
        {U'盛', {"shèng", "chéng"}},
        {U'林', {"lín"}},
        {U'刁', {"diāo"}},
        {U'钟', {"zhōng"}},
        {U'徐', {"xú"}},
        {U'邱', {"qiū"}},
        {U'骆', {"luò"}},
        {U'高', {"gāo"}},
        {U'夏', {"xià"}},
        {U'蔡', {"cài"}},
        {U'田', {"tián"}},
        {U'樊', {"fán"}},
        {U'胡', {"hú"}},
        {U'凌', {"líng"}},
        {U'霍', {"huò"}},
        {U'虞', {"yú"}},
        {U'万', {"wàn", "mò"}},
        {U'支', {"zhī"}},
        {U'柯', {"kē"}},
        {U'昝', {"zǎn"}},
        {U'管', {"guǎn"}},
        {U'卢', {"lú"}},
        {U'莫', {"mò", "mù"}},
        {U'经', {"jīng"}},
        {U'房', {"fáng"}},
        {U'裘', {"qiú"}},
        {U'缪', {"móu", "miù", "miào", "mù", "liǎo"}},
        {U'干', {"gān", "gàn"}},
        {U'解', {"jiě", "jiè", "xiè"}},
        {U'应', {"yīng", "yìng"}},
        {U'宗', {"zōng"}},
        {U'宣', {"xuān"}},
        {U'丁', {"dīng", "zhēng"}},
        {U'鄂', {"è"}},
        {U'仇', {"chóu", "qiú"}},
        {U'甘', {"gān"}},
        {U'厉', {"lì"}},
        {U'戎', {"róng"}},
        {U'祖', {"zǔ"}},
        {U'武', {"wǔ"}},
        {U'刘', {"liú"}},
        {U'景', {"jǐng", "yǐng"}},
        {U'詹', {"zhān"}},
        {U'束', {"shù"}},
        {U'龙', {"lóng"}},
        {U'叶', {"yè", "xié"}},
        {U'白', {"bái"}},
        {U'怀', {"huái"}},
        {U'蒲', {"pú"}},
        {U'台', {"tái", "tāi"}},
        {U'从', {"cóng", "zòng"}},
        {U'鄂', {"è"}},
        {U'索', {"suǒ"}},
        {U'咸', {"xián", "jiān"}},
        {U'籍', {"jí"}},
        {U'赖', {"lài"}},
        {U'蔺', {"lìn"}},
        {U'卓', {"zhuó"}},
        {U'屠', {"tú"}},
        {U'池', {"chí"}},
        {U'乔', {"qiáo"}},
        {U'阴', {"yīn"}},
        {U'鬱', {"yù"}},
        {U'胥', {"xū"}},
        {U'关', {"guān"}},
        {U'苗', {"miáo"}},
        {U'查', {"chá", "zhā"}},
        {U'后', {"hòu"}},
        {U'荆', {"jīng"}},
        {U'红', {"hóng", "gōng"}},
        {U'游', {"yóu"}},
        {U'竺', {"zhú"}},
        {U'权', {"quán"}},
        {U'逯', {"lù"}},
        {U'盖', {"gài", "gě", "hé"}},
        {U'益', {"yì"}},
        {U'桓', {"huán"}},
        {U'公', {"gōng"}},
        {U'岳', {"yuè"}},
        {U'帅', {"shuài"}},
        {U'缑', {"gōu"}},
        {U'亢', {"kàng"}},
        {U'况', {"kuàng"}},
        {U'郈', {"hòu"}},
        {U'琴', {"qín"}},
        {U'商', {"shāng"}},
        {U'牟', {"móu", "mù"}},
        {U'佘', {"shé"}},
        {U'佴', {"èr", "nài"}},
        {U'伯', {"bó", "bǎi", "bà"}},
        {U'赏', {"shǎng"}},
        {U'墨', {"mò"}},
        {U'哈', {"hā", "hǎ", "hà"}},
        {U'谯', {"qiáo"}},
        {U'笪', {"dá"}},
        {U'年', {"nián"}},
        {U'爱', {"ài"}},
        {U'阳', {"yáng"}},
        {U'佟', {"tóng"}},
        {U'那', {"nà", "nǎ", "nèi", "nā"}},
    };
}

void PinyinData::init_surname_map() {
    surname_map_ = {
        {U'单', "shàn"},
        {U'解', "xiè"},
        {U'查', "zhā"},
        {U'盖', "gě"},
        {U'仇', "qiú"},
        {U'朴', "piáo"},
        {U'区', "ōu"},
        {U'华', "huà"},
        {U'任', "rén"},
        {U'燕', "yān"},
        {U'盛', "shèng"},
        {U'应', "yīng"},
        {U'宁', "nìng"},
        {U'葛', "gě"},
        {U'翟', "zhái"},
        {U'乐', "yuè"},
        {U'纪', "jǐ"},
        {U'缪', "miào"},
        {U'种', "chóng"},
        {U'秘', "bì"},
        {U'洗', "xiǎn"},
        {U'祭', "zhài"},
        {U'员', "yùn"},
        {U'能', "nài"},
        {U'占', "zhān"},
        {U'阿', "ē"},
        {U'那', "nā"},
        {U'哈', "hǎ"},
    };
}

const std::vector<std::string>& PinyinData::get_pinyins(char32_t codepoint) const {
    auto it = pinyin_map_.find(codepoint);
    if (it != pinyin_map_.end()) {
        return it->second;
    }
    return empty_list_;
}

bool PinyinData::has_pinyin(char32_t codepoint) const {
    return pinyin_map_.find(codepoint) != pinyin_map_.end();
}

const SurnameMap& PinyinData::get_surnames() const {
    return surname_map_;
}

bool PinyinData::is_surname(char32_t codepoint) const {
    return surname_map_.find(codepoint) != surname_map_.end();
}

std::string PinyinData::get_surname_pinyin(char32_t codepoint) const {
    auto it = surname_map_.find(codepoint);
    if (it != surname_map_.end()) {
        return it->second;
    }
    auto it2 = pinyin_map_.find(codepoint);
    if (it2 != pinyin_map_.end() && !it2->second.empty()) {
        return it2->second[0];
    }
    return "";
}

size_t PinyinData::total_chars() const {
    return pinyin_map_.size();
}

size_t PinyinData::total_surnames() const {
    return surname_map_.size();
}

void PinyinData::clear() {
    pinyin_map_.clear();
    surname_map_.clear();
    loaded_from_files_ = false;
}

void PinyinData::reload() {
    clear();
    init_pinyin_map();
    init_surname_map();
}

}
