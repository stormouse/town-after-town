#pragma once

#include <map>
#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <iostream>

namespace tora::sim::fortune {

struct Point
{
    double x;
    double y;
    Point() = default;
    Point(double x, double y) : x{ x }, y{ y } {}
};

struct Circle
{
    Point origin;
    double radius;
};

struct Site
{
    int id;
    Point location;
};

struct Arc
{
    int id;
    int site;
    Point location; // site location!
    int s1;
    int s2;
    inline bool isValid() const { return site >= 0; }
};

struct Segment {
    Point a;
    Point b;
    bool finished;

    int site1 = -1;
    int site2 = -1;

    Segment() : a{}, b{}, finished{ false } {}
    Segment(Point a) : a{ a }, b{}, finished{ false } {}
    Segment(Point a, Point b) : a{ a }, b{ b }, finished{ true } {}

    void finish(Point b) {
        this->b = b;
        finished = true;
    }
};

using ArcRef = std::list<Arc>::iterator;
std::ostream& operator<<(std::ostream& os, const ArcRef& ar);

struct Event {
    int type;
    double y;
    int site;
    bool active;
    static constexpr int SITE = 0;
    static constexpr int VERTEX = 1;

    static Event Vertex(ArcRef arc, double y) {
        return Event{ .type = VERTEX, .y = y, .site = arc->site, .active = true };
    }

    static Event Site(int site, double y) {
        return Event{ .type = SITE, .y = y, .site = site, .active = true };
    }
};

struct State
{
    using EventId = int;

    std::vector<Site> sites;
    std::list<Arc> beachline;

    std::unordered_map<int, EventId> arcEvents;

    std::vector<Event> events;
    std::multimap<int, EventId> eventQueue;
    
    std::vector<Point> midPoints;
    std::vector<Point> voronoiVertices;
    std::vector<Segment> segments;

    double sweepLineY = 0;
    int nextArcId = 0;

    State(const std::vector<Site>& sites);
    bool step();
    void run();

    void addSiteEvent(const Site& site);
    bool checkVertexEvent(ArcRef arc, double sweepLineY);
    void clearVertexEvent(ArcRef arc);
    int createSegments(ArcRef a, ArcRef b, Point s);

    void save(const std::string& filename);
    static std::optional<State> load(const std::string& filename);
};


// https://ics.uci.edu/~eppstein/junkyard/circumcenter.html
inline Circle circumcircle(Point a, Point b, Point c)
{
    double d = (a.x - c.x) * (b.y - c.y) - (b.x - c.x) * (a.y - c.y);

    double x = (((a.x - c.x) * (a.x + c.x) + (a.y - c.y) * (a.y + c.y)) / 2 * (b.y - c.y)
        - ((b.x - c.x) * (b.x + c.x) + (b.y - c.y) * (b.y + c.y)) / 2 * (a.y - c.y))
        / d;

    double y = (((b.x - c.x) * (b.x + c.x) + (b.y - c.y) * (b.y + c.y)) / 2 * (a.x - c.x)
        - ((a.x - c.x) * (a.x + c.x) + (a.y - c.y) * (a.y + c.y)) / 2 * (b.x - c.x))
        / d;

    double r = sqrt((a.x - x) * (a.x - x) + (a.y - y) * (a.y - y));

    return Circle{
        .origin = Point(x, y),
        .radius = r
    };
}

inline Point lowestPoint(const Circle& c)
{
    return Point(c.origin.x, c.origin.y + c.radius);
}

inline Point breakpoint(Point p1, Point p2, double l)
{
    double x1 = p1.x, y1 = p1.y, x2 = p2.x, y2 = p2.y;
    double d1 = 1.0 / (2.0 * (y1 - l));
    double d2 = 1.0 / (2.0 * (y2 - l));
    double a = d1 - d2;
    double b = 2.0 * (x2 * d2 - x1 * d1);
    double c = (y1 * y1 + x1 * x1 - l * l) * d1 - (y2 * y2 + x2 * x2 - l * l) * d2;
    double delta = b * b - 4.0 * a * c;
    double x = (-b - sqrt(delta)) / (2.0 * a);
    double y = (x * x - 2 * p1.x * x + p1.x * p1.x + p1.y * p1.y - l * l) / (2 * p1.y - 2 * l);

    return Point(x, y);
}

inline std::vector<Point> breakpoints(std::list<Arc>& beachline, double sweepLineY)
{
    auto bps = std::vector<Point>();
    auto it = beachline.begin();
    if (it != beachline.end()) {
        for (auto next = std::next(it); it != beachline.end() && next != beachline.end(); it++, next++) {
            bps.emplace_back(breakpoint(it->location, next->location, sweepLineY));
        }
    }
    return bps;
}

inline ArcRef findArcAbove(std::list<Arc>& beachline, Point p) {
    auto it = beachline.begin();
    if (it != beachline.end()) {
        for (auto next = std::next(it); it != beachline.end() && next != beachline.end(); it++, next++) {
            if (p.x < breakpoint(it->location, next->location, p.y).x) {
                return it;
            }
        }
    }
    return it;
}

inline Point parabolaIntersect(Point siteAbove, Point newSite) {
    if (newSite.y == siteAbove.y) {
        return Point(siteAbove);
    }

    auto x = newSite.x;
    auto y = newSite.y;
    auto x0 = siteAbove.x;
    auto y0 = siteAbove.y;

    return Point(x, (y0 + y) * 0.5 - (x - x0) * (x - x0) / (2 * (y - y0)));
}

inline double distSqr(const Point& a, const Point& b) {
    return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
}








//struct SitePair
//{
//    int a;
//    int b;
//
//    SitePair() = default;
//    SitePair(int a, int b) : a{ a }, b{ b } {}
//    SitePair(const SitePair& other) : a{ other.a }, b{ other.b } {}
//
//    bool operator==(const auto& other) const noexcept {
//        return a == other.a && b == other.b;
//    }
//
//    struct hash {
//        size_t operator()(const SitePair& usp) const noexcept {
//            return (usp.a) ^ (usp.b << 1);
//        }
//    };
//};
//
//struct SiteTriplet
//{
//    int a;
//    int b;
//    int c;
//
//    SiteTriplet() = default;
//    SiteTriplet(int a, int b, int c) : a{ a }, b{ b }, c{ c } {}
//    SiteTriplet(const SiteTriplet& other) : a{ other.a }, b{ other.b }, c{ other.c } {}
//
//    bool operator==(const auto& other) const noexcept {
//        int me[]{ a, b, c };
//        int others[]{ other.a, other.b, other.c };
//
//        std::sort(me, me + 3);
//        std::sort(others, others + 3);
//        return me[0] == others[0] && me[1] == others[1] && me[2] == others[2];
//    }
//
//    struct hash {
//        size_t operator()(const SiteTriplet& usp) const noexcept {
//            int me[]{ usp.a, usp.b, usp.c };
//            std::sort(me, me + 3);
//            return (((me[0]) ^ (me[1] << 1)) >> 1) ^ (me[2] << 1);
//        }
//    };
//};


//inline int findSiteIndexAbove(std::vector<SiteArc>& beachline, Point p) {
//
//    if (beachline.size() == 0) return -1;
//
//    auto bps = breakpoints(beachline, p.y);
//
//    for (int i = 0; i < bps.size(); i++) {
//        if (p.x < bps[i].location.x) {
//            return i;
//        }
//    }
//
//    return beachline.size() - 1;
//}


} // namespace tora::sim::fortune