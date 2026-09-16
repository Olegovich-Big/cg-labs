#include "geometry.hpp"
int main(int argc, char **argv)
{
    return guarded([&] {
        App app(L"ЛР 01 — Домик в деревне");
        House2D house;
        return app.run(argc, argv, [&] {
            app.ortho();
            rect(0, 0, 1000, 700, {.50, .79, .94});
            ellipse(810, 130, 65, 65, {1, .85, .24});
            ellipse(190, 130, 95, 26, {.95, .98, 1});
            ellipse(260, 110, 70, 35, {.95, .98, 1});
            rect(0, 460, 1000, 240, {.35, .60, .28});
            house.draw(220, 120, 1.15);
        });
    });
}
