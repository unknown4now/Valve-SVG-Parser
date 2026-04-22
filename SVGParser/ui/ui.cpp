#include "ui.h"

namespace ui
{
    RECT r_add = {20, 60, 220, 110};
    RECT r_run = {20, 360, 480, 410};
    RECT r_x =   {470, 10, 490, 30};
    RECT r_min = {440, 10, 460, 30};

    int h_id = 0, p_id = 0;
    std::vector<log_msg> logs;
    std::vector<std::filesystem::path> g_files;
    HFONT f_main = nullptr, f_small = nullptr;

    void add_log(std::string t, COLORREF c)
    {
        logs.push_back({ t, c });
        if (logs.size() > 14) logs.erase(logs.begin());
    }

    void draw_rect(HDC hdc, RECT r, int c, COLORREF clr, bool p)
    {
        if (p) { r.left += 2; r.top += 2; r.right += 2; r.bottom += 2; }
        HBRUSH b = CreateSolidBrush(clr);
        HPEN pn = CreatePen(PS_SOLID, 1, clr);
        SelectObject(hdc, b); SelectObject(hdc, pn);
        RoundRect(hdc, r.left, r.top, r.right, r.bottom, c, c);
        DeleteObject(b); DeleteObject(pn);
    }

    void render(HWND hw, HDC hdc)
    {
        RECT c; GetClientRect(hw, &c);
        HDC m_dc = CreateCompatibleDC(hdc);
        HBITMAP m_bm = CreateCompatibleBitmap(hdc, c.right, c.bottom);
        SelectObject(m_dc, m_bm);

        HBRUSH b = CreateSolidBrush(col_bg);
        FillRect(m_dc, &c, b); DeleteObject(b);
        SetBkMode(m_dc, 1);
        SelectObject(m_dc, f_main);
        SetTextColor(m_dc, col_txt);
        TextOutA(m_dc, 20, 20, "VALVE SVG PARSER", 16);

        draw_rect(m_dc, r_min, 4, (h_id == 4) ? col_pnl_hov : col_pnl, p_id == 4);
        TextOutA(m_dc, r_min.left + 5, r_min.top - 2, "_", 1);
        draw_rect(m_dc, r_x, 4, (h_id == 3) ? col_act_hov : col_act, p_id == 3);
        TextOutA(m_dc, r_x.left + 5, r_x.top + 1, "X", 1);
        draw_rect(m_dc, { 20, 120, 220, 340 }, 10, col_pnl);
        draw_rect(m_dc, { 240, 60, 480, 340 }, 10, RGB(10, 10, 15));

        SelectObject(m_dc, f_small);
        int y = 70;
        for (auto& l : logs)
        {
            SetTextColor(m_dc, l.clr);
            TextOutA(m_dc, 255, y, l.msg.c_str(), (int)l.msg.length());
            y += 18;
        }

        SetTextColor(m_dc, col_txt);
        y = 130;
        for (auto& f : g_files)
        {
            if (y > 320) break;
            std::string n = f.filename().string();
            TextOutA(m_dc, 30, y, n.c_str(), (int)n.length());
            y += 18;
        }

        draw_rect(m_dc, r_add, 8, (h_id == 1) ? col_pnl_hov : col_pnl, p_id == 1);
        draw_rect(m_dc, r_run, 8, (h_id == 2) ? col_act_hov : col_act, p_id == 2);
        SelectObject(m_dc, f_main);
        SetTextColor(m_dc, col_txt);
        int o1 = (p_id == 1) ? 2 : 0, o2 = (p_id == 2) ? 2 : 0;
        TextOutA(m_dc, 85 + o1, 75 + o1, "ADD FILES", 9);
        TextOutA(m_dc, 195 + o2, 375 + o2, "CONVERT ALL", 11);
        BitBlt(hdc, 0, 0, c.right, c.bottom, m_dc, 0, 0, SRCCOPY);
        DeleteObject(m_bm); DeleteDC(m_dc);
    }
}