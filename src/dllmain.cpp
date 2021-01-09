#include "gldraw.h"
#include "gltext.h"
#include "hook.h"
#include "mem.h"
#include "proc.h"
#include "replace.h"
#include <winuser.h>
#include <psapi.h>
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

string text = "Test asdfasdfasdf";

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

void GetWindowTitle()
{
    char windowTitle[1024];
    GetWindowTextA(g_HWND, windowTitle, 1024);
    text = string(windowTitle);
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

        text = GetExePath(procID);
        found_HWND = true;
    }
    //GetWindowTitle();
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

    //cout << "Hooked" << endl;

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call,
    LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)HackThread,
            hModule, 0, nullptr));
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}