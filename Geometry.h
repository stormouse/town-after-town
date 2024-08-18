#pragma once

#include <vector>

namespace tora::geometry {

struct Point
{
    double x;
    double y;
    Point() = default;
    Point(double x, double y) : x{ x }, y{ y } {}

    inline Point operator+(const Point& other) const {
        return Point{ x + other.x, y + other.y };
    }

    inline Point operator-(const Point& other) const {
        return Point{ x - other.x, y - other.y };
    }

    inline Point& operator+=(const Point& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    inline Point& operator-=(const Point& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    inline Point operator*(double scalar) const {
        return Point{ x * scalar, y * scalar };
    }

    inline Point operator/(double scalar) const {
        return Point{ x / scalar, y / scalar };
    }

    inline Point& operator*= (double scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    inline Point& operator/= (double scalar) {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    inline bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }

    inline bool operator!=(const Point& other) const {
        return !(*this == other);
    }
};

using Vector2 = Point;

inline double dot(const Vector2& a, const Vector2& b) {
    return a.x * b.x + a.y * b.y;
}

inline double cross(const Vector2& a, const Vector2& b) {
    return a.x * b.y - a.y * b.x;
}

inline double distSqr(const Point& a, const Point& b) {
    return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
}

inline bool pointEq(const Point& a, const Point& b, double distThreshold = 1e-3) {
    return distSqr(a, b) < distThreshold * distThreshold;
}

struct Circle
{
    Point origin;
    double radius;
};

struct Segment
{
    Point v1;
    Point v2;
};

struct Polygon
{
    std::vector<Point> vertices;

    bool contains(const Point& p) const;
};

// shrinks clockwise polygons, inflates counter-clockwise polygons
Polygon offset(const Polygon& polygon, double amount);

int windingDirection(const Polygon& polygon);

} // namespace tora::geometry