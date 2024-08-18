#pragma once

#include <unordered_map>
#include <vector>

#include "Geometry.h"

namespace tora::geometry {

struct Site
{
    int id;
};

struct Edge
{
    int v1;
    int v2;
};

class Voronoi
{
    std::vector<Site> sites;
    std::vector<Point> vertices;
    std::vector<Edge> edges;
    std::unordered_map<int, std::vector<Edge>> siteEdges;
};

} // namespace tora::geometry