#include "geometry.hpp"
struct Piece
{
    int kind, x, z;
    bool black;
};
int main(int argc, char **argv)
{
    return guarded([&] {
        App app(L"ЛР 06 — Шахматы: мат в два хода | пробел: пауза, R: сначала");
        app.orbit = true;
        app.distance = 15;
        app.pitch = 48;
        Model board(asset(L"board.obj"));
        std::vector<std::unique_ptr<Model>> models;
        for (const auto *side : {L"ivory", L"ebony"})
            for (const auto *kind : {L"pawn", L"rook", L"knight", L"bishop", L"queen", L"king"})
                models.push_back(
                    std::make_unique<Model>(asset((std::wstring(kind) + L"_" + side + L".obj").c_str())));
        std::vector<Piece> pieces;
        int order[] = {1, 2, 3, 4, 5, 3, 2, 1};
        for (int x = 0; x < 8; ++x)
        {
            pieces.push_back({order[x], x, 7, false});
            pieces.push_back({0, x, 6, false});
            pieces.push_back({0, x, 1, true});
            pieces.push_back({order[x], x, 0, true});
        }
        struct Move
        {
            int x, z, tx, tz;
        };
        const Move moves[] = {{5, 6, 5, 5}, {4, 1, 4, 3}, {6, 6, 6, 4}, {3, 0, 7, 4}};
        app.onKey = [&](int k) {
            if (k == 'R')
                app.time = 0;
        };
        return app.run(argc, argv, [&] {
            app.camera();
            app.light({-4, 8, 3});
            board.draw();
            double phase = fmod(app.time, 24.);
            int completed = std::clamp(int((phase - 2) / 4), 0, 4);
            double progress = phase < 2 ? 0 : std::clamp((phase - 2 - completed * 4) / 2., 0., 1.);
            auto current = pieces;
            for (int i = 0; i < completed; ++i)
                for (auto &p : current)
                    if (p.x == moves[i].x && p.z == moves[i].z)
                    {
                        p.x = moves[i].tx;
                        p.z = moves[i].tz;
                        break;
                    }
            for (auto p : current)
            {
                double x = p.x, z = p.z, y = .04;
                if (completed < 4 && p.x == moves[completed].x && p.z == moves[completed].z)
                {
                    double s = progress * progress * (3 - 2 * progress);
                    x += (moves[completed].tx - x) * s;
                    z += (moves[completed].tz - z) * s;
                    y += .35 * sin(pi * progress);
                }
                glPushMatrix();
                glTranslated(x - 3.5, y, z - 3.5);
                if (p.black)
                    glRotated(180, 0, 1, 0);
                models[(p.black ? 6 : 0) + p.kind]->draw();
                glPopMatrix();
            }
            const wchar_t *labels[] = {L"ЛР 06 — 1. f3 | пробел: пауза, R: сначала",
                                       L"ЛР 06 — 1... e5 | пробел: пауза, R: сначала",
                                       L"ЛР 06 — 2. g4 | пробел: пауза, R: сначала",
                                       L"ЛР 06 — 2... Фh4# | пробел: пауза, R: сначала",
                                       L"ЛР 06 — Мат! Чёрные выиграли. | R: сначала"};
            SetWindowTextW(app.window, labels[completed]);
        });
    });
}
