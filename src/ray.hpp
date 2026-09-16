#pragma once
#include "math.hpp"
#include <limits>
namespace ray
{
struct Ray
{
    Vec3 origin, direction;
};
struct Plane
{
    Vec3 n;
    double d;
}; // Solid interior: dot(n,p) <= d.
struct Transform
{
    // Affine matrices in row-major order; inverse transpose transforms normals.
    std::array<double, 16> m{}, inverse{};
    Transform(Vec3 translation = {}, Vec3 scale = {1, 1, 1}, double angle = 0)
    {
        double c = cos(angle), s = sin(angle);
        m = {c * scale.x,  0, s * scale.z, translation.x, 0, scale.y, 0, translation.y,
             -s * scale.x, 0, c * scale.z, translation.z, 0, 0,       0, 1};
        inverse = {c / scale.x, 0, -s / scale.x, 0, 0, 1 / scale.y, 0, 0,
                   s / scale.z, 0, c / scale.z,  0, 0, 0,           0, 1};
        Vec3 t = vector(inverse, -translation);
        inverse[3] = t.x;
        inverse[7] = t.y;
        inverse[11] = t.z;
    }
    static Vec3 vector(const std::array<double, 16> &a, Vec3 p)
    {
        return {a[0] * p.x + a[1] * p.y + a[2] * p.z, a[4] * p.x + a[5] * p.y + a[6] * p.z,
                a[8] * p.x + a[9] * p.y + a[10] * p.z};
    }
    static Vec3 point(const std::array<double, 16> &a, Vec3 p)
    {
        return vector(a, p) + Vec3{a[3], a[7], a[11]};
    }
    Vec3 normal(Vec3 p) const
    {
        return unit({inverse[0] * p.x + inverse[4] * p.y + inverse[8] * p.z,
                     inverse[1] * p.x + inverse[5] * p.y + inverse[9] * p.z,
                     inverse[2] * p.x + inverse[6] * p.y + inverse[10] * p.z});
    }
};
struct Material
{
    Vec3 ambient{.12, .12, .12}, diffuse{.7, .7, .7}, specular{.8, .8, .8};
    double shine = 60;
};
struct Object
{
    std::vector<Plane> planes;
    Transform transform;
    Material material;
};
struct Hit
{
    double t = std::numeric_limits<double>::infinity();
    Vec3 point, normal;
    int object = -1;
};
inline std::vector<Plane> cube()
{
    return {{{1, 0, 0}, 1},  {{-1, 0, 0}, 1}, {{0, 1, 0}, 1},
            {{0, -1, 0}, 1}, {{0, 0, 1}, 1},  {{0, 0, -1}, 1}};
}
inline std::vector<Plane> octahedron()
{
    std::vector<Plane> p;
    for (int x : {-1, 1})
        for (int y : {-1, 1})
            for (int z : {-1, 1})
                p.push_back({{double(x), double(y), double(z)}, 1});
    return p;
}
inline std::vector<Plane> tetrahedron(const std::array<Vec3, 4> &v)
{
    std::vector<Plane> planes;
    for (int i = 0; i < 4; ++i)
    {
        Vec3 a = v[(i + 1) % 4], b = v[(i + 2) % 4], c = v[(i + 3) % 4], n = unit(cross(b - a, c - a));
        double d = dot(n, a);
        if (dot(n, v[i]) > d)
        {
            n = -n;
            d = -d;
        }
        planes.push_back({n, d});
    }
    return planes;
}
inline bool intersect(const Object &object, Ray world, Hit &hit)
{
    Ray local{Transform::point(object.transform.inverse, world.origin),
              Transform::vector(object.transform.inverse, world.direction)};
    double enter = -std::numeric_limits<double>::infinity(), leave = std::numeric_limits<double>::infinity();
    Vec3 en, ex;
    for (auto plane : object.planes)
    {
        double denominator = dot(plane.n, local.direction), numerator = plane.d - dot(plane.n, local.origin);
        if (std::abs(denominator) < 1e-12)
        {
            if (numerator < 0)
                return false;
            continue;
        }
        double t = numerator / denominator;
        if (denominator < 0)
        {
            if (t > enter)
            {
                enter = t;
                en = plane.n;
            }
        }
        else if (t < leave)
        {
            leave = t;
            ex = plane.n;
        }
        if (enter > leave)
            return false;
    }
    double t = enter > 1e-5 ? enter : leave;
    if (t <= 1e-5 || t >= hit.t || !std::isfinite(t))
        return false;
    hit.t = t;
    hit.point = world.origin + world.direction * t;
    hit.normal = object.transform.normal(enter > 1e-5 ? en : ex);
    if (dot(hit.normal, world.direction) > 0)
        hit.normal = -hit.normal;
    return true;
}
struct Light
{
    Vec3 position{-3, 6, 4}, ambient{.5, .5, .5}, diffuse{1, 1, .95}, specular{1, 1, 1};
    double radius = .7;
};
class Scene
{
  public:
    std::vector<Object> objects;
    Light light;
    bool soft = true;
    Hit nearest(Ray r) const
    {
        Hit h;
        for (size_t i = 0; i < objects.size(); ++i)
            if (intersect(objects[i], r, h))
                h.object = int(i);
        return h;
    }
    Vec3 trace(Ray r) const
    {
        Hit h = nearest(r);
        if (h.object < 0)
        {
            double s = .5 * (unit(r.direction).y + 1);
            return Vec3{.07, .10, .17} * (1 - s) + Vec3{.25, .36, .5} * s;
        }
        auto m = objects[h.object].material;
        if (h.object == 0)
        {
            int tile = int(floor(h.point.x)) + int(floor(h.point.z));
            m.diffuse = (tile % 2) ? Vec3{.22, .28, .35} : Vec3{.62, .68, .73};
        }
        Vec3 result = mul(m.ambient, light.ambient);
        int count = soft ? 16 : 1;
        for (int i = 0; i < count; ++i)
        {
            Vec3 offset{};
            if (soft)
            {
                double a = i * 2.3999632297, radius = light.radius * sqrt((i + .5) / count);
                offset = {radius * cos(a), 0, radius * sin(a)};
            }
            Vec3 delta = light.position + offset - h.point;
            double distance = std::sqrt(dot(delta, delta));
            Vec3 l = delta / distance;
            Hit shadow = nearest({h.point + h.normal * 1e-4, l});
            if (shadow.t < distance - 1e-4)
                continue;
            double lambert = std::max(0., dot(h.normal, l));
            Vec3 reflected = h.normal * (2 * dot(h.normal, l)) - l;
            double spec = lambert > 0 ? pow(std::max(0., dot(unit(-r.direction), reflected)), m.shine) : 0;
            result =
                result +
                (mul(m.diffuse, light.diffuse) * lambert + mul(m.specular, light.specular) * spec) / count;
        }
        return result;
    }
};
inline Scene demo()
{
    Scene s;
    Object floor;
    floor.planes = cube();
    floor.transform = Transform({0, -1.5, 0}, {7, .1, 7});
    s.objects.push_back(floor);
    Object a;
    a.planes = cube();
    a.transform = Transform({-2, -.35, 0}, {.8, 1, .7}, .4);
    a.material.diffuse = {.12, .57, .85};
    s.objects.push_back(a);
    Object b;
    b.planes = tetrahedron({Vec3{0, 1.4, 0}, Vec3{-1, -1, 1}, Vec3{1, -1, 1}, Vec3{0, -1, -1.2}});
    b.transform = Transform({0, -.3, .7}, {.9, 1, .9}, -.3);
    b.material.diffuse = {.92, .42, .10};
    s.objects.push_back(b);
    Object c;
    c.planes = octahedron();
    c.transform = Transform({2, 0, 0}, {1.1, 1.4, 1.1}, .25);
    c.material.diffuse = {.5, .22, .78};
    c.material.shine = 100;
    s.objects.push_back(c);
    return s;
}
} // namespace ray
