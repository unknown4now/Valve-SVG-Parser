#include <windows.h>
#include <commdlg.h>
#include <vector>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <gdiplus.h>
#include "ui/ui.h"
#include "parser.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

void get_scaling_params(GraphicsPath* gp, float& scale, float& tx, float& ty)
{
    RectF b;
    gp->GetBounds(&b);

    if (b.Width == 0 || b.Height == 0) return;

    float padding = 0.15f;
    float available = 512.0f * (1.0f - (padding * 2.0f));
    float sw = available / b.Width;
    float sh = available / b.Height;

    scale = (sw < sh) ? sw : sh;
    float cx = b.X + (b.Width / 2.0f);
    float cy = b.Y + (b.Height / 2.0f);

    tx = 256.0f - (scale * cx);
    ty = 256.0f - (scale * cy);
}

// readin raw bytes and parsing vsvg data here
void parse_and_convert(std::filesystem::path path)
{
    auto in = std::ifstream(path, std::ios::binary | std::ios::ate);
    if (!in.is_open()) return;

    const auto size = (size_t)in.tellg();
    in.seekg(0, std::ios::beg);

    auto buf = std::vector<char>{};
    buf.resize(size);

    if (!in.read(buf.data(), size)) return;

    auto raw = svg::parse(buf);
    if (raw.empty())
    {
        ui::add_log("fail: " + path.filename().string(), col_act);
        return;
    }

    std::wstring w_raw(raw.begin(), raw.end());
    GraphicsPath* gp = new GraphicsPath();
    gp->AddString(w_raw.c_str(), -1, nullptr, 0, 10.0f, PointF(0.0f, 0.0f), nullptr);

    float s = 1.0f, x = 0.0f, y = 0.0f;
    get_scaling_params(gp, s, x, y);
    delete gp;

    // this just for fun, u can edit here how u want
    std::stringstream ss;
    ss << std::fixed << std::setprecision(4);
    ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    ss << "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 512 512\" width=\"100%\" height=\"100%\" preserveAspectRatio=\"xMidYMid meet\">\n";
    ss << "  <rect x=\"-500%\" y=\"-500%\" width=\"1000%\" height=\"1000%\" fill=\"#0F0F14\"/>\n";
    ss << "  <g fill=\"white\" transform=\"translate(" << x << ", " << y << ") scale(" << s << ")\">\n";
    ss << "    " << raw << "\n";
    ss << "  </g>\n";
    ss << "</svg>\n";

    auto out_p = path;
    out_p += ".svg";
    auto out = std::ofstream(out_p, std::ios::out | std::ios::trunc);
    out << ss.str();
    out.close();
    ui::add_log("Done: " + out_p.filename().string(), col_ok);
}


void do_work()
{
    if (ui::g_files.empty())
    {
        ui::add_log("Error: No files", col_act);
        return;
    }

    ui::add_log("Converting...", col_act);

    for (const auto& p : ui::g_files)
    {
        parse_and_convert(p);
    }

    ui::g_files.clear();
}

/*
 * ui & entry
 * handles window messages and gdi+ init
 */

LRESULT CALLBACK wnd_proc(HWND hw, UINT m, WPARAM wp, LPARAM lp)
{
    switch (m)
    {
    case WM_CREATE:
        ui::f_main = CreateFontW(18, 0, 0, 0, 700, 0, 0, 0, 0, 0, 0, 5, 0, L"Segoe UI");
        ui::f_small = CreateFontW(14, 0, 0, 0, 400, 0, 0, 0, 0, 0, 0, 5, 0, L"Segoe UI");
        ui::add_log("Ready", col_ok);
        break;

    case WM_NCHITTEST:
    {
        POINT pt = { LOWORD(lp), HIWORD(lp) }; ScreenToClient(hw, &pt);
        if (PtInRect(&ui::r_x, pt) || PtInRect(&ui::r_min, pt) || PtInRect(&ui::r_add, pt) || PtInRect(&ui::r_run, pt)) return HTCLIENT;
        return (DefWindowProc(hw, m, wp, lp) == HTCLIENT) ? HTCAPTION : HTCLIENT;
    }

    case WM_MOUSEMOVE:
    {
        POINT pt = { LOWORD(lp), HIWORD(lp) };
        int old = ui::h_id;
        if (PtInRect(&ui::r_add, pt)) ui::h_id = 1;
        else if (PtInRect(&ui::r_run, pt)) ui::h_id = 2;
        else if (PtInRect(&ui::r_x, pt)) ui::h_id = 3;
        else if (PtInRect(&ui::r_min, pt)) ui::h_id = 4;
        else ui::h_id = 0;
        if (old != ui::h_id) InvalidateRect(hw, 0, 0);
        TRACKMOUSEEVENT t = { sizeof(t), TME_LEAVE, hw, 0 }; TrackMouseEvent(&t);
        break;
    }

    case WM_LBUTTONDOWN:
    {
        POINT pt = { LOWORD(lp), HIWORD(lp) };
        if (PtInRect(&ui::r_add, pt)) ui::p_id = 1;
        else if (PtInRect(&ui::r_run, pt)) ui::p_id = 2;
        else if (PtInRect(&ui::r_x, pt)) ui::p_id = 3;
        else if (PtInRect(&ui::r_min, pt)) ui::p_id = 4;
        InvalidateRect(hw, 0, 0); break;
    }

    case WM_LBUTTONUP:
    {
        POINT pt = { LOWORD(lp), HIWORD(lp) };
        if (ui::p_id == 1 && PtInRect(&ui::r_add, pt)) {
            char f_buf[4096] = { 0 }; OPENFILENAMEA of = { sizeof(of), hw };
            of.lpstrFile = f_buf; of.nMaxFile = 4096; of.lpstrFilter = "Valve\0*.vsvg_c\0";
            of.Flags = OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_FILEMUSTEXIST;
            if (GetOpenFileNameA(&of)) {
                std::string d = f_buf; char* s = f_buf + d.length() + 1;
                if (*s == 0) ui::g_files.push_back(d);
                else while (*s != 0) { ui::g_files.push_back(std::filesystem::path(d) / s); s += strlen(s) + 1; }
                ui::add_log("Files added");
            }
        }
        else if (ui::p_id == 2 && PtInRect(&ui::r_run, pt)) do_work();
        else if (ui::p_id == 3 && PtInRect(&ui::r_x, pt)) PostQuitMessage(0);
        else if (ui::p_id == 4 && PtInRect(&ui::r_min, pt)) ShowWindow(hw, SW_MINIMIZE);
        ui::p_id = 0; InvalidateRect(hw, 0, 0); break;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps; HDC hdc = BeginPaint(hw, &ps);
        ui::render(hw, hdc);
        EndPaint(hw, &ps); break;
    }
    case WM_MOUSELEAVE: ui::h_id = 0; InvalidateRect(hw, 0, 0); break;
    case WM_DESTROY: PostQuitMessage(0); break;
    }
    return DefWindowProc(hw, m, wp, lp);
}

int WINAPI WinMain(HINSTANCE h, HINSTANCE p, LPSTR l, int n)
{
    GdiplusStartupInput gsi;
    ULONG_PTR gtoken;
    GdiplusStartup(&gtoken, &gsi, NULL);

    WNDCLASSEXA w = { sizeof(w), 3, wnd_proc, 0, 0, h, 0, LoadCursor(0, (LPCSTR)32512), 0, 0, "m_ui", 0 };
    RegisterClassExA(&w);
    HWND hw = CreateWindowExA(0, "m_ui", "svg", 0x80000000 | 0x10000000, 500, 300, 500, 430, 0, 0, h, 0);

    MSG msg;
    while (GetMessage(&msg, 0, 0, 0)) { TranslateMessage(&msg); DispatchMessage(&msg); }

    GdiplusShutdown(gtoken);
    return 0;
}