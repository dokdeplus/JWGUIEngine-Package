#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include "Export.h"
#include "CoreTypes.h"
#include "UIRenderItem.h"
#include "AnimationClock.h"

namespace jwgui {

class UIScrollBarItem;

class JWGUI_API UIRenderView : public std::enable_shared_from_this<UIRenderView> {
public:
    UIRenderView(float x, float y, float w, float h);
    virtual ~UIRenderView();

    virtual void OnLayout();
    // Once, from Initialize: build parts and defaults here rather than in the
    // constructor, so subclasses can change them.
    virtual void OnCreate() {}
    virtual void BuildRenderItems();
    virtual void UpdateCachedStyles();

    virtual void AddChild(std::shared_ptr<UIRenderView> child);
    virtual void RemoveChild(std::shared_ptr<UIRenderView> child);
    // Adds `child` in front of `before`, a child of this view; false (and
    // nothing changes) when `before` is not one.
    bool InsertChildBefore(std::shared_ptr<UIRenderView> child, const UIRenderView* before);
    UIRenderView* GetParent() const { return m_parent; }
    const std::vector<std::shared_ptr<UIRenderView>>& GetChildren() const { return m_children; }
    
    // Paints the view at its parent's origin: the compositor part (culling,
    // opacity, visual transform) around RenderContent. Overriding this is
    // allowed but skips that part; a control paints in RenderContent.
    virtual void RenderAll(IRenderer* renderer, float offsetX, float offsetY);
    // The view's own painting at (absX, absY), its top-left on screen: the
    // shadow, clip, items, children, outlines and scroll bars by default.
    virtual void RenderContent(IRenderer* renderer, float absX, float absY);

    virtual bool OnMouseDown(float x, float y, MouseButton button);
    virtual bool OnMouseUp(float x, float y, MouseButton button);
    // delta is in wheel notches (positive = away from the user); trackpads and
    // smooth wheels send fractions. Shift+wheel scrolls sideways.
    virtual bool OnMouseWheel(float delta, float x, float y, const KeyModifiers& modifiers);
    // Trackpad pinch: scale factor since the last event. Default: children
    // under the point, then this view's own zoom if enabled.
    virtual bool OnMagnify(float scale, float x, float y);
    virtual bool OnMouseMove(float x, float y);
    virtual void OnGlobalMouseDown(float x, float y);
    
    virtual void OnMouseEnter();
    virtual void OnMouseLeave();

    // Keyboard and IME events
    virtual bool OnKeyDown(KeyCode key, const KeyModifiers& modifiers);
    virtual bool OnChar(unsigned int charCode);
    virtual bool OnImeComposition(const std::string& compStr);
    // A tick of a timer this view started (StartTimer), or of a static one
    // (SetAnimationTimer, every view hears those). Call the base: the tool
    // tip's delay is one of them.
    virtual void OnTimer(unsigned int timerId);

    enum class ScrollbarVisibility {
        Auto,
        Visible,
        Hidden
    };

    void SetContentSize(float w, float h);
    void GetContentSize(float& w, float& h) const;

    void SetScrollOffset(float x, float y);
    void GetScrollOffset(float& x, float& y) const;

    void SetScale(float scale);
    float GetScale() const;

    void SetZoomEnabled(bool enable);
    bool IsZoomEnabled() const;

    void SetScrollbarVisibility(ScrollbarVisibility h, ScrollbarVisibility v);
    void SetScrollbarThickness(float thickness);
    float GetScrollbarThickness() const;
    bool ShowsHScroll() const;
    bool ShowsVScroll() const;

    // --- Focus, hover and the pressed view: kept per window ---
    // A window is a tree with its own root (the main window, a floating dock
    // window). Each keeps which of its views has the focus, is under the
    // pointer and took the last mouse down, so two windows never share a
    // focus. A view reaches its window's state through its root.
    struct WindowInput {
        UIRenderView* focused = nullptr;
        UIRenderView* hovered = nullptr;
        std::weak_ptr<UIRenderView> pressed;  // the view under the last mouse down
    };
    UIRenderView* GetRoot();
    const UIRenderView* GetRoot() const;
    // This window's state, made on the root when first needed.
    WindowInput& WindowInputState();
    UIRenderView* WindowFocusedView() const;
    UIRenderView* WindowHoveredView() const;
    // The window that last took input (a mouse down, a key, SetFocus): what
    // the static GetFocusedView answers for.
    static void SetActiveWindow(UIRenderView* root);
    static UIRenderView* GetActiveWindow();

    virtual void SetFocus();
    virtual void KillFocus();
    virtual bool IsFocused() const;
    // Runs once when the focus arrives or leaves (a caret to show, a timer to
    // stop); repeated SetFocus / KillFocus calls do not reach it.
    virtual void OnFocusChanged(bool focused) { (void)focused; }
    virtual bool IsTabStop() const { return false; }
    // While focused, Tab and Shift+Tab go to OnKeyDown instead of moving the
    // focus (a code editor indents with them).
    virtual bool AcceptsTab() const { return false; }
    // The focused view of the active window; the hovered view of the window
    // under the pointer.
    static UIRenderView* GetFocusedView();
    static UIRenderView* GetHoveredView();
    static void FocusAdjacent(bool reverse);
    static void FocusAdjacent(UIRenderView* root, bool reverse);
    
    virtual void DumpTree(int depth = 0) const;

    static void UpdateHover(UIRenderView* rootView, float x, float y);
    // Hover leaves the window (the pointer left it; null = the window under
    // the pointer).
    static void ClearHover(UIRenderView* rootView = nullptr);

    void SetRect(const UIRect& rect);
    UIRect GetRect() const;
    UIRect GetAbsoluteRect() const;
    // A point given to the topmost ancestor (window coordinates for the root),
    // in this view's local space, through each ancestor's scroll and zoom the
    // way mouse events are routed.
    void MapFromRoot(float x, float y, float& outX, float& outY) const;
    // Painted screen box: ancestor scroll and scale (popups, overlay hit).
    UIRect GetScreenRect() const;

    void BringToFront();

    // Inside this view's box (own coordinates); the right and bottom edges
    // belong to the next view.
    bool HitTest(float x, float y) const;
    // The scroll bar a point of this view lies on, if any.
    enum class ScrollbarHit { None, Horizontal, Vertical };
    ScrollbarHit HitScrollbarAt(float x, float y) const;
    bool HitScrollbar(float x, float y) const { return HitScrollbarAt(x, y) != ScrollbarHit::None; }
    bool IsDraggingScroll() const;
    virtual UIRenderView* FindHoveredView(float x, float y);
    
    virtual void SetState(UIState state);
    UIState GetState() const { return m_state; }
    bool HasState(UIState flag) const { return Has(m_state, flag); }
    bool IsHovered() const { return HasState(UIState::Hovered); }
    virtual bool IsPressed() const { return HasState(UIState::Pressed); }
    bool IsSelected() const { return HasState(UIState::Selected); }
    bool IsDisabled() const { return HasState(UIState::Disabled); }
    // A disabled view and everything under it look disabled and get no mouse
    // input, focus or Tab stop: SetEnabled sets the view's own flag, and the
    // Disabled state bit is the effective one (the view or an ancestor is
    // disabled). SetState keeps the bit; only SetEnabled changes it.
    void SetEnabled(bool enabled);
    bool IsEnabled() const { return !IsDisabled(); }
    bool IsEnabledSelf() const { return m_enabledSelf; }
    void AddState(UIState flag);
    void RemoveState(UIState flag);

    virtual void SetStyle(std::shared_ptr<UIStyle> style);
    std::shared_ptr<UIStyle> GetStyle() const { return m_style; }

    void SetLocalNone(UIStylePropertyID prop, UIState stateMask = UIState::Normal);
    void SetLocalColor(UIStylePropertyID prop, uint32_t color, UIState stateMask = UIState::Normal);
    void SetLocalFloat(UIStylePropertyID prop, float value, UIState stateMask = UIState::Normal);
    void SetLocalBackground(UIStylePropertyID prop, const UIBackground& bg, UIState stateMask = UIState::Normal);
    void SetLocalThickness(UIStylePropertyID prop, const Thickness& value, UIState stateMask = UIState::Normal);
    void SetLocalString(UIStylePropertyID prop, const std::string& value, UIState stateMask = UIState::Normal);
    void SetLocalCornerRadius(UIStylePropertyID prop, const CornerRadius& value, UIState stateMask = UIState::Normal);
    void SetLocalBoxShadow(UIStylePropertyID prop, const UIBoxShadow& value, UIState stateMask = UIState::Normal);
    void ClearLocal();
    // The control's own rules from the scene (UISceneNode::localStyle; see
    // UILocalStyle): per part, read before the sheet. Null = none.
    void SetLocalStyle(std::shared_ptr<const UILocalStyle> style);
    const std::shared_ptr<const UILocalStyle>& GetLocalStyle() const { return m_localStyle; }
    // The color this view draws `part` with now, in the Normal state ("" =
    // the view itself), for the editor's color swatches. A Background gives
    // its solid color (a gradient: its first stop). Controls whose parts fall
    // back to other parts (DigitalClock.hours -> .segment) answer for them.
    virtual bool ResolvePartColor(const std::string& part, UIStylePropertyID prop, uint32_t& out) const;
    // The same in another state (Selected: a switch that is on), from the
    // style rules alone.
    bool ResolveStateColor(const std::string& part, UIStylePropertyID prop, UIState state, uint32_t& out) const;
    // Any style value `part` resolves to in `state` (a border width, a
    // radius, a padding), as the style rules give it: for the editor's rows.
    bool ResolvePartValue(const std::string& part, UIStylePropertyID prop, UIState state,
                          UIStylePropertyValue& out) const;
    // A part's value as the rules give it in this view's state, remembered
    // until the style or the state changes: for views that paint parts
    // themselves every frame (ListView's header, CodeEdit's gutter and
    // tokens). "" = the view itself. partOnly: only rules that name the part,
    // no fall back to the view's own value.
    bool CachedPartValue(const std::string& part, UIStylePropertyID prop, bool partOnly,
                         UIStylePropertyValue& out) const;
    // The same as a color: a Background gives its solid color or first stop.
    // False (or `fallback`) when the rules have none.
    bool FindCachedPartColor(const std::string& part, UIStylePropertyID prop, bool partOnly, uint32_t& out) const;
    uint32_t CachedPartColor(const std::string& part, UIStylePropertyID prop, bool partOnly, uint32_t fallback) const;
    // The same as a number (a thickness, a font size) or a string (a font
    // family); `fallback` when the rules have none or the value is another kind.
    float CachedPartFloat(const std::string& part, UIStylePropertyID prop, bool partOnly, float fallback) const;
    std::string CachedPartString(const std::string& part, UIStylePropertyID prop, bool partOnly,
                                 const std::string& fallback) const;
    // The part's background as written (solid or gradient); false when none.
    bool CachedPartBackground(const std::string& part, bool partOnly, UIBackground& out) const;
    // The root view of the window this view is in (the main window, or a
    // floating dock window): where its popups go, in the coordinates of
    // GetScreenRect. The app's root view when the view is not attached.
    std::shared_ptr<UIRenderView> GetWindowRoot();

    virtual void OnThemeChanged();
    // The values this view draws with changed: its style sheet, theme or
    // local rules, its type or class, or its state (hover, press, focus,
    // disabled). A view that paints with values of its own refills them here,
    // once, instead of asking the style while it paints; CachedPart* read
    // through the fresh cache. Called after the view's items are refreshed;
    // m_animateStyle is true while it runs for a state or focus change, so a
    // view that paints its own box can ease its colors as the items do.
    virtual void OnStyleChanged() {}

    // Size that fits this view's content (text, image, children), for
    // SizingMode::Content. False when the view has no natural size.
    virtual bool MeasureContent(float& width, float& height) const;

    using InvalidateFunc = void(*)(const UIRect* rect);
    using SetTimerFunc = void(*)(bool enable, unsigned int intervalMs, unsigned int timerId);
    enum class ToolTipAction {
        Schedule,
        Hide,
        Timer,
        Destroyed
    };
    using ToolTipHook = void(*)(UIRenderView* view, ToolTipAction action);
    static constexpr unsigned int kToolTipTimerId = 1003;
    
    static InvalidateFunc s_invalidateFunc;
    static SetTimerFunc s_setTimerFunc;
    static ToolTipHook s_toolTipHook;

    static void RequestInvalidate(const UIRect* rect = nullptr);
    // The host timer `timerId` alone: a tick then walks the whole tree (every
    // view's OnTimer). Prefer StartTimer.
    static void SetAnimationTimer(bool enable, unsigned int intervalMs, unsigned int timerId);
    // A timer this view owns: its OnTimer(timerId) gets the ticks, and so do
    // the other views that started the same id, nobody else. Start again to
    // change the interval; the host timer stays while any owner remains, and
    // a destroyed view gives its timers up.
    void StartTimer(unsigned int intervalMs, unsigned int timerId);
    void StopTimer(unsigned int timerId);
    // Application, on a tick: the views that own `timerId`, or, when none
    // does (a static SetAnimationTimer), every view under `root`.
    static void DispatchTimer(unsigned int timerId, UIRenderView* root);

    void SetId(const std::string& id) { m_id = id; }
    const std::string& GetId() const { return m_id; }

    std::shared_ptr<UIRenderView> FindById(const std::string& id) const;
    std::vector<std::shared_ptr<UIRenderView>> FindByClass(const std::string& className) const;

    template<typename T>
    std::shared_ptr<T> FindByIdAs(const std::string& id) const {
        return std::dynamic_pointer_cast<T>(FindById(id));
    }

    void SetClassName(const std::string& className) { m_className = className; }
    const std::string& GetClassName() const { return m_className; }

    void SetType(const std::string& type);
    const std::string& GetType() const { return m_type; }
    // Types this view's type builds on, nearest first (an app subclass of a
    // gauge: {"RadialGauge"}). Their style rules apply where the own type has none.
    void SetBaseTypes(std::vector<std::string> baseTypes);
    const std::vector<std::string>& GetBaseTypes() const { return m_baseTypes; }

    // Second creation step, after the constructor: calls OnCreate once, so a
    // subclass's overrides take part (a constructor cannot reach them).
    // ControlRegistry::Create calls it; code that makes a control directly
    // may call it too (later calls do nothing).
    void Initialize();
    bool IsInitialized() const { return m_initialized; }

    void SetHitTestVisible(bool visible) { m_hitTestVisible = visible; }
    bool IsHitTestVisible() const { return m_hitTestVisible; }

    void SetToolTip(const std::string& text);
    const std::string& GetToolTip() const { return m_toolTip; }

    // Mouse cursor while the pointer is over this view. Views without one use
    // their parent's (Arrow at the root). Application applies it on mouse move;
    // while a button is held, the view that got the press keeps its cursor.
    void SetMouseCursor(CursorType cursor) { m_cursor = cursor; m_hasCursor = true; }
    void ClearMouseCursor() { m_hasCursor = false; }
    bool HasMouseCursor() const { return m_hasCursor; }
    CursorType GetMouseCursor() const { return m_cursor; }
    static CursorType ResolveMouseCursor(const UIRenderView* view);

    void SetOpacity(float opacity) { m_opacity = opacity; RequestInvalidate(); }
    float GetOpacity() const { return m_opacity; }

    void Animate(AnimProperty prop, float from, float to, float durationMs,
                 Easing easing = Easing::EaseOutCubic, std::function<void()> onDone = {});
    void CancelAnimations();

    void SetClosing(bool closing) { m_closing = closing; }
    bool IsClosing() const { return m_closing; }

    // This view as a shared_ptr; null when nothing owns it that way (a view
    // on the stack in a test). For animations and callbacks that must not
    // outlive the view.
    std::shared_ptr<UIRenderView> SharedSelf() { return weak_from_this().lock(); }

    void UpdateCachedStylesAnimated();

public:
    UIRect m_rect;
    UIState m_state = UIState::Normal;
    std::shared_ptr<UIStyle> m_style;
    bool m_clipChildren = true;
    bool m_closing = false;
    bool m_animateStyle = false;
    bool m_hitTestVisible = true;
    bool m_hasCursor = false;
    CursorType m_cursor = CursorType::Arrow;
    float m_opacity = 1.0f;
    float m_visualTx = 0.0f;
    float m_visualTy = 0.0f;
    float m_visualScale = 1.0f;
    std::string m_id;
    std::string m_className;
    std::string m_type;
    std::vector<std::string> m_baseTypes;
    bool m_initialized = false;
    std::string m_toolTip;

private:
    // The tree links: read through GetParent / GetChildren, changed only by
    // AddChild, InsertChildBefore and RemoveChild. The window state is
    // declared before the children so a child's destructor still reaches it.
    UIRenderView* m_parent = nullptr;
    std::unique_ptr<WindowInput> m_windowInput;
    std::vector<std::shared_ptr<UIRenderView>> m_children;

public:
    std::vector<std::shared_ptr<UIRenderItem>> m_renderItems;
    std::unordered_map<UIStylePropertyID, std::unordered_map<uint32_t, UIStylePropertyValue>> m_localProperties;
    std::shared_ptr<const UILocalStyle> m_localStyle;
    
    CornerRadius m_cachedCornerRadius;
    UIBoxShadow m_cachedBoxShadow;
    bool m_hasBoxShadow = false;
    float m_contentWidth = 0.0f;
    float m_contentHeight = 0.0f;
    float m_scrollX = 0.0f;
    float m_scrollY = 0.0f;
    float m_scale = 1.0f;
    bool m_zoomEnabled = false;
    ScrollbarVisibility m_hScrollVisibility = ScrollbarVisibility::Auto;
    ScrollbarVisibility m_vScrollVisibility = ScrollbarVisibility::Auto;
    float m_scrollbarThickness = 10.0f;
    
    // Internal state for dragging scrollbars
    bool m_isDraggingHScroll = false;
    bool m_isDraggingVScroll = false;
    float m_dragStartY = 0.0f;
    float m_dragStartX = 0.0f;
    float m_dragStartScrollX = 0.0f;
    float m_dragStartScrollY = 0.0f;

private:
    void ApplyState(UIState state);
    // Corner radius and shadow are the view's own (items take the rest).
    void CacheViewStyles();

    // The window's state when the root has one; null otherwise (never made here).
    WindowInput* FindWindowInput() const;
    // Hover, focus and the press leave this subtree (detached or disabled),
    // with the usual leave / lose-focus calls.
    void LeaveSubtree();
    // The same without any call: the subtree is going away.
    void ForgetSubtree();
    // The child of this view that `view` is or lies under; null when none.
    UIRenderView* ChildToward(const UIRenderView* view) const;
    // Puts the Disabled bit where the own flags and the ancestors say, down
    // the subtree (after SetEnabled, or a move in the tree).
    void UpdateEffectiveEnabled();
    // Links `child` in at `index`: AddChild and InsertChildBefore.
    void AttachChild(std::shared_ptr<UIRenderView> child, size_t index);
    // A rule of this view changed. An inheritable property (text color, font)
    // reaches the views below, so their caches refresh too.
    void LocalRuleChanged(bool inheritable);
    bool m_enabledSelf = true;

    // --- Scrolling ---
    // Where the bars sit in the view's own frame; the geometry that painting,
    // hit tests and dragging share.
    struct ScrollbarLayout {
        bool horizontal = false;
        bool vertical = false;
        UIRect hRect;
        UIRect vRect;
    };
    ScrollbarLayout GetScrollbarLayout() const;
    // The bar's item, made on first use and given this frame's geometry.
    UIScrollBarItem& ScrollBarFor(ScrollbarHit bar, const ScrollbarLayout& bars);
    void PaintScrollBars(IRenderer* renderer, float absX, float absY);
    float MaxScrollX() const;
    float MaxScrollY() const;
    // Keeps the offset inside the content at the current zoom.
    void ClampScroll();
    // Zooms by `factor` with the content under (x, y) staying under it.
    void ZoomAt(float factor, float x, float y);

    std::shared_ptr<UIScrollBarItem> m_hScrollBar;
    std::shared_ptr<UIScrollBarItem> m_vScrollBar;
    ScrollbarHit m_scrollbarHover = ScrollbarHit::None;  // the bar under the pointer

    // CachedPartValue's memory, dropped in CacheViewStyles.
    struct PartCacheEntry {
        std::string part;
        UIStylePropertyID prop = 0;
        bool partOnly = false;
        bool found = false;
        UIStylePropertyValue value;
    };
    mutable std::vector<PartCacheEntry> m_partCache;
};
} // namespace jwgui
