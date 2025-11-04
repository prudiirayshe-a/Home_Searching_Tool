//
// Created by onwil on 11/1/2025.
//
#include "index.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>

// split CSV 
static std::vector<std::string> split_csv_simple(const std::string& line) {
    std::vector<std::string> out;
    out.reserve(12);
    std::string cur;
    cur.reserve(line.size());
    for (char c : line) {
        if (c == ',') { out.push_back(cur); cur.clear(); }
        else cur.push_back(c);
    }
    out.push_back(cur);
    return out;
}

// locate city state lat lon indices
bool SpatialIndex::parseHeader(const std::string& header, int& idxCity, int& idxState, int& idxLat, int& idxLon) {
    idxCity = idxState = idxLat = idxLon = -1;
    auto cols = split_csv_simple(header);
    for (int i = 0; i < static_cast<int>(cols.size()); ++i) {
        std::string name = CityKey::trim(cols[i]);
        std::transform(name.begin(), name.end(), name.begin(),  [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        if (name == "city") idxCity = i;
        else if (name == "state") idxState = i;
        else if (name == "lat" || name == "latitude") idxLat = i;
        else if (name == "lon" || name == "lng" || name == "longitude") idxLon = i;
    }
    return (idxCity >= 0 && idxState >= 0 && idxLat >= 0 && idxLon >= 0);
}

// parse one data row
bool SpatialIndex::parseRowLite(const std::string& line, int idxCity, int idxState, int idxLat, int idxLon, RowLite& out) {
    auto cols = split_csv_simple(line);
    int need = std::max(std::max(idxCity, idxState), std::max(idxLat, idxLon));
    if (static_cast<int>(cols.size()) <= need) return false;
    out.city  = CityKey::trim(cols[idxCity]);
    out.state = CityKey::trim(cols[idxState]);
    if (out.city.empty() || out.state.empty()) return false;
    char* endp = nullptr;
    out.lat = std::strtod(cols[idxLat].c_str(), &endp);
    if (!(endp && *endp=='\0')) return false;
    endp = nullptr;
    out.lon = std::strtod(cols[idxLon].c_str(), &endp);
    if (!(endp && *endp=='\0')) return false;
    if (out.lat < -90.0 || out.lat > 90.0 || out.lon < -180.0 || out.lon > 180.0) return false;
    auto norm = CityKey::fromRaw(out.city, out.state);
    out.city = std::move(norm.city);
    out.state = std::move(norm.state);
    return true;
}

// load CSV and build indices
void SpatialIndex::loadCSV(const std::string& path, bool logProgress) {
    loaded_ = skipped_ = 0;
    points_.clear();
    centers_.clear();
    adj_by_dist_.clear();
    std::error_code ec;
    auto abs = std::filesystem::absolute(path, ec);
    if (logProgress) {
        std::cout << "Opening CSV: " << (ec ? path : abs.string()) << "\n";
    }
    std::ifstream fin(path);
    if (!fin) throw std::runtime_error("Cannot open CSV: " + (ec ? path : abs.string()));
    std::string header;
    if (!std::getline(fin, header)) {
        throw std::runtime_error("Empty CSV or failed to read header.");
    }
    int idxCity=-1, idxState=-1, idxLat=-1, idxLon=-1;
    if (!parseHeader(header, idxCity, idxState, idxLat, idxLon)) {
        throw std::runtime_error("Header missing required columns (need city,state,lat,lon). Header was: " + header);
    }
    std::string line;
    while (std::getline(fin, line)) {
        if (line.empty()) continue;
        RowLite r{};
        if (!parseRowLite(line, idxCity, idxState, idxLat, idxLon, r)) {
            ++skipped_;
            continue;
        }
        points_[r.state][r.city].push_back(PointLL{r.lat, r.lon});
        ++loaded_;
    }

    computeCenters();
    buildAdjacency();
    if (logProgress) {
        std::cout << "Loaded rows: " << loaded_
                  << ", skipped: " << skipped_ << "\n"; 
    }
}

// compute mean center per city
void SpatialIndex::computeCenters() {
    centers_.clear();
    for (auto& kvState : points_) {
        const std::string& st = kvState.first;
        for (auto& kvCity : kvState.second) {
            const std::string& ci = kvCity.first;
            const auto& vec = kvCity.second;
            double sumLat = 0.0, sumLon = 0.0;
            for (const auto& p : vec) { sumLat += p.lat; sumLon += p.lon; }
            double n = vec.empty() ? 1.0 : static_cast<double>(vec.size());
            centers_[st][ci] = CenterLL{ sumLat / n, sumLon / n };
        }
    }
}

// precompute distances from center
void SpatialIndex::buildAdjacency() {
    adj_by_dist_.clear();
    for (auto& kvS : points_) {
        const std::string& st = kvS.first;
        for (auto& kvC : kvS.second) {
            const std::string& ci = kvC.first;
            const auto& vec = kvC.second;
            auto itS = centers_.find(st);
            if (itS == centers_.end()) continue;
            auto itC = itS->second.find(ci);
            if (itC == itS->second.end()) continue;
            CenterLL ctr = itC->second;

            auto& adj = adj_by_dist_[st][ci];
            adj.reserve(vec.size());
            for (std::size_t i = 0; i < vec.size(); ++i) {
                const auto& p = vec[i];
                double d = haversine_km(ctr.lat0, ctr.lon0, p.lat, p.lon);
                adj.push_back(Edge{ i, d });
            }
            std::sort(adj.begin(), adj.end(), [](const Edge& a, const Edge& b){ return a.dist_km < b.dist_km; });
        }
    }
}

// get center
std::optional<CenterLL> SpatialIndex::getCenter(const std::string& stateRaw, const std::string& cityRaw) const {
    auto key = CityKey::fromRaw(cityRaw, stateRaw);
    auto itS = centers_.find(key.state);
    if (itS == centers_.end()) return std::nullopt;
    auto itC = itS->second.find(key.city);
    if (itC == itS->second.end()) return std::nullopt;
    return itC->second;
}

// sorted states
std::vector<std::string> SpatialIndex::getStates() const {
    std::vector<std::string> states;
    states.reserve(points_.size());
    for (const auto& kv : points_) states.push_back(kv.first);
    std::sort(states.begin(), states.end());
    return states;
}

// sorted cities for a state
std::vector<std::string> SpatialIndex::getCities(const std::string& stateRaw) const {
    auto st = CityKey::to_upper(CityKey::trim(stateRaw));
    std::vector<std::string> cities;
    auto it = points_.find(st);
    if (it == points_.end()) return cities;
    cities.reserve(it->second.size());
    for (const auto& kv : it->second) cities.push_back(kv.first);
    std::sort(cities.begin(), cities.end());
    return cities;
}

// great-circle distance. I needed this cause just a box kept missing houses
double SpatialIndex::haversine_km(double lat1, double lon1, double lat2, double lon2) {
    constexpr double R = 6371.0;
    double p1 = deg2rad(lat1), p2 = deg2rad(lat2);
    double dphi = deg2rad(lat2 - lat1);
    double dlambda = deg2rad(lon2 - lon1);
    double a = std::sin(dphi/2)*std::sin(dphi/2) +
               std::cos(p1)*std::cos(p2)*std::sin(dlambda/2)*std::sin(dlambda/2);
    double c = 2 * std::asin(std::sqrt(a));
    return R * c;
}

// exact query with bounding box 
std::vector<PointLL> SpatialIndex::queryKm(const std::string& stateRaw, const std::string& cityRaw, double radius_km) const {
    std::vector<PointLL> out;
    if (radius_km < 0.0) return out;
    auto key = CityKey::fromRaw(cityRaw, stateRaw);
    auto itS = points_.find(key.state);
    if (itS == points_.end()) return out;
    auto itC = itS->second.find(key.city);
    if (itC == itS->second.end()) return out;
    const auto& vec = itC->second;
    auto itSC = centers_.find(key.state);
    if (itSC == centers_.end()) return out;
    auto itCC = itSC->second.find(key.city);
    if (itCC == itSC->second.end()) return out;
    CenterLL ctr = itCC->second;
    double lat_km = 111.32;
    double lon_km = lat_km * std::cos(deg2rad(ctr.lat0));
    double dLat = radius_km / lat_km;
    double dLon = (lon_km > 1e-9) ? (radius_km / lon_km) : 180.0;
    for (const auto& p : vec) {
        if (std::abs(p.lat - ctr.lat0) > dLat) continue;
        if (std::abs(p.lon - ctr.lon0) > dLon) continue;
        double d = haversine_km(ctr.lat0, ctr.lon0, p.lat, p.lon);
        if (d <= radius_km) out.push_back(p);
    }
    return out;
}

//query from distances calculated 
std::vector<PointLL> SpatialIndex::queryKmFast(const std::string& stateRaw, const std::string& cityRaw, double radius_km) const {
    std::vector<PointLL> out;
    if (radius_km < 0.0) return out;
    auto key = CityKey::fromRaw(cityRaw, stateRaw);
    auto itSPoints = points_.find(key.state);
    if (itSPoints == points_.end()) return out;
    auto itCPoints = itSPoints->second.find(key.city);
    if (itCPoints == itSPoints->second.end()) return out;
    const auto& pts = itCPoints->second;
    auto itSAdj = adj_by_dist_.find(key.state);
    if (itSAdj == adj_by_dist_.end()) return out;
    auto itCAdj = itSAdj->second.find(key.city);
    if (itCAdj == itSAdj->second.end()) return out;
    const auto& adj = itCAdj->second;
    auto it = std::upper_bound(adj.begin(), adj.end(), radius_km, [](double r, const Edge& e){ return r < e.dist_km; });
    std::size_t k = static_cast<std::size_t>(std::distance(adj.begin(), it));
    out.reserve(k);
    for (std::size_t i = 0; i < k; ++i) {
        out.push_back(pts[adj[i].idx]);
    }
    return out;
}
