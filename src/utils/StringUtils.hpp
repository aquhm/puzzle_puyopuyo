#pragma once
#include <string>
#include <string_view>
#include <Windows.h>


namespace StringUtils
{
    inline std::wstring Utf8ToWide(const std::string& str)
    {
        if (str.empty()) return std::wstring();

        // UTF-8에서 와이드 문자열로 변환
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
        if (size_needed == 0) return std::wstring();

        std::wstring wstr(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);
        return wstr;
    }

    inline std::string WideToUtf8(const std::wstring& wstr)
    {
        if (wstr.empty()) return std::string();

        // 와이드 문자열에서 UTF-8로 변환
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
        if (size_needed == 0) return std::string(); // 오류 처리

        std::string str(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &str[0], size_needed, nullptr, nullptr);
        return str;
    }

    inline std::wstring AnsiToWide(const std::string& str)
    {
        if (str.empty()) return std::wstring();

        // ANSI (현재 코드 페이지)에서 와이드 문자열로 변환
        int size_needed = MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), nullptr, 0);
        if (size_needed == 0) return std::wstring(); // 오류 처리

        std::wstring wstr(size_needed, 0);
        MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);
        return wstr;
    }

    inline std::string WideToAnsi(const std::wstring& wstr)
    {
        if (wstr.empty()) return std::string();

        // 와이드 문자열에서 ANSI (현재 코드 페이지)로 변환
        int size_needed = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
        if (size_needed == 0) return std::string(); // 오류 처리

        std::string str(size_needed, 0);
        WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), (int)wstr.size(), &str[0], size_needed, nullptr, nullptr);
        return str;
    }
}