#pragma once

#include "../Core/Application.h"
#include "../Controls/UIPanel.h"
#include "UINavigator.h"
#include "UIScenePack.h"
#include <memory>
#include <string>
#include <vector>

namespace jwgui {

class ZipArchive;

// An app whose screens come from a .jwui or a .jwpack (JWGUIPlayer, or your
// own exe). A .jwpack carries its own res (see AppPackage), read in memory.
//
// OnInit loads the pack, the Project resources (<.jwui dir>/res/app.jwres) and
// the app sheet (UIScenePack::app), opens the window at the start scene's size
// and shows that scene. The scene fills the window and reflows on resize. Go to
// buttons switch scenes through UINavigator.
//
// Derive and override OnSceneLoaded to bind logic with FindById; call the base
// OnInit first if you override OnInit.
class JWGUI_API SceneApplication : public Application {
public:
    // UTF-8 path of the .jwui or .jwpack. Call before Run(). Not set: the
    // package inside the program, when it has one (AppPackage::EmbedInExe).
    void SetProjectPath(const std::string& path) { m_projectPath = path; }
    // Resource folder override (default <.jwui dir>/res).
    void SetResourceDir(const std::string& dir) { m_resourceDir = dir; }
    // Key of a protected .jwpack. Not set: the key the program carries
    // (AppPackage::ReadOwnKey: JWGUIPack key / project, or the exported player).
    void SetPackageKey(const std::vector<uint8_t>& key) { m_packageKey = key; }
    // The same as text, as the editor shows it (Export > Package key > Copy).
    // False when the text is not a key.
    bool SetPackageKey(const std::string& keyText);
    // Scene to show first (default: the pack's start scene).
    void SetStartScene(const std::string& sceneId) { m_startSceneId = sceneId; }

    const std::string& ProjectPath() const { return m_projectPath; }
    // Why the project could not be shown. Empty on success.
    const std::string& LoadError() const { return m_loadError; }
    std::shared_ptr<UIScenePack> Pack() const { return m_pack; }

    // Changes the built-in theme ("Dark" / "Light") while running; the
    // project sheet goes on top again and every view takes the new colors.
    void SetTheme(const std::string& theme);
    bool NavigateTo(const std::string& sceneId);
    const std::string& CurrentSceneId() const { return m_navigator.CurrentId(); }
    std::shared_ptr<UIRenderView> CurrentSceneRoot() const { return m_sceneRoot; }

    void OnInit() override;

protected:
    // A scene was instantiated and put in the window.
    virtual void OnSceneLoaded(const std::string& sceneId, UIRenderView* root);
    // Shown instead of a scene when the project cannot be loaded.
    virtual std::shared_ptr<UIRenderView> CreateErrorView(const std::string& message);

private:
    std::string m_projectPath;
    std::string m_resourceDir;
    std::string m_startSceneId;
    std::string m_loadError;
    std::wstring m_title;
    std::shared_ptr<UIScenePack> m_pack;
    std::shared_ptr<ZipArchive> m_archive; // the opened .jwpack
    std::vector<uint8_t> m_packageKey;
    std::shared_ptr<UIPanel> m_sceneHost;
    std::shared_ptr<UIRenderView> m_sceneRoot;
    UINavigator m_navigator;

    std::string ResolveResourceDir() const;
    bool LoadProject(UIScenePack& pack, std::string& error);
    void LoadResources(const std::string& resDir);
    void ApplyAppStyle(const std::string& resDir);
    void ShowView(std::shared_ptr<UIRenderView> view);
    void OnNavigated();
};

} // namespace jwgui
