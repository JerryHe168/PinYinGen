#include "polyphone_handler.h"
#include "pinyin_data.h"
#include "encoding_utils.h"
#include <mutex>
#include <cstddef>

namespace pinyingen {

namespace {

PolyphoneHandler* g_instance = nullptr;
std::once_flag g_init_flag;

}

PolyphoneHandler& PolyphoneHandler::instance() {
    std::call_once(g_init_flag, []() {
        g_instance = new PolyphoneHandler();
    });
    return *g_instance;
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
            {U'好', "hao3", {{1, {U'人', U'事', U'处', U'友'}}}},
            {U'好', "hao4", {{-1, {U'爱', U'喜', U'学', U'奇'}}}},
        }},
        {U'乐', {
            {U'乐', "le4", {{-1, {U'快', U'欢', U'娱'}}}},
            {U'乐', "yue4", {{-1, {U'音'}}, {1, {U'器', U'队', U'谱'}}}},
        }},
        {U'都', {
            {U'都', "dou1", {{1, {U'是', U'有', U'在', U'要'}}}},
            {U'都', "du1", {{-1, {U'首', U'成', U'市'}}}},
        }},
        {U'长', {
            {U'长', "chang2", {{1, {U'度', U'短', U'久', U'远', U'江', U'城'}}, {-1, {U'绵', U'细', U'特'}}}},
            {U'长', "zhang3", {{1, {U'大', U'辈', U'官', U'成'}}, {-1, {U'成', U'生', U'助', U'院'}}}},
        }},
        {U'发', {
            {U'发', "fa1", {{1, {U'现', U'生', U'展', U'送', U'明', U'生', U'出'}}, {-1, {U'开', U'出', U'触', U'出'}}}},
            {U'发', "fa4", {{1, {U'头', U'毛', U'理'}}, {-1, {U'理', U'头', U'白', U'脱'}}}},
        }},
        {U'重', {
            {U'重', "zhong4", {{1, {U'要', U'量', U'点', U'大'}}, {-1, {U'体', U'隆', U'慎'}}}},
            {U'重', "chong2", {{1, {U'复', U'新', U'叠', U'庆'}}, {-1, {U'双', U'再', U'敬'}}}},
        }},
        {U'相', {
            {U'相', "xiang1", {{1, {U'信', U'互', U'同', U'反', U'识', U'交'}}, {-1, {U'互', U'相', U'福'}}}},
            {U'相', "xiang4", {{1, {U'照', U'片', U'貌', U'册'}}, {-1, {U'照', U'画', U'本'}}}},
        }},
        {U'难', {
            {U'难', "nan2", {{1, {U'题', U'度', U'过', U'受', U'忘', U'保'}}, {-1, {U'困', U'艰', U'灾', U'为'}}}},
            {U'难', "nan4", {{1, {U'民', U'友'}}, {-1, {U'灾', U'遇', U'患', U'求'}}}},
        }},
        {U'盛', {
            {U'盛', "sheng4", {{1, {U'开', U'大', U'况', U'世', U'产'}}, {-1, {U'兴', U'鼎', U'茂', U'昌'}}}},
            {U'盛', "cheng2", {{1, {U'饭', U'器'}}, {-1, {U'盛'}}}},
        }},
        {U'地', {
            {U'地', "di4", {{1, {U'方', U'土', U'面', U'球', U'点', U'位'}}, {-1, {U'土', U'大', U'陆', U'草'}}}},
            {U'地', "de", {{-1, {U'慢', U'快', U'轻', U'重', U'好', U'坏'}}}},
        }},
        {U'得', {
            {U'得', "de2", {{1, {U'到', U'体', U'力', U'意', U'胜'}}, {-1, {U'获', U'取', U'赢'}}}},
            {U'得', "dei3", {{1, {U'可', U'必', U'须'}}}},
            {U'得', "de", {{-1, {U'跑', U'走', U'飞', U'长', U'说'}}}},
        }},
    };
}

void PolyphoneHandler::init_name_rules() {
    surname_defaults_ = {
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
    
    name_defaults_ = {
        {U'行', "xíng"},
        {U'好', "hǎo"},
        {U'乐', "lè"},
        {U'都', "dū"},
        {U'长', "cháng"},
        {U'发', "fā"},
        {U'重', "zhòng"},
        {U'相', "xiāng"},
        {U'难', "nán"},
        {U'盛', "shèng"},
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
