#pragma once

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

class PathUtil
{
private:
    PathUtil() = delete; // 정적 클래스이므로 인스턴스화 방지

public:
    static constexpr const char* ASSETS_ROOT = "assets";
    static constexpr const char* IMAGE_DIR = "assets/image";
    static constexpr const char* FONT_DIR = "assets/font";
    static constexpr const char* BG_DIR = "assets/image/BG";

    static std::string GetImagePath(const std::string& filename)
    {
        return CombinePath(IMAGE_DIR, filename);
    }

    static std::string GetFontPath(const std::string& filename)
    {
        return CombinePath(FONT_DIR, filename);
    }

    static std::string GetBgPath(const std::string& filename)
    {
        return CombinePath(BG_DIR, filename);
    }

    static std::string GetImagePath()
    {
        return CombinePath(GetExecutableBasePath(), IMAGE_DIR);
    }

    static std::string GetFontPath()
    {
        return CombinePath(GetExecutableBasePath(), FONT_DIR);
    }

    static std::string GetBgPath()
    {
        return CombinePath(GetExecutableBasePath(), BG_DIR);
    }

    static std::string CombinePaths(const std::string& basePath, const std::string& relativePath)
    {
        fs::path base(basePath);
        fs::path relative(relativePath);

        fs::path fullPath = base / relative;
        return fullPath.string();
    }

    static std::string CombinePath(const std::string& directory, const std::string& filename)
    {
        if (directory.empty())
        {
            return filename;
        }

        if (directory.back() == '/' || directory.back() == '\\')
        {
            return directory + filename;
        }

        return directory + "/" + filename;
    }

    // 실행 파일의 기본 경로를 반환하는 함수
    static std::string GetExecutableBasePath()
    {
        try
        {
            fs::path exePath = fs::current_path();
            return exePath.string() + "\\"; // Windows용 백슬래시 추가
        }
        catch (const std::filesystem::filesystem_error&)
        {
            // 오류 처리
            return ""; // 또는 적절한 오류 로그 추가
        }
    }
};