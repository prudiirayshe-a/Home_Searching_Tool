//
// Created by onwil on 11/3/2025.
//

#ifndef CITYKEY_H
#define CITYKEY_H
#pragma once
#include <string>
#include <cctype>
#include <algorithm>
#include <utility>




struct CityKey {
    std::string city;
    std::string state;

    bool operator==(const CityKey& o) const noexcept {
        return city == o.city && state == o.state;
    }

    static std::string trim(std::string s) {
        auto notspace = [](unsigned char c){ return !std::isspace(c); };
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), notspace));
        s.erase(std::find_if(s.rbegin(), s.rend(), notspace).base(), s.end());
        return s;
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
struct CityKeyHash {
    std::size_t operator()(const CityKey& k) const noexcept {
        std::size_t h1 = std::hash<std::string>{}(k.city);
        std::size_t h2 = std::hash<std::string>{}(k.state);
        return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1<<6) + (h1>>2));
    }
};
#endif //CITYKEY_H
