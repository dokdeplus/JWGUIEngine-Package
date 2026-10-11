#pragma once

#include "UIIndicatorBase.h"

namespace jwgui {

// A pilot lamp: Off, On or Blink in a color. Scene type `Lamp` (Toolbox
// Extended › INDICATORS). The label (the node's text) sits under a Circle or
// Square lamp and inside a Rect one.
//
// Style parts (Type.part): Lamp (box, bare), .bezel (OutlineColor,
// OutlineThickness: the ring around the lens), .off (ForegroundColor: the
// unlit lens; default the lamp color, dark), .caption (label). The color
// itself is the `color` property: a zone name (Type.zone-<name>) or #RRGGBB.
class JWGUI_API UILamp : public UIIndicatorBase {
public:
    UILamp(float x, float y, float w, float h);

    void SetLampState(LampState state);
    LampState GetLampState() const { return m_state; }
    void SetLampStateName(const std::string& name);  // "Off" / "On" / "Blink"

    // "ok" (green), "warn", "alarm", "info", "normal" or another zone name,
    // or "#RRGGBB".
    void SetColor(const std::string& color);
    const std::string& GetColor() const { return m_color; }

    void SetShape(LampShape shape);
    LampShape GetShape() const { return m_shape; }
    void SetShapeName(const std::string& name);  // "Circle" / "Square" / "Rect"

    // Lit now (On, or Blink in its lit half).
    bool IsLitNow() const { return IsLit(m_state); }
    // The lit color the spec gives now.
    uint32_t LitColor() const;

protected:
    void DrawIndicator(IRenderer* renderer, float x, float y, float w, float h) override;
    bool NeedsBlink() const override { return m_state == LampState::Blink; }

private:
    LampState m_state = LampState::On;
    std::string m_color = "ok";
    LampShape m_shape = LampShape::Circle;
};

} // namespace jwgui
