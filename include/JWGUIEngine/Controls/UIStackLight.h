#pragma once

#include "UIIndicatorBase.h"
#include <array>

namespace jwgui {

// A machine's signal tower: 2 to 5 lenses stacked on a pole, each Off, On or
// Blink. Scene type `StackLight` (Toolbox Extended › INDICATORS). Tier 1 is
// the top one.
//
// Style parts (Type.part): StackLight (box, bare), .tier1 ... .tier5
// (ForegroundColor: the lens colors, red / amber / green / blue / white by
// default), .off (unlit lenses; default each color, dark), .body (Background:
// cap, gaps and pole), .bezel, .caption.
class JWGUI_API UIStackLight : public UIIndicatorBase {
public:
    static constexpr int kMaxTiers = 5;

    UIStackLight(float x, float y, float w, float h);

    void SetTiers(int count);  // 2..5
    int GetTiers() const { return m_tiers; }
    // tier 0 = the top lens.
    void SetTierState(int tier, LampState state);
    LampState GetTierState(int tier) const;
    // The lens color of a tier (.tier<n>).
    uint32_t TierColor(int tier) const;

protected:
    void DrawIndicator(IRenderer* renderer, float x, float y, float w, float h) override;
    bool NeedsBlink() const override;

private:
    int m_tiers = 3;
    std::array<LampState, kMaxTiers> m_states = { LampState::Off, LampState::Off, LampState::On, LampState::Off,
                                                  LampState::Off };
};

} // namespace jwgui
