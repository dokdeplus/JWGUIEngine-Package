#pragma once

#include "../Core/UIRenderView.h"
#include "../Core/Event.h"

namespace jwgui {

enum class SliderOrientation {
    Horizontal,
    Vertical
};

class JWGUI_API UISlider : public UIRenderView {
public:
    UISlider(float x, float y, float w, float h);
    ~UISlider() override = default;

    bool IsTabStop() const override { return !IsDisabled(); }

    void OnLayout() override;
    void BuildRenderItems() override;

    bool OnMouseDown(float x, float y, MouseButton button) override;
    bool OnMouseMove(float x, float y) override;
    bool OnMouseUp(float x, float y, MouseButton button) override;
    bool OnMouseWheel(float delta, float x, float y, const KeyModifiers& modifiers) override;
    bool OnKeyDown(KeyCode key, const KeyModifiers& modifiers) override;

    void SetValue(double value);
    double GetValue() const { return m_value; }

    void SetRange(double minValue, double maxValue);
    double GetMinimum() const { return m_min; }
    double GetMaximum() const { return m_max; }

    void SetStep(double step);
    double GetStep() const { return m_step; }

    void SetOrientation(SliderOrientation orientation);
    SliderOrientation GetOrientation() const { return m_orientation; }

    EventHandler<double> ValueChangedEvent;

private:
    static constexpr float kTrackThickness = 4.0f;
    static constexpr float kHandleSize = 14.0f;

    double m_value = 0.0;
    double m_min = 0.0;
    double m_max = 100.0;
    double m_step = 1.0;
    SliderOrientation m_orientation = SliderOrientation::Horizontal;

    std::shared_ptr<UIRenderItem> m_trackItem;
    std::shared_ptr<UIRenderItem> m_fillItem;
    std::shared_ptr<UIRenderItem> m_handleItem;

    void LayoutParts();
    double ClampAndSnap(double value) const;
    void SetValueFromPointer(float x, float y, bool notify);
    void Nudge(double direction, const KeyModifiers& modifiers);
    float ValueToT() const;
};

} // namespace jwgui
