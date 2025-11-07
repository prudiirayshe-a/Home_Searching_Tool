#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include "index.h"

using namespace std;
//trims. Added this to match the map keys. Without it the csv cant find what we're looking for
static string trim(string s) {
    size_t b = 0, e = s.size();
    while (b < e && isspace((unsigned char)s[b])) ++b;
    while (e > b && isspace((unsigned char)s[e - 1])) --e;
    return s.substr(b, e - b);
}

int main() {
    //csv path
    string csv;
    cout << "CSV path: ";
    getline(cin, csv);
    csv = trim(csv);
    //makes sure it loaded, needed this for debug
    SpatialIndex index;
    index.loadCSV(csv, true);
    //states formatting
    vector<string> states = index.getStates();
    cout << "States (" << states.size() << "): ";
    for (size_t i = 0; i < states.size(); ++i) {
        cout << states[i] << (i + 1 < states.size() ? ", " : "\n");
    }
    //state input
    string stateIn;
    cout << "State (e.g., FL): ";
    getline(cin, stateIn);
    stateIn = trim(stateIn);

    //city formatting. For frontend to change if you want
    vector<string> cities = index.getCities(stateIn);
    cout << "Cities in " << stateIn << " (" << cities.size() << "):\n";
    for (size_t i = 0; i < cities.size(); ++i) {
        cout << "  " << (i + 1) << ". " << cities[i] << "\n";
    }
    //city input
    string cityIn;
    cout << "City (exact or number 1-" << cities.size() << "): ";
    getline(cin, cityIn);
    cityIn = trim(cityIn);
    //formats how the city is selcted (1-4). Hoenstly having string typing just caused more problems with trims
    bool allDigits = !cityIn.empty() &&
        all_of(cityIn.begin(), cityIn.end(), [](unsigned char c){ return isdigit(c); });
    if (allDigits) {
        int idx = stoi(cityIn);
        if (idx >= 1 && (size_t)idx <= cities.size()) {
            cityIn = cities[(size_t)idx - 1];
        }
    }
    //more debug for lat and lot calculations
    CenterLL center;
    index.getCenter(stateIn, cityIn, center);
    cout << "Center: (" << center.lat0 << ", " << center.lon0 << ")\n";
    //radius input
    string rstr;
    cout << "Radius (km): ";
    getline(cin, rstr);
    rstr = trim(rstr);
    double radius_km = rstr.empty() ? 0.0 : stod(rstr);
    //prints
    vector<PointLL> pts = index.queryKmFast(stateIn, cityIn, radius_km);
    cout << "Houses within " << radius_km << " km of " << cityIn << ", " << stateIn
         << ": " << pts.size() << "\n";

    return 0;
}
