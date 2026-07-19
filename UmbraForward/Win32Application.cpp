#include "stdafx.h"
#include "Win32Application.h"

HWND Win32Application::m_hwnd = nullptr;

int Win32Application::Run(RenderAppBase* pRBase, HINSTANCE hInstance, int nCmdShow)
{
    // Parse the command line parameters
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    pRBase->ParseCommandLineArgs(argv, argc);
    LocalFree(argv);

    // Initialize the window class.
    WNDCLASSEXW windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = L"UmbraForwardClass";
    RegisterClassExW(&windowClass);

    RECT windowRect = { 0, 0, static_cast<LONG>(pRBase->GetWidth()), static_cast<LONG>(pRBase->GetHeight()) };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    m_hwnd = CreateWindowW(
        windowClass.lpszClassName,
        L"Umbra Forward",
        WS_OVERLAPPEDWINDOW,           
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr,
        nullptr,
        hInstance,
        pRBase);

    // Initialize the sample.
    pRBase->OnInit();

    ShowWindow(m_hwnd, nCmdShow);

    // Main sample loop.
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            pRBase->OnUpdate();
            pRBase->OnRender();
        }
    }

    pRBase->OnDestroy();

    return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK Win32Application::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    RenderAppBase* pSample = reinterpret_cast<RenderAppBase*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (message)
    {
    case WM_CREATE:
    {
        LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
    }
    return 0;

    case WM_KEYDOWN:
        if (pSample)
        {
            pSample->OnKeyDown(static_cast<UINT8>(wParam));
        }
        return 0;

    case WM_KEYUP:
        if (pSample)
        {
            pSample->OnKeyUp(static_cast<UINT8>(wParam));
        }
        return 0;


    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}