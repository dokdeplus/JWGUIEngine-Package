// A small OpenGL game made without JWGUIEngine (a Win32 window, WGL, a
// spinning cube) that shows JWGUIEngine's controls over its picture with
// UIOverlayLayer. The game keeps its window and its loop; each frame it draws
// its picture, then has the UI drawn over it. Every window message goes to
// the overlay layer first, and what the UI takes the game leaves.
//
// Click the scene to fire; Esc shows or hides the menu; Space pauses.
#include <windows.h>
#include <GL/gl.h>
#include <cmath>
#include <string>
#include "UIOverlayLayer.h"
#include "UIButton.h"
#include "UIComboBox.h"
#include "UIEditBox.h"
#include "UIPanel.h"
#include "UISlider.h"
#include "UIStatic.h"

using namespace jwgui;

namespace {

struct Game {
    float angle = 0.0f;
    float speed = 1.0f;       // turns of the cube, from the slider
    bool paused = false;
    int shots = 0;
    float flash = 0.0f;       // a shot lights the cube up for a moment
    int color = 0;            // from the combo box
};

const float kColors[3][3] = { { 0.95f, 0.55f, 0.20f }, { 0.20f, 0.70f, 0.65f }, { 0.60f, 0.40f, 0.85f } };

Game g_game;
UIOverlayLayer g_ui;
std::shared_ptr<UIPanel> g_menu;
std::shared_ptr<UIStatic> g_status;
std::shared_ptr<UIButton> g_pause;
bool g_menuShown = true;

void UpdateStatus() {
    if (!g_status) return;
    g_status->SetText("Shots: " + std::to_string(g_game.shots) + (g_game.paused ? "  (paused)" : ""));
    if (g_pause) g_pause->SetText(g_game.paused ? "Resume" : "Pause");
}

void SetPaused(bool paused) {
    g_game.paused = paused;
    UpdateStatus();
}

void ShowMenu(bool show) {
    if (show == g_menuShown || !g_menu) return;
    g_menuShown = show;
    if (show) g_ui.Root()->AddChild(g_menu);
    else g_ui.Root()->RemoveChild(g_menu);
    g_ui.Invalidate();
}

// The UI over the game: a menu panel with the engine's controls, laid out
// in a column. The overlay layer's root places it by position (layout units:
// pixels at 100 %); inside it the panel's layout places the controls.
void BuildUI() {
    g_menu = std::make_shared<UIPanel>(16.0f, 16.0f, 272.0f, 330.0f);
    g_menu->SetLayoutType(LayoutType::Flex);
    g_menu->GetLayoutNode()->SetFlexDirection(FlexDirection::Column);
    g_menu->GetLayoutNode()->SetAlignItems(AlignItems::Stretch);
    g_menu->GetLayoutNode()->SetPadding(Thickness(16.0f));
    g_menu->GetLayoutNode()->SetGap(8.0f);
    g_ui.Root()->AddChild(g_menu);
    auto add = [](std::shared_ptr<UIRenderView> view, float height) {
        g_menu->AddControl(view)->SetHeight({SizingMode::Fixed, height});
    };

    auto title = std::make_shared<UIStatic>(0.0f, 0.0f, 0.0f, 0.0f);
    title->SetText("JWGUIEngine over an OpenGL game");
    add(title, 24.0f);

    auto row = std::make_shared<UIPanel>(0.0f, 0.0f, 0.0f, 0.0f);
    row->SetDrawBackground(false);
    row->SetLayoutType(LayoutType::Flex);
    row->GetLayoutNode()->SetFlexDirection(FlexDirection::Row);
    row->GetLayoutNode()->SetAlignItems(AlignItems::Stretch);
    row->GetLayoutNode()->SetGap(8.0f);
    add(row, 32.0f);
    g_pause = std::make_shared<UIButton>(0.0f, 0.0f, 0.0f, 0.0f);
    g_pause->ClickEvent.AddListener([](UIRenderView*, const EventArgs&) { SetPaused(!g_game.paused); });
    row->AddControl(g_pause)->SetFlexGrow(1.0f);
    auto quit = std::make_shared<UIButton>(0.0f, 0.0f, 0.0f, 0.0f);
    quit->SetText("Quit");
    quit->ClickEvent.AddListener([](UIRenderView*, const EventArgs&) { PostQuitMessage(0); });
    row->AddControl(quit)->SetFlexGrow(1.0f);

    auto speedLabel = std::make_shared<UIStatic>(0.0f, 0.0f, 0.0f, 0.0f);
    speedLabel->SetText("Speed");
    add(speedLabel, 20.0f);
    auto speed = std::make_shared<UISlider>(0.0f, 0.0f, 0.0f, 0.0f);
    speed->SetRange(0.0, 4.0);
    speed->SetValue(1.0);
    speed->ValueChangedEvent.AddListener([](UIRenderView*, const double& value) {
        g_game.speed = static_cast<float>(value);
    });
    add(speed, 28.0f);

    auto colorLabel = std::make_shared<UIStatic>(0.0f, 0.0f, 0.0f, 0.0f);
    colorLabel->SetText("Cube color");
    add(colorLabel, 20.0f);
    auto color = std::make_shared<UIComboBox>(0.0f, 0.0f, 0.0f, 0.0f);
    for (const char* name : { "Orange", "Teal", "Violet" }) color->AddItem(name);
    color->SetSelectedIndex(0);
    color->SelectionChangedEvent.AddListener([](UIRenderView*, const SelectionEventArgs& args) {
        if (args.selectedIndex >= 0 && args.selectedIndex < 3) g_game.color = args.selectedIndex;
    });
    add(color, 30.0f);

    auto name = std::make_shared<UIEditBox>(0.0f, 0.0f, 0.0f, 0.0f);
    name->SetText("Player");   // typing here does not move or fire in the game
    add(name, 30.0f);

    g_status = std::make_shared<UIStatic>(0.0f, 0.0f, 0.0f, 0.0f);
    add(g_status, 24.0f);
    UpdateStatus();
    g_menu->OnLayout();
}

// --- The game's own drawing: fixed-function OpenGL ---

void DrawCube(float flash) {
    const float* c = kColors[g_game.color];
    const float shade[6] = { 1.0f, 0.8f, 0.9f, 0.7f, 0.6f, 0.85f };
    const float v[8][3] = { { -1, -1, -1 }, { 1, -1, -1 }, { 1, 1, -1 }, { -1, 1, -1 },
                            { -1, -1, 1 }, { 1, -1, 1 }, { 1, 1, 1 }, { -1, 1, 1 } };
    const int faces[6][4] = { { 4, 5, 6, 7 }, { 1, 0, 3, 2 }, { 0, 4, 7, 3 },
                              { 5, 1, 2, 6 }, { 3, 7, 6, 2 }, { 0, 1, 5, 4 } };
    glBegin(GL_QUADS);
    for (int f = 0; f < 6; ++f) {
        const float s = shade[f];
        glColor3f(std::fmin(1.0f, c[0] * s + flash), std::fmin(1.0f, c[1] * s + flash), std::fmin(1.0f, c[2] * s + flash));
        for (int i = 0; i < 4; ++i) glVertex3fv(v[faces[f][i]]);
    }
    glEnd();
}

void DrawGame(int width, int height) {
    glViewport(0, 0, width, height);
    glClearColor(0.08f, 0.10f, 0.16f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    const double aspect = height > 0 ? static_cast<double>(width) / height : 1.0;
    glFrustum(-0.1 * aspect, 0.1 * aspect, -0.1, 0.1, 0.2, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.8f, 0.0f, -6.0f);
    glRotatef(g_game.angle, 0.3f, 1.0f, 0.2f);
    DrawCube(g_game.flash);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    // The UI first: what it takes, the game does not act on.
    if (g_ui.IsStarted() && g_ui.HandleWin32Message(hwnd, message, wParam, lParam)) {
        return message == WM_SETCURSOR ? TRUE : 0;
    }
    switch (message) {
    case WM_LBUTTONDOWN:   // a click on the scene, not on the UI
        ++g_game.shots;
        g_game.flash = 0.6f;
        UpdateStatus();
        return 0;
    case WM_KEYDOWN:       // keys the UI does not want (no edit box focused)
        if (wParam == VK_ESCAPE) ShowMenu(!g_menuShown);
        else if (wParam == VK_SPACE) SetPaused(!g_game.paused);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }
}

} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR, int show) {
    // The game's own window and GL context.
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    WNDCLASSW wc = {};
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hIcon = LoadIconW(instance, MAKEINTRESOURCEW(1));
    wc.lpszClassName = L"UIOverlayLayerExample";
    RegisterClassW(&wc);
    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"UIOverlayLayer", WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, nullptr, nullptr, instance, nullptr);
    if (!hwnd) return 1;
    HDC dc = GetDC(hwnd);
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    SetPixelFormat(dc, ChoosePixelFormat(dc, &pfd), &pfd);
    HGLRC glrc = wglCreateContext(dc);
    wglMakeCurrent(dc, glrc);

    // The overlay layer, with the game's context current.
    RECT client = {};
    GetClientRect(hwnd, &client);
    UIOverlayLayerOptions options;
    options.width = client.right;
    options.height = client.bottom;
    options.window = hwnd;   // its DPI gives the scale; the IME goes through it
    std::string error;
    if (!g_ui.Start(options, &error)) {
        MessageBoxA(hwnd, error.c_str(), "UIOverlayLayer", MB_ICONERROR);
        return 1;
    }
    BuildUI();
    ShowWindow(hwnd, show);

    LARGE_INTEGER frequency, last, now;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&last);
    MSG msg = {};
    bool running = true;
    while (running) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) running = false;
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        if (!running) break;
        QueryPerformanceCounter(&now);
        const float dt = static_cast<float>(now.QuadPart - last.QuadPart) / static_cast<float>(frequency.QuadPart);
        last = now;
        if (!g_game.paused) g_game.angle += 60.0f * g_game.speed * dt;
        g_game.flash = std::fmax(0.0f, g_game.flash - 2.0f * dt);

        GetClientRect(hwnd, &client);
        DrawGame(client.right, client.bottom);
        g_ui.Render();   // the UI over the picture (the window's framebuffer)
        SwapBuffers(dc);
    }

    g_ui.Stop();   // with the context still current
    g_menu.reset();
    g_status.reset();
    g_pause.reset();
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(glrc);
    ReleaseDC(hwnd, dc);
    DestroyWindow(hwnd);
    return 0;
}
