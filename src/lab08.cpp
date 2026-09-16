#include "app.hpp"
#include "ray.hpp"
#include <atomic>
#include <thread>
int main(int argc, char **argv)
{
    return guarded([&] {
        App app(L"ЛР 08 — Трассировка лучей | S: мягкие тени, мышь: камера");
        app.orbit = true;
        app.distance = 10;
        app.pitch = 20;
        ray::Scene scene = ray::demo();
        GLuint texture = 0;
        bool dirty = true;
        double oldYaw = 0, oldPitch = 0, oldDistance = 0;
        app.onKey = [&](int k) {
            if (k == 'S')
            {
                scene.soft = !scene.soft;
                dirty = true;
            }
        };
        app.onMouse = [&](int event, int, int) {
            if (event == 2)
                dirty = true;
        };
        int result = app.run(argc, argv, [&] {
            if (dirty || oldYaw != app.yaw || oldPitch != app.pitch || oldDistance != app.distance)
            {
                const int w = 640, h = 440;
                Pixels image;
                image.w = w;
                image.h = h;
                image.rgba.resize(w * h * 4);
                double yaw = app.yaw * pi / 180, pitch = app.pitch * pi / 180;
                Vec3 eye{app.distance * cos(pitch) * sin(yaw), app.distance * sin(pitch),
                         app.distance * cos(pitch) * cos(yaw)},
                    forward = unit(-eye), right = unit(cross(forward, {0, 1, 0})), up = cross(right, forward);
                std::atomic<int> row{0};
                auto work = [&] {
                    int y;
                    while ((y = row.fetch_add(1)) < h)
                        for (int x = 0; x < w; ++x)
                        {
                            double sx = (2 * (x + .5) / w - 1) * double(w) / h * .41421356,
                                   sy = (1 - 2 * (y + .5) / h) * .41421356;
                            Vec3 c = scene.trace({eye, unit(forward + right * sx + up * sy)});
                            size_t i = (size_t(y) * w + x) * 4;
                            image.rgba[i] =
                                static_cast<unsigned char>(255 * pow(std::clamp(c.x, 0., 1.), 1 / 2.2));
                            image.rgba[i + 1] =
                                static_cast<unsigned char>(255 * pow(std::clamp(c.y, 0., 1.), 1 / 2.2));
                            image.rgba[i + 2] =
                                static_cast<unsigned char>(255 * pow(std::clamp(c.z, 0., 1.), 1 / 2.2));
                            image.rgba[i + 3] = 255;
                        }
                };
                std::vector<std::thread> workers;
                for (unsigned i = 0; i < std::clamp(std::thread::hardware_concurrency(), 1u, 8u); ++i)
                    workers.emplace_back(work);
                for (auto &t : workers)
                    t.join();
                if (texture)
                    glDeleteTextures(1, &texture);
                texture = upload(image);
                dirty = false;
                oldYaw = app.yaw;
                oldPitch = app.pitch;
                oldDistance = app.distance;
            }
            app.ortho(640, 440);
            texturedQuad(texture, 0, 0, 640, 440);
        });
        glDeleteTextures(1, &texture);
        return result;
    });
}
