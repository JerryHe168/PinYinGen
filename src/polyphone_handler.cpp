#include "polyphone_handler.h"
#include "pinyin_data.h"
#include "encoding_utils.h"
#include <mutex>
#include <cstddef>

namespace pinyingen {

namespace {

}

PolyphoneHandler& PolyphoneHandler::instance() {
    static PolyphoneHandler instance;
    return instance;
}

PolyphoneHandler::PolyphoneHandler() {
    init_rules();
    init_name_rules();
}

void PolyphoneHandler::init_rules() {
    rules_ = {
        {U'行', {
            {U'行', "xing2", {{1, {U'走', U'动', U'为', U'人'}}}},
            {U'行', "hang2", {{-1, {U'银', U'商', U'排', U'列', U'同', U'业'}}}},
        }},
        {U'好', {
            {U'好', "hao3", {{1, {U'人', U'事', U'处', U'友', U'心'}}, {-1, {U'很', U'太', U'真', U'最', U'更', U'多', U'美'}}}},
            {U'好', "hao4", {{1, {U'奇', U'学', U'动'}}, {-1, {U'爱', U'喜', U'嗜', U'友'}}}},
        }},
        {U'乐', {
            {U'乐', "le4", {{-1, {U'快', U'欢', U'娱'}}}},
            {U'乐', "yue4", {{-1, {U'音'}}, {1, {U'器', U'队', U'谱', U'章'}}}},
        }},
        {U'都', {
            {U'都', "dou1", {{1, {U'是', U'有', U'在', U'要', U'好'}}, {-1, {U'全'}}}},
            {U'都', "du1", {{-1, {U'首', U'成', U'市', U'大', U'古'}}}},
        }},
        {U'长', {
            {U'长', "chang2", {{1, {U'度', U'短', U'久', U'远', U'江', U'城', U'发', U'期', U'时', U'途', U'的', U'处', U'长', U'袖', U'袍', U'裙', U'裤', U'刀', U'剑', U'枪', U'桥', U'路', U'河', U'江', U'山', U'城', U'市', U'乡', U'村', U'街', U'道', U'巷', U'楼', U'房', U'屋', U'门', U'窗', U'墙', U'柱', U'梁', U'线', U'条', U'管', U'棒', U'杆', U'针', U'丝', U'带', U'绳', U'链', U'环', U'圈', U'弧', U'弯', U'曲', U'直', U'宽', U'窄', U'高', U'低', U'深', U'浅', U'厚', U'薄', U'粗', U'细', U'小', U'轻', U'重', U'快', U'慢', U'急', U'缓', U'远', U'近', U'前', U'后', U'左', U'右', U'上', U'下', U'内', U'外', U'中', U'间', U'东', U'西', U'南', U'北', U'长', U'短', U'久', U'暂', U'永', U'远'}}, {-1, {U'绵', U'细', U'特', U'漫', U'修', U'拉', U'加', U'好', U'很', U'太', U'真', U'最', U'更', U'多', U'美', U'超', U'特', U'相', U'挺', U'怪', U'蛮', U'够', U'不', U'过', U'于', U'极', U'全', U'满', U'齐', U'整', U'完', U'正', U'偏', U'歪', U'斜', U'平', U'直', U'曲', U'弯', U'圆', U'方', U'尖', U'钝', U'锐', U'利', U'粗', U'细', U'长', U'短', U'宽', U'窄', U'高', U'低', U'深', U'浅', U'厚', U'薄', U'大', U'小', U'轻', U'重', U'快', U'慢', U'急', U'缓', U'远', U'近', U'前', U'后', U'左', U'右', U'上', U'下', U'内', U'外', U'中', U'间', U'东', U'西', U'南', U'北', U'多', U'少', U'许', U'些', U'点'}}}},
            {U'长', "zhang3", {{1, {U'大', U'辈', U'官', U'成', U'进', U'高'}}, {-1, {U'成', U'生', U'助', U'院'}}}},
        }},
        {U'发', {
            {U'发', "fa1", {{1, {U'现', U'生', U'展', U'送', U'明', U'出', U'动'}}, {-1, {U'开', U'出', U'触', U'爆', U'散'}}}},
            {U'发', "fa4", {{1, {U'头', U'毛', U'理'}}, {-1, {U'理', U'头', U'白', U'脱', U'金', U'毛'}}}},
        }},
        {U'重', {
            {U'重', "zhong4", {{1, {U'要', U'量', U'点', U'大', U'力', U'任'}}, {-1, {U'体', U'隆', U'慎', U'严'}}}},
            {U'重', "chong2", {{1, {U'复', U'新', U'叠', U'庆', U'建', U'来'}}, {-1, {U'双', U'再', U'敬'}}}},
        }},
        {U'相', {
            {U'相', "xiang1", {{1, {U'信', U'互', U'同', U'反', U'识', U'交', U'助'}}, {-1, {U'互', U'相', U'福'}}}},
            {U'相', "xiang4", {{1, {U'照', U'片', U'貌', U'册', U'面'}}, {-1, {U'照', U'画', U'本', U'看'}}}},
        }},
        {U'难', {
            {U'难', "nan2", {{1, {U'题', U'度', U'过', U'受', U'忘', U'保', U'以'}}, {-1, {U'困', U'艰', U'灾', U'为'}}}},
            {U'难', "nan4", {{1, {U'民', U'友', U'兄'}}, {-1, {U'灾', U'遇', U'患', U'求'}}}},
        }},
        {U'盛', {
            {U'盛', "sheng4", {{1, {U'开', U'大', U'况', U'世', U'产', U'利'}}, {-1, {U'兴', U'鼎', U'茂', U'昌', U'繁'}}}},
            {U'盛', "cheng2", {{1, {U'饭', U'器'}}, {-1, {U'盛'}}}},
        }},
        {U'地', {
            {U'地', "di4", {{1, {U'方', U'土', U'面', U'球', U'点', U'位', U'下'}}, {-1, {U'土', U'大', U'陆', U'草', U'天'}}}},
            {U'地', "de", {{-1, {U'慢', U'快', U'轻', U'重', U'好', U'坏'}}}},
        }},
        {U'得', {
            {U'得', "de2", {{1, {U'到', U'体', U'力', U'意', U'胜', U'罪'}}, {-1, {U'获', U'取', U'赢', U'值'}}}},
            {U'得', "dei3", {{1, {U'可', U'必', U'须', U'还'}}}},
            {U'得', "de", {{-1, {U'跑', U'走', U'飞', U'长', U'说', U'写'}}}},
        }},
    };
}

void PolyphoneHandler::init_name_rules() {
    surname_defaults_ = {
        {U'单', "shan4"},
        {U'解', "xie4"},
        {U'查', "zha1"},
        {U'盖', "ge3"},
        {U'仇', "qiu2"},
        {U'朴', "piao2"},
        {U'区', "ou1"},
        {U'华', "hua4"},
        {U'任', "ren2"},
        {U'燕', "yan1"},
        {U'盛', "sheng4"},
        {U'应', "ying1"},
        {U'宁', "ning4"},
        {U'葛', "ge3"},
        {U'翟', "zhai2"},
        {U'乐', "yue4"},
        {U'纪', "ji3"},
        {U'缪', "miao4"},
        {U'种', "chong2"},
        {U'秘', "bi4"},
        {U'洗', "xian3"},
        {U'祭', "zhai4"},
        {U'员', "yun4"},
        {U'能', "nai4"},
        {U'占', "zhan1"},
        {U'阿', "e1"},
        {U'那', "na1"},
        {U'哈', "ha3"},
    };
    
    name_defaults_ = {
        {U'行', "xing2"},
        {U'好', "hao3"},
        {U'乐', "le4"},
        {U'都', "du1"},
        {U'长', "chang2"},
        {U'发', "fa1"},
        {U'重', "zhong4"},
        {U'相', "xiang1"},
        {U'难', "nan2"},
        {U'盛', "sheng4"},
    };
}

bool PolyphoneHandler::match_context(
    const std::vector<char32_t>& text,
    size_t index,
    const std::vector<std::tuple<ptrdiff_t, std::vector<char32_t>>>& contexts
) const {
    for (const auto& ctx : contexts) {
        ptrdiff_t offset = std::get<0>(ctx);
        const auto& chars = std::get<1>(ctx);
        
        ptrdiff_t target_pos = static_cast<ptrdiff_t>(index) + offset;
        if (target_pos >= 0 && static_cast<size_t>(target_pos) < text.size()) {
            char32_t target = text[static_cast<size_t>(target_pos)];
            for (char32_t c : chars) {
                if (c == target) {
                    return true;
                }
            }
        }
    }
    return false;
}

std::string PolyphoneHandler::smart_select(
    const std::vector<char32_t>& text,
    size_t index,
    const std::vector<std::string>& candidates
) const {
    if (candidates.empty()) {
        return {};
    }
    if (candidates.size() == 1) {
        return candidates[0];
    }
    
    if (index < text.size()) {
        char32_t c = text[index];
        auto it = rules_.find(c);
        if (it != rules_.end()) {
            for (const auto& rule : it->second) {
                if (match_context(text, index, rule.context)) {
                    return rule.pinyin;
                }
            }
        }
    }
    
    return candidates[0];
}

std::vector<std::string> PolyphoneHandler::get_all_pinyins(char32_t codepoint) const {
    return PinyinData::instance().get_pinyins(codepoint);
}

bool PolyphoneHandler::is_polyphone(char32_t codepoint) const {
    const auto& pinyins = PinyinData::instance().get_pinyins(codepoint);
    return pinyins.size() > 1;
}

std::string PolyphoneHandler::get_name_pinyin(char32_t codepoint, bool is_surname) const {
    if (is_surname) {
        auto it = surname_defaults_.find(codepoint);
        if (it != surname_defaults_.end()) {
            return it->second;
        }
        return {};
    }
    
    auto it = name_defaults_.find(codepoint);
    if (it != name_defaults_.end()) {
        return it->second;
    }
    
    const auto& pinyins = PinyinData::instance().get_pinyins(codepoint);
    if (!pinyins.empty()) {
        return pinyins[0];
    }
    
    return {};
}

}
