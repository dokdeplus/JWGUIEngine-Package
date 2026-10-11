#pragma once

#include "UIScene.h"
#include <memory>
#include <string>
#include <vector>

namespace jwgui {

class UIStyle;
struct StyleDiagnostic;

// How the pack runs as its own app (JWGUIPlayer / SceneApplication). The
// window opens at the start scene's size. Saved as <App .../> under <JWGUI>.
struct JWGUI_API UIAppSettings {
    std::string title;          // Window title. Empty = the start scene id.
    bool resizable = true;
    std::string theme = "Dark"; // Built-in sheet ("Dark" / "Light"); the project sheet goes on top.
    bool animation = true;      // AnimationEnabled, applied on top of the sheet.
    std::string style;          // Project sheet (.jwss, or an XML .jwstyle), relative to the project res folder.
    // Format locale of the app (dates, times, numbers; UILocale::SetLocale):
    // a name ("ko-KR") or empty = the OS / browser locale of whoever runs it.
    std::string locale;
    // UI language of the app (UIStrings::SetLanguage): a name ("ko") or empty
    // = the OS / browser language of whoever runs it. Texts written "@id" in
    // the scenes and the engine's own words follow it.
    std::string language;
    // The language the project's own texts are written in: "@id" texts fall
    // back to its table (lang/<defaultLanguage>.json). Empty = English.
    std::string defaultLanguage;
    // Key of the protected .jwpack exports (base64, AppPackage). Kept only in
    // the author's .jwui: the copy inside a package never has it.
    std::string packageKey;

    // Builds the app sheet the same way in the Player, the web build and the
    // editor preview: the built-in theme, then the project sheet on top
    // (`sheetText` = the text of the file named by `style`; empty = none), then
    // the animation switch. A .jwss also changes the theme's colors (@palette)
    // and applies every rule that reads; an XML .jwstyle applies whole or not
    // at all. False when the sheet has errors (listed in `diagnostics`).
    bool ApplyStyle(UIStyle& target, const std::string& sheetText,
                    std::vector<StyleDiagnostic>* diagnostics = nullptr) const;

    bool operator==(const UIAppSettings& other) const;
    bool operator!=(const UIAppSettings& other) const { return !(*this == other); }
};

// The .jwui format version (<JWGUI version="1">). It goes up only when an
// older build would misread a file (a meaning changed). Additions keep it: an
// older build keeps attributes it does not know in UISceneNode::props and
// elements it does not know as nodes, and writes both back.
constexpr int kSceneFormatVersion = 1;

// A .jwui file: one or more screens. Not an MFC document and not a stylesheet.
class JWGUI_API UIScenePack {
public:
    // The version the file was written with (kSceneFormatVersion for a new
    // pack). Saving writes the higher of it and this build's version, so a
    // file from a later build is never stamped as older than it is.
    int formatVersion = kSceneFormatVersion;
    std::string startId = "Main";
    UIAppSettings app;
    std::vector<std::shared_ptr<UIScene>> scenes;

    static std::shared_ptr<UIScenePack> CreateDefault();

    UIScene* Find(const std::string& id);
    const UIScene* Find(const std::string& id) const;
    UIScene* StartScene();
    const UIScene* StartScene() const;

    std::shared_ptr<UIScene> AddScene(const std::string& id);
    bool RemoveScene(const std::string& id);
    std::string UniqueSceneId(const std::string& prefix) const;

    // error (optional) receives a short English reason on failure.
    // A failed Load leaves this pack unchanged.
    bool Save(const char* path, std::string* error = nullptr) const;
    bool Load(const char* path, std::string* error = nullptr);
    std::string SaveToString() const;
    bool LoadFromString(const std::string& xml, std::string* error = nullptr);
};

} // namespace jwgui
