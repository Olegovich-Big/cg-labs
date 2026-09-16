#include "geometry.hpp"
#include "shader.hpp"
int main(int argc, char **argv)
{
    return guarded([&] {
        App app(L"ЛР 11 — Фонг и Ламберт: шахматный конь | мышь: камера, пробел: пауза");
        app.orbit = true;
        app.distance = 4.8;
        Model knight(asset(L"knight_ivory.obj"));
        Shader shader(phongVertex, phongFragment);
        return app.run(argc, argv, [&] {
            app.camera();
            Vec3 light{2.4 * cos(app.time), 1.3, 1.5 * sin(app.time)};
            shader.bind();
            shader.triple("lightPosition", eyePosition(light));
            shader.integer("colorMap", 0);
            glPushMatrix();
            glTranslated(0, -.7, 0);
            knight.draw([&](bool t) { shader.integer("textured", t ? 1 : 0); });
            glPopMatrix();
            shader.stop();
            glDisable(GL_LIGHTING);
            glPushMatrix();
            glTranslated(light.x, light.y, light.z);
            color({1, .94, .7});
            sphere(.07);
            glPopMatrix();
        });
    });
}
