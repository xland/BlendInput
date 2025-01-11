#include <Windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#include <versionhelpers.h>
#include <blend2d/context.h>

#include "InputWindow.h"

InputWindow::InputWindow()
{
	x = 0; y = 0;
	w = 2560; h = 1000;
    initFont();
	initWindow();
}

InputWindow::~InputWindow()
{
}



void InputWindow::initWindow()
{
    WNDCLASSEX wcx{};
    auto hinstance = GetModuleHandle(NULL);
    wcx.cbSize = sizeof(wcx);
    wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcx.lpfnWndProc = &InputWindow::routeWinMsg;
    wcx.cbWndExtra = sizeof(InputWindow*);
    wcx.hInstance = hinstance;
    wcx.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcx.lpszClassName = L"BlendInput";
    auto flag = RegisterClassEx(&wcx);
    hwnd = CreateWindowEx(NULL, wcx.lpszClassName, wcx.lpszClassName, WS_POPUP, x, y, w, h, NULL, NULL, hinstance, static_cast<LPVOID>(this));
    enableAlpha();
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    img = std::make_unique<BLImage>(w * 1.5, h * 1.5, BL_FORMAT_PRGB32);
    BLContext ctx(*img.get());
    ctx.clearAll();
    ctx.fillAll(BLRgba32(0xFFFFFFFF));
    const char regularText[] = "Hello Blend2D!";
    BLPoint pos(250, 80);
    ctx.setFillStyle(BLRgba32(0xFF000000));
    ctx.fillUtf8Text(pos, *font.get(), regularText);
    ctx.end();
    img->writeToFile("allen123.png");
}

bool InputWindow::enableAlpha()
{
    if (!IsWindowsVistaOrGreater()) { return false; }
    BOOL isCompositionEnable = false;
    //检查DWM是否启用
    DwmIsCompositionEnabled(&isCompositionEnable);
    if (!isCompositionEnable) { return true; }
    DWORD currentColor = 0;
    BOOL isOpaque = false;
    //检查是否支持毛玻璃效果
    DwmGetColorizationColor(&currentColor, &isOpaque);
    if (!isOpaque || IsWindows8OrGreater())
    {
        HRGN region = CreateRectRgn(0, 0, -1, -1);
        DWM_BLURBEHIND bb = { 0 };
        bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
        bb.hRgnBlur = region;
        bb.fEnable = TRUE;
        DwmEnableBlurBehindWindow(hwnd, &bb);
        DeleteObject(region);
        return true;
    }
    else // For Window7
    {
        DWM_BLURBEHIND bb = { 0 };
        bb.dwFlags = DWM_BB_ENABLE;
        DwmEnableBlurBehindWindow(hwnd, &bb);
        return false;
    }
}

void InputWindow::show()
{
    //SetTimer(hwnd, 1001, 20, NULL);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
}

LRESULT InputWindow::routeWinMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    auto obj = reinterpret_cast<InputWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (!obj) {
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    switch (msg)
    {
        case WM_NCCALCSIZE:
        {
            if (wParam == TRUE) {
                NCCALCSIZE_PARAMS* pncsp = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);
                pncsp->rgrc[0] = pncsp->rgrc[1]; //窗口客户区覆盖整个窗口
                return 0;
            }
            return DefWindowProc(hWnd, msg, wParam, lParam);
        }
        case WM_DESTROY:
        {
            SetWindowLongPtr(hWnd, GWLP_USERDATA,NULL);
            UnregisterClass(L"BlendInput", NULL);
            PostQuitMessage(0);
            return 0;
        }
        default:
        {
            return obj->processWinMsg(msg, wParam, lParam);
        }
    }
}

LRESULT InputWindow::processWinMsg(UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_PAINT) {
        paint();
        return 0;
    }
    else if (msg == WM_TIMER) {
        if (wParam == 1001)
        {
            static bool flag = true;
            BLContext ctx(*img.get());
            ctx.clearAll();
            ctx.fillAll(flag?BLRgba32(0x66B83E52): BLRgba32(0x99B83E66));
            flag = !flag; 
            InvalidateRect(hwnd, nullptr, false);
        }
    }
    else if (msg == WM_LBUTTONDOWN) {
        auto x = GET_X_LPARAM(lParam);
        auto y = GET_Y_LPARAM(lParam);
        BLContext ctx(*img.get());
        ctx.clearAll();
        ctx.fillAll(BLRgba32(0x66B83E52));

        BLGradient linear(BLLinearGradientValues(x, y, x+100, y+100));
        linear.addStop(0.0, BLRgba32(0xFFFFFFFF));
        linear.addStop(0.5, BLRgba32(0xFF5FAFDF));
        linear.addStop(1.0, BLRgba32(0xFF2F5FDF));
        ctx.setFillStyle(linear);

        ctx.fillRect(x, y, 100, 100);
        RECT rect{ .left{x}, .top{y}, .right{x+100}, .bottom{y+100} };
        InvalidateRect(hwnd, nullptr, false);
        //InvalidateRect(hwnd, &rect, false);
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void InputWindow::initFont()
{
    BLFontFace face;
    BLResult err = face.createFromFile("C:\\Windows\\Fonts\\msyh.ttc"); //黑体
    if (err) {
        printf("Failed to load a font face (err=%u)\n", err);
        return;
    }
    font = std::make_unique<BLFont>();
    font->createFromFace(face, fontSize);
}

void InputWindow::paint()
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    BLImageData data;
    img->getData(&data);
    //LONG dirtyX{ ps.rcPaint.left }, dirtyY{ ps.rcPaint.top },
    //dirtyW{ ps.rcPaint.right - ps.rcPaint.left },
    //dirtyH{ ps.rcPaint.bottom - ps.rcPaint.top };
    BITMAPINFO bmi = { sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB, w * 4 * h, 0, 0, 0, 0 };
    //SetDIBitsToDevice(hdc, 0, 0, w, h, 0, 0, 0, h, data.pixelData, &bmi, DIB_RGB_COLORS);
    StretchDIBits(
        hdc,          // 目标设备上下文句柄
        0,        // 目标矩形左上角的 x 坐标
        0,        // 目标矩形左上角的 y 坐标
        w,    // 目标矩形的宽度
        h,   // 目标矩形的高度
        0,         // 源矩形左上角的 x 坐标
        0,         // 源矩形左上角的 y 坐标
        w,     // 源矩形的宽度
        h,    // 源矩形的高度
        data.pixelData,      // 指向位图像素数据的指针
        &bmi, // 指向位图信息结构的指针
        DIB_RGB_COLORS,       // 调色板的使用方式
        SRCCOPY           // 栅格操作码
    );
    EndPaint(hwnd, &ps);
}