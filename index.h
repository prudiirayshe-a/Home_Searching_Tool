//
// Created by onwil on 11/1/2025.
//
#ifndef INDEX_H
#define INDEX_H
#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <utility>
#include <optional>
#include <cstddef>
#include "CityKey.h"

// lat/lon
struct PointLL { double lat{}; double lon{}; };
// center
struct CenterLL { double lat0{}; double lon0{}; };
// parsed row
struct RowLite { std::string city; std::string state; double lat{}; double lon{}; };
// edge 
struct Edge { std::size_t idx; double dist_km; };

class SpatialIndex {
public:
    void loadCSV(const std::string& path, bool logProgress = false);
    std::optional<CenterLL> getCenter(const std::string& stateRaw, const std::string& cityRaw) const;
    std::vector<std::string> getStates() const;
    std::vector<std::string> getCities(const std::string& stateRaw) const;
    std::vector<PointLL> queryKm(const std::string& stateRaw, const std::string& cityRaw, double radius_km) const;
    std::vector<PointLL> queryKmFast(const std::string& stateRaw, const std::string& cityRaw, double radius_km) const;

    std::size_t totalRowsLoaded() const { return loaded_; }
    std::size_t totalRowsSkipped() const { return skipped_; }

private:
    //points
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<PointLL>>> points_;
    //center
    std::unordered_map<std::string, std::unordered_map<std::string, CenterLL>> centers_;
    // edges sorted by distance
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Edge>>> adj_by_dist_;

    std::size_t loaded_{0};
    std::size_t skipped_{0};

    static bool parseHeader(const std::string& header, int& idxCity, int& idxState, int& idxLat, int& idxLon);
    static bool parseRowLite(const std::string& line, int idxCity, int idxState, int idxLat, int idxLon, RowLite& out);

    static double haversine_km(double lat1, double lon1, double lat2, double lon2);
    static inline double deg2rad(double x) { return x * 0.01745329251994329576923690768489; }

    void computeCenters();
    void buildAdjacency();
};
#endif // INDEX_H
