#include "geometry.hpp"
#include "shader.hpp"
#include <random>
struct Snow
{
    Vec3 p;
    double speed, drift;
};
int main(int argc, char **argv)
{
    return guarded([&] {
        App app(L"ЛР 09 — Зимний сквер, снег | мышь: камера, пробел: пауза");
        app.orbit = true;
        app.distance = 17;
        app.pitch = 20;
        Model house(asset(L"house.obj")), tree(asset(L"tree.obj")), bench(asset(L"bench.obj")),
            snowman(asset(L"snowman.obj")), lamp(asset(L"lamp.obj"));
        Shader phong(phongVertex, phongFragment);
        std::mt19937 rng(42);
        std::uniform_real_distribution<double> u(0, 1);
        std::vector<Snow> snow;
        for (int i = 0; i < 1600; ++i)
            snow.push_back(
                {{u(rng) * 16 - 8, u(rng) * 9, u(rng) * 14 - 7}, .45 + u(rng) * .6, u(rng) * 2 * pi});
        return app.run(argc, argv, [&] {
            app.camera();
            app.light();
            phong.bind();
            phong.triple("lightPosition", eyePosition({1, 7, 4}));
            phong.integer("colorMap", 0);
            phong.integer("textured", 0);
            glDisable(GL_COLOR_MATERIAL);
            Material ground;
            ground.diffuse = {.8, .9, 1};
            material(ground);
            box({0, -.15, 0}, {16, .2, 14}, {1, 1, 1});
            auto draw = [&](Model &m, double x, double z) {
                glPushMatrix();
                glTranslated(x, 0, z);
                m.draw([&](bool t) { phong.integer("textured", t ? 1 : 0); });
                glPopMatrix();
            };
            draw(house, -2, -3);
            draw(house, 2, -3);
            draw(tree, -5, -1);
            draw(tree, 5, -2);
            draw(tree, 4, 3);
            draw(bench, -2, 1);
            draw(snowman, 1, 1);
            draw(lamp, -.5, 0);
            phong.stop();
            glDisable(GL_LIGHTING);
            GLdouble matrix[16];
            glGetDoublev(GL_MODELVIEW_MATRIX, matrix);
            Vec3 right{matrix[0], matrix[4], matrix[8]}, up{matrix[1], matrix[5], matrix[9]};
            std::vector<Vec3> positions;
            for (auto s : snow)
            {
                double y = 9 - fmod(s.p.y + app.time * s.speed, 9.);
                positions.push_back({s.p.x + .25 * sin(app.time * .7 + s.drift + y), y,
                                     s.p.z + .2 * cos(app.time * .5 + s.drift)});
            }
            std::sort(positions.begin(), positions.end(), [&](Vec3 a, Vec3 b) {
                return dot({matrix[2], matrix[6], matrix[10]}, a) <
                       dot({matrix[2], matrix[6], matrix[10]}, b);
            });
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);
            for (auto p : positions)
            {
                double r = .035;
                glBegin(GL_TRIANGLE_FAN);
                glColor4d(.95, .98, 1, .9);
                vertex(p);
                glColor4d(.95, .98, 1, 0);
                for (int i = 0; i <= 8; ++i)
                    vertex(p + right * (r * cos(i * pi / 4)) + up * (r * sin(i * pi / 4)));
                glEnd();
            }
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        });
    });
}
