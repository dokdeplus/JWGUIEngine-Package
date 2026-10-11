#pragma once
#include "CoreTypes.h"
#include "FileDialog.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace jwgui {

struct UILocaleData;

// JWGUI_SCALE=1.5 in the environment (?scale=1.5 in the browser) forces the
// UI scale, to try a layout at 150 % on any monitor. 0 = not set.
JWGUI_API float ForcedUIScale();

// A host speaks device pixels: client sizes, mouse points, paint and
// invalidate rects, screen points and window rects. GetScale tells how many
// device pixels make one layout unit; Application converts, so views and
// layout never see device pixels.
class JWGUI_API IPlatformHost {
public:
    virtual ~IPlatformHost() = default;

    virtual bool CreateWindowHost(
        const WindowCreateOptions& options,
        IWindowEventHandler* eventHandler) = 0;

    virtual void* GetWindowHandle() const = 0;
    virtual void Show() = 0;
    virtual void UpdateNow() = 0;
    virtual void EnableDropFiles(bool enable) = 0;

    virtual void SetOpacity(float alpha) = 0;
    virtual void SetWindowPos(int x, int y, int w, int h) = 0;
    virtual void ClientToScreen(float& x, float& y) const = 0;
    virtual void ScreenToClient(float& x, float& y) const = 0;
    virtual void GetClientSize(int& width, int& height) const = 0;

    // Owned tool windows (dock float). Handle is an HWND. Does not quit the app on destroy.
    virtual void* CreateOwnedWindow(
        const WindowCreateOptions& options,
        IWindowEventHandler* eventHandler) = 0;
    virtual void DestroyOwnedWindow(void* handle) = 0;
    virtual void ShowOwnedWindow(void* handle, bool show) = 0;
    virtual void SetOwnedWindowPos(void* handle, int x, int y, int w, int h) = 0;
    virtual void InvalidateOwnedWindow(void* handle) = 0;
    virtual void InvalidateOwnedWindowRect(void* handle, float x, float y, float w, float h) = 0;
    virtual void OwnedClientToScreen(void* handle, float& x, float& y) const = 0;
    virtual void OwnedScreenToClient(void* handle, float& x, float& y) const = 0;
    virtual void GetOwnedWindowClientSize(void* handle, int& width, int& height) const = 0;
    virtual void SetMouseCapture(void* handle) = 0;
    virtual void GetCursorScreenPosition(float& x, float& y) const = 0;
    // Moves an owned window so its client area starts at this screen point
    // (the frame, if any, goes around it).
    virtual void SetOwnedWindowClientPos(void* handle, int x, int y) { SetOwnedWindowPos(handle, x, y, 0, 0); }

    // Device pixels per layout unit: the monitor's DPI / 96 on Windows, the
    // browser's devicePixelRatio. Changes are reported with
    // IWindowEventHandler::OnScaleChanged. ForcedUIScale overrides it.
    virtual float GetScale() const { return 1.0f; }
    virtual float GetOwnedWindowScale(void* handle) const {
        (void)handle;
        return GetScale();
    }

    // Asks the main window to close, as if the user pressed its close button.
    // The handler's OnCloseRequested can still veto it. No-op where unsupported.
    virtual void RequestClose() {}
    // Main window caption (UTF-8). No-op where unsupported.
    virtual void SetTitle(const std::string& utf8) { (void)utf8; }

    virtual bool PumpEvents() = 0;
    virtual void WaitForInput(unsigned int timeoutMs) = 0;
    virtual void WakeLoop() = 0;
    virtual void RequestInvalidate() = 0;
    virtual void RequestInvalidateRect(float x, float y, float w, float h) = 0;
    virtual bool GetLastPaintRect(float& x, float& y, float& w, float& h) const = 0;
    virtual void SetAnimationTimer(bool enable, unsigned int intervalMs, unsigned int timerId) = 0;
    virtual void StartFrameClock() = 0;
    virtual void StopFrameClock() = 0;
    virtual void SetMouseCursor(CursorType cursor) = 0;

    // Child processes (editor Run). Paths and args are UTF-8; args are quoted
    // as needed. The handle is platform-specific; null on failure or where
    // unsupported (web).
    virtual void* StartProcess(const std::string& exePath, const std::vector<std::string>& args) {
        (void)exePath;
        (void)args;
        return nullptr;
    }
    virtual bool IsProcessRunning(void* process) const { (void)process; return false; }
    // Ends the process if it still runs, then releases the handle.
    virtual void StopProcess(void* process) { (void)process; }
    // Releases the handle and leaves the process running.
    virtual void ReleaseProcess(void* process) { (void)process; }

    virtual bool SetClipboardText(const std::string& utf8) = 0;
    virtual bool GetClipboardText(std::string& outUtf8) const = 0;
    // Eyedropper: the user clicks anywhere on the screen and `done` gets that
    // pixel's color (picked = false: cancelled with Esc or a right click).
    // `done` runs later, on the UI thread. Where it is not supported,
    // CanPickScreenColor is false and `done` runs at once with false.
    virtual bool CanPickScreenColor() const { return false; }
    virtual void PickScreenColor(std::function<void(bool picked, uint32_t argb)> done) {
        if (done) done(false, 0);
    }
    virtual bool IsPrimaryMouseButtonDown() const = 0;
    virtual KeyModifiers GetKeyModifiers() const = 0;

    // The OS file dialog / the browser's file picker (see FileDialog.h).
    // Where it is not supported `done` runs at once with no paths.
    virtual void ShowFileDialog(const FileDialogOptions& options, FileDialogDone done) {
        (void)options;
        if (done) done({});
    }
    // After writing the file a Save dialog named. Desktop: nothing to do (the
    // file is where the user asked). Browser: the file goes out as a download.
    virtual void DeliverSavedFile(const std::string& path) { (void)path; }

    // The main window over the whole monitor, without a frame (F11 in a
    // viewer); the browser asks for full-screen on the canvas.
    virtual void SetFullscreen(bool fullscreen) { (void)fullscreen; }
    virtual bool IsFullscreen() const { return false; }

    // From any thread: the UI thread soon gets IWindowEventHandler::OnPosted
    // (once for several requests). Application::Post is built on it.
    virtual void RequestPosted() { WakeLoop(); }

    // UI languages (UIStrings' source): the OS display languages or the
    // browser's languages, best first ("ko-KR", "en-US"); empty = unknown.
    virtual std::vector<std::string> SystemUILanguages() const { return {}; }
    // The font the OS draws its own UI with ("Segoe UI", "맑은 고딕"): the
    // engine's default font family (UIStyle::DefaultFontFamily). Empty =
    // unknown, a generic sans-serif then.
    virtual std::string SystemFontFamily() const { return std::string(); }

    // Format locale (UILocale's source). The OS / browser locale name
    // ("ko-KR"); empty = unknown.
    virtual std::string SystemLocaleName() const { return std::string(); }
    // Writes what the platform knows about a locale over `data` (name "" =
    // the system's, with the user's own region changes); false = unknown
    // there. When the OS settings change the host calls UILocale::SystemChanged.
    virtual bool QueryLocale(const std::string& name, UILocaleData& data) const {
        (void)name;
        (void)data;
        return false;
    }
};
} // namespace jwgui
