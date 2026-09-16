#include "app.hpp"
int main(int argc, char **argv)
{
    return guarded([&] {
        App app(L"ЛР 02 — Просмотр изображений | File → Open / O");
        app.fileMenu();
        Pixels p = loadImage(asset(L"sample.png"));
        GLuint tex = upload(p);
        app.onKey = [&](int key) {
            if (key == 'O')
            {
                auto path = openImage(app.window);
                if (!path.empty())
                    try
                    {
                        auto next = loadImage(path);
                        GLuint t = upload(next);
                        glDeleteTextures(1, &tex);
                        tex = t;
                        p = std::move(next);
                    }
                    catch (const std::exception &e)
                    {
                        MessageBoxA(app.window, e.what(), "Image error", MB_ICONERROR);
                    }
            }
        };
        int result = app.run(argc, argv, [&] {
            app.ortho(app.width, app.height);
            for (int y = 0; y < app.height; y += 24)
                for (int x = 0; x < app.width; x += 24)
                {
                    double c = ((x / 24 + y / 24) % 2) ? .72 : .9;
                    rect(x, y, 24, 24, {c, c, c});
                }
            double s = std::min({1., double(app.width) / p.w, double(app.height) / p.h});
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            texturedQuad(tex, (app.width - p.w * s) / 2, (app.height - p.h * s) / 2, p.w * s, p.h * s);
            glDisable(GL_BLEND);
        });
        glDeleteTextures(1, &tex);
        return result;
    });
}
