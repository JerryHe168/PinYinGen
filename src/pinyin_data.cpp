#include "pinyin_data.h"
#include <mutex>

namespace pinyingen {

std::once_flag PinyinData::init_flag_;
PinyinData* PinyinData::instance_ = nullptr;

PinyinData& PinyinData::instance() {
    std::call_once(init_flag_, []() {
        instance_ = new PinyinData();
    });
    return *instance_;
}

PinyinData::PinyinData() {
    init_pinyin_map();
    init_surname_map();
}

void PinyinData::init_pinyin_map() {
    pinyin_map_ = {
        {U'张', {"zhāng"}},
        {U'王', {"wáng", "wàng"}},
        {U'李', {"lǐ"}},
        {U'刘', {"liú"}},
        {U'陈', {"chén"}},
        {U'杨', {"yáng"}},
        {U'赵', {"zhào"}},
        {U'黄', {"huáng"}},
        {U'周', {"zhōu"}},
        {U'吴', {"wú"}},
        {U'徐', {"xú"}},
        {U'孙', {"sūn", "xùn"}},
        {U'胡', {"hú"}},
        {U'朱', {"zhū"}},
        {U'高', {"gāo"}},
        {U'林', {"lín"}},
        {U'何', {"hé", "hè", "hē"}},
        {U'郭', {"guō"}},
        {U'马', {"mǎ"}},
        {U'罗', {"luó"}},
        {U'梁', {"liáng"}},
        {U'宋', {"sòng"}},
        {U'郑', {"zhèng"}},
        {U'谢', {"xiè"}},
        {U'韩', {"hán"}},
        {U'唐', {"táng"}},
        {U'冯', {"féng", "píng"}},
        {U'于', {"yú"}},
        {U'董', {"dǒng"}},
        {U'萧', {"xiāo"}},
        {U'程', {"chéng"}},
        {U'曹', {"cáo"}},
        {U'袁', {"yuán"}},
        {U'邓', {"dèng"}},
        {U'许', {"xǔ", "hǔ"}},
        {U'傅', {"fù"}},
        {U'沈', {"shěn", "chén"}},
        {U'曾', {"zēng", "céng"}},
        {U'彭', {"péng", "bāng"}},
        {U'吕', {"lǚ"}},
        {U'苏', {"sū"}},
        {U'卢', {"lú"}},
        {U'蒋', {"jiǎng"}},
        {U'蔡', {"cài"}},
        {U'贾', {"jiǎ", "gǔ"}},
        {U'丁', {"dīng", "zhēng"}},
        {U'魏', {"wèi"}},
        {U'薛', {"xuē"}},
        {U'叶', {"yè", "xié"}},
        {U'阎', {"yán"}},
        {U'余', {"yú"}},
        {U'潘', {"pān"}},
        {U'杜', {"dù"}},
        {U'戴', {"dài"}},
        {U'夏', {"xià"}},
        {U'钟', {"zhōng"}},
        {U'汪', {"wāng"}},
        {U'田', {"tián"}},
        {U'任', {"rén", "rèn"}},
        {U'姜', {"jiāng"}},
        {U'范', {"fàn"}},
        {U'方', {"fāng"}},
        {U'石', {"shí", "dàn"}},
        {U'姚', {"yáo"}},
        {U'谭', {"tán"}},
        {U'廖', {"liào"}},
        {U'邹', {"zōu"}},
        {U'熊', {"xióng"}},
        {U'金', {"jīn"}},
        {U'陆', {"lù", "liù"}},
        {U'郝', {"hǎo", "shì"}},
        {U'孔', {"kǒng"}},
        {U'白', {"bái"}},
        {U'崔', {"cuī"}},
        {U'康', {"kāng"}},
        {U'毛', {"máo"}},
        {U'邱', {"qiū"}},
        {U'秦', {"qín"}},
        {U'江', {"jiāng"}},
        {U'史', {"shǐ"}},
        {U'顾', {"gù"}},
        {U'侯', {"hóu"}},
        {U'邵', {"shào"}},
        {U'孟', {"mèng"}},
        {U'龙', {"lóng"}},
        {U'万', {"wàn", "mò"}},
        {U'段', {"duàn"}},
        {U'雷', {"léi"}},
        {U'钱', {"qián"}},
        {U'汤', {"tāng", "shāng"}},
        {U'尹', {"yǐn"}},
        {U'黎', {"lí"}},
        {U'易', {"yì"}},
        {U'常', {"cháng"}},
        {U'武', {"wǔ"}},
        {U'乔', {"qiáo"}},
        {U'贺', {"hè"}},
        {U'赖', {"lài"}},
        {U'龚', {"gōng"}},
        {U'文', {"wén"}},
        {U'华', {"huá", "huà", "huā"}},
        {U'伟', {"wěi"}},
        {U'强', {"qiáng", "qiǎng", "jiàng"}},
        {U'军', {"jūn"}},
        {U'平', {"píng"}},
        {U'明', {"míng"}},
        {U'超', {"chāo"}},
        {U'勇', {"yǒng"}},
        {U'娟', {"juān"}},
        {U'艳', {"yàn"}},
        {U'娜', {"nà", "nuó"}},
        {U'敏', {"mǐn"}},
        {U'静', {"jìng"}},
        {U'丽', {"lì", "lí"}},
        {U'芬', {"fēn"}},
        {U'芳', {"fāng"}},
        {U'燕', {"yàn", "yān"}},
        {U'玲', {"líng"}},
        {U'桂', {"guì"}},
        {U'英', {"yīng"}},
        {U'慧', {"huì"}},
        {U'红', {"hóng", "gōng"}},
        {U'梅', {"méi"}},
        {U'琴', {"qín"}},
        {U'素', {"sù"}},
        {U'云', {"yún"}},
        {U'莲', {"lián"}},
        {U'真', {"zhēn"}},
        {U'雪', {"xuě"}},
        {U'爱', {"ài"}},
        {U'妹', {"mèi"}},
        {U'霞', {"xiá"}},
        {U'香', {"xiāng"}},
        {U'月', {"yuè"}},
        {U'珠', {"zhū"}},
        {U'翠', {"cuì"}},
        {U'玉', {"yù"}},
        {U'凤', {"fèng"}},
        {U'洁', {"jié"}},
        {U'彩', {"cǎi"}},
        {U'春', {"chūn"}},
        {U'花', {"huā"}},
        {U'兰', {"lán"}},
        {U'凤', {"fèng"}},
        {U'杰', {"jié"}},
        {U'磊', {"lěi"}},
        {U'洋', {"yáng"}},
        {U'艳', {"yàn"}},
        {U'萍', {"píng"}},
        {U'雅', {"yǎ"}},
        {U'芝', {"zhī"}},
        {U'文', {"wén"}},
        {U'华', {"huá"}},
        {U'辉', {"huī"}},
        {U'兴', {"xīng", "xìng"}},
        {U'荣', {"róng"}},
        {U'鑫', {"xīn"}},
        {U'鹏', {"péng"}},
        {U'涛', {"tāo"}},
        {U'明', {"míng"}},
        {U'亮', {"liàng"}},
        {U'宇', {"yǔ"}},
        {U'浩', {"hào"}},
        {U'凯', {"kǎi"}},
        {U'博', {"bó"}},
        {U'文', {"wén"}},
        {U'轩', {"xuān"}},
        {U'航', {"háng"}},
        {U'诚', {"chéng"}},
        {U'瑞', {"ruì"}},
        {U'豪', {"háo"}},
        {U'天', {"tiān"}},
        {U'伦', {"lún"}},
        {U'嘉', {"jiā"}},
        {U'俊', {"jùn"}},
        {U'驰', {"chí"}},
        {U'彬', {"bīn"}},
        {U'富', {"fù"}},
        {U'顺', {"shùn"}},
        {U'信', {"xìn", "shēn"}},
        {U'子', {"zǐ", "zì"}},
        {U'梓', {"zǐ"}},
        {U'晨', {"chén"}},
        {U'辰', {"chén"}},
        {U'逸', {"yì"}},
        {U'依', {"yī"}},
        {U'怡', {"yí"}},
        {U'诗', {"shī"}},
        {U'琳', {"lín"}},
        {U'欣', {"xīn"}},
        {U'涵', {"hán"}},
        {U'萱', {"xuān"}},
        {U'怡', {"yí"}},
        {U'妍', {"yán"}},
        {U'瑶', {"yáo"}},
        {U'彤', {"tóng"}},
        {U'琪', {"qí"}},
        {U'璇', {"xuán"}},
        {U'瑾', {"jǐn"}},
        {U'玮', {"wěi"}},
        {U'珏', {"jué"}},
        {U'珂', {"kē"}},
        {U'琼', {"qióng"}},
        {U'瑶', {"yáo"}},
        {U'环', {"huán"}},
        {U'玲', {"líng"}},
        {U'珊', {"shān"}},
        {U'珍', {"zhēn"}},
        {U'珠', {"zhū"}},
        {U'琳', {"lín"}},
        {U'琅', {"láng"}},
        {U'瑚', {"hú"}},
        {U'璋', {"zhāng"}},
        {U'璜', {"huáng"}},
        {U'璧', {"bì"}},
        {U'玺', {"xǐ"}},
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
        {U'赵', "zhào"},
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
    return pinyin_map_.count(codepoint) > 0;
}

const SurnameMap& PinyinData::get_surnames() const {
    return surname_map_;
}

bool PinyinData::is_surname(char32_t codepoint) const {
    return surname_map_.count(codepoint) > 0;
}

std::string PinyinData::get_surname_pinyin(char32_t codepoint) const {
    auto it = surname_map_.find(codepoint);
    if (it != surname_map_.end()) {
        return it->second;
    }
    auto pinyins = get_pinyins(codepoint);
    if (!pinyins.empty()) {
        return pinyins[0];
    }
    return {};
}

size_t PinyinData::total_chars() const {
    return pinyin_map_.size();
}

size_t PinyinData::total_surnames() const {
    return surname_map_.size();
}

}
