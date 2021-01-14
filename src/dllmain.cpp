#include "gldraw.h"
#include "gltext.h"
#include "hook.h"
#include "mem.h"
#include "proc.h"
#include "replace.h"

#include <winuser.h>
#include <psapi.h>

#include <unordered_map>
#include <iostream>
#include <string>

typedef BOOL(__stdcall* twglSwapBuffers)(HDC hDc);

twglSwapBuffers owglSwapBuffers;
twglSwapBuffers wglSwapBuffersGateway;

GL::Font glFont;
const int FONT_HEIGHT = 20;
const int FONT_WIDTH = 40;

bool drawText = true;

using namespace std;

string text;
string osu_path;
string window_title;
string new_window_title;

unordered_map<string, string> edit_params = {
    {"AR", "10"},
    {"CS", "6"}
};

HWND g_HWND = NULL;
bool found_HWND = false;

BOOL CALLBACK EnumWindowsProcMy(HWND hwnd, LPARAM lParam)
{
    DWORD lpdwProcessId;
    GetWindowThreadProcessId(hwnd, &lpdwProcessId);
    if (lpdwProcessId == lParam)
    {
        g_HWND = hwnd;
        return FALSE;
    }
    return TRUE;
}

void Draw()
{
    if (!glFont.bBuilt || wglGetCurrentDC() != glFont.hdc)
    {
        glFont.Build(FONT_HEIGHT);
    }

    GL::SetupOrtho();

    glFont.Print(5, 15, rgb::green, "%s", text.c_str());

    GL::RestoreGL();
}

string GetWindowTitle()
{
    char window_title[1024];
    GetWindowTextA(g_HWND, window_title, 1024);
    return string(window_title);
}

void Input()
{
    if (GetAsyncKeyState(VK_END) & 1)
    {
        drawText = !drawText;
    }
}

string GetExePath(DWORD procID)
{
    HANDLE processHandle = NULL;
    TCHAR filename[MAX_PATH];

    processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, procID);
    GetModuleFileNameEx(processHandle, NULL, filename, MAX_PATH);
    wstring filename_w = wstring(filename);
    return string(filename_w.begin(), filename_w.end());
}

BOOL __stdcall hkwglSwapBuffers(HDC hDc)
{
    if (!found_HWND)
    {
        DWORD procID = GetProcId(L"osu!.exe");

        EnumWindows(EnumWindowsProcMy, procID);

        string osuExePath = GetExePath(procID);
        size_t backslashPos = osuExePath.rfind('\\');
        osu_path = osuExePath.substr(0, backslashPos + 1) + "Songs\\";
        text = "AR: " + edit_params["AR"] + ", CS: " + edit_params["CS"];
        found_HWND = true;
    }
    new_window_title = GetWindowTitle();
    if (window_title != new_window_title)
    {
        if (new_window_title != "osu!")
        {
            vector<string> osu_map_parts;

            split(new_window_title, "-", osu_map_parts);
            string no_osu = join(osu_map_parts.begin() + 1, osu_map_parts.end(), "-");
            no_osu.erase(0, 1);

            osu_map_parts.clear();
            split(no_osu, "[", osu_map_parts);

            string& title = osu_map_parts[0];
            title.pop_back();

            const string& diff = "[" + osu_map_parts[1];

            replace(osu_path, edit_params, title, diff);
        }
        window_title = new_window_title;
    }
    Input();
    if (drawText)
    {
        Draw();
    }
    return wglSwapBuffersGateway(hDc);
}

DWORD WINAPI HackThread(HMODULE hModule)
{
    //AllocConsole();
    //FILE* f;
    //freopen_s(&f, "CONOUT$", "w", stdout);

    Hook SwapBuffersHook("wglSwapBuffers", "opengl32.dll", (BYTE*)hkwglSwapBuffers, (BYTE*)&wglSwapBuffersGateway, 5);
    SwapBuffersHook.Enable();

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call,
    LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)HackThread,
            hModule, 0, nullptr));
    default:
        break;
    }
    return TRUE;
}