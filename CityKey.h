//
// Created by onwil on 11/3/2025.
//
#ifndef CITYKEY_H
#define CITYKEY_H
#include <string>
#include <cctype>
#include <algorithm>

struct CityKey {
    std::string city;
    std::string state;

    bool operator==(const CityKey& o) const noexcept {
        return city == o.city && state == o.state;
    }

    static std::string trim(std::string s) {
        auto isspace_uc = [](unsigned char c){ return std::isspace(c); };
        auto b = s.begin(), e = s.end();
        while (b != e && isspace_uc(static_cast<unsigned char>(*b))) ++b;
        while (e != b && isspace_uc(static_cast<unsigned char>(*(e-1)))) --e;
        return std::string(b, e);
    }

    static std::string to_upper(std::string s) {
        for (auto& ch : s) {
            ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
        }
        return s;
    }

    static std::string to_title(std::string s) {
        bool newWord = true;
        for (auto& ch : s) {
            unsigned char u = static_cast<unsigned char>(ch);
            if (std::isspace(u) || ch=='-' || ch=='\'') {
                newWord = true;
                continue;
            }
            if (newWord) {
                ch = static_cast<char>(std::toupper(u));
                newWord = false;
            } else {
                ch = static_cast<char>(std::tolower(u));
            }
        }
        return s;
    }

    static CityKey fromRaw(std::string cityRaw, std::string stateRaw) {
        auto c = to_title(trim(std::move(cityRaw)));
        auto s = to_upper(trim(std::move(stateRaw)));
        return CityKey{c, s};
    }
};

#endif // CITYKEY_H
