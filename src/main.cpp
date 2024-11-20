#include <windows.h>
<<<<<<< HEAD
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

using namespace Microsoft::WRL;

=======
#include <iostream>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include "Renderer.h"
#include <stdexcept>

using namespace Microsoft::WRL;

// 窗口过程函数
>>>>>>> 78dd9f2 (finish the first step of this whole project)
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

<<<<<<< HEAD
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    // 创建窗口
=======
// 入口点函数
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    // 创建控制台窗口
    if (AllocConsole()) {
        freopen("CONOUT$", "w", stdout);  // 重定向标准输出到控制台
        freopen("CONOUT$", "w", stderr);  // 重定向标准错误输出到控制台
        std::cout << "Console window is now open!" << std::endl;  // 输出测试信息
    }

    // 注册窗口类
>>>>>>> 78dd9f2 (finish the first step of this whole project)
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"D3D12Renderer";

    RegisterClass(&wc);

<<<<<<< HEAD
=======
    // 创建窗口
>>>>>>> 78dd9f2 (finish the first step of this whole project)
    HWND hwnd = CreateWindowExW(  // 使用宽字符版本
        0, L"D3D12Renderer", L"Direct3D 12 Renderer",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720,
        nullptr, nullptr, hInstance, nullptr);

    ShowWindow(hwnd, nShowCmd);

<<<<<<< HEAD
=======
    // 创建渲染器实例并初始化
    Renderer renderer;
    try {
        std::cout << "Before initializing renderer" << std::endl;
        renderer.Initialize(hwnd);  // 初始化渲染器
        std::cout << "Renderer initialized successfully!" << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Initialization Failed: " << e.what() << std::endl;
        MessageBoxA(hwnd, e.what(), "Initialization Failed", MB_OK | MB_ICONERROR);
        return -1;
    }

>>>>>>> 78dd9f2 (finish the first step of this whole project)
    // 主消息循环
    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
<<<<<<< HEAD
=======

        // 每一帧渲染
        renderer.Render();
>>>>>>> 78dd9f2 (finish the first step of this whole project)
    }

    return 0;
}
