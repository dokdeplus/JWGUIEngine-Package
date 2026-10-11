// The program of "Your first C++ app": loads hello.jwui and makes the button
// answer. Open hello.jwui in JWGUIEditor to change the screen; no rebuild.
#include "EntryPoint.h"         // gives Windows its WinMain; include it in one .cpp only
#include "SceneApplication.h"
#include "UIButton.h"
#include "UIEditBox.h"
#include "UIStatic.h"

using namespace jwgui;

class HelloApp : public SceneApplication {
protected:
    // Runs each time a scene has been loaded and put in the window.
    void OnSceneLoaded(const std::string& sceneId, UIRenderView* root) override {
        if (sceneId != "Main") return;

        auto button = root->FindByIdAs<UIButton>("helloButton");
        auto edit = root->FindByIdAs<UIEditBox>("nameEdit");
        auto answer = root->FindByIdAs<UIStatic>("answer");
        if (!button || !edit || !answer) return;   // someone renamed a control

        button->ClickEvent.AddListener([edit, answer](UIRenderView*, const EventArgs&) {
            answer->SetText("Hello, " + edit->GetText() + "!");
        });
    }
};

int main(int argc, char** argv) {
    HelloApp app;
    app.SetProjectPath(argc > 1 ? argv[1] : "hello.jwui");   // UTF-8 path
    return app.Run();
}
