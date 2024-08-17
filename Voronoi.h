#pragma once

#include <unordered_map>
#include <vector>

namespace tora::sim::voronoi {

struct Site
{
    int id;
};

struct Vertex
{
    double x;
    double y;
};

struct Edge
{
    int v1;
    int v2;
};

class Voronoi
{
    std::vector<Site> sites;
    std::vector<Vertex> vertices;
    std::vector<Edge> edges;
    std::unordered_map<int, std::vector<Edge>> siteEdges;
};


} // namespace tora::sim::voronoi