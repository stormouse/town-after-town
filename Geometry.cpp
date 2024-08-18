#include "Geometry.h"

#include <algorithm>
#include <iostream>
#include <list>
#include <unordered_map>
#include <unordered_set>

namespace tora::geometry
{

struct NamedVertex
{
    int name;
    Vector2 p;
};

bool Polygon::contains(const Point& p) const
{
    bool inside = false;
    for (int i = 0, j = vertices.size() - 1; i < vertices.size(); j = i++) {
        if ((vertices[i].y > p.y) != (vertices[j].y > p.y) &&
            p.x < (vertices[j].x - vertices[i].x) * (p.y - vertices[i].y) / (vertices[j].y - vertices[i].y) + vertices[i].x) {
            inside = !inside;
        }
    }
    return inside;
}

int windingDirection(const Polygon& polygon);
bool intersect(const Segment& s1, const Segment& s2, Point& outIntersection);

Polygon tora::geometry::offset(const Polygon& polygon, double amount)
{
    std::list<NamedVertex> vertices;

    for (int i = 0; i < polygon.vertices.size(); i++) {
        auto& vertex = polygon.vertices[i];
        auto& nextVertex = polygon.vertices[(i + 1) % polygon.vertices.size()];

        auto vector = Point{ nextVertex.x - vertex.x, nextVertex.y - vertex.y };

        // Rotate the vector 90 degrees clockwise
        auto temp = vector.x;
        vector.x = vector.y;
        vector.y = -temp;

        // Normalize the vector
        auto length = sqrt(vector.x * vector.x + vector.y * vector.y);
        vector.x /= length;
        vector.y /= length;

        // Multiply the vector by amount
        vector.x *= amount;
        vector.y *= amount;

        // Push new vertices
        vertices.push_back({ .name = static_cast<int>(vertices.size()), .p = vertex + vector });
        vertices.push_back({ .name = static_cast<int>(vertices.size()), .p = nextVertex + vector });
    }

    // create intersection points
    {
        int vid = static_cast<int>(vertices.size());
        std::cout << "vid: " << vid << "\n";

        // key: starting vertex of a segment
        // value: list of intersection points
        std::unordered_map<int, std::vector<NamedVertex>> segmentIntersections;

        // find intersections
        auto v1 = vertices.begin();
        while (v1 != vertices.end()) {
            auto v2 = std::next(v1);
            if (v2 == vertices.end()) {
                v2 = vertices.begin();
            }
            auto s1 = Segment(v1->p, v2->p);
            auto v3 = v2;
            while (v3 != vertices.end()) {
                auto v4 = std::next(v3);
                if (v4 == vertices.end()) {
                    v4 = vertices.begin();
                }

                auto s2 = Segment(v3->p, v4->p);
                Point intersection;
                if (intersect(s1, s2, intersection)) {
                    auto nv = NamedVertex{ .name = vid++, .p = intersection };
                    segmentIntersections[v1->name].push_back(nv);
                    segmentIntersections[v2->name].push_back(nv);
                }
                v3++;
            }
            v1++;
        }

        // add intersections along the path
        auto v = vertices.begin();
        while (v != vertices.end()) {
            auto next = std::next(v);
            auto it = segmentIntersections.find(v->name);
            if (it != segmentIntersections.end()) {
                auto& intersections = it->second;
                std::sort(intersections.begin(), intersections.end(), [&v](const auto& a, const auto& b) {
                    return dot(a.p - v->p, v->p) < dot(b.p - v->p, v->p);
                    });
                v = vertices.insert(next, intersections.begin(), intersections.end());
                v = next;
            }
            else {
                v++;
            }
        }
    }

    double originalWinding = windingDirection(polygon);

    Polygon result;
    // final result consists of vertices on loops with same winding numbers
    {
        std::unordered_set<int> visited;

        std::cout << "Walking vertices: ";

        auto v = vertices.begin();
        while (v != vertices.end()) {
            if (visited.find(v->name) != visited.end()) {
                std::cout << v->name << " ";
                v++;
                continue;
            }

            Polygon loop;
            auto start = v;
            do {
                loop.vertices.push_back(v->p);
                visited.insert(v->name);
                std::cout << v->name << " ";
                v++;
                if (v == vertices.end()) {
                    v = vertices.begin();
                }
            } while (v != start);

            if (windingDirection(loop) == originalWinding) {
                result.vertices.insert(result.vertices.end(), loop.vertices.begin(), loop.vertices.end());
            }
        }

        std::cout << std::endl;
    }

    return result;
}

int windingDirection(const Polygon& polygon)
{
    double sum = 0;
    for (int i = 0; i < polygon.vertices.size(); i++) {
        auto& p1 = polygon.vertices[i];
        auto& p2 = polygon.vertices[(i + 1) % polygon.vertices.size()];
        sum += (p2.x - p1.x) * (p2.y + p1.y);
    }
    return sum > 0 ? 1 : -1;
}

bool intersect(const Segment& s1, const Segment& s2, Point& outIntersection) {
    auto& p1 = s1.v1;
    auto& q1 = s1.v2;
    auto& p2 = s2.v1;
    auto& q2 = s2.v2;

    auto r = q1 - p1;
    auto s = q2 - p2;

    auto rxs = cross(r, s);
    auto qmp = p2 - p1;

    if (rxs == 0 && cross(qmp, r) == 0) {
        // collinear
        return false;
    }

    if (rxs == 0 && cross(qmp, r) != 0) {
        // parallel
        return false;
    }

    auto t = cross(qmp, s) / rxs;
    auto u = cross(qmp, r) / rxs;

    if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
        outIntersection = p1 + r * t;
        return true;
    }

    return false;
}

} // namespace tora::geometry
