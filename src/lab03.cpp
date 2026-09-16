#include "geometry.hpp"
int main(int argc, char **argv)
{
    return guarded([&] {
        App app(L"ЛР 03 — 1: кривая Безье (перетаскивайте точки), 2: домик");
        int mode = 1, selected = -1;
        std::array<Vec3, 4> p = {Vec3{120, 540, 0}, {240, 80, 0}, {710, 100, 0}, {870, 520, 0}};
        House2D house;
        app.onKey = [&](int k) {
            if (k == '1' || k == '2')
            {
                mode = k - '0';
                selected = -1;
            }
        };
        app.onMouse = [&](int event, int x, int y) {
            Vec3 q = app.screenToWorld(x, y);
            if (event == 2)
                selected = -1;
            else if (mode == 1 && event == 0)
            {
                for (int i = 0; i < 4; ++i)
                    if (dot(q - p[i], q - p[i]) < 225)
                        selected = i;
            }
            else if (event == 1 && selected >= 0)
                p[selected] = {std::clamp(q.x, 20., 980.), std::clamp(q.y, 20., 680.), 0};
        };
        return app.run(argc, argv, [&] {
            app.ortho();
            if (mode == 2)
            {
                rect(0, 0, 1000, 700, {.53, .8, .95});
                ellipse(850, 100, 60, 60, {1, .85, .25});
                rect(0, 460, 1000, 240, {.3, .62, .3});
                house.draw(220, 120, 1.1);
                return;
            }
            color({.5, .6, .7});
            glBegin(GL_LINES);
            glVertex2d(55, 640);
            glVertex2d(955, 640);
            glVertex2d(55, 640);
            glVertex2d(55, 45);
            for (int i = 1; i <= 15; ++i)
            {
                double x = 55 + i * 55;
                glVertex2d(x, 635);
                glVertex2d(x, 645);
            }
            for (int i = 1; i <= 11; ++i)
            {
                double y = 640 - i * 50;
                glVertex2d(50, y);
                glVertex2d(60, y);
            }
            glVertex2d(955, 640);
            glVertex2d(943, 633);
            glVertex2d(955, 640);
            glVertex2d(943, 647);
            glVertex2d(55, 45);
            glVertex2d(48, 57);
            glVertex2d(55, 45);
            glVertex2d(62, 57);
            glEnd();
            glEnable(GL_LINE_STIPPLE);
            glLineStipple(2, 0xAAAA);
            color({.65, .67, .75});
            glBegin(GL_LINE_STRIP);
            for (auto v : p)
                vertex(v);
            glEnd();
            glDisable(GL_LINE_STIPPLE);
            color({.25, .86, .93});
            glLineWidth(3);
            glBegin(GL_LINE_STRIP);
            for (int i = 0; i <= 400; ++i)
                vertex(bezier(p, i / 400.));
            glEnd();
            glLineWidth(1);
            for (auto v : p)
                ellipse(v.x, v.y, 9, 9, {1, .65, .2});
        });
    });
}
