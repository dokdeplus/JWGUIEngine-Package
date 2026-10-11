#pragma once

#include "UIPanel.h"
#include "UIEditBox.h"
#include "../Core/Event.h"
#include <memory>
#include <string>

namespace jwgui {

class JWGUI_API UISpinBox : public UIPanel {
public:
    UISpinBox(float x, float y, float w, float h);
    ~UISpinBox() override = default;

    void SetFocus() override;
    bool IsTabStop() const override { return !IsDisabled(); }

    void SetValue(double value);
    double GetValue() const { return m_value; }

    void SetRange(double minValue, double maxValue);
    double GetMinimum() const { return m_min; }
    double GetMaximum() const { return m_max; }

    void SetStep(double step);
    double GetStep() const { return m_step; }

    void SetDecimals(int decimals);
    int GetDecimals() const { return m_decimals; }

    EventHandler<double> ValueChangedEvent;

    void SetStyle(std::shared_ptr<UIStyle> style) override;
    void UpdateCachedStyles() override;
    void RenderContent(IRenderer* renderer, float absX, float absY) override;
    bool OnMouseWheel(float delta, float x, float y, const KeyModifiers& modifiers) override;
    bool OnMouseUp(float x, float y, MouseButton button) override;
    void OnTimer(unsigned int timerId) override;

    void StepBy(int direction);

private:
    class Buttons;

    std::shared_ptr<UIEditBox> m_edit;
    std::shared_ptr<Buttons> m_buttons;

    double m_value = 0.0;
    double m_min = -1.0e12;
    double m_max = 1.0e12;
    double m_step = 1.0;
    int m_decimals = 0;

    bool m_syncingText = false;
    int m_repeatDir = 0;
    bool m_repeatFast = false;

    static const unsigned int REPEAT_TIMER_ID = 1002;

    void ApplyChildStyles();
    void SyncFocusChrome();
    void SyncTextFromValue();
    void CommitFromText();
    double ClampValue(double value) const;
    double RoundValue(double value) const;
    std::string FormatValue(double value) const;
    bool CharAllowed(unsigned int charCode) const;
    void StartRepeat(int direction);
    void StopRepeat();
    bool RepeatButtonHeld() const;
};

} // namespace jwgui
