#include "shader.hpp"
int main(int argc, char **argv)
{
    return guarded([&] {
        App app(L"ЛР 07 — 1: sinc, 2: флаг, 3: сфера ↔ тор | пробел: пауза");
        int mode = 3;
        app.orbit = true;
        Shader sinc(R"(#version 120
void main(){vec4 p=gl_Vertex;p.y=abs(p.x)<0.00001?1.0:sin(p.x)/p.x;gl_Position=gl_ModelViewProjectionMatrix*p;})",
                    R"(#version 120
void main(){gl_FragColor=vec4(0.2,0.85,1.0,1.0);})");
        Shader flag(R"(#version 120
varying vec2 p;void main(){p=gl_Vertex.xy;gl_Position=ftransform();})",
                    R"(#version 120
varying vec2 p;void main(){vec3 c=p.y<250.0?vec3(1.0):p.y<450.0?vec3(0.0,0.22,0.65):vec3(0.84,0.08,0.15);gl_FragColor=vec4(c,1.0);})");
        Shader morph(R"(#version 120
uniform float phase;
void main(){float u=gl_Vertex.x;float v=gl_Vertex.y;
vec3 sphere=vec3(sin(v*0.5)*cos(u),cos(v*0.5),sin(v*0.5)*sin(u))*1.8;
vec3 torus=vec3((1.3+0.55*cos(v))*cos(u),0.55*sin(v),(1.3+0.55*cos(v))*sin(u));
gl_Position=gl_ModelViewProjectionMatrix*vec4(mix(sphere,torus,phase),1.0);})",
                     R"(#version 120
void main(){gl_FragColor=vec4(0.3,0.85,0.95,1.0);})");
        app.onKey = [&](int k) {
            if (k >= '1' && k <= '3')
            {
                mode = k - '0';
                app.orbit = mode == 3;
            }
        };
        return app.run(argc, argv, [&] {
            if (mode == 1)
            {
                app.ortho(22, 8);
                glTranslated(11, 4, 0);
                glScaled(1, -1, 1);
                color({.5, .5, .5});
                glBegin(GL_LINES);
                glVertex2d(-10, 0);
                glVertex2d(10, 0);
                glVertex2d(0, -3);
                glVertex2d(0, 3);
                glEnd();
                sinc.bind();
                glLineWidth(2);
                glBegin(GL_LINE_STRIP);
                for (int i = 0; i <= 2000; ++i)
                    glVertex3d(-10 + i * .01, 0, 0);
                glEnd();
                glLineWidth(1);
                sinc.stop();
            }
            else if (mode == 2)
            {
                app.ortho();
                flag.bind();
                rect(50, 50, 900, 600, {1, 1, 1});
                flag.stop();
            }
            else
            {
                app.camera();
                morph.bind();
                morph.uniform("phase", float(.5 - .5 * cos(app.time * .8)));
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                for (int j = 0; j < 40; ++j)
                {
                    glBegin(GL_QUAD_STRIP);
                    for (int i = 0; i <= 64; ++i)
                    {
                        glVertex3d(i * 2 * pi / 64, j * 2 * pi / 40, 0);
                        glVertex3d(i * 2 * pi / 64, (j + 1) * 2 * pi / 40, 0);
                    }
                    glEnd();
                }
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                morph.stop();
            }
        });
    });
}
