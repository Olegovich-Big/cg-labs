#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>
constexpr double pi = 3.14159265358979323846;
struct Vec3
{
    double x = 0, y = 0, z = 0;
    Vec3 operator+(Vec3 b) const
    {
        return {x + b.x, y + b.y, z + b.z};
    }
    Vec3 operator-(Vec3 b) const
    {
        return {x - b.x, y - b.y, z - b.z};
    }
    Vec3 operator-() const
    {
        return {-x, -y, -z};
    }
    Vec3 operator*(double s) const
    {
        return {x * s, y * s, z * s};
    }
    Vec3 operator/(double s) const
    {
        return *this * (1 / s);
    }
};
inline double dot(Vec3 a, Vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
inline Vec3 cross(Vec3 a, Vec3 b)
{
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}
inline Vec3 unit(Vec3 v)
{
    double n = std::sqrt(dot(v, v));
    return n > 1e-12 ? v / n : Vec3{};
}
inline Vec3 mul(Vec3 a, Vec3 b)
{
    return {a.x * b.x, a.y * b.y, a.z * b.z};
}
inline Vec3 bezier(const std::array<Vec3, 4> &p, double t)
{
    double s = 1 - t;
    return p[0] * (s * s * s) + p[1] * (3 * s * s * t) + p[2] * (3 * s * t * t) + p[3] * (t * t * t);
}
