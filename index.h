//
// Created by onwil on 11/1/2025.
//
#ifndef INDEX_H
#define INDEX_H

#include <string>
#include <unordered_map>
#include <vector>
#include <utility>
#include <cstddef>
#include "CityKey.h"

using namespace std;

// lat/lon
struct PointLL {
    double lat{};
    double lon{};
    int price{0};
};

// center
struct CenterLL {
    double lat0{};
    double lon0{};
};

// parsed row
struct RowLite {
    string city;
    string state;
    double lat{};
    double lon{};
    int price{0};
};

// edge
struct Edge {
    size_t idx;
    double dist_km;
};

class SpatialIndex {
public:
    void loadCSV(string path, bool logProgress = false);


    bool getCenter(string stateRaw, string cityRaw, CenterLL& outCenter);

    vector<string> getStates();
    vector<string> getCities(string stateRaw);

    vector<PointLL> queryKm(string stateRaw, string cityRaw, double radius_km);
    vector<PointLL> queryKmFast(string stateRaw, string cityRaw, double radius_km);

    size_t totalRowsLoaded() { 
        return loaded_; 
    }
    size_t totalRowsSkipped() { 
        return skipped_; 
    }

private:
    // points
    unordered_map<string, unordered_map<string, vector<PointLL>>> points_;
    // center
    unordered_map<string, unordered_map<string, CenterLL>> centers_;
    // edges sorted by distance
    unordered_map<string, unordered_map<string, vector<Edge>>> adj_by_dist_;

    size_t loaded_{0};
    size_t skipped_{0};

    static bool parseHeader(string header, int& idxCity, int& idxState, int& idxLat, int& idxLon, int& idxPrice);
    static bool parseRowLite(string line, int idxCity, int idxState, int idxLat, int idxLon, int idxPrice RowLite& out);

    static double haversine_km(double lat1, double lon1, double lat2, double lon2);
    static double deg2rad(double x) { 
        return x * 0.01745; 
    }

    void computeCenters();
    void buildAdjacency();
};

#endif // INDEX_H

