#include "geometry.hpp"
int main(int argc, char **argv)
{
    return guarded([&] {
        App app(L"ЛР 04 — Кубооктаэдр | мышь: вращение, колесо: масштаб");
        app.orbit = true;
        std::vector<Vec3> vertices;
        for (int zero = 0; zero < 3; ++zero)
            for (int a : {-1, 1})
                for (int b : {-1, 1})
                {
                    Vec3 v{double(a), double(b), 0};
                    if (zero == 0)
                        v = {0, double(a), double(b)};
                    if (zero == 1)
                        v = {double(a), 0, double(b)};
                    vertices.push_back(v);
                }
        std::vector<std::vector<Vec3>> faces;
        std::vector<Vec3> normals;
        for (int axis = 0; axis < 3; ++axis)
            for (int s : {-1, 1})
            {
                Vec3 n{};
                if (axis == 0)
                    n.x = s;
                if (axis == 1)
                    n.y = s;
                if (axis == 2)
                    n.z = s;
                normals.push_back(n);
            }
        for (int a : {-1, 1})
            for (int b : {-1, 1})
                for (int c : {-1, 1})
                    normals.push_back(unit({double(a), double(b), double(c)}));
        for (auto n : normals)
        {
            std::vector<Vec3> f;
            double d = std::abs(n.x) + std::abs(n.y) + std::abs(n.z) > 1.1 ? 2 / std::sqrt(3.) : 1;
            for (auto v : vertices)
                if (std::abs(dot(n, v) - d) < 1e-6)
                    f.push_back(v);
            Vec3 center = n * d, u = unit(f[0] - center), v = cross(n, u);
            std::sort(f.begin(), f.end(), [&](Vec3 a, Vec3 b) {
                return atan2(dot(a - center, v), dot(a - center, u)) <
                       atan2(dot(b - center, v), dot(b - center, u));
            });
            faces.push_back(f);
        }
        return app.run(argc, argv, [&] {
            app.camera();
            app.light();
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(1, 1);
            for (size_t i = 0; i < faces.size(); ++i)
            {
                color({.4 + .3 * sin(i * 2.4), .5 + .3 * sin(i * 2.4 + 2), .5 + .3 * sin(i * 2.4 + 4)});
                normal(normals[i]);
                glBegin(GL_POLYGON);
                for (auto v : faces[i])
                    vertex(v);
                glEnd();
            }
            glDisable(GL_POLYGON_OFFSET_FILL);
            glDisable(GL_LIGHTING);
            color({0, 0, 0});
            glLineWidth(2);
            for (auto &f : faces)
            {
                glBegin(GL_LINE_LOOP);
                for (auto v : f)
                    vertex(v);
                glEnd();
            }
            glLineWidth(1);
        });
    });
}
