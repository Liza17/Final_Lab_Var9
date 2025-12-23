#pragma once

#ifndef _SILENCE_CXX20_U8PATH_DEPRECATION_WARNING
#define _SILENCE_CXX20_U8PATH_DEPRECATION_WARNING
#endif

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <vector>
#include <ctime>      // Для time_t и struct tm
#include <fcntl.h>    // Для _O_U16TEXT
#include <io.h>       // Для _setmode

#ifdef _WIN32
#define NOMINMAX 
#include <windows.h>
#endif

namespace utils {
    const std::wstring DATA_DIR = L"data/";

    inline void setupConsole() {
#ifdef _WIN32
        (void)_setmode(_fileno(stdout), _O_U16TEXT);
        (void)_setmode(_fileno(stdin), _O_U16TEXT);
        (void)_setmode(_fileno(stderr), _O_U16TEXT);
#endif
    }


    inline std::wstring utf8_to_wstring(const std::string& str) {
        if (str.empty()) return std::wstring();
#ifdef _WIN32
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
        std::wstring wstrTo(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
        return wstrTo;
#else
        return std::wstring(str.begin(), str.end());
#endif
    }

    inline std::wstring getPath(const std::wstring& filename) {
        if (std::filesystem::exists(filename)) return filename;
        std::wstring pathInData = DATA_DIR + filename;
        if (std::filesystem::exists(pathInData)) return pathInData;
        return filename;
    }

    inline std::wstring readFile(const std::wstring& filename) {
        std::wstring path = getPath(filename);


        std::ifstream t(path, std::ios::in | std::ios::binary);
        if (!t.is_open()) {
            throw std::runtime_error("Cannot open file");
        }

        std::stringstream buffer;
        buffer << t.rdbuf();
        std::string contentUtf8 = buffer.str();

        if (contentUtf8.size() >= 3 &&
            (unsigned char)contentUtf8[0] == 0xEF &&
            (unsigned char)contentUtf8[1] == 0xBB &&
            (unsigned char)contentUtf8[2] == 0xBF) {
            contentUtf8 = contentUtf8.substr(3);
        }

        return utf8_to_wstring(contentUtf8);
    }

    inline time_t parseISO8601(const std::wstring& ts) {
        int y, M, d, h, m, s;
        if (swscanf_s(ts.c_str(), L"%d-%d-%dT%d:%d:%d", &y, &M, &d, &h, &m, &s) == 6) {
            struct tm t = { 0 };
            t.tm_year = y - 1900;
            t.tm_mon = M - 1;
            t.tm_mday = d;
            t.tm_hour = h;
            t.tm_min = m;
            t.tm_sec = s;
            t.tm_isdst = -1;
            return mktime(&t);
        }
        return 0;
    }
}