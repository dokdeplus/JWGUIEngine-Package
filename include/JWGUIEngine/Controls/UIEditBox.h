#pragma once

#include "../Core/UIRenderView.h"
#include "../Core/Event.h"
#include <functional>
#include <string>
#include <vector>

namespace jwgui {

class JWGUI_API UIEditBox : public UIRenderView {
public:
    UIEditBox(float x, float y, float w, float h);
    ~UIEditBox() override = default;

    void SetText(const std::string& text);
    const std::string& GetText() const;
    // Dim text shown while the box is empty and not focused (what it takes).
    void SetPlaceholder(const std::string& text);
    const std::string& GetPlaceholder() const { return m_placeholder; }

    // Fires when focus leaves (commit).
    EventHandler<std::string> TextChangedEvent;
    // Fires on each edit by the user (typing, delete, paste, cut), not on
    // SetText. For live filters such as a search box.
    EventHandler<std::string> TextEditedEvent;

    // true = consume and skip the default EditBox handling (Qt-style event filter).
    using KeyIntercept = std::function<bool(KeyCode, const KeyModifiers&)>;
    using CharIntercept = std::function<bool(unsigned int)>;
    using ImeIntercept = std::function<bool(const std::string&)>;
    void SetKeyIntercept(KeyIntercept fn) { m_keyIntercept = std::move(fn); }
    void SetCharIntercept(CharIntercept fn) { m_charIntercept = std::move(fn); }
    void SetImeIntercept(ImeIntercept fn) { m_imeIntercept = std::move(fn); }

    // Multi-line: Enter inserts a line break, the text scrolls vertically (wheel,
    // caret) and pasted line breaks are kept. Single-line pastes keep the first
    // line only. Password boxes are always single-line.
    void SetMultiLine(bool multiLine);
    bool IsMultiLine() const;

    void SetReadOnly(bool readOnly);
    bool IsReadOnly() const;
    void SetPassword(bool password);
    bool IsPassword() const;

    void SetTextAlign(TextAlignH hAlign, TextAlignV vAlign);
    TextAlignH GetTextAlignH() const;
    TextAlignV GetTextAlignV() const;

    void RenderContent(IRenderer* renderer, float absX, float absY) override;

    bool OnMouseDown(float x, float y, MouseButton button) override;
    bool OnMouseMove(float x, float y) override;
    bool OnMouseUp(float x, float y, MouseButton button) override;
    bool OnMouseWheel(float delta, float x, float y, const KeyModifiers& modifiers) override;
    void OnGlobalMouseDown(float x, float y) override;

    bool OnKeyDown(KeyCode key, const KeyModifiers& modifiers) override;
    bool OnChar(unsigned int charCode) override;
    bool OnImeComposition(const std::string& compStr) override;
    void OnTimer(unsigned int timerId) override;

    void OnFocusChanged(bool focused) override;
    void SetTabStop(bool tabStop) { m_tabStop = tabStop; }
    bool IsTabStop() const override { return m_tabStop && !IsDisabled(); }

    void OnStyleChanged() override;

private:
    // Fires TextEditedEvent on scope exit when the text changed.
    struct EditScope;

    void UpdateLines();
    void DeleteSelection();
    void MoveCaret(int newPos, bool shift);
    int CoordToByteIndex(float x, float y);
    int MapDisplayIndexToCaret(int displayIndex) const;
    bool CanMutate() const { return !m_isReadOnly; }
    std::string NormalizeInput(const std::string& text) const;

    std::vector<std::string> SplitLines(const std::string& str) const;

private:
    uint32_t m_cachedFgColor = 0xFFFFFFFF;
    uint32_t m_cachedOutlineColor = 0xFF555555;
    float m_cachedOutlineThick = 1.0f;
    Thickness m_cachedPadding = Thickness(4.0f);
    Thickness m_cachedBorderWidth;
    bool m_hasBorderWidth = false;
    float m_cachedFontSize = 14.0f;
    std::string m_cachedFontFamily = UIStyle::DefaultFontFamily();
    UIBackground m_cachedBg;
    uint32_t m_cachedSelBgColor = 0x66007ACC;
    uint32_t m_cachedCaretColor = 0xFFFFFFFF;
    std::string m_text;
    std::string m_placeholder;
    std::string m_imeCompStr;
    
    bool m_isMultiLine = false;
    bool m_isReadOnly = false;
    bool m_isPassword = false;
    TextAlignH m_hAlign = TextAlignH::Left;
    TextAlignV m_vAlign = TextAlignV::Top;

    int m_caretPos = 0;
    int m_selectionStart = -1;
    // Text scroll inside the box (not the base view's content scroll).
    float m_textScrollX = 0.0f;
    float m_textScrollY = 0.0f;
    float m_lastTextHeight = 0.0f;
    float m_lastViewHeight = 0.0f;
    // Caret/text state at the last scroll-into-view. Scrolling follows the caret
    // only when this changes, so the mouse wheel can scroll away from it.
    int m_scrolledCaret = -1;
    size_t m_scrolledTextSize = 0;
    size_t m_scrolledImeSize = 0;
    bool m_scrolledFocused = false;
    
    bool m_isSelecting = false;
    bool m_caretVisible = true;
    bool m_tabStop = true;

    KeyIntercept m_keyIntercept;
    CharIntercept m_charIntercept;
    ImeIntercept m_imeIntercept;

    static const unsigned int CARET_TIMER_ID = 1001;

    struct LineInfo {
        int startIdx;
        int length;
        std::string str;
    };
    std::vector<LineInfo> m_lines;

    struct CharPos {
        int byteIdx;
        float x;
    };
    std::vector<std::vector<CharPos>> m_renderedLines;

    float m_lastLineHeight = 16.0f;
    float m_lastStartY = 0.0f;

    UIRect m_lastCaretRect;
};

} // namespace jwgui
