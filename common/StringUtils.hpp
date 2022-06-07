//
//  StringUtils.hpp
//  devbase
//
//  Created by yajun18 on 2022/3/22.
//  Copyright © 2022 shuyj. All rights reserved.
//

#ifndef StringUtils_hpp
#define StringUtils_hpp

#include <string>
#include <locale>
#include <codecvt>

std::string StringFormat(const char* fmt, ...);

inline std::wstring to_wide_string(const std::string& input)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(input);
}

inline std::string to_byte_string(const std::wstring& input)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(input);
}

#endif /* StringUtils_hpp */
