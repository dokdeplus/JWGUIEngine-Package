#pragma once

#include "../Core/UIRenderView.h"

namespace jwgui {

class JWGUI_API UIProgressBar : public UIRenderView {
public:
    UIProgressBar(float x, float y, float w, float h);
    ~UIProgressBar() override;

    void OnLayout() override;
    void BuildRenderItems() override;

    void SetValue(double value);
    double GetValue() const { return m_value; }

    void SetRange(double minValue, double maxValue);
    double GetMinimum() const { return m_min; }
    double GetMaximum() const { return m_max; }

    void SetIndeterminate(bool indeterminate);
    bool IsIndeterminate() const { return m_indeterminate; }

private:
    static constexpr float kIndeterminateRatio = 0.32f;
    static constexpr float kIndeterminateMs = 1200.0f;

    double m_value = 0.0;
    double m_min = 0.0;
    double m_max = 100.0;
    bool m_indeterminate = false;
    float m_indetT = 0.0f;

    std::shared_ptr<UIRenderItem> m_trackItem;
    std::shared_ptr<UIRenderItem> m_fillItem;

    void LayoutParts();
    double ClampValue(double value) const;
    float ValueToT() const;
    void StartIndeterminate();
    void StopIndeterminate();
    void AnimateIndeterminatePass(const std::shared_ptr<UIRenderView>& self);
};

} // namespace jwgui
