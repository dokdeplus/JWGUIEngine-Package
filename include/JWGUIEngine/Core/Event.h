#pragma once
#include "Export.h"
#include <functional>
#include <string>
#include <vector>

namespace jwgui {

class UIRenderView;

// Base args. The constructors are in the header so app code can raise an
// event of its own control with the engine's argument types.
struct EventArgs {
    virtual ~EventArgs() = default;
};

// Input Events
struct MouseEventArgs : public EventArgs {
    float x, y;
    int button; // 0=Left, 1=Right, 2=Middle
    MouseEventArgs(float _x, float _y, int _b = 0) : x(_x), y(_y), button(_b) {}
};

struct KeyEventArgs : public EventArgs {
    int keyCode;
    KeyEventArgs(int code) : keyCode(code) {}
};

struct CharEventArgs : public EventArgs {
    wchar_t charCode;
    CharEventArgs(wchar_t code) : charCode(code) {}
};

// Control Events
struct ToggleEventArgs : public EventArgs {
    bool isChecked;
    ToggleEventArgs(bool c) : isChecked(c) {}
};

struct SelectionEventArgs : public EventArgs {
    int selectedIndex;
    SelectionEventArgs(int idx) : selectedIndex(idx) {}
};

struct TabDragEventArgs : public EventArgs {
    int tabIndex = -1;
    float grabOffsetX = 0.0f;
    float grabOffsetY = 0.0f;
    TabDragEventArgs(int idx, float gx, float gy) : tabIndex(idx), grabOffsetX(gx), grabOffsetY(gy) {}
};

class UITreeViewItem;

struct TreeSelectionEventArgs : public EventArgs {
    UITreeViewItem* item = nullptr;
    float x = 0;
    float y = 0;
    TreeSelectionEventArgs(UITreeViewItem* i, float px = 0, float py = 0) : item(i), x(px), y(py) {}
};

// Where dragged tree items were dropped, relative to target.
enum class TreeDropPosition { Before, After, Inside };

struct TreeDropEventArgs : public EventArgs {
    UITreeViewItem* target = nullptr;
    TreeDropPosition position = TreeDropPosition::Inside;
    TreeDropEventArgs(UITreeViewItem* t, TreeDropPosition p) : target(t), position(p) {}
};

struct TreeExpandEventArgs : public EventArgs {
    UITreeViewItem* item = nullptr;
    bool expanded = false;
    TreeExpandEventArgs(UITreeViewItem* i, bool e) : item(i), expanded(e) {}
};

struct ValueChangedEventArgs : public EventArgs {
    int value;
    ValueChangedEventArgs(int v) : value(v) {}
};

enum class MessageBoxResult {
    Ok,
    Yes,
    No,
    Cancel
};

struct MessageBoxResultEventArgs : public EventArgs {
    MessageBoxResult result;
    MessageBoxResultEventArgs(MessageBoxResult r) : result(r) {}
};

// A gauge's value moved into another zone (GaugeScale zones, by name;
// empty = outside every zone).
struct GaugeZoneEventArgs : public EventArgs {
    std::string oldZone;
    std::string newZone;
    double value = 0.0;
    GaugeZoneEventArgs(std::string from, std::string to, double v)
        : oldZone(std::move(from)), newZone(std::move(to)), value(v) {}
};

// Strongly-typed Event Handler. Header-only, so app code can declare events
// with its own argument types (subclassed controls).
template <typename T>
class EventHandler {
public:
    using Callback = std::function<void(UIRenderView* sender, const T& args)>;

    void AddListener(Callback cb) {
        if (cb) m_callbacks.push_back(std::move(cb));
    }
    void Invoke(UIRenderView* sender, const T& args) {
        // A copy: a callback may add or clear listeners.
        auto callbacks = m_callbacks;
        for (auto& cb : callbacks) {
            if (cb) cb(sender, args);
        }
    }
    void Clear() { m_callbacks.clear(); }
    // No listener: a sender can skip building costly arguments.
    bool IsEmpty() const { return m_callbacks.empty(); }

private:
    std::vector<Callback> m_callbacks;
};

} // namespace jwgui
