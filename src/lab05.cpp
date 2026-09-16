#include "geometry.hpp"
class Rocket
{
    GLuint tex;

  public:
    Rocket() : tex(upload(loadImage(asset(L"rocket.png")))) {}
    ~Rocket()
    {
        glDeleteTextures(1, &tex);
    }
    void draw()
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex);
        color({1, 1, 1});
        glPushMatrix();
        glTranslated(0, -1.6, 0);
        frustum(.65, .65, 2.8);
        glTranslated(0, 2.8, 0);
        frustum(.65, 0, 1.1);
        glPopMatrix();
        glDisable(GL_TEXTURE_2D);
        for (int i = 0; i < 4; ++i)
        {
            double a = i * pi / 2;
            glPushMatrix();
            glTranslated(.45 * cos(a), -2, .45 * sin(a));
            color({.3, .34, .4});
            frustum(.28, .18, .55);
            glPopMatrix();
            glPushMatrix();
            glRotated(i * 90., 0, 1, 0);
            color({.85, .16, .1});
            normal({0, 0, 1});
            glBegin(GL_TRIANGLES);
            vertex({.6, -.5, .04});
            vertex({1.3, -1.7, .04});
            vertex({.6, -1.7, .04});
            normal({0, 0, -1});
            vertex({.6, -.5, -.04});
            vertex({.6, -1.7, -.04});
            vertex({1.3, -1.7, -.04});
            glEnd();
            glPopMatrix();
        }
    }
};
int main(int argc, char **argv)
{
    return guarded([&] {
        App app(L"ЛР 05 — Текстурированная ракета | мышь: вращение");
        app.orbit = true;
        app.distance = 10;
        Rocket rocket;
        return app.run(argc, argv, [&] {
            app.camera();
            app.light();
            rocket.draw();
        });
    });
}
