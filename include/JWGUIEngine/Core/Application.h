#pragma once

#include "CoreTypes.h"
#include "IPlatformHost.h"
#include "IRenderer.h"
#include "UILocale.h"
#include "UIStrings.h"
#include "UIRenderView.h"
#include "UIStyle.h"
#include "TimerQueue.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace jwgui {

class ImageManager;
class ResourceManager;

class JWGUI_API Application : public IWindowEventHandler {
public:
    Application();
    virtual ~Application();

    // 사용자가 재정의하여 초기 설정을 수행할 수 있도록 합니다. (RootView 설정 등)
    virtual void OnInit() = 0;

    // 애플리케이션 실행 진입점 (라이브러리 내부에서 호출됨)
    int Run();

    void BroadcastThemeChanged();

    std::shared_ptr<IPlatformHost> GetHost() const { return m_host; }
    std::shared_ptr<IRenderer> GetRenderer() const { return m_renderer; }
    std::shared_ptr<UIRenderView> GetRootView() const { return m_rootView; }
    std::shared_ptr<UIStyle> GetDefaultStyle() const { return m_defaultStyle; }
    ImageManager* GetImageManager() const { return m_imageManager.get(); }
    ResourceManager* GetResources() const { return m_resourceManager.get(); }

    // 루트 뷰를 설정합니다. OnInit() 내에서 호출해야 합니다.
    void SetRootView(std::shared_ptr<UIRenderView> rootView) { m_rootView = rootView; }

    // Device pixels per layout unit of the main window (DPI / 96,
    // devicePixelRatio; JWGUI_SCALE forces it). Views, layout, styles and
    // events are all in layout units; only hosts see device pixels.
    float GetScale() const { return m_scale; }

    // Screen points in layout units: desktop pixels divided by the main
    // window's scale, so they add up with client points and window sizes.
    void ClientToScreen(float& x, float& y) const;
    void ScreenToClient(float& x, float& y) const;
    void OwnedClientToScreen(void* handle, float& x, float& y) const;
    void GetCursorScreenPosition(float& x, float& y) const;

    // Owned OS windows used by floating dock widgets. Handle is platform HWND.
    // Positions are screen points and sizes layout units (see above).
    void* CreateOwnedWindow(const WindowCreateOptions& options, std::shared_ptr<UIRenderView> root);
    void DestroyOwnedWindow(void* handle);
    void SetOwnedWindowPos(void* handle, int x, int y, int w, int h);
    // Puts the window's client area (its top-left) at this screen point.
    void SetOwnedWindowClientPos(void* handle, float x, float y);
    void ShowOwnedWindow(void* handle, bool show);
    void SetOwnedWindowClosedHandler(void* handle, std::function<void()> onClosed);
    void InvalidateAllWindows();
    void NotePartialInvalidate();
    void SetHoverFrozen(bool freeze);

    // Sets the OS cursor directly (splitter sashes, dock sashes, editor canvas).
    // Use this instead of IPlatformHost::SetMouseCursor so the view cursor is
    // applied again once the pointer moves onto another view.
    void SetHostCursor(CursorType cursor);
    // Applies the hovered view's cursor now, even if it has not changed.
    void RefreshViewCursor();

    // 창 닫기(X 버튼, Alt+F4, RequestClose) 직전에 호출됩니다. false를 반환하면 닫지 않습니다.
    // 예: 저장하지 않은 변경이 있으면 확인 대화상자를 띄우고 false 반환 후, 결과에 따라 Quit().
    void SetCloseRequestHandler(std::function<bool()> handler) { m_closeRequestHandler = std::move(handler); }
    // App-wide shortcuts. Called for every key press before the focused view;
    // return true to consume the key.
    void SetKeyDownHandler(std::function<bool(KeyCode, const KeyModifiers&)> handler) { m_keyDownHandler = std::move(handler); }
    // 닫기 요청을 보냅니다. 핸들러가 거부할 수 있습니다.
    void RequestClose();
    // 핸들러를 거치지 않고 앱을 종료합니다.
    void Quit();

    // 싱글톤 인스턴스 접근
    static Application* Get() { return s_instance; }

    // Timers on the UI thread. delayMs / intervalMs in ms. With an owner the
    // timer stops by itself once that view is destroyed. Returns an id for
    // ClearTimer (0 when fn is empty).
    TimerId SetTimeout(unsigned int delayMs, std::function<void()> fn,
                       const std::shared_ptr<UIRenderView>& owner = nullptr);
    // alignToWall: runs on multiples of intervalMs of the time of day, so
    // clocks with intervalMs 1000 change together, right on the second.
    TimerId SetInterval(unsigned int intervalMs, std::function<void()> fn,
                        const std::shared_ptr<UIRenderView>& owner = nullptr, bool alignToWall = false);
    void ClearTimer(TimerId id);

    // From any thread: `fn` runs soon on the UI thread, in the order posted.
    // The way for a worker thread to hand its result to the views.
    void Post(std::function<void()> fn);
    // `work` runs on a worker thread (a small pool); it must not touch views,
    // styles or images. `done` then runs on the UI thread. In the browser
    // (one thread) `work` runs on the UI thread a moment later instead, so
    // keep each piece short there. Work still waiting when the app ends is
    // dropped, and its `done` never runs.
    void RunAsync(std::function<void()> work, std::function<void()> done = {});

    // Files or folders dropped on the main window (UTF-8 paths) and the point
    // they fell on, in layout units. Setting a handler turns dropping on.
    void SetDropFilesHandler(std::function<void(const std::vector<std::string>& paths, float x, float y)> handler);

    // Keep a view alive until the current input/paint event finishes.
    // Popups closed from a child click would otherwise be destroyed mid-stack.
    void KeepAlive(std::shared_ptr<UIRenderView> view);

protected:
    // Window 옵션을 변경하고 싶을 때 OnInit() 내부에서 호출할 수 있습니다.
    void SetWindowOptions(const WindowCreateOptions& options) { m_options = options; }
    const WindowCreateOptions& GetWindowOptions() const { return m_options; }

    // IWindowEventHandler 구현
    void OnPaint() override;
    void OnResize(int width, int height) override;
    void OnScaleChanged(float scale) override;
    void OnMouseMove(float x, float y) override;
    void OnMouseLeaveWindow() override;
    void OnMouseDown(float x, float y, MouseButton button) override;
    void OnMouseUp(float x, float y, MouseButton button) override;
    void OnMouseWheel(float delta, float x, float y, const KeyModifiers& modifiers) override;
    void OnMagnify(float scale, float x, float y) override;
    void OnKeyDown(KeyCode key, const KeyModifiers& modifiers) override;
    void OnChar(unsigned int charCode) override;
    void OnImeComposition(const std::string& compStr) override;
    void OnTimer(unsigned int timerId) override;
    void OnFrameTick(double nowMs) override;
    void OnDestroy() override;
    bool OnCloseRequested() override;
    void OnDropFiles(const std::vector<std::string>& paths, float x, float y) override;
    void OnPosted() override;

private:
    static Application* s_instance;

    std::shared_ptr<IPlatformHost> m_host;
    std::shared_ptr<IRenderer> m_renderer;
    std::shared_ptr<UIRenderView> m_rootView;
    std::shared_ptr<UIStyle> m_defaultStyle;
    std::unique_ptr<ImageManager> m_imageManager;
    std::unique_ptr<ResourceManager> m_resourceManager;
    WindowCreateOptions m_options;
    float m_scale = 1.0f;
    std::vector<std::shared_ptr<UIRenderView>> m_eventKeepAlive;
    bool m_partialInvalidateThisEvent = false;
    bool m_hoverFrozen = false;
    bool m_quitting = false;
    std::function<bool()> m_closeRequestHandler;
    std::function<bool(KeyCode, const KeyModifiers&)> m_keyDownHandler;
    std::function<void(const std::vector<std::string>&, float, float)> m_dropFilesHandler;
    // Post's queue and RunAsync's worker threads.
    struct AsyncState;
    std::unique_ptr<AsyncState> m_async;
    // View cursor (UIRenderView::SetMouseCursor). The pressed view keeps it
    // until the button is released, like OS mouse capture.
    CursorType m_viewCursor = CursorType::Arrow;
    std::weak_ptr<UIRenderView> m_cursorCapture;
    // Compared only, never dereferenced. With m_hostCursorOverridden it tells
    // when a direct SetHostCursor must give way to the view cursor again.
    const UIRenderView* m_cursorSource = nullptr;
    bool m_hostCursorOverridden = false;

    struct OwnedSurface;
    std::vector<std::shared_ptr<OwnedSurface>> m_ownedSurfaces;

    // SetTimeout / SetInterval share one host timer, armed for the earliest one.
    static constexpr unsigned int kTimerQueueId = 0x4A570001u;
    TimerQueue m_timers;
    bool m_timerArmed = false;
    void ArmTimerQueue();

    void FlushKeepAlive();
    // Takes the host's scale (renderer too); true when it changed.
    bool SyncScale();
    // The root view over the client area (device pixels in).
    void LayoutRoot(int pixelWidth, int pixelHeight);
    void UpdateViewCursor();
    void BeginCursorCapture();
    void EndCursorCapture();
    // Mouse up only reaches views the pointer is over or that are pressed, from
    // the root down. A control released outside a parent that is neither (a
    // Toolbox button dragged onto the canvas) never hears it and stays Pressed;
    // this hands the release to it directly. Call before EndCursorCapture.
    void ReleaseStrandedPress(UIRenderView* root, float x, float y, MouseButton button);
    // Remembers the view under a mouse down in the window's input state (root
    // = the window's root view). Not the hovered view: hover can be cleared
    // (mouse leave) or absent (touch, pen).
    void NotePressTarget(UIRenderView* root, float x, float y);
    // The focused view of the main window.
    UIRenderView* MainFocusedView() const;
    OwnedSurface* FindOwnedSurface(void* handle);
    void DestroyOwnedSurface(OwnedSurface* surface, bool destroyHwnd);
};

} // namespace jwgui

