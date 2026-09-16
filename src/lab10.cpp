#include "shader.hpp"
int main(int argc, char **argv)
{
    return guarded([&] {
        App app(L"ЛР 10 — Sharpen | O: открыть, ←/→: резкость, 0: оригинал");
        app.fileMenu();
        Pixels p = loadImage(asset(L"sharpen.png"));
        GLuint texture = upload(p);
        float strength = 1;
        Shader shader(R"(#version 120
void main(){gl_Position=ftransform();gl_TexCoord[0]=gl_MultiTexCoord0;})",
                      R"(#version 120
uniform sampler2D image;uniform vec2 texel;uniform float strength;
void main(){vec2 p=gl_TexCoord[0].xy;vec4 center=texture2D(image,p);vec3 neighbors=texture2D(image,p+vec2(texel.x,0)).rgb+texture2D(image,p-vec2(texel.x,0)).rgb+texture2D(image,p+vec2(0,texel.y)).rgb+texture2D(image,p-vec2(0,texel.y)).rgb;gl_FragColor=vec4(clamp(center.rgb*(1.0+4.0*strength)-strength*neighbors,0.0,1.0),center.a);})");
        app.onKey = [&](int k) {
            if (k == VK_LEFT)
                strength = std::max(0.f, strength - .1f);
            if (k == VK_RIGHT)
                strength = std::min(5.f, strength + .1f);
            if (k == '0')
                strength = 0;
            if (k == 'O')
            {
                auto path = openImage(app.window);
                if (!path.empty())
                    try
                    {
                        auto next = loadImage(path);
                        GLuint t = upload(next);
                        glDeleteTextures(1, &texture);
                        texture = t;
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
            double s = std::min(double(app.width) / p.w, double(app.height) / p.h);
            shader.bind();
            shader.integer("image", 0);
            shader.pair("texel", 1.f / p.w, 1.f / p.h);
            shader.uniform("strength", strength);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            texturedQuad(texture, (app.width - p.w * s) / 2, (app.height - p.h * s) / 2, p.w * s, p.h * s);
            glDisable(GL_BLEND);
            shader.stop();
        });
        glDeleteTextures(1, &texture);
        return result;
    });
}
