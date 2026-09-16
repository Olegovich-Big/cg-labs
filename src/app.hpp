#pragma once
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <gdiplus.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include "math.hpp"
namespace fs = std::filesystem;
inline fs::path asset(const wchar_t *name)
{
    wchar_t exe[32768];
    GetModuleFileNameW(nullptr, exe, 32768);
    fs::path dir = fs::path(exe).parent_path();
    for (int i = 0; i < 4; ++i, dir = dir.parent_path())
        if (fs::exists(dir / L"assets" / name))
            return dir / L"assets" / name;
    throw std::runtime_error("Asset not found");
}
inline std::wstring openImage(HWND owner)
{
    wchar_t file[32768]{};
    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = owner;
    ofn.lpstrFile = file;
    ofn.nMaxFile = 32768;
    ofn.lpstrFilter = L"Images (PNG, JPEG, BMP)\0*.png;*.jpg;*.jpeg;*.bmp\0All files\0*.*\0";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    return GetOpenFileNameW(&ofn) ? file : L"";
}
struct Pixels
{
    int w = 0, h = 0;
    std::vector<unsigned char> rgba;
};
inline Pixels loadImage(const fs::path &path)
{
    Gdiplus::Bitmap bitmap(path.c_str());
    if (bitmap.GetLastStatus() != Gdiplus::Ok)
        throw std::runtime_error("Cannot decode image");
    Pixels p;
    p.w = static_cast<int>(bitmap.GetWidth());
    p.h = static_cast<int>(bitmap.GetHeight());
    if (p.w <= 0 || p.h <= 0 || p.w > 16384 || p.h > 16384)
        throw std::runtime_error("Unsupported image dimensions");
    p.rgba.resize(size_t(p.w) * p.h * 4);
    Gdiplus::Rect rect(0, 0, p.w, p.h);
    Gdiplus::BitmapData data{};
    if (bitmap.LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &data) != Gdiplus::Ok)
        throw std::runtime_error("Cannot read image pixels");
    for (int y = 0; y < p.h; ++y)
        for (int x = 0; x < p.w; ++x)
        {
            auto s = static_cast<unsigned char *>(data.Scan0) + y * data.Stride + x * 4;
            auto d = &p.rgba[(size_t(y) * p.w + x) * 4];
            d[0] = s[2];
            d[1] = s[1];
            d[2] = s[0];
            d[3] = s[3];
        }
    bitmap.UnlockBits(&data);
    return p;
}
inline GLuint upload(const Pixels &p)
{
    GLint maxSize = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
    if (p.w > maxSize || p.h > maxSize)
        throw std::runtime_error("Image exceeds GPU texture limit");
    GLuint t;
    glGenTextures(1, &t);
    glBindTexture(GL_TEXTURE_2D, t);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 0x812F);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 0x812F);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, p.w, p.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, p.rgba.data());
    return t;
}
inline void color(Vec3 c)
{
    glColor3d(c.x, c.y, c.z);
}
inline void vertex(Vec3 p)
{
    glVertex3d(p.x, p.y, p.z);
}
inline void normal(Vec3 n)
{
    glNormal3d(n.x, n.y, n.z);
}
inline void rect(double x, double y, double w, double h, Vec3 c)
{
    color(c);
    glBegin(GL_QUADS);
    glVertex2d(x, y);
    glVertex2d(x + w, y);
    glVertex2d(x + w, y + h);
    glVertex2d(x, y + h);
    glEnd();
}
inline void ellipse(double x, double y, double rx, double ry, Vec3 c)
{
    color(c);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2d(x, y);
    for (int i = 0; i <= 64; ++i)
    {
        double a = 2 * pi * i / 64;
        glVertex2d(x + rx * cos(a), y + ry * sin(a));
    }
    glEnd();
}
inline void texturedQuad(GLuint t, double x, double y, double w, double h)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, t);
    glColor4f(1, 1, 1, 1);
    glBegin(GL_QUADS);
    glTexCoord2d(0, 0);
    glVertex2d(x, y);
    glTexCoord2d(1, 0);
    glVertex2d(x + w, y);
    glTexCoord2d(1, 1);
    glVertex2d(x + w, y + h);
    glTexCoord2d(0, 1);
    glVertex2d(x, y + h);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}
inline void saveBMP(const fs::path &path, int w, int h, const std::vector<unsigned char> &rgb)
{
    int stride = (w * 3 + 3) & ~3;
    BITMAPFILEHEADER fh{};
    BITMAPINFOHEADER ih{};
    fh.bfType = 0x4D42;
    fh.bfOffBits = sizeof(fh) + sizeof(ih);
    fh.bfSize = fh.bfOffBits + stride * h;
    ih.biSize = sizeof(ih);
    ih.biWidth = w;
    ih.biHeight = h;
    ih.biPlanes = 1;
    ih.biBitCount = 24;
    ih.biSizeImage = stride * h;
    std::ofstream out(path, std::ios::binary);
    if (!out)
        throw std::runtime_error("Cannot write capture");
    out.write(reinterpret_cast<char *>(&fh), sizeof(fh));
    out.write(reinterpret_cast<char *>(&ih), sizeof(ih));
    std::vector<unsigned char> row(stride);
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            size_t k = (size_t(y) * w + x) * 3;
            row[x * 3] = rgb[k + 2];
            row[x * 3 + 1] = rgb[k + 1];
            row[x * 3 + 2] = rgb[k];
        }
        out.write(reinterpret_cast<char *>(row.data()), stride);
    }
}
class App
{
    HDC dc = nullptr;
    HGLRC rc = nullptr;
    ULONG_PTR gdip = 0;
    bool drag = false;
    POINT previous{};
    static LRESULT CALLBACK proc(HWND h, UINT msg, WPARAM wp, LPARAM lp)
    {
        App *a = reinterpret_cast<App *>(GetWindowLongPtrW(h, GWLP_USERDATA));
        if (msg == WM_NCCREATE)
        {
            a = static_cast<App *>(reinterpret_cast<CREATESTRUCTW *>(lp)->lpCreateParams);
            SetWindowLongPtrW(h, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(a));
            a->window = h;
        }
        if (!a)
            return DefWindowProcW(h, msg, wp, lp);
        switch (msg)
        {
        case WM_SIZE:
            a->width = std::max(1, int(LOWORD(lp)));
            a->height = std::max(1, int(HIWORD(lp)));
            return 0;
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(h, &ps);
            EndPaint(h, &ps);
            return 0;
        }
        case WM_CLOSE:
            DestroyWindow(h);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_KEYDOWN:
            if (wp == VK_ESCAPE)
                DestroyWindow(h);
            else if (wp == VK_SPACE)
                a->paused = !a->paused;
            else if (a->onKey)
                a->onKey(int(wp));
            return 0;
        case WM_COMMAND:
            if (a->onKey)
                a->onKey(int(LOWORD(wp)));
            return 0;
        case WM_LBUTTONDOWN:
            a->drag = true;
            a->previous = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
            SetCapture(h);
            if (a->onMouse)
                a->onMouse(0, a->previous.x, a->previous.y);
            return 0;
        case WM_LBUTTONUP:
            a->drag = false;
            ReleaseCapture();
            if (a->onMouse)
                a->onMouse(2, GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
            return 0;
        case WM_CAPTURECHANGED:
            a->drag = false;
            if (a->onMouse)
                a->onMouse(2, 0, 0);
            return 0;
        case WM_MOUSEMOVE:
            if (a->drag)
            {
                int x = GET_X_LPARAM(lp), y = GET_Y_LPARAM(lp);
                if (a->orbit)
                {
                    a->yaw += (x - a->previous.x) * .5;
                    a->pitch = std::clamp(a->pitch + (y - a->previous.y) * .5, -85., 85.);
                }
                if (a->onMouse)
                    a->onMouse(1, x, y);
                a->previous = {x, y};
            }
            return 0;
        case WM_MOUSEWHEEL:
            a->distance = std::clamp(a->distance * std::pow(.9, GET_WHEEL_DELTA_WPARAM(wp) / 120.), 2., 80.);
            return 0;
        }
        return DefWindowProcW(h, msg, wp, lp);
    }

  public:
    HWND window = nullptr;
    int width = 1000, height = 720;
    double yaw = 25, pitch = 20, distance = 8, time = 0;
    bool orbit = false, paused = false;
    std::function<void(int)> onKey;
    std::function<void(int, int, int)> onMouse;
    App(const wchar_t *title)
    {
        Gdiplus::GdiplusStartupInput input;
        if (Gdiplus::GdiplusStartup(&gdip, &input, nullptr) != Gdiplus::Ok)
            throw std::runtime_error("GDI+ initialization failed");
        WNDCLASSW wc{};
        wc.style = CS_OWNDC;
        wc.lpfnWndProc = proc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = L"CGLaboratory";
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        RegisterClassW(&wc);
        window = CreateWindowW(wc.lpszClassName, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                               width, height, nullptr, nullptr, wc.hInstance, this);
        if (!window)
            throw std::runtime_error("Window creation failed");
        dc = GetDC(window);
        PIXELFORMATDESCRIPTOR p{};
        p.nSize = sizeof(p);
        p.nVersion = 1;
        p.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        p.iPixelType = PFD_TYPE_RGBA;
        p.cColorBits = 32;
        p.cDepthBits = 24;
        p.cStencilBits = 8;
        int pf = ChoosePixelFormat(dc, &p);
        if (!pf || !SetPixelFormat(dc, pf, &p))
            throw std::runtime_error("Pixel format unavailable");
        rc = wglCreateContext(dc);
        if (!rc || !wglMakeCurrent(dc, rc))
            throw std::runtime_error("OpenGL context unavailable");
        std::cout << "OpenGL " << glGetString(GL_VERSION) << " / " << glGetString(GL_RENDERER) << '\n';
    }
    ~App()
    {
        if (rc)
        {
            wglMakeCurrent(nullptr, nullptr);
            wglDeleteContext(rc);
        }
        if (dc)
            ReleaseDC(window, dc);
        if (IsWindow(window))
            DestroyWindow(window);
        if (gdip)
            Gdiplus::GdiplusShutdown(gdip);
    }
    void fileMenu()
    {
        HMENU menu = CreateMenu(), file = CreatePopupMenu();
        AppendMenuW(file, MF_STRING, 'O', L"&Open...\tO");
        AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(file), L"&File");
        SetMenu(window, menu);
    }
    void ortho(double w = 1000, double h = 700)
    {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        double s = std::min(width / w, height / h), vw = width / s, vh = height / s;
        glOrtho((w - vw) / 2, (w + vw) / 2, (h + vh) / 2, (h - vh) / 2, -10, 10);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }
    Vec3 screenToWorld(int x, int y, double w = 1000, double h = 700) const
    {
        double s = std::min(width / w, height / h);
        return {(x - width / 2.) / s + w / 2, (y - height / 2.) / s + h / 2, 0};
    }
    void camera()
    {
        glEnable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(45, double(width) / height, .1, 200);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslated(0, 0, -distance);
        glRotated(pitch, 1, 0, 0);
        glRotated(yaw, 0, 1, 0);
    }
    void light(Vec3 pos = {3, 5, 4})
    {
        GLfloat p[] = {float(pos.x), float(pos.y), float(pos.z), 1}, ambient[] = {.16f, .16f, .18f, 1},
                white[] = {1, 1, 1, 1};
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_NORMALIZE);
        glLightfv(GL_LIGHT0, GL_POSITION, p);
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
        glLightfv(GL_LIGHT0, GL_SPECULAR, white);
        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 40);
    }
    int run(int argc, char **argv, const std::function<void()> &draw)
    {
        fs::path capture;
        double captureTime = 1.25;
        int frames = 0;
        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];
            if (arg == "--capture" && i + 1 < argc)
                capture = fs::u8path(argv[++i]);
            else if (arg == "--time" && i + 1 < argc)
                captureTime = std::stod(argv[++i]);
            else if (arg == "--mode" && i + 1 < argc)
            {
                int key = std::stoi(argv[++i]);
                if (onKey)
                    onKey('0' + key);
            }
        }
        if (capture.empty())
            ShowWindow(window, SW_SHOW);
        auto last = std::chrono::steady_clock::now();
        MSG msg{};
        bool running = true;
        while (running)
        {
            while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    running = false;
                    break;
                }
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
            if (!running)
                break;
            auto now = std::chrono::steady_clock::now();
            if (!paused)
                time += std::chrono::duration<double>(now - last).count();
            last = now;
            if (!capture.empty())
                time = captureTime;
            glViewport(0, 0, width, height);
            glClearColor(.055f, .075f, .115f, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            draw();
            GLenum error = glGetError();
            if (error != GL_NO_ERROR)
                throw std::runtime_error("OpenGL error: " + std::to_string(error));
            if (!capture.empty() && ++frames == 2)
            {
                std::vector<unsigned char> rgb(size_t(width) * height * 3);
                glPixelStorei(GL_PACK_ALIGNMENT, 1);
                glReadBuffer(GL_BACK);
                glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, rgb.data());
                saveBMP(capture, width, height, rgb);
                std::cout << "Capture saved\n";
                break;
            }
            SwapBuffers(dc);
            MsgWaitForMultipleObjects(0, nullptr, FALSE, 16, QS_ALLINPUT);
        }
        return 0;
    }
};
template <class F> int guarded(F f)
{
    try
    {
        return f();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
