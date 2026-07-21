// UmbraForward.cpp: 定义应用程序的入口点。
//

#include "stdafx.h"
#include "SampleTriangle.h"


_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    // Create the sample application and run it. The previous implementation
    // returned immediately which caused no window to be shown.
    SampleTriangle sample(1280, 720, L"Umbra Forward");
    return Win32Application::Run(&sample, hInstance, nCmdShow);
}

