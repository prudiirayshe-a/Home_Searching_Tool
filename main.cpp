#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include "index.h"

// trim spaces
static std::string trim(std::string s) {
    auto notspace = [](unsigned char c){ return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notspace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notspace).base(), s.end());
    return s;
}
int main(int argc, char** argv) {
    const std::string csv = argv[1];
    SpatialIndex index;
    try {
        index.loadCSV(csv, true); 
    } catch (const std::exception& e) {
        std::cerr << "ERROR loading CSV: " << e.what() << "\n";
        return 1;
    }
    auto allStates = index.getStates();
    if (allStates.empty()) {
        std::cerr << "No states found.\n";
        return 1;
    }
    std::cout << "Available states (" << allStates.size() << "): ";
    for (size_t i = 0; i < allStates.size(); ++i) {
        std::cout << allStates[i] << (i + 1 < allStates.size() ? ", " : "\n");
    }
    std::string stateIn;
    std::vector<std::string> cities;
    while (true) {
        std::cout << "Type state (e.g., FL): ";
        if (!std::getline(std::cin, stateIn)) {
            std::cerr << "Input error.\n";
            return 1;
        }
        stateIn = trim(stateIn);
        if (stateIn.empty()) continue;

        cities = index.getCities(stateIn);
        if (cities.empty()) {
            std::cout << "No cities found for that state. Try again.\n";
            continue;
        }
        break;
    }
    std::cout << "Cities in " << stateIn << " (" << cities.size() << "):\n";
    for (size_t i = 0; i < cities.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << cities[i] << "\n";
    }
    std::string cityIn;
    while (true) {
        std::cout << "Type city exactly as shown (or number 1-" << cities.size() << "): ";
        if (!std::getline(std::cin, cityIn)) {
            std::cerr << "Input error.\n";
            return 1;
        }
        cityIn = trim(cityIn);
        if (cityIn.empty()) continue;
        bool chosen = false;
        if (std::all_of(cityIn.begin(), cityIn.end(),
                        [](unsigned char c){ return std::isdigit(c); })) {
            int idx = std::stoi(cityIn);
            if (idx >= 1 && static_cast<size_t>(idx) <= cities.size()) {
                cityIn = cities[static_cast<size_t>(idx - 1)];
                chosen = true;
            }
        }
        if (!chosen) {
            auto it = std::find(cities.begin(), cities.end(), cityIn);
            if (it == cities.end()) {
                std::cout << "City not in list. Try again.\n";
                continue;
            }
        }

        auto center = index.getCenter(stateIn, cityIn);
        if (!center) {
            std::cout << "No center for " << cityIn << ", " << stateIn << ". Try again.\n";
            continue;
        } else {
            std::cout << "Center " << cityIn << ", " << stateIn << " = ("
                      << center->lat0 << ", " << center->lon0 << ")\n";
            break;
        }
    }
    std::string line;
    double radius_km = 0.0;
    while (true) {
        std::cout << "Type distance from city center (km): ";
        if (!std::getline(std::cin, line)) {
            std::cerr << "Input error.\n";
            return 1;
        }
        line = trim(line);
        if (line.empty()) continue;
        try {
            size_t pos = 0;
            radius_km = std::stod(line, &pos);
            if (pos != line.size() || radius_km < 0.0) {
                std::cout << "Please enter a non-negative number.\n";
                continue;
            }
            break;
        } catch (...) {
            std::cout << "Invalid number. Try again.\n";
        }
    }

    auto pts = index.queryKmFast(stateIn, cityIn, radius_km);
    std::cout << "Houses within " << radius_km << " km of " << cityIn << ", " << stateIn
              << ": " << pts.size() << "\n";

    return 0;
}
