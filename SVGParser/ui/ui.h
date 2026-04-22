#pragma once
#include <windows.h>
#include <vector>
#include <string>
#include <filesystem>


#define col_bg          RGB(15, 15, 20)      
#define col_pnl         RGB(25, 25, 35)      
#define col_pnl_hov     RGB(45, 45, 55)
#define col_act         RGB(255, 46, 99)     
#define col_act_hov     RGB(255, 80, 130)
#define col_txt         RGB(230, 230, 230)   
#define col_ok          RGB(50, 255, 120)    


struct log_msg
{
	std::string msg;
	COLORREF clr;
};

namespace ui
{
    extern RECT r_add, r_run, r_x, r_min;
    extern int h_id, p_id;
    extern std::vector<log_msg> logs;
    extern std::vector<std::filesystem::path> g_files;
    extern HFONT f_main, f_small;

    void add_log(std::string t, COLORREF c = col_txt);
    void draw_rect(HDC hdc, RECT r, int c, COLORREF clr, bool p = false);
    void render(HWND hw, HDC hdc);
}
