#pragma once

#if defined(JWGUIENGINE_STATIC)
#define JWGUI_API
#elif defined(_MSC_VER)
#ifdef JWGUIENGINE_EXPORTS
#define JWGUI_API __declspec(dllexport)
#else
#define JWGUI_API __declspec(dllimport)
#endif
#else
#define JWGUI_API __attribute__((visibility("default")))
#endif
