// ndll-sportedout-windows.ndll  (hxcpp 4.3.x, Windows x64)
// - set_transparent(bool,int,int,int) y set_transparent(r,g,b) [3 args]
// - Protegido contra crashes (SEH) + validación de tipos + log en sportedout_ndll.log
#define _CRT_SECURE_NO_WARNINGS
#define IMPLEMENT_API
#include <hx/CFFIPrime.h>
#include <windows.h>
#include <dxgi.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <string>
#include <vector>
 
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "dxgi.lib")
 
static void LOG(const char* fmt, ...) {
    FILE* f = fopen("sportedout_ndll.log", "a");
    if (!f) return;
    va_list ap; va_start(ap, fmt);
    vfprintf(f, fmt, ap);
    va_end(ap);
    fputc('\n', f);
    fclose(f);
}
 
static std::string toUtf8(const std::wstring& w) {
    if (w.empty()) return "";
    int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), NULL, 0, NULL, NULL);
    std::string s(n, 0);
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), &s[0], n, NULL, NULL);
    return s;
}
static std::wstring toWide(const char* s) {
    int n = MultiByteToWideChar(CP_UTF8, 0, s, -1, NULL, 0);
    std::wstring w(n > 0 ? n - 1 : 0, 0);
    if (n > 1) MultiByteToWideChar(CP_UTF8, 0, s, -1, &w[0], n);
    return w;
}
 
static HWND g_best = NULL;
static long long g_bestArea = 0;
 
static BOOL CALLBACK enumOwn(HWND h, LPARAM) {
    DWORD pid = 0; GetWindowThreadProcessId(h, &pid);
    if (pid != GetCurrentProcessId() || !IsWindowVisible(h) || GetWindow(h, GW_OWNER)) return TRUE;
    RECT rc; if (!GetWindowRect(h, &rc)) return TRUE;
    long long a = (long long)(rc.right - rc.left) * (rc.bottom - rc.top);
    if (a > g_bestArea) { g_bestArea = a; g_best = h; }
    return TRUE;
}
 
static HWND getWindow() {
    HWND h = GetActiveWindow();
    if (h) return GetAncestor(h, GA_ROOT);
    h = GetForegroundWindow();
    if (h) { DWORD pid = 0; GetWindowThreadProcessId(h, &pid); if (pid == GetCurrentProcessId()) return h; }
    g_best = NULL; g_bestArea = 0;
    EnumWindows(enumOwn, 0);
    return g_best;
}
 
struct SysInfo { bool laptop; bool hybrid; int adapters; std::string gpus; };
 
static SysInfo detect() {
    SysInfo si = { false, false, 0, "" };
    SYSTEM_POWER_STATUS ps;
    if (GetSystemPowerStatus(&ps)) si.laptop = (ps.BatteryFlag != 128 && ps.BatteryFlag != 255);
 
    IDXGIFactory1* f = NULL;
    if (SUCCEEDED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&f)) && f) {
        bool intel = false, disc = false;
        IDXGIAdapter1* a = NULL;
        for (UINT i = 0; f->EnumAdapters1(i, &a) != DXGI_ERROR_NOT_FOUND; i++) {
            DXGI_ADAPTER_DESC1 d; a->GetDesc1(&d);
            a->Release();
            if (d.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
            std::wstring n = d.Description;
            si.adapters++;
            if (!si.gpus.empty()) si.gpus += " + ";
            si.gpus += toUtf8(n);
            if (n.find(L"Intel") != std::wstring::npos) intel = true;
            else if (n.find(L"NVIDIA") != std::wstring::npos || n.find(L"AMD") != std::wstring::npos ||
                     n.find(L"Radeon") != std::wstring::npos) disc = true;
        }
        f->Release();
        si.hybrid = si.laptop && si.adapters > 1 && (intel || disc);
    }
    return si;
}
 
static void detectInto(SysInfo* out) { *out = detect(); }
 
static bool detectGuarded(SysInfo* out) {
    __try { detectInto(out); return true; }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}
 
static int computeAutoMode() {
    SysInfo si = { false, false, 0, "" };
    if (!detectGuarded(&si)) return 1;
    if (si.hybrid) return 3;
    if (si.laptop) return 2;
    return 1;
}
 
static int g_mode = 0;
 
static int resolveMode() {
    if (g_mode != 0) return g_mode;
    static int cached = -1;
    if (cached < 0) { cached = computeAutoMode(); LOG("modo automatico = %d", cached); }
    return cached;
}
 

static void refreshFrame(HWND h) {
    RedrawWindow(h, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
}
 
static void refreshFrameNow(HWND h) {
    SetWindowPos(h, NULL, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
}
 
static bool applyTransparentImpl(int r, int g, int b) {
    HWND h = getWindow();
    if (!h) { LOG("set_transparent: no se encontro la ventana"); return false; }
 
    LONG_PTR ex = GetWindowLongPtr(h, GWL_EXSTYLE);
    SetWindowLongPtr(h, GWL_EXSTYLE, ex | WS_EX_LAYERED);
 
    int mode = resolveMode();
    COLORREF key = RGB(r & 255, g & 255, b & 255);
    BOOL ok;
    if (mode == 3) ok = SetLayeredWindowAttributes(h, key, 255, LWA_COLORKEY | LWA_ALPHA);
    else           ok = SetLayeredWindowAttributes(h, key, 0, LWA_COLORKEY);
 
    if (mode >= 2) refreshFrame(h);
    LOG("set_transparent hwnd=%p color=(%d,%d,%d) modo=%d ok=%d err=%lu",
        (void*)h, r, g, b, mode, (int)ok, ok ? 0UL : GetLastError());
    return ok != 0;
}
 
static bool applyTransparentGuarded(int r, int g, int b) {
    __try { return applyTransparentImpl(r, g, b); }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        LOG("set_transparent: EXCEPCION 0x%08lX", GetExceptionCode());
        return false;
    }
}
 
value set_transparent(bool unused, int r, int g, int b) {
    LOG("set_transparent(4 args) llamado");
    return alloc_bool(applyTransparentGuarded(r, g, b));
}
DEFINE_PRIME4(set_transparent);
 

value set_transparent3(int r, int g, int b) {
    LOG("set_transparent(3 args) llamado: %d, %d, %d", r, g, b);
    return alloc_bool(applyTransparentGuarded(r, g, b));
}
DEFINE_PRIME3(set_transparent3);
#pragma comment(linker, "/EXPORT:set_transparent__3=set_transparent3__3")
 
static bool applyAlphaImpl(int a) {
    HWND h = getWindow();
    if (!h) return false;
    if (a < 0) a = 0;
    if (a > 255) a = 255;
    LONG_PTR ex = GetWindowLongPtr(h, GWL_EXSTYLE);
    SetWindowLongPtr(h, GWL_EXSTYLE, ex | WS_EX_LAYERED);
    BOOL ok = SetLayeredWindowAttributes(h, 0, (BYTE)a, LWA_ALPHA);
    refreshFrame(h);
    LOG("set_alpha %d ok=%d", a, (int)ok);
    return ok != 0;
}
static bool applyAlphaGuarded(int a) {
    __try { return applyAlphaImpl(a); }
    __except (EXCEPTION_EXECUTE_HANDLER) { LOG("set_alpha: EXCEPCION 0x%08lX", GetExceptionCode()); return false; }
}
value set_alpha(int a) { return alloc_bool(applyAlphaGuarded(a)); }
DEFINE_PRIME1(set_alpha);
 
static bool removeImpl() {
    HWND h = getWindow();
    if (!h) { LOG("remove_transparent: no se encontro la ventana"); return false; }
    LONG_PTR ex = GetWindowLongPtr(h, GWL_EXSTYLE);
    SetWindowLongPtr(h, GWL_EXSTYLE, ex & ~WS_EX_LAYERED);
    refreshFrame(h);
    LOG("remove_transparent hwnd=%p", (void*)h);
    return true;
}
static bool removeGuarded() {
    __try { return removeImpl(); }
    __except (EXCEPTION_EXECUTE_HANDLER) { LOG("remove_transparent: EXCEPCION 0x%08lX", GetExceptionCode()); return false; }
}
value remove_transparent() { return alloc_bool(removeGuarded()); }
DEFINE_PRIME0(remove_transparent);
 
value set_transparent_mode(int mode) { g_mode = (mode < 0 || mode > 3) ? 0 : mode; LOG("modo forzado = %d", g_mode); return alloc_int(g_mode); }
DEFINE_PRIME1(set_transparent_mode);
 
value get_system_info() {
    SysInfo si = { false, false, 0, "" };
    if (!detectGuarded(&si)) return alloc_string("no se pudo detectar el sistema");
    std::string s = std::string(si.laptop ? "laptop" : "pc") + (si.hybrid ? " (hibrida)" : "") +
                    " | modo=" + std::to_string(resolveMode()) + " | GPU: " + si.gpus;
    LOG("info: %s", s.c_str());
    return alloc_string(s.c_str());
}
DEFINE_PRIME0(get_system_info);
 
static void showTaskbars(int cmd) {
    HWND t = FindWindowA("Shell_TrayWnd", NULL);
    if (t) ShowWindow(t, cmd);
    HWND s = NULL;
    while ((s = FindWindowExA(NULL, s, "Shell_SecondaryTrayWnd", NULL)) != NULL) ShowWindow(s, cmd);
}
value hide_taskbar() { showTaskbars(SW_HIDE); return alloc_bool(true); }
DEFINE_PRIME0(hide_taskbar);
value show_taskbar() { showTaskbars(SW_SHOW); return alloc_bool(true); }
DEFINE_PRIME0(show_taskbar);
 
static void setSysMenu(bool on) {
    HWND h = getWindow(); if (!h) return;
    LONG_PTR st = GetWindowLongPtr(h, GWL_STYLE);
    SetWindowLongPtr(h, GWL_STYLE, on ? (st | WS_SYSMENU) : (st & ~(LONG_PTR)WS_SYSMENU));
    refreshFrameNow(h);
}
value hide_window_icon() { setSysMenu(false); return alloc_bool(true); }
DEFINE_PRIME0(hide_window_icon);
value show_window_icon() { setSysMenu(true); return alloc_bool(true); }
DEFINE_PRIME0(show_window_icon);
 
value get_wallpaper() {
    wchar_t buf[MAX_PATH] = {0};
    SystemParametersInfoW(SPI_GETDESKWALLPAPER, MAX_PATH, buf, 0);
    return alloc_string(toUtf8(buf).c_str());
}
DEFINE_PRIME0(get_wallpaper);
 
value change_wallpaper(value path) {
    if (!path || !val_is_string(path)) return alloc_bool(false);
    std::wstring w = toWide(val_string(path));
    BOOL ok = SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, (PVOID)w.c_str(), SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
    return alloc_bool(ok != 0);
}
DEFINE_PRIME1(change_wallpaper);
 
value get_ram() {
    ULONGLONG kb = 0;
    if (GetPhysicallyInstalledSystemMemory(&kb)) return alloc_int((int)(kb / 1024)); // MB
    MEMORYSTATUSEX m; m.dwLength = sizeof(m);
    if (GlobalMemoryStatusEx(&m)) return alloc_int((int)(m.ullTotalPhys / (1024 * 1024)));
    return alloc_int(0);
}
DEFINE_PRIME0(get_ram);
 
static std::vector<HWND> g_hidden;
 
static BOOL CALLBACK enumOthers(HWND h, LPARAM) {
    DWORD pid = 0; GetWindowThreadProcessId(h, &pid);
    if (pid == GetCurrentProcessId() || !IsWindowVisible(h) || GetWindow(h, GW_OWNER)) return TRUE;
    if (GetWindowLongPtr(h, GWL_EXSTYLE) & WS_EX_TOOLWINDOW) return TRUE;
    char title[256] = {0}; GetWindowTextA(h, title, 255);
    if (!title[0]) return TRUE;
    char cls[128] = {0}; GetClassNameA(h, cls, 127);
    if (!strcmp(cls, "Shell_TrayWnd") || !strcmp(cls, "Progman") || !strcmp(cls, "WorkerW") ||
        !strcmp(cls, "Shell_SecondaryTrayWnd")) return TRUE;
    g_hidden.push_back(h);
    ShowWindow(h, SW_HIDE);
    return TRUE;
}
static void restoreHidden() {
    for (HWND h : g_hidden) if (IsWindow(h)) ShowWindow(h, SW_SHOWNA);
    g_hidden.clear();
}
value hide_windows(value /*arg*/) { restoreHidden(); EnumWindows(enumOthers, 0); return alloc_int((int)g_hidden.size()); }
DEFINE_PRIME1(hide_windows);
value show_windows(value /*arg*/) { restoreHidden(); return alloc_bool(true); }
DEFINE_PRIME1(show_windows);
 
BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_DETACH) restoreHidden();
    return TRUE;
}
 
extern "C" void sportedout_windows_main() {}
DEFINE_ENTRY_POINT(sportedout_windows_main);