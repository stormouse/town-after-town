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
        // std::cout << "vid: " << vid << "\n";

        // key: starting vertex of a segment
        // value: list of intersection points
        std::unordered_map<int, std::vector<NamedVertex>> segmentIntersections;

        // find intersections
        auto v1 = vertices.begin();
        while (v1 != vertices.end()) {
            auto v2 = std::next(v1);
            if (v2 == vertices.end())
                v2 = vertices.begin();

            Segment s1{ .v1 = v1->p, .v2 = v2->p };

            auto v3 = std::next(v2);

            while (v3 != vertices.end()) {
                auto v4 = std::next(v3);

                if (v4 == vertices.end())
                    v4 = vertices.begin();
                
                if (v4 == v1) break;

                Segment s2{ .v1 = v3->p, .v2 = v4->p };

                Point intersection;
                if (intersect(s1, s2, intersection)) {
                    auto nv = NamedVertex{ .name = vid++, .p = intersection };
                    segmentIntersections[v1->name].push_back(nv);
                    segmentIntersections[v3->name].push_back(nv);
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
                auto& np = next == vertices.end() ? vertices.begin()->p : next->p;
                std::sort(intersections.begin(), intersections.end(), [&](const auto& a, const auto& b) {
                    return dot(a.p - v->p, np - v->p) < dot(b.p - v->p, np - v->p);
                    });

                // std::cout << "adding ";
                for (const auto& intc : intersections) std::cout << "(" << intc.p.x << ", " << intc.p.y << ") ";
                // std::cout << "between " << "(" << v->p.x << ", " << v->p.y << ") and (" << next->p.x << ", " << next->p.y << ")\n";
                vertices.insert(next, intersections.begin(), intersections.end());
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

        // vertices.insert(vertices.end(), *vertices.begin()); // close the path as a loop

        int steps = vertices.size() * 2;
        auto nodeMap = std::unordered_map<int, std::list<NamedVertex>::iterator>();
        auto current = vertices.begin();

        // std::cout << "Walking vertices:\n";
        for (int step = 0; step < steps; step++) {
           //  std::cout << "    " << current->name << "(" << current->p.x << ", " << current->p.y << ")\n";
            if (nodeMap.contains(current->name)) {
                // Loop detected: start node is nodeMap[current->name] and end node is current
                // std::cout << "    loop!\n";
                Polygon loop;
                for (auto it = nodeMap[current->name]; it != current; it = std::next(it) == vertices.end() ? vertices.begin() : std::next(it)) {
                    // std::cout << "        " << it->name << "(" << it->p.x << ", " << it->p.y << ")\n";
                    loop.vertices.push_back(it->p);
                }
                if (windingDirection(loop) != originalWinding) {
                    // std::cout << "        " << "dropped!\n";
                    for (int i = 0; i < loop.vertices.size(); i++) {
                        auto t = current;
                        if (current == vertices.begin()) {
                            current = std::prev(vertices.end());
                        }
                        else {
                            current--;
                        }
                        vertices.erase(t);
                    }
                } else {
                    // std::cout << "        " << "kept!\n";
                }
                nodeMap.clear();
            }
            else {
                // Mark this node as visited
                nodeMap[current->name] = current;
            }
            current = std::next(current);
            if (current == vertices.end()) current = vertices.begin();
        }
        // std::cout << std::endl;

        auto& path = result.vertices;
        for (auto& vertex : vertices) {
            path.push_back(vertex.p);
        }
    }

    return result;
}

int windingDirection(const Polygon& polygon)
{
    double sum = 0;
    for (int i = 0; i < polygon.vertices.size(); i++) {
        auto& p1 = polygon.vertices[i];
        auto& p2 = polygon.vertices[(i + 1) % polygon.vertices.size()];
        sum += (p1.x * p2.y - p2.x * p1.y);
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
