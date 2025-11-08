//
// Created by onwil on 11/1/2025.
//
#include "index.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>

using namespace std;

//Hello this is a big peice of code so heres whats happening

// splits our csv
static vector<string> split_csv_simple(string line) {
    vector<string> out;
    string cur;
    bool inQuotes = false;

    for (size_t i = 0; i < line.size(); ++i) {
        char ch = line[i];
        if (ch == '"') {
            if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                cur.push_back('"');
                ++i;
            } else {
                inQuotes = !inQuotes;
            }
        } else if (ch == ',' && !inQuotes) {
            out.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(ch);
        }
    }
    out.push_back(cur);
    return out;
}

static inline void strip_outer_quotes(string& s) {
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
        s = s.substr(1, s.size() - 2);
    }
}
//lowercases our string
static inline void to_lower_inplace(string& s) {
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return (char)tolower(c); });
}

//where we parse stuff
// Find indices for city/state/lat/lon in header
bool SpatialIndex::parseHeader(string header, int& idxCity, int& idxState, int& idxLat, int& idxLon) {
    idxCity = idxState = idxLat = idxLon = -1;

    vector<string> columns = split_csv_simple(header);
    for (int i = 0; i < (int)columns.size(); ++i) {
        string name = CityKey::trim(columns[i]);
        to_lower_inplace(name);
        //this stuff is for names. More of a useful thing for anyone who wants this code and if the csv is changed
        if (name == "city" || name == "city_name") {
            idxCity = i;
        } else if (name == "state" || name == "state_name" || name == "state_code" || name == "state_abbr") {
            idxState = i;
        } else if (name == "lat" || name == "latitude" || name == "lat_deg") {
            idxLat = i;
        } else if (name == "lon" || name == "lng" || name == "longitude" || name == "long" || name == "lon_deg") {
            idxLon = i;
        }
    }
    return (idxCity >= 0 && idxState >= 0 && idxLat >= 0 && idxLon >= 0);
}

// Parse one data row into RowLite
bool SpatialIndex::parseRowLite(string line, int idxCity, int idxState, int idxLat, int idxLon, RowLite& out) {
    vector<string> columns = split_csv_simple(line);
    int maxNeeded = max(max(idxCity, idxState), max(idxLat, idxLon));
    if ((int)columns.size() <= maxNeeded) {
        return false;
    }

    string cityField  = CityKey::trim(columns[idxCity]);
    string stateField = CityKey::trim(columns[idxState]);
    strip_outer_quotes(cityField);
    strip_outer_quotes(stateField);
    if (cityField.empty() || stateField.empty()) {
        return false;
    }

    string latField = CityKey::trim(columns[idxLat]);
    string lonField = CityKey::trim(columns[idxLon]);
    strip_outer_quotes(latField);
    strip_outer_quotes(lonField);

    char* endp = 0;
    out.lat = strtod(latField.c_str(), &endp);
    if (!(endp && *endp == '\0')) {
        return false;
    }
    endp = 0;
    out.lon = strtod(lonField.c_str(), &endp);
    if (!(endp && *endp == '\0')) {
        return false;
    }

    if (out.lat < -90.0 || out.lat > 90.0 || out.lon < -180.0 || out.lon > 180.0) {
        return false;
    }

    CityKey norm = CityKey::fromRaw(cityField, stateField);
    out.city  = std::move(norm.city);
    out.state = std::move(norm.state);
    return true;
}

//building our index
// Load CSV and build points, centers, and distance lists
void SpatialIndex::loadCSV(string path, bool logProgress) {
    loaded_ = 0;
    skipped_ = 0;
    points_.clear();
    centers_.clear();
    adj_by_dist_.clear();

    if (logProgress) {
        cout << "Opening CSV: " << path << "\n";
    }

    ifstream in(path);
    if (!in) {
        if (logProgress) {
            cout << "Failed to open CSV\n";
        }
        return;
    }

    string header;
    if (!getline(in, header)) {
        if (logProgress) {
            cout << "Empty CSV or failed header read\n";
        }
        return;
    }

    int idxCity = -1, idxState = -1, idxLat = -1, idxLon = -1;
    if (!parseHeader(header, idxCity, idxState, idxLat, idxLon)) {
        if (logProgress) {
            cout << "Header missing city/state/lat/lon\n";
        }
        return;
    }

    string row;
    while (getline(in, row)) {
        if (row.empty()) {
            continue;
        }
        RowLite parsed;
        if (!parseRowLite(row, idxCity, idxState, idxLat, idxLon, parsed)) {
            skipped_ += 1;
            continue;
        }
        points_[parsed.state][parsed.city].push_back(PointLL{parsed.lat, parsed.lon});
        loaded_ += 1;
    }

    computeCenters();
    buildAdjacency();

    if (logProgress) {
        cout << "Loaded rows: " << loaded_ << ", skipped: " << skipped_ << "\n";
    }
}

// Compute mean center per city
void SpatialIndex::computeCenters() {
    centers_.clear();
    for (auto itState = points_.begin(); itState != points_.end(); ++itState) {
        string state = itState->first;
        auto& cityMap = itState->second;

        for (auto itCity = cityMap.begin(); itCity != cityMap.end(); ++itCity) {
            string city = itCity->first;
            vector<PointLL>& pts = itCity->second;

            double sumLat = 0.0;
            double sumLon = 0.0;
            for (size_t i = 0; i < pts.size(); ++i) {
                sumLat += pts[i].lat;
                sumLon += pts[i].lon;
            }
            double n = pts.empty() ? 1.0 : (double)pts.size();
            centers_[state][city] = CenterLL{ sumLat / n, sumLon / n };
        }
    }
}

// Precompute and sort distances from each city center
void SpatialIndex::buildAdjacency() {
    adj_by_dist_.clear();

    for (auto itState = points_.begin(); itState != points_.end(); ++itState) {
        string state = itState->first;
        auto& cityMap = itState->second;

        for (auto itCity = cityMap.begin(); itCity != cityMap.end(); ++itCity) {
            string city = itCity->first;
            vector<PointLL>& pts = itCity->second;

            auto itCenterState = centers_.find(state);
            if (itCenterState == centers_.end()) {
                continue;
            }
            auto itCenterCity = itCenterState->second.find(city);
            if (itCenterCity == itCenterState->second.end()) {
                continue;
            }
            CenterLL center = itCenterCity->second;

            vector<Edge>& edges = adj_by_dist_[state][city];
            edges.clear();
            edges.reserve(pts.size());

            for (size_t i = 0; i < pts.size(); ++i) {
                double dkm = haversine_km(center.lat0, center.lon0, pts[i].lat, pts[i].lon);
                edges.push_back(Edge{ i, dkm });
            }

            sort(edges.begin(), edges.end(),
                 [](Edge a, Edge b){ return a.dist_km < b.dist_km; });
        }
    }
}

//box queury so like the surronding stuff
// Lookup center, returns true and fills outCenter if found
bool SpatialIndex::getCenter(string stateRaw, string cityRaw, CenterLL& outCenter) {
    CityKey key = CityKey::fromRaw(cityRaw, stateRaw);

    auto itS = centers_.find(key.state);
    if (itS == centers_.end()) {
        return false;
    }
    auto itC = itS->second.find(key.city);
    if (itC == itS->second.end()) {
        return false;
    }
    outCenter = itC->second;
    return true;
}

// Sorted states list
vector<string> SpatialIndex::getStates() {
    vector<string> states;
    states.reserve(points_.size());
    for (auto it = points_.begin(); it != points_.end(); ++it) {
        states.push_back(it->first);
    }
    sort(states.begin(), states.end());
    return states;
}

// Sorted cities list for a normalized state
vector<string> SpatialIndex::getCities(string stateRaw) {
    string st = CityKey::to_upper(CityKey::trim(stateRaw));
    vector<string> cities;

    auto it = points_.find(st);
    if (it == points_.end()) {
        return cities;
    }

    auto& cityMap = it->second;
    cities.reserve(cityMap.size());
    for (auto itC = cityMap.begin(); itC != cityMap.end(); ++itC) {
        cities.push_back(itC->first);
    }
    sort(cities.begin(), cities.end());
    return cities;
}

// Great-circle distance (km). (True circle not a box.) the box is below so its like a circle in a box
double SpatialIndex::haversine_km(double lat1, double lon1, double lat2, double lon2) {
    double R = 6371.0;
    double p1 = deg2rad(lat1);
    double p2 = deg2rad(lat2);
    double dphi = deg2rad(lat2 - lat1);
    double dlambda = deg2rad(lon2 - lon1);

    // Correct formula: a = sin^2(dphi/2) + cos(p1)cos(p2)sin^2(dlambda/2)
    double s1 = sin(dphi * 0.5);
    double s2 = sin(dlambda * 0.5);
    double a = (s1 * s1) + (cos(p1) * cos(p2) * s2 * s2);
    double c = 2.0 * asin(sqrt(a));
    return R * c;
}

// Exact circle with a bounding-box prefilter
vector<PointLL> SpatialIndex::queryKm(string stateRaw, string cityRaw, double radius_km) {
    vector<PointLL> results;
    if (radius_km < 0.0) {
        return results;
    }

    CityKey key = CityKey::fromRaw(cityRaw, stateRaw);

    auto itS = points_.find(key.state);
    if (itS == points_.end()) {
        return results;
    }
    auto itC = itS->second.find(key.city);
    if (itC == itS->second.end()) {
        return results;
    }
    vector<PointLL>& pts = itC->second;

    auto itSC = centers_.find(key.state);
    if (itSC == centers_.end()) {
        return results;
    }
    auto itCC = itSC->second.find(key.city);
    if (itCC == itSC->second.end()) {
        return results;
    }
    CenterLL center = itCC->second;

    // Rough box first, then exact circle
    double km_per_lat = 111.32;
    double km_per_lon = km_per_lat * cos(deg2rad(center.lat0));
    double dLat = radius_km / km_per_lat;
    double dLon = (km_per_lon > 1e-9) ? (radius_km / km_per_lon) : 180.0;

    for (size_t i = 0; i < pts.size(); ++i) {
        if (fabs(pts[i].lat - center.lat0) > dLat) {
            continue;
        }
        if (fabs(pts[i].lon - center.lon0) > dLon) {
            continue;
        }
        double d = haversine_km(center.lat0, center.lon0, pts[i].lat, pts[i].lon);
        if (d <= radius_km) {
            results.push_back(pts[i]);
        }
    }
    return results;
}

// Fast query using precomputed sorted edge distances from center
vector<PointLL> SpatialIndex::queryKmFast(string stateRaw, string cityRaw, double radius_km) {
    vector<PointLL> results;
    if (radius_km < 0.0) {
        return results;
    }

    CityKey key = CityKey::fromRaw(cityRaw, stateRaw);

    auto itSPoints = points_.find(key.state);
    if (itSPoints == points_.end()) {
        return results;
    }
    auto itCPoints = itSPoints->second.find(key.city);
    if (itCPoints == itSPoints->second.end()) {
        return results;
    }
    vector<PointLL>& pts = itCPoints->second;

    auto itSAdj = adj_by_dist_.find(key.state);
    if (itSAdj == adj_by_dist_.end()) {
        return results;
    }
    auto itCAdj = itSAdj->second.find(key.city);
    if (itCAdj == itSAdj->second.end()) {
        return results;
    }
    vector<Edge>& edges = itCAdj->second;

    // first index with dist_km > radius_km
    auto cutoff = upper_bound(
    edges.begin(), edges.end(), radius_km,
    [](double r, Edge e){ return r < e.dist_km; }
);

    size_t count = (size_t)distance(edges.begin(), cutoff);
    results.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        size_t idx = edges[i].idx;
        if (idx < pts.size()) {
            results.push_back(pts[idx]);
        }
    }
    return results;
}


