#pragma once

#include "../Core/UIRenderView.h"
#include "DateTimeSegments.h"
#include <string>
#include <vector>

namespace jwgui {

class JWGUI_API UIDateTimeBar : public UIRenderView {
public:
    UIDateTimeBar(float x, float y, float w, float h);
    ~UIDateTimeBar() override = default;

    void SetAccessory(DateTimeAccessory accessory);
    DateTimeAccessory GetAccessory() const { return m_accessory; }
    // The text as built for the last paint ("10/4/2026", "오후 3:04:05").
    std::string GetDisplayText() const;

    void OnStyleChanged() override;
    void RenderContent(IRenderer* renderer, float absX, float absY) override;

    bool OnMouseDown(float x, float y, MouseButton button) override;
    bool OnMouseUp(float x, float y, MouseButton button) override;
    bool OnMouseWheel(float delta, float x, float y, const KeyModifiers& modifiers) override;
    bool OnKeyDown(KeyCode key, const KeyModifiers& modifiers) override;
    bool OnChar(unsigned int charCode) override;
    void OnGlobalMouseDown(float x, float y) override;
    void OnTimer(unsigned int timerId) override;
    void OnFocusChanged(bool focused) override;
    bool IsTabStop() const override { return !IsDisabled(); }

protected:
    std::vector<DateTimeSegment> m_segments;
    int m_active = -1;
    std::string m_typeBuffer;
    bool m_twelveHour = false;
    DateTimeAccessory m_accessory = DateTimeAccessory::Spin;

    uint32_t m_cachedFgColor = 0xFFFFFFFF;
    uint32_t m_cachedOutlineColor = 0xFF555555;
    uint32_t m_cachedArrowColor = 0xFFFFFFFF;
    float m_cachedOutlineThick = 1.0f;
    Thickness m_cachedPadding = Thickness(4.0f);
    float m_cachedFontSize = 14.0f;
    std::string m_cachedFontFamily = UIStyle::DefaultFontFamily();
    UIBackground m_cachedBg;
    uint32_t m_cachedSelBgColor = 0xFF264F78;

    std::vector<float> m_segX;
    std::vector<float> m_segW;

    void SelectEditable(int index);
    void ClearFieldSelection();
    void MoveActive(int dir);
    void CommitTypeBuffer();
    void ClearTypeBuffer();
    void OnSegmentsRebuilt();
    int FirstEditable() const;
    int NextEditable(int from, int dir) const;
    bool IsEditable(int index) const;
    const DateTimeSegment* ActiveSegment() const;
    std::string DisplayText(int index) const;
    void EnsureActive();

    float AccessoryWidth() const;
    bool HitAccessory(float x, float y) const;
    int HitSpinPart(float y) const;
    void DrawAccessory(IRenderer* renderer, float absX, float absY);
    void StartRepeat(int direction);
    void StopRepeat();
    bool RepeatButtonHeld() const;

    // Builds m_segments from the format. Called again when the locale
    // changes (UILocale::Generation), since a Locale format follows it.
    virtual void RebuildSegments() {}
    // 0 = AM, 1 = PM for a typed character ('a' / 'p', or the first letter
    // of the locale's word when only one of them starts with it); -1 = none.
    static int MeridianForChar(unsigned int charCode);

    virtual bool AllowsCalendar() const { return false; }
    virtual void OnCalendarClicked() {}
    virtual void OnAccessoryChanged() {}
    virtual bool HitCalendarPopup(float localX, float localY) const { (void)localX; (void)localY; return false; }

    virtual std::string FieldText(const DateTimeSegment& seg) const = 0;
    virtual void StepActive(int dir) = 0;
    virtual void ApplyTypedNumber(DateTimeField field, int value) = 0;
    virtual bool ApplyTypedMonth(int month) = 0;
    virtual void ApplyAmPm(bool pm) = 0;
    virtual void NotifyChanged() = 0;

private:
    unsigned int m_localeGeneration = 0;
    int m_repeatDir = 0;
    bool m_repeatFast = false;
    static const unsigned int REPEAT_TIMER_ID = 1002;
};

} // namespace jwgui
