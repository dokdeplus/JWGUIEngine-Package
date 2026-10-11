#pragma once

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#include <stdlib.h>
#include <string>
#include <vector>

// 사용자가 구현해야 하는 표준 main 함수
extern int main(int argc, char** argv);

// Include this header in exactly one .cpp (the one that defines main).
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;

    // main gets its arguments as UTF-8 (file names in any language). A
    // program that starts at wWinMain has no __argv, only the wide one.
    std::vector<std::string> texts;
    int count = 0;
    if (LPWSTR* wide = CommandLineToArgvW(GetCommandLineW(), &count)) {
        for (int i = 0; i < count; ++i) {
            const int size = WideCharToMultiByte(CP_UTF8, 0, wide[i], -1, nullptr, 0, nullptr, nullptr);
            std::string text(size > 0 ? static_cast<size_t>(size - 1) : 0, '\0');
            if (size > 1) WideCharToMultiByte(CP_UTF8, 0, wide[i], -1, &text[0], size, nullptr, nullptr);
            texts.push_back(std::move(text));
        }
        LocalFree(wide);
    }
    if (texts.empty()) texts.emplace_back();
    std::vector<char*> arguments;
    for (std::string& text : texts) arguments.push_back(&text[0]);
    arguments.push_back(nullptr);

    // 표준 main 함수 호출
    return main(static_cast<int>(texts.size()), arguments.data());
}
#endif
