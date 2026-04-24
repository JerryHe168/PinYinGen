#include "name_generator.h"
#include "pinyin_data.h"
#include "polyphone_handler.h"
#include "tone_handler.h"
#include "encoding_utils.h"
#include <random>
#include <chrono>
#include <sstream>

namespace pinyingen {

NameGenerator& NameGenerator::instance() {
    static NameGenerator inst;
    return inst;
}

NameGenerator::NameGenerator()
    : rng_(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count())) {
    init_common_surnames();
    init_given_name_chars();
    surname_dist_ = std::uniform_int_distribution<size_t>(0, common_surnames_.size() - 1);
    given_name_dist_ = std::uniform_int_distribution<size_t>(0, given_name_chars_.size() - 1);
}

void NameGenerator::init_common_surnames() {
    common_surnames_ = {
        "王", "李", "张", "刘", "陈", "杨", "赵", "黄", "周", "吴",
        "徐", "孙", "胡", "朱", "高", "林", "何", "郭", "马", "罗",
        "梁", "宋", "郑", "谢", "韩", "唐", "冯", "于", "董", "萧",
        "程", "曹", "袁", "邓", "许", "傅", "沈", "曾", "彭", "吕",
        "苏", "卢", "蒋", "蔡", "贾", "丁", "魏", "薛", "叶", "阎",
        "余", "潘", "杜", "戴", "夏", "钟", "汪", "田", "任", "姜",
        "范", "方", "石", "姚", "谭", "廖", "邹", "熊", "金", "陆",
        "郝", "孔", "白", "崔", "康", "毛", "邱", "秦", "江", "史",
        "顾", "侯", "邵", "孟", "龙", "万", "段", "雷", "钱", "汤",
        "尹", "黎", "易", "常", "武", "乔", "贺", "赖", "龚", "文",
        "庞", "樊", "兰", "殷", "施", "陶", "洪", "翟", "安", "颜",
        "倪", "严", "牛", "温", "芦", "季", "俞", "章", "鲁", "葛",
        "伍", "韦", "申", "尤", "毕", "聂", "丛", "焦", "向", "柳",
        "邢", "路", "岳", "齐", "沿", "梅", "莫", "庄", "辛", "管",
        "祝", "左", "涂", "谷", "祁", "时", "舒", "耿", "牟", "卜",
        "詹", "关", "苗", "凌", "费", "纪", "靳", "盛", "童",
        "欧", "甄", "项", "曲", "成", "游", "阳", "裴", "席", "卫",
        "查", "屈", "鲍", "位", "覃", "霍", "翁", "隋", "植",
        "甘", "景", "薄", "单", "包", "司", "柏", "宁", "柯", "阮",
        "桂", "闵", "欧阳", "解", "强", "柴", "华", "车", "冉", "房",
        "边", "辜", "吉", "饶", "刁", "瞿", "戚", "丘", "古", "米",
        "池", "滕", "晋", "苑", "邬", "臧", "畅", "宫", "来", "嵺",
        "苟", "全", "褚", "廉", "简", "娄", "盖", "符", "奚", "木",
        "穆", "党", "燕", "郎", "邸", "冀", "谈", "姬", "屠", "连",
        "郜", "晏", "栾", "郁", "商", "蒙", "计", "喻", "揭", "窦",
        "迟", "宇", "敖", "糜", "鄢", "冷", "卓", "花", "仇", "艾",
        "蓝", "都", "巩", "稽", "井", "练", "仲", "乐", "虞", "卞",
        "封", "竺", "冼", "原", "官", "衣", "楚", "佟", "栗", "匡",
        "宗", "应", "台", "巫", "鞠", "僧", "桑", "荆", "谌", "银",
        "扬", "明", "沙", "薄", "伏", "岑", "习", "胥", "保", "和",
    };
}

void NameGenerator::init_given_name_chars() {
    given_name_chars_ = {
        "伟", "强", "华", "明", "超", "勇", "军", "杰", "磊", "涛",
        "鹏", "浩", "宇", "博", "文", "轩", "航", "诚", "瑞", "豪",
        "天", "伦", "嘉", "俊", "驰", "彬", "富", "顺", "信", "子",
        "梓", "晨", "辰", "逸", "泽", "昊", "然", "睿", "翔", "旭",
        "阳", "峰", "毅", "辉", "兴", "荣", "鑫", "亮", "洋", "凯",
        "娟", "艳", "娜", "敏", "静", "丽", "芬", "芳", "燕", "玲",
        "桂", "英", "慧", "红", "梅", "琴", "素", "云", "莲", "真",
        "雪", "爱", "妹", "霞", "香", "月", "珠", "翠", "玉", "凤",
        "洁", "彩", "春", "花", "兰", "萍", "雅", "芝", "琳", "欣",
        "涵", "萱", "怡", "妍", "瑶", "彤", "琪", "璇", "瑾", "玮",
        "珏", "珂", "琼", "环", "珊", "珍", "琅", "瑚", "璋", "璜",
        "璧", "玺", "诗", "梦", "雨", "思", "若", "冰", "清", "婉",
        "婕", "婧", "婵", "婷", "媚", "媛", "嫣", "妙", "佳", "伊",
        "依", "柔", "曼", "莉", "璐", "茜", "菲", "芸", "蕾", "蕊",
        "薇", "馨", "馥", "岚", "霏", "露", "霜", "雾", "霄", "雯",
        "霓", "虹", "星", "日", "地", "山", "水",
        "海", "河", "江", "湖", "洋", "泉", "溪", "波", "涛", "浪",
        "潮", "汐", "沐", "浴", "泳", "瀚", "润",
        "澈", "澄", "净", "白", "素", "纯", "朴",
        "实", "诚", "信", "义", "仁", "礼", "智", "慧", "德", "行",
        "道", "法", "术", "艺", "武", "敢", "刚",
        "韧", "和", "顺", "安", "康", "健",
        "禄", "寿", "财", "宝", "金",
        "铜", "铁", "锡", "铅", "锌", "铝", "镁", "钙", "钠",
        "钾", "碳", "硅", "磷", "硫", "氯", "氩", "氦", "氖", "氪",
        "氙", "氡", "氢", "氧", "氮", "氟", "溴", "碘", "砹", "硼",
    };
}

std::string NameGenerator::get_random_surname(bool allow_multiple) const {
    size_t idx = surname_dist_(rng_);
    if (idx >= common_surnames_.size()) {
        idx = 0;
    }
    return common_surnames_[idx];
}

std::string NameGenerator::get_random_given_name(size_t length) const {
    if (length == 0 || given_name_chars_.empty()) {
        return {};
    }
    
    std::ostringstream oss;
    for (size_t i = 0; i < length; ++i) {
        size_t idx = given_name_dist_(rng_);
        if (idx >= given_name_chars_.size()) {
            idx = 0;
        }
        oss << given_name_chars_[idx];
    }
    return oss.str();
}

NameGenerator::GeneratedName NameGenerator::generate(const GenerateOptions& options) const {
    GeneratedName result;
    
    result.surname = get_random_surname(options.surname_count > 1);
    result.given_name = get_random_given_name(options.given_name_length);
    result.name = result.surname + result.given_name;
    
    if (options.include_pinyin) {
        result.pinyin = convert_name(result.name, options.pinyin_options);
    }
    
    return result;
}

std::vector<NameGenerator::GeneratedName> NameGenerator::generate_batch(
    size_t count,
    const GenerateOptions& options
) const {
    std::vector<GeneratedName> results;
    results.reserve(count);
    
    for (size_t i = 0; i < count; ++i) {
        results.push_back(generate(options));
    }
    
    return results;
}

NamePinyin NameGenerator::convert_name(
    std::string_view name,
    const PinyinConverter::Options& options
) const {
    NamePinyin result;
    
    if (name.empty()) {
        return result;
    }
    
    std::vector<char32_t> codepoints;
    try {
        codepoints = EncodingUtils::string_to_codepoints(name);
    } catch (...) {
        return result;
    }
    
    if (codepoints.empty()) {
        return result;
    }
    
    const auto& polyphone = PolyphoneHandler::instance();
    const auto& data = PinyinData::instance();
    
    char32_t surname_char = codepoints[0];
    std::string surname_pinyin = polyphone.get_name_pinyin(surname_char, true);
    
    if (surname_pinyin.empty()) {
        const auto& pinyins = data.get_pinyins(surname_char);
        if (!pinyins.empty()) {
            surname_pinyin = pinyins[0];
        }
    }
    
    result.surname = ToneHandler::convert_tone(surname_pinyin, options.tone_style);
    
    std::ostringstream full_oss;
    std::ostringstream initials_oss;
    
    full_oss << result.surname;
    initials_oss << ToneHandler::convert_tone(surname_pinyin, ToneStyle::FirstLetter);
    
    for (size_t i = 1; i < codepoints.size(); ++i) {
        char32_t c = codepoints[i];
        std::string pinyin = polyphone.get_name_pinyin(c, false);
        
        if (pinyin.empty()) {
            const auto& pinyins = data.get_pinyins(c);
            if (!pinyins.empty()) {
                pinyin = pinyins[0];
            }
        }
        
        std::string converted = ToneHandler::convert_tone(pinyin, options.tone_style);
        result.given_name.push_back(converted);
        
        full_oss << options.separator << converted;
        initials_oss << ToneHandler::convert_tone(pinyin, ToneStyle::FirstLetter);
    }
    
    result.full_pinyin = full_oss.str();
    result.initials = initials_oss.str();
    
    return result;
}

const std::vector<std::string>& NameGenerator::get_common_surnames() const {
    return common_surnames_;
}

const std::vector<std::string>& NameGenerator::get_given_name_chars() const {
    return given_name_chars_;
}

void NameGenerator::set_seed(unsigned int seed) {
    rng_.seed(seed);
}

}
