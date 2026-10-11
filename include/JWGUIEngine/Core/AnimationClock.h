#pragma once

#include "Export.h"
#include "UIStyle.h"
#include <functional>
#include <memory>
#include <vector>

namespace jwgui {

class UIRenderView;
class UIRenderItem;
class IPlatformHost;

enum class Easing {
    Linear = 0,
    EaseOutCubic = 1,
    EaseInOutCubic = 2
};

enum class AnimProperty {
    Opacity = 0,
    TranslateX = 1,
    TranslateY = 2,
    Scale = 3,
    Color = 4,
    Custom = 5
};

constexpr float kAnimColorHoverMs = 220.0f;
constexpr float kAnimColorPressMs = 160.0f;
constexpr float kAnimOverlayMs = 160.0f;
constexpr float kAnimSwitchThumbMs = 140.0f;
constexpr float kAnimOverlayScaleFrom = 0.96f;

class JWGUI_API AnimationClock {
public:
    static AnimationClock& Get();
    static double NowMs();
    static float ApplyEasing(Easing easing, float t);
    static uint32_t LerpArgb(uint32_t from, uint32_t to, float t);
    static float ColorTransitionMs(UIState state);
    static bool IsEnabled(const UIRenderView* view);
    static float MotionMs(const UIRenderView* view, float fallbackMs);
    static float ColorMs(const UIRenderView* view, UIState state, const UIRenderItem* item = nullptr);
    static Easing ColorEasing(const UIRenderView* view, UIState state, const UIRenderItem* item = nullptr);

    AnimationClock(const AnimationClock&) = delete;
    AnimationClock& operator=(const AnimationClock&) = delete;
    ~AnimationClock();

    void BindHost(IPlatformHost* host);
    void Unbind();

    void Tick(double nowMs);
    bool HasActive() const;

    void AnimateFloat(std::shared_ptr<UIRenderView> view, AnimProperty prop,
                      float from, float to, float durationMs, Easing easing,
                      std::function<void()> onDone = {});
    void AnimateColor(std::shared_ptr<UIRenderView> view, std::shared_ptr<UIRenderItem> item,
                      UIStylePropertyID prop, uint32_t from, uint32_t to,
                      float durationMs, Easing easing);
    void AnimateCustom(std::shared_ptr<UIRenderView> view, float from, float to,
                       float durationMs, Easing easing,
                       std::function<void(float)> apply,
                       std::function<void()> onDone = {});

    void Cancel(UIRenderView* view);
    void Cancel(UIRenderView* view, AnimProperty prop);

private:
    AnimationClock() = default;

    struct Animation;
    IPlatformHost* m_host = nullptr;
    std::vector<std::shared_ptr<Animation>> m_anims;
    std::vector<std::shared_ptr<Animation>> m_pendingAdd;
    bool m_ticking = false;
    bool m_clockRunning = false;

    void Add(std::shared_ptr<Animation> anim);
    void RemoveMatching(UIRenderView* view, bool matchProp, AnimProperty prop);
    void SyncClock();
};

class JWGUI_API OverlayAnimation {
public:
    static void PlayEnter(const std::shared_ptr<UIRenderView>& view, bool withScale);
    static void PlayExit(const std::shared_ptr<UIRenderView>& view, bool withScale,
                         std::function<void()> onRemoved);
};

} // namespace jwgui
