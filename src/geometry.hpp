#pragma once
#include "app.hpp"
#include <map>
inline void sphere(double r)
{
    auto q = gluNewQuadric();
    gluQuadricNormals(q, GLU_SMOOTH);
    gluQuadricTexture(q, GL_TRUE);
    gluSphere(q, r, 40, 24);
    gluDeleteQuadric(q);
}
// Surface of revolution around Y, including caps, with analytic slope normals.
inline void frustum(double bottom, double top, double h)
{
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= 64; ++i)
    {
        double a = 2 * pi * i / 64, c = cos(a), s = sin(a);
        normal(unit({c, (bottom - top) / h, s}));
        glTexCoord2d(double(i) / 64, 1);
        glVertex3d(bottom * c, 0, bottom * s);
        glTexCoord2d(double(i) / 64, 0);
        glVertex3d(top * c, h, top * s);
    }
    glEnd();
    for (int k = 0; k < 2; ++k)
    {
        double r = k ? top : bottom, y = k ? h : 0;
        normal({0, k ? 1. : -1., 0});
        glBegin(GL_TRIANGLE_FAN);
        glTexCoord2d(.5, .5);
        glVertex3d(0, y, 0);
        for (int i = 0; i <= 64; ++i)
        {
            double a = (k ? -1 : 1) * 2 * pi * i / 64;
            glTexCoord2d(.5 + .5 * cos(a), .5 + .5 * sin(a));
            glVertex3d(r * cos(a), y, r * sin(a));
        }
        glEnd();
    }
}
inline void box(Vec3 p, Vec3 size, Vec3 c)
{
    color(c);
    static const int faces[6][4] = {{0, 3, 2, 1}, {4, 5, 6, 7}, {0, 4, 7, 3},
                                    {1, 2, 6, 5}, {0, 1, 5, 4}, {3, 7, 6, 2}};
    Vec3 v[8] = {{-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
                 {-1, -1, 1},  {1, -1, 1},  {1, 1, 1},  {-1, 1, 1}};
    for (auto &f : faces)
    {
        normal(unit(cross(v[f[1]] - v[f[0]], v[f[2]] - v[f[0]])));
        glBegin(GL_QUADS);
        for (int j = 0; j < 4; ++j)
        {
            glTexCoord2d(j == 1 || j == 2, j >= 2);
            vertex(p + mul(v[f[j]], size) * .5);
        }
        glEnd();
    }
}
struct House2D
{
    Vec3 walls{.69, .39, .19}, roof{.57, .13, .11};
    void draw(double x, double y, double scale = 1) const
    {
        glPushMatrix();
        glTranslated(x, y, 0);
        glScaled(scale, scale, 1);
        rect(300, 80, 35, 105, {.42, .22, .16});
        rect(120, 185, 280, 230, walls);
        for (int i = 0; i < 10; ++i)
            rect(120, 195 + i * 22, 280, 3, {.43, .23, .12});
        color(roof);
        glBegin(GL_TRIANGLES);
        glVertex2d(95, 190);
        glVertex2d(260, 55);
        glVertex2d(425, 190);
        glEnd();
        rect(158, 235, 95, 100, {.94, .86, .58});
        rect(166, 243, 79, 84, {.35, .73, .88});
        rect(202, 243, 7, 84, {.96, .92, .75});
        rect(166, 281, 79, 7, {.96, .92, .75});
        rect(298, 282, 65, 133, {.32, .19, .11});
        ellipse(350, 355, 4, 4, {1, .75, .2});
        for (int i = 0; i < 13; ++i)
        {
            double a = 45 + i * 37;
            rect(a, 365, 20, 90, {.9, .78, .51});
            color({.9, .78, .51});
            glBegin(GL_TRIANGLES);
            glVertex2d(a, 365);
            glVertex2d(a + 10, 350);
            glVertex2d(a + 20, 365);
            glEnd();
        }
        rect(40, 389, 470, 10, {.76, .60, .35});
        rect(40, 427, 470, 10, {.76, .60, .35});
        glPopMatrix();
    }
};
struct Material
{
    Vec3 ambient{.15, .15, .15}, diffuse{.7, .7, .7}, specular{.35, .35, .35};
    float shine = 40;
    GLuint texture = 0;
};
inline void material(const Material &m)
{
    GLfloat a[] = {float(m.ambient.x), float(m.ambient.y), float(m.ambient.z), 1},
            d[] = {float(m.diffuse.x), float(m.diffuse.y), float(m.diffuse.z), 1},
            s[] = {float(m.specular.x), float(m.specular.y), float(m.specular.z), 1};
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, a);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, d);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, s);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, m.shine);
}
class Model
{
    struct Corner
    {
        int v = 0, t = 0, n = 0;
    };
    struct Face
    {
        std::array<Corner, 3> c;
        std::string m;
    };
    std::vector<Vec3> positions, normals, uv;
    std::vector<Face> faces;
    std::map<std::string, Material> materials;
    void readMTL(const fs::path &path)
    {
        std::ifstream in(path);
        if (!in)
            throw std::runtime_error("Cannot open MTL");
        std::string line, name;
        while (std::getline(in, line))
        {
            std::istringstream s(line);
            std::string op;
            s >> op;
            if (op == "newmtl")
                s >> name;
            else if (op == "Ka")
                s >> materials[name].ambient.x >> materials[name].ambient.y >> materials[name].ambient.z;
            else if (op == "Kd")
                s >> materials[name].diffuse.x >> materials[name].diffuse.y >> materials[name].diffuse.z;
            else if (op == "Ks")
                s >> materials[name].specular.x >> materials[name].specular.y >> materials[name].specular.z;
            else if (op == "Ns")
            {
                s >> materials[name].shine;
                materials[name].shine = std::clamp(materials[name].shine, 0.f, 128.f);
            }
            else if (op == "map_Kd")
            {
                std::string file;
                s >> file;
                materials[name].texture = upload(loadImage(path.parent_path() / file));
            }
        }
    }
    static int index(int i, size_t count)
    {
        int j = i > 0 ? i - 1 : int(count) + i;
        if (j < 0 || j >= int(count))
            throw std::runtime_error("Invalid OBJ index");
        return j;
    }

  public:
    explicit Model(const fs::path &path)
    {
        std::ifstream in(path);
        if (!in)
            throw std::runtime_error("Cannot open OBJ");
        std::string line, current;
        while (std::getline(in, line))
        {
            std::istringstream s(line);
            std::string op;
            s >> op;
            if (op == "v" || op == "vn" || op == "vt")
            {
                Vec3 p;
                s >> p.x >> p.y;
                if (op != "vt")
                    s >> p.z;
                (op == "v" ? positions : op == "vn" ? normals : uv).push_back(p);
            }
            else if (op == "mtllib")
            {
                std::string m;
                s >> m;
                readMTL(path.parent_path() / m);
            }
            else if (op == "usemtl")
                s >> current;
            else if (op == "f")
            {
                std::vector<Corner> polygon;
                std::string token;
                while (s >> token)
                {
                    Corner c;
                    size_t a = token.find('/'), b = a == std::string::npos ? a : token.find('/', a + 1);
                    c.v = index(std::stoi(token.substr(0, a)), positions.size());
                    c.t = -1;
                    c.n = -1;
                    if (a != std::string::npos && b != a + 1 && a + 1 < token.size())
                        c.t = index(std::stoi(token.substr(a + 1, b - a - 1)), uv.size());
                    if (b != std::string::npos && b + 1 < token.size())
                        c.n = index(std::stoi(token.substr(b + 1)), normals.size());
                    polygon.push_back(c);
                }
                for (size_t i = 1; i + 1 < polygon.size(); ++i)
                    faces.push_back({{polygon[0], polygon[i], polygon[i + 1]}, current});
            }
        }
        if (faces.empty())
            throw std::runtime_error("Empty OBJ model");
    }
    ~Model()
    {
        for (auto &m : materials)
            if (m.second.texture)
                glDeleteTextures(1, &m.second.texture);
    }
    Model(const Model &) = delete;
    void draw(const std::function<void(bool)> &textureUniform = {}) const
    {
        glDisable(GL_COLOR_MATERIAL);
        std::string last = "\xff";
        for (const auto &f : faces)
        {
            if (last != f.m)
            {
                last = f.m;
                auto it = materials.find(last);
                Material m = it == materials.end() ? Material{} : it->second;
                material(m);
                if (m.texture)
                {
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, m.texture);
                }
                else
                    glDisable(GL_TEXTURE_2D);
                if (textureUniform)
                    textureUniform(m.texture != 0);
            }
            Vec3 n = unit(
                cross(positions[f.c[1].v] - positions[f.c[0].v], positions[f.c[2].v] - positions[f.c[0].v]));
            glBegin(GL_TRIANGLES);
            for (auto c : f.c)
            {
                normal(c.n < 0 ? n : normals[c.n]);
                if (c.t >= 0)
                    glTexCoord2d(uv[c.t].x, uv[c.t].y);
                else
                    glTexCoord2d(0, 0);
                vertex(positions[c.v]);
            }
            glEnd();
        }
        glDisable(GL_TEXTURE_2D);
    }
};
