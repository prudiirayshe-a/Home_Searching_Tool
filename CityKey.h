//
// Created by onwil on 11/3/2025.
//

#ifndef CITYKEY_H
#define CITYKEY_H
#include <string>
#include <cctype>
#include <algorithm>
#include <utility>

//normalized vari
struct CityKey {
    std::string city;
    std::string state;
//comparison
    bool operator==(const CityKey& o) const noexcept {
        return city == o.city && state == o.state;
    }
    static std::string to_upper(std::string s) {
        for (auto& ch : s) ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
        return s;
    }

    static std::string to_title(std::string s) {
        bool newWord = true;
        for (auto& ch : s) {
            unsigned char u = static_cast<unsigned char>(ch);
            if (std::isspace(u) || ch=='-' || ch=='\'') { newWord = true; continue; }
            ch = static_cast<char>(newWord ? std::toupper(u) : std::tolower(u));
            newWord = false;
        }
        return s;
    }
    static CityKey fromRaw(std::string cityRaw, std::string stateRaw) {
        auto c = to_title(trim(std::move(cityRaw)));
        auto s = to_upper(trim(std::move(stateRaw)));
        return CityKey{c, s};
    }
};

#endif //CITYKEY_H



