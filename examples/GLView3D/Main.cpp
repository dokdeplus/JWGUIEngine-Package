// A desk globe in a GLView, with the engine's controls around it and over it.
//
// Three parts of the engine work together here:
//   - UIGLView (GL View) gives the program an OpenGL 3.3 picture inside a
//     window. The program draws the globe, its stand and the floor with its
//     own GL calls.
//   - UIOverlayLayer puts the engine's controls over that picture: a settings
//     panel, a button and a title, drawn into the same GL framebuffer after
//     the 3D scene. The view hands every mouse and key event to the overlay
//     layer first; what the UI does not take moves the camera. A game made
//     with its own engine uses the overlay layer the same way
//     (examples/UIOverlayLayer, Windows).
//   - Ordinary window controls frame the view: a menu bar (File, View) and a
//     status bar with what the camera and the picture are doing.
//
//   left drag      turn around the globe
//   right drag     move (middle drag too)
//   wheel, pinch   zoom
//   Esc            show or hide the settings
//
// The map is earth.jpg (Natural Earth II, public domain: earth-LICENSE.txt):
// next to the program built here, and inside the program in the release's
// bin, where JWGUIPack put it, protected. File > Open map... puts any world map picture
// (equirectangular, twice as wide as high) on the globe. The guide's
// Controls > GL View and C++ > UI in a game or 3D view pages explain the
// parts. The math (vectors, 4 x 4 matrices, the camera, the meshes) is
// written out here so the example needs nothing but the engine.

// The OpenGL loader that comes with the engine: one source file defines the
// functions, and it is included before any other OpenGL header.
#define GLAD_GL_IMPLEMENTATION
#include "glad_gl.h"

#include "EntryPoint.h"         // gives Windows its WinMain; include it in one .cpp only
#include "AppPackage.h"
#include "Application.h"
#include "Bitmap.h"
#include "FileDialog.h"
#include "IPlatformHost.h"
#include "ProgramPath.h"
#include "UIButton.h"
#include "UICheckBox.h"
#include "UIComboBox.h"
#include "UIGLView.h"
#include "UIMenu.h"
#include "UIMenuBar.h"
#include "UIOverlayLayer.h"
#include "UIPanel.h"
#include "UISlider.h"
#include "UIStatic.h"
#include "UIStatusBar.h"
#include "ZipArchive.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <functional>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

using namespace jwgui;

namespace {

constexpr float kPi = 3.14159265f;

// ---------------------------------------------------------------------------
// Vectors and matrices (column-major, as OpenGL reads them)
// ---------------------------------------------------------------------------
struct Vec3 {
    float x = 0.0f, y = 0.0f, z = 0.0f;
};
Vec3 operator+(Vec3 a, Vec3 b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
Vec3 operator-(Vec3 a, Vec3 b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
Vec3 operator*(Vec3 a, float s) { return { a.x * s, a.y * s, a.z * s }; }
float Dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
Vec3 Cross(Vec3 a, Vec3 b) { return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x }; }
Vec3 Normalize(Vec3 a) {
    const float length = std::sqrt(Dot(a, a));
    return length > 0.0f ? a * (1.0f / length) : a;
}

struct Mat4 {
    float m[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
};
Mat4 operator*(const Mat4& a, const Mat4& b) {
    Mat4 r;
    for (int column = 0; column < 4; ++column) {
        for (int row = 0; row < 4; ++row) {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k) sum += a.m[k * 4 + row] * b.m[column * 4 + k];
            r.m[column * 4 + row] = sum;
        }
    }
    return r;
}
Vec3 TransformPoint(const Mat4& a, Vec3 p) {
    return { a.m[0] * p.x + a.m[4] * p.y + a.m[8] * p.z + a.m[12],
             a.m[1] * p.x + a.m[5] * p.y + a.m[9] * p.z + a.m[13],
             a.m[2] * p.x + a.m[6] * p.y + a.m[10] * p.z + a.m[14] };
}
Mat4 Perspective(float fovY, float aspect, float zNear, float zFar) {
    const float f = 1.0f / std::tan(fovY / 2.0f);
    Mat4 r;
    r.m[0] = f / aspect;
    r.m[5] = f;
    r.m[10] = (zFar + zNear) / (zNear - zFar);
    r.m[11] = -1.0f;
    r.m[14] = 2.0f * zFar * zNear / (zNear - zFar);
    r.m[15] = 0.0f;
    return r;
}
Mat4 LookAt(Vec3 eye, Vec3 target, Vec3 up) {
    const Vec3 f = Normalize(target - eye);
    const Vec3 s = Normalize(Cross(f, up));
    const Vec3 u = Cross(s, f);
    Mat4 r;
    r.m[0] = s.x; r.m[4] = s.y; r.m[8] = s.z;
    r.m[1] = u.x; r.m[5] = u.y; r.m[9] = u.z;
    r.m[2] = -f.x; r.m[6] = -f.y; r.m[10] = -f.z;
    r.m[12] = -Dot(s, eye);
    r.m[13] = -Dot(u, eye);
    r.m[14] = Dot(f, eye);
    return r;
}
Mat4 Translate(Vec3 t) {
    Mat4 r;
    r.m[12] = t.x; r.m[13] = t.y; r.m[14] = t.z;
    return r;
}
Mat4 RotateY(float angle) {
    Mat4 r;
    r.m[0] = std::cos(angle); r.m[8] = std::sin(angle);
    r.m[2] = -std::sin(angle); r.m[10] = std::cos(angle);
    return r;
}
Mat4 RotateZ(float angle) {
    Mat4 r;
    r.m[0] = std::cos(angle); r.m[4] = -std::sin(angle);
    r.m[1] = std::sin(angle); r.m[5] = std::cos(angle);
    return r;
}

// ---------------------------------------------------------------------------
// The camera: it circles a target point at a distance
// ---------------------------------------------------------------------------
struct OrbitCamera {
    Vec3 target{ 0.55f, 1.3f, 0.0f };   // the globe a little left of the settings
    float yaw = 0.35f;       // around the vertical axis
    float pitch = 0.18f;     // up from the floor
    float distance = 5.6f;

    Vec3 Eye() const {
        return target + Vec3{ std::cos(pitch) * std::sin(yaw), std::sin(pitch), std::cos(pitch) * std::cos(yaw) } * distance;
    }
    void Turn(float dx, float dy) {
        yaw -= dx * 0.01f;
        pitch = std::clamp(pitch + dy * 0.01f, -1.45f, 1.45f);
    }
    void Zoom(float factor) { distance = std::clamp(distance * factor, 1.6f, 30.0f); }
    // Moves the target in the screen's plane; dx, dy in pixels of a view
    // `height` pixels high, so the point under the pointer stays under it.
    void Move(float dx, float dy, float height) {
        const Vec3 forward = Normalize(target - Eye());
        const Vec3 right = Normalize(Cross(forward, Vec3{ 0.0f, 1.0f, 0.0f }));
        const Vec3 up = Cross(right, forward);
        const float perPixel = 2.0f * distance * std::tan(kFov / 2.0f) / std::max(height, 1.0f);
        target = target - right * (dx * perPixel) + up * (dy * perPixel);
    }
    static constexpr float kFov = 40.0f * kPi / 180.0f;
};

// ---------------------------------------------------------------------------
// Meshes: position, normal and texture coordinates per vertex
// ---------------------------------------------------------------------------
struct Mesh {
    std::vector<float> vertices;  // x y z, nx ny nz, u v
    std::vector<unsigned int> indices;
    unsigned int Add(Vec3 p, Vec3 n, float u = 0.0f, float v = 0.0f) {
        vertices.insert(vertices.end(), { p.x, p.y, p.z, n.x, n.y, n.z, u, v });
        return static_cast<unsigned int>(vertices.size() / 8 - 1);
    }
    // Quads between two rows of `columns + 1` vertices that start at a and b.
    void Strip(unsigned int a, unsigned int b, int columns) {
        for (unsigned int i = 0; i < static_cast<unsigned int>(columns); ++i)
            indices.insert(indices.end(), { a + i, b + i, a + i + 1, a + i + 1, b + i, b + i + 1 });
    }
};

// The globe: a sphere of radius 1, the map's longitude around y (Greenwich
// toward +z), the top row of the map at the north pole.
Mesh MakeGlobe(int rings, int segments) {
    Mesh mesh;
    for (int r = 0; r <= rings; ++r) {
        const float v = static_cast<float>(r) / static_cast<float>(rings);
        const float latitude = kPi / 2.0f - v * kPi;
        for (int s = 0; s <= segments; ++s) {
            const float u = static_cast<float>(s) / static_cast<float>(segments);
            const float longitude = u * 2.0f * kPi - kPi;
            const Vec3 n{ std::cos(latitude) * std::sin(longitude), std::sin(latitude), std::cos(latitude) * std::cos(longitude) };
            mesh.Add(n, n, u, v);
        }
    }
    for (int r = 0; r < rings; ++r)
        mesh.Strip(static_cast<unsigned int>(r * (segments + 1)), static_cast<unsigned int>((r + 1) * (segments + 1)), segments);
    return mesh;
}

// A ring around the z axis (in the x-y plane), from angle `from` to `to`.
Mesh MakeRing(float radius, float tube, float from, float to, int steps, int sides) {
    Mesh mesh;
    for (int i = 0; i <= steps; ++i) {
        const float a = from + (to - from) * static_cast<float>(i) / static_cast<float>(steps);
        const Vec3 center{ std::cos(a) * radius, std::sin(a) * radius, 0.0f };
        for (int s = 0; s <= sides; ++s) {
            const float b = 2.0f * kPi * static_cast<float>(s) / static_cast<float>(sides);
            const Vec3 n{ std::cos(b) * std::cos(a), std::cos(b) * std::sin(a), std::sin(b) };
            mesh.Add(center + n * tube, n);
        }
    }
    for (int i = 0; i < steps; ++i)
        mesh.Strip(static_cast<unsigned int>(i * (sides + 1)), static_cast<unsigned int>((i + 1) * (sides + 1)), sides);
    return mesh;
}

// A turned shape around the y axis from a profile of (radius, height)
// points, bottom to top; each step of the profile is a band with its own
// normal, so the edges stay sharp like turned wood or metal.
Mesh MakeLathe(const std::vector<std::pair<float, float>>& profile, int sides) {
    Mesh mesh;
    for (size_t p = 0; p + 1 < profile.size(); ++p) {
        const float r0 = profile[p].first, y0 = profile[p].second;
        const float r1 = profile[p + 1].first, y1 = profile[p + 1].second;
        // The band's slope gives its normal: out, and up or down.
        const Vec3 slope = Normalize({ y1 - y0, r0 - r1, 0.0f });
        const unsigned int first = static_cast<unsigned int>(mesh.vertices.size() / 8);
        for (int row = 0; row < 2; ++row) {
            for (int s = 0; s <= sides; ++s) {
                const float a = 2.0f * kPi * static_cast<float>(s) / static_cast<float>(sides);
                const float r = row ? r1 : r0, y = row ? y1 : y0;
                mesh.Add({ std::cos(a) * r, y, std::sin(a) * r }, { std::cos(a) * slope.x, slope.y, std::sin(a) * slope.x });
            }
        }
        mesh.Strip(first, first + static_cast<unsigned int>(sides + 1), sides);
    }
    return mesh;
}

// The floor: lines every half unit.
Mesh MakeGrid() {
    Mesh mesh;
    for (int i = -10; i <= 10; ++i) {
        const float t = static_cast<float>(i) * 0.5f;
        for (Vec3 p : { Vec3{ t, 0, -5 }, Vec3{ t, 0, 5 }, Vec3{ -5, 0, t }, Vec3{ 5, 0, t } })
            mesh.indices.push_back(mesh.Add(p, { 0, 1, 0 }));
    }
    return mesh;
}

// A flat square on the floor for the soft shadow.
Mesh MakeShadow(float half) {
    Mesh mesh;
    const unsigned int a = mesh.Add({ -half, 0.001f, -half }, { 0, 1, 0 });
    const unsigned int b = mesh.Add({ half, 0.001f, -half }, { 0, 1, 0 });
    const unsigned int c = mesh.Add({ half, 0.001f, half }, { 0, 1, 0 });
    const unsigned int d = mesh.Add({ -half, 0.001f, half }, { 0, 1, 0 });
    mesh.indices = { a, b, c, a, c, d };
    return mesh;
}

// A mesh on the graphics card.
struct GpuMesh {
    GLuint vao = 0, vbo = 0, ebo = 0;
    GLsizei count = 0;
    GLenum mode = GL_TRIANGLES;

    void Upload(const Mesh& mesh, GLenum drawMode) {
        mode = drawMode;
        count = static_cast<GLsizei>(mesh.indices.size());
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(mesh.vertices.size() * sizeof(float)), mesh.vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(mesh.indices.size() * sizeof(unsigned int)), mesh.indices.data(), GL_STATIC_DRAW);
        const GLsizei stride = 8 * sizeof(float);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(0));
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(3 * sizeof(float)));
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(6 * sizeof(float)));
        for (GLuint i = 0; i < 3; ++i) glEnableVertexAttribArray(i);
        glBindVertexArray(0);
    }
    void Draw() const {
        glBindVertexArray(vao);
        glDrawElements(mode, count, GL_UNSIGNED_INT, nullptr);
    }
    void Free() {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
        vao = vbo = ebo = 0;
    }
};

// ---------------------------------------------------------------------------
// The map: a picture, and where its water is (shiny) from its colors
// ---------------------------------------------------------------------------
std::string FindFile(const std::string& name) {
    // Next to the program, in the macOS bundle's Resources, the working folder.
    const std::string folder = ProgramFolder();
    for (const std::string& path : { folder + "/" + name, folder + "/../Resources/" + name, name }) {
        std::error_code error;
        if (std::filesystem::exists(std::filesystem::u8path(path), error)) return path;
    }
    return name;
}

// The earth map. A program can carry its files inside itself (JWGUIPack
// folder <folder> <program> puts a folder there as res/..., encrypted): then
// nothing ships next to it. Without that, earth.jpg next to the program.
bool LoadEarth(Bitmap& picture) {
    std::vector<uint8_t> zip;
    std::vector<uint8_t> bytes;
    ZipArchive archive;
    if (AppPackage::HasEmbedded() && AppPackage::ReadEmbedded(zip) && archive.Open(std::move(zip)) &&
        archive.Read("res/earth.jpg", bytes)) {
        return picture.Decode(bytes.data(), bytes.size());
    }
    return picture.Load(FindFile("earth.jpg"));
}

GLuint MakeTexture(GLint format, int width, int height, GLenum pixelFormat, const void* pixels) {
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, pixelFormat, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return texture;
}

struct EarthMaps {
    GLuint color = 0;   // the map (sRGB)
    GLuint water = 0;   // 1 on water, 0 on land

    void Upload(const Bitmap& map) {
        Free();
        if (map.IsEmpty()) {   // no map: a blue ball
            const uint32_t blue = 0xFF3060A0u;
            const uint8_t sea = 255;
            color = MakeTexture(GL_SRGB8_ALPHA8, 1, 1, GL_BGRA, &blue);
            water = MakeTexture(GL_R8, 1, 1, GL_RED, &sea);
            return;
        }
        // Bitmap's 0xAARRGGBB pixels are B, G, R, A bytes in memory; rows top
        // first, as the map's v runs from the north pole down.
        color = MakeTexture(GL_SRGB8_ALPHA8, map.Width(), map.Height(), GL_BGRA, map.Pixels());
        std::vector<uint8_t> sea(static_cast<size_t>(map.Width()) * static_cast<size_t>(map.Height()));
        for (size_t i = 0; i < sea.size(); ++i) {
            const uint32_t p = map.Pixels()[i];
            const int r = (p >> 16) & 0xFF, g = (p >> 8) & 0xFF, b = p & 0xFF;
            sea[i] = (b > r + 30 && b > g + 5) ? 255 : 0;   // blue enough: water
        }
        water = MakeTexture(GL_R8, map.Width(), map.Height(), GL_RED, sea.data());
    }
    void Free() {
        if (color) glDeleteTextures(1, &color);
        if (water) glDeleteTextures(1, &water);
        color = water = 0;
    }
};

// ---------------------------------------------------------------------------
// Shaders
// ---------------------------------------------------------------------------
// Everything solid: metal and roughness lighting from the sun and the
// light around, the map on the globe (shiny water, a thin blue air at the
// edge), tone mapping to the screen's colors.
const char* kVertexShader = R"(#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
uniform mat4 model;
uniform mat4 viewProjection;
out vec3 worldPosition;
out vec3 worldNormal;
out vec2 texCoord;
void main() {
    vec4 world = model * vec4(position, 1.0);
    worldPosition = world.xyz;
    worldNormal = mat3(model) * normal;
    texCoord = uv;
    gl_Position = viewProjection * world;
}
)";

const char* kFragmentShader = R"(#version 330 core
in vec3 worldPosition;
in vec3 worldNormal;
in vec2 texCoord;
uniform int globe;            // 1: the map, the water and the air
uniform sampler2D colorMap;
uniform sampler2D waterMap;
uniform vec3 baseColor;
uniform float metal;
uniform float rough;
uniform vec3 eye;
uniform vec3 sun;             // toward the sun
uniform vec3 sky;             // the light from around
out vec4 fragColor;

const float PI = 3.14159265;

vec3 Light(vec3 n, vec3 v, vec3 l, vec3 color, vec3 base, float metal, float rough) {
    vec3 h = normalize(l + v);
    float nl = max(dot(n, l), 0.0), nv = max(dot(n, v), 1e-4), nh = max(dot(n, h), 0.0), vh = max(dot(v, h), 0.0);
    float a = rough * rough, a2 = a * a;
    float d = a2 / (PI * pow(nh * nh * (a2 - 1.0) + 1.0, 2.0));
    float k = (rough + 1.0) * (rough + 1.0) / 8.0;
    float g = (nl / (nl * (1.0 - k) + k)) * (nv / (nv * (1.0 - k) + k));
    vec3 f0 = mix(vec3(0.04), base, metal);
    vec3 f = f0 + (1.0 - f0) * pow(1.0 - vh, 5.0);
    vec3 specular = d * g * f / max(4.0 * nl * nv, 1e-4);
    vec3 diffuse = (1.0 - f) * (1.0 - metal) * base / PI;
    return (diffuse + specular) * color * nl;
}

void main() {
    vec3 n = normalize(worldNormal);
    if (!gl_FrontFacing) n = -n;
    vec3 v = normalize(eye - worldPosition);
    vec3 base = baseColor;
    float m = metal, r = rough;
    if (globe == 1) {
        base = texture(colorMap, texCoord).rgb;   // linear (an sRGB texture)
        r = mix(0.85, 0.32, texture(waterMap, texCoord).r);
        m = 0.0;
    }
    vec3 color = Light(n, v, sun, vec3(3.4, 3.2, 3.0), base, m, r);
    // Around: the light from everywhere, dimmer from below; metal mirrors it.
    vec3 around = sky * mix(0.35, 1.0, n.y * 0.5 + 0.5);
    vec3 f0 = mix(vec3(0.04), base, m);
    vec3 fresnel = f0 + (max(vec3(1.0 - r), f0) - f0) * pow(1.0 - max(dot(n, v), 0.0), 5.0);
    color += around * (base * (1.0 - m) * (1.0 - fresnel) + fresnel * mix(1.0, 0.5, r));
    if (globe == 1) {
        // The air: a thin blue at the edge, on the side the sun lights.
        float edge = pow(1.0 - max(dot(n, v), 0.0), 3.0);
        color += vec3(0.25, 0.5, 1.0) * edge * (0.25 + max(dot(n, sun), 0.0)) * 0.6;
    }
    color = color * (2.51 * color + 0.03) / (color * (2.43 * color + 0.59) + 0.14);  // tone map (ACES fit)
    fragColor = vec4(pow(clamp(color, 0.0, 1.0), vec3(1.0 / 2.2)), 1.0);
}
)";

// The floor: grid lines fading out, and a soft shadow under the globe.
const char* kFloorVertexShader = R"(#version 330 core
layout(location = 0) in vec3 position;
uniform mat4 viewProjection;
out vec2 floorPosition;
void main() {
    floorPosition = position.xz;
    gl_Position = viewProjection * vec4(position, 1.0);
}
)";

const char* kFloorFragmentShader = R"(#version 330 core
in vec2 floorPosition;
uniform vec4 color;
uniform float shadowRadius;   // 0: lines
out vec4 fragColor;
void main() {
    if (shadowRadius > 0.0) {
        float r = length(floorPosition) / shadowRadius;
        fragColor = vec4(0.0, 0.0, 0.0, 0.5 * pow(clamp(1.0 - r, 0.0, 1.0), 1.5));
        return;
    }
    float fade = 1.0 - smoothstep(2.0, 5.0, length(floorPosition));
    fragColor = vec4(color.rgb, color.a * fade);
}
)";

GLuint MakeProgram(const char* vertexSource, const char* fragmentSource) {
    auto compile = [](GLenum kind, const char* source) {
        const GLuint shader = glCreateShader(kind);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);
        return shader;
    };
    const GLuint vs = compile(GL_VERTEX_SHADER, vertexSource);
    const GLuint fs = compile(GL_FRAGMENT_SHADER, fragmentSource);
    const GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

// ---------------------------------------------------------------------------
// The scene: what is drawn, as the settings say
// ---------------------------------------------------------------------------
struct Backdrop {
    const char* name;
    float clear[3];   // the background
    float sky[3];     // the light from around
    float grid[4];
};
const Backdrop kBackdrops[] = {
    { "Night", { 0.035f, 0.04f, 0.06f }, { 0.10f, 0.11f, 0.14f }, { 0.40f, 0.48f, 0.60f, 0.45f } },
    { "Studio", { 0.16f, 0.17f, 0.19f }, { 0.45f, 0.47f, 0.52f }, { 0.55f, 0.58f, 0.64f, 0.5f } },
    { "Daylight", { 0.62f, 0.72f, 0.84f }, { 0.50f, 0.55f, 0.62f }, { 0.30f, 0.36f, 0.45f, 0.5f } },
};

struct Settings {
    bool spin = true;
    float speed = 1.0f;         // the globe's turning, times the default
    float sunAngle = 40.0f;     // degrees around the globe
    bool tilt = true;           // the earth's 23.4 degree axis
    bool stand = true;
    bool floor = true;
    int backdrop = 0;
};

struct Scene3D {
    OrbitCamera camera;
    Settings settings;
    float spin = 0.0f;          // the globe's own turn (radians)
    Bitmap map;                 // a map waiting to go on the card
    bool mapChanged = true;
    double drawMs = 0.0;        // how long the last picture took

    GLuint program = 0, floorProgram = 0;
    GpuMesh globe, ring, pins, base, grid, shadow;
    EarthMaps maps;

    static constexpr float kRingRadius = 1.08f;   // the meridian ring around the globe
    static constexpr float kStemTop = 0.42f;      // where the ring sits on the stand

    // A new map: it goes on the card at the next picture (the GL context is
    // current only while the view draws).
    bool SetMap(Bitmap picture) {
        if (picture.IsEmpty()) return false;
        map = std::move(picture);
        mapChanged = true;
        return true;
    }

    void Init() {
        // Once, with the context current: the GL functions, then the objects.
        gladLoadGL(GLSurface::GetProcAddress);
        program = MakeProgram(kVertexShader, kFragmentShader);
        floorProgram = MakeProgram(kFloorVertexShader, kFloorFragmentShader);
        globe.Upload(MakeGlobe(96, 192), GL_TRIANGLES);
        // A half ring from the south pin, under and around to the north pin.
        ring.Upload(MakeRing(kRingRadius, 0.028f, -kPi / 2.0f - 0.25f, kPi / 2.0f + 0.25f, 96, 16), GL_TRIANGLES);
        // The pins along the axis, from the globe to the ring.
        pins.Upload(MakeLathe({ { 0.0f, -kRingRadius }, { 0.018f, -kRingRadius }, { 0.018f, kRingRadius }, { 0.0f, kRingRadius } }, 16), GL_TRIANGLES);
        base.Upload(MakeLathe({ { 0.0f, 0.0f }, { 0.62f, 0.0f }, { 0.62f, 0.05f }, { 0.55f, 0.10f }, { 0.16f, 0.14f },
                                { 0.07f, 0.20f }, { 0.045f, 0.24f }, { 0.045f, kStemTop - 0.04f }, { 0.07f, kStemTop - 0.01f },
                                { 0.05f, kStemTop + 0.02f }, { 0.0f, kStemTop + 0.02f } }, 48), GL_TRIANGLES);
        grid.Upload(MakeGrid(), GL_LINES);
        shadow.Upload(MakeShadow(1.1f), GL_TRIANGLES);
    }

    // Where the globe's center is and how its axis leans: the ring stands on
    // the stem at its lowest point, the globe sits in the ring.
    Mat4 Axis() const {
        const float tilt = settings.tilt ? 23.44f * kPi / 180.0f : 0.0f;
        const Mat4 lean = RotateZ(-tilt);
        const Vec3 lowest = TransformPoint(lean, { 0.0f, -kRingRadius, 0.0f });
        return Translate(Vec3{ 0.0f, kStemTop, 0.0f } - lowest) * lean;
    }

    void Draw(const GLFrame& frame) {
        const auto started = std::chrono::steady_clock::now();
        if (mapChanged) {
            maps.Upload(map);
            map = Bitmap();   // on the card now
            mapChanged = false;
        }
        const Backdrop& backdrop = kBackdrops[settings.backdrop];
        // All GL views share one context: set every state this picture needs.
        glViewport(0, 0, frame.width, frame.height);
        glClearColor(backdrop.clear[0], backdrop.clear[1], backdrop.clear[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);

        const float aspect = static_cast<float>(frame.width) / static_cast<float>(std::max(frame.height, 1));
        const Vec3 eye = camera.Eye();
        const Mat4 viewProjection = Perspective(OrbitCamera::kFov, aspect, 0.05f, 100.0f) * LookAt(eye, camera.target, { 0.0f, 1.0f, 0.0f });
        const float sunAngle = settings.sunAngle * kPi / 180.0f;
        const Vec3 sun = Normalize({ std::sin(sunAngle), 0.45f, std::cos(sunAngle) });

        glUseProgram(program);
        glUniformMatrix4fv(glGetUniformLocation(program, "viewProjection"), 1, GL_FALSE, viewProjection.m);
        glUniform3f(glGetUniformLocation(program, "eye"), eye.x, eye.y, eye.z);
        glUniform3f(glGetUniformLocation(program, "sun"), sun.x, sun.y, sun.z);
        glUniform3f(glGetUniformLocation(program, "sky"), backdrop.sky[0], backdrop.sky[1], backdrop.sky[2]);
        glUniform1i(glGetUniformLocation(program, "colorMap"), 0);
        glUniform1i(glGetUniformLocation(program, "waterMap"), 1);
        const GLint model = glGetUniformLocation(program, "model");
        const GLint isGlobe = glGetUniformLocation(program, "globe");
        auto material = [&](float r, float g, float b, float metal, float rough) {
            glUniform3f(glGetUniformLocation(program, "baseColor"), r, g, b);
            glUniform1f(glGetUniformLocation(program, "metal"), metal);
            glUniform1f(glGetUniformLocation(program, "rough"), rough);
        };

        const Mat4 axis = Axis();
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, maps.water);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, maps.color);
        glUniform1i(isGlobe, 1);
        glUniformMatrix4fv(model, 1, GL_FALSE, (axis * RotateY(spin)).m);
        globe.Draw();

        glUniform1i(isGlobe, 0);
        if (settings.stand) {
            material(0.80f, 0.60f, 0.30f, 1.0f, 0.28f);   // brass
            glUniformMatrix4fv(model, 1, GL_FALSE, axis.m);
            ring.Draw();
            pins.Draw();
            material(0.09f, 0.055f, 0.035f, 0.0f, 0.4f);   // dark wood
            const Mat4 identity;
            glUniformMatrix4fv(model, 1, GL_FALSE, identity.m);
            base.Draw();
        }

        // The floor after the solid things, blended, without writing depth.
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        glUseProgram(floorProgram);
        glUniformMatrix4fv(glGetUniformLocation(floorProgram, "viewProjection"), 1, GL_FALSE, viewProjection.m);
        glUniform4f(glGetUniformLocation(floorProgram, "color"), backdrop.grid[0], backdrop.grid[1], backdrop.grid[2], backdrop.grid[3]);
        glUniform1f(glGetUniformLocation(floorProgram, "shadowRadius"), 0.0f);
        if (settings.floor) grid.Draw();
        glUniform1f(glGetUniformLocation(floorProgram, "shadowRadius"), 1.1f);
        shadow.Draw();
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glBindVertexArray(0);
        drawMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count();
    }

    void Release() {
        for (GpuMesh* mesh : { &globe, &ring, &pins, &base, &grid, &shadow }) mesh->Free();
        maps.Free();
        glDeleteProgram(program);
        glDeleteProgram(floorProgram);
        program = floorProgram = 0;
    }
};

// ---------------------------------------------------------------------------
// The view: the 3D picture, the overlay layer's controls over it, and the
// mouse and keys going to the UI first, then to the camera
// ---------------------------------------------------------------------------
class GlobeView : public UIGLView {
public:
    explicit GlobeView(std::shared_ptr<Scene3D> scene) : UIGLView(0.0f, 0.0f, 0.0f, 0.0f), m_scene(std::move(scene)) {
        // The handlers run with the context current: the overlay layer
        // starts, draws and stops in it too.
        SetInitHandler([this] { m_scene->Init(); });
        SetDrawHandler([this](const GLFrame& frame) {
            m_scene->Draw(frame);
            DrawOverlay(frame);
        });
        SetReleaseHandler([this] {
            m_overlay.Stop();
            m_scene->Release();
        });
    }

    // The app builds the controls when the overlay layer starts (the first
    // picture) and hands back the settings panel and the button over it.
    std::function<void(GlobeView& view, UIOverlayLayer& overlay)> buildOverlay;
    void SetSettingsPanel(std::shared_ptr<UIPanel> panel, std::shared_ptr<UIButton> button) {
        m_settings = std::move(panel);
        m_settingsButton = std::move(button);
    }

    void ShowSettings(bool show) {
        if (show == m_settingsShown) return;
        m_settingsShown = show;
        if (m_settings && m_overlay.IsStarted()) {
            if (show) m_overlay.Root()->AddChild(m_settings);
            else m_overlay.Root()->RemoveChild(m_settings);
            m_overlay.Invalidate();
        }
        if (m_settingsButton) m_settingsButton->SetText(show ? "Hide settings" : "Settings");
        if (onSettingsShown) onSettingsShown(show);
        Redraw();
    }
    bool SettingsShown() const { return m_settingsShown; }
    std::function<void(bool shown)> onSettingsShown;

    // --- Input: the UI first. Positions here are layout units of the view;
    // the overlay layer takes device pixels of the picture. ---
    bool OnMouseDown(float x, float y, MouseButton button) override {
        SetFocus();   // keys come here (Esc, and typing into the UI)
        if (m_overlay.IsStarted() && m_overlay.MouseDown(x * m_scale, y * m_scale, button)) {
            m_uiPress = true;
            UIChanged();
            return true;
        }
        m_button = button;
        m_dragging = true;
        m_lastX = x;
        m_lastY = y;
        return true;
    }
    bool OnMouseUp(float x, float y, MouseButton button) override {
        if (m_uiPress) {
            m_uiPress = false;
            m_overlay.MouseUp(x * m_scale, y * m_scale, button);
            UIChanged();
            return true;
        }
        m_dragging = false;
        return true;
    }
    bool OnMouseMove(float x, float y) override {
        if (m_dragging) {
            const float dx = x - m_lastX, dy = y - m_lastY;
            m_lastX = x;
            m_lastY = y;
            if (m_button == MouseButton::Left) m_scene->camera.Turn(dx, dy);
            else m_scene->camera.Move(dx, dy, GetRect().height);
            Redraw();
            return true;
        }
        if (!m_overlay.IsStarted()) return false;
        const bool taken = m_overlay.MouseMove(x * m_scale, y * m_scale);
        SetMouseCursor(m_overlay.Cursor());   // an I-beam over an edit box, and so on
        UIChanged();                          // hover shows on the controls
        return taken || m_uiPress;
    }
    bool OnMouseWheel(float delta, float x, float y, const KeyModifiers& modifiers) override {
        if (m_overlay.IsStarted() && m_overlay.MouseWheel(delta, x * m_scale, y * m_scale, modifiers)) {
            UIChanged();
            return true;
        }
        m_scene->camera.Zoom(std::pow(0.88f, delta));
        Redraw();
        return true;
    }
    bool OnMagnify(float scale, float, float) override {
        if (scale > 0.0f) m_scene->camera.Zoom(1.0f / scale);
        Redraw();
        return true;
    }
    void OnMouseLeave() override {
        UIGLView::OnMouseLeave();
        if (m_overlay.IsStarted()) m_overlay.MouseLeave();
        UIChanged();
    }
    bool OnKeyDown(KeyCode key, const KeyModifiers& modifiers) override {
        // Keys are the UI's only while it wants them (an open dropdown, a
        // focused edit box); otherwise the scene's.
        if (m_overlay.IsStarted() && m_overlay.WantsKeyboard()) {
            m_overlay.KeyDown(key, modifiers);
            UIChanged();
            return true;
        }
        if (key == KeyCode::Escape) {
            ShowSettings(!m_settingsShown);
            return true;
        }
        return UIGLView::OnKeyDown(key, modifiers);
    }
    bool OnChar(unsigned int codePoint) override {
        if (!m_overlay.IsStarted() || !m_overlay.WantsTextInput()) return false;
        m_overlay.Char(codePoint);
        UIChanged();
        return true;
    }
    bool OnImeComposition(const std::string& text) override {
        if (!m_overlay.IsStarted() || !m_overlay.WantsTextInput()) return false;
        m_overlay.Composition(text);
        UIChanged();
        return true;
    }
    void OnFocusChanged(bool focused) override {
        if (!focused && m_overlay.IsStarted()) m_overlay.FocusLost();
    }
    void OnTimer(unsigned int timerId) override {
        if (timerId != kUITimer) return UIGLView::OnTimer(timerId);
        // A few frames after each change: the controls' hover and press
        // animations, a dropdown opening.
        Redraw();
        if (--m_uiFrames <= 0) StopTimer(kUITimer);
    }

private:
    void DrawOverlay(const GLFrame& frame) {
        m_scale = frame.scale;
        if (!m_overlay.IsStarted()) {
            UIOverlayLayerOptions options;
            options.width = frame.width;
            options.height = frame.height;
            options.scale = frame.scale;
            options.getProcAddress = [](const char* name) { return reinterpret_cast<void*>(GLSurface::GetProcAddress(name)); };
            std::string error;
            if (!m_overlay.Start(options, &error)) return;   // the 3D picture alone
            if (buildOverlay) buildOverlay(*this, m_overlay);
            if (m_settings && m_settingsShown) m_overlay.Root()->AddChild(m_settings);
        } else if (frame.width != m_overlay.Width() || frame.height != m_overlay.Height()) {
            m_overlay.SetSize(frame.width, frame.height);
        }
        if (m_overlay.Scale() != frame.scale) m_overlay.SetScale(frame.scale);
        PlaceControls(static_cast<float>(frame.width) / frame.scale);
        // Over the framebuffer the view drew into (bound now).
        m_overlay.Render();
    }

    // The button and the panel at the right edge (layout units).
    void PlaceControls(float width) {
        for (UIRenderView* control : { static_cast<UIRenderView*>(m_settingsButton.get()), static_cast<UIRenderView*>(m_settings.get()) }) {
            if (!control) continue;
            const UIRect r = control->GetRect();
            const float x = width - r.width - 16.0f;
            if (std::abs(r.x - x) > 0.5f) {
                control->SetRect(UIRect(x, r.y, r.width, r.height));
                control->OnLayout();
            }
        }
    }

    void UIChanged() {
        Redraw();
        m_uiFrames = 45;
        StartTimer(16, kUITimer);
    }

    static constexpr unsigned int kUITimer = 1;
    std::shared_ptr<Scene3D> m_scene;
    UIOverlayLayer m_overlay;
    std::shared_ptr<UIPanel> m_settings;
    std::shared_ptr<UIButton> m_settingsButton;
    bool m_settingsShown = true;
    float m_scale = 1.0f;
    bool m_uiPress = false;
    MouseButton m_button = MouseButton::Left;
    bool m_dragging = false;
    float m_lastX = 0.0f, m_lastY = 0.0f;
    int m_uiFrames = 0;
};

// ---------------------------------------------------------------------------
// The app: a menu bar, the GL view, a status bar; the overlay layer's
// controls over the view
// ---------------------------------------------------------------------------
class GLView3DApp : public Application {
public:
    void OnInit() override {
        WindowCreateOptions options;
        options.width = 1100;
        options.height = 760;
        options.title = L"GLView 3D";
        SetWindowOptions(options);
        auto style = GetDefaultStyle();

        auto root = std::make_shared<UIPanel>(0.0f, 0.0f, 1100.0f, 760.0f);
        root->SetStyle(style);
        root->SetType(UIStyleType::Window);
        root->SetLayoutType(LayoutType::Flex);
        root->GetLayoutNode()->SetFlexDirection(FlexDirection::Column);
        root->GetLayoutNode()->SetAlignItems(AlignItems::Stretch);

        m_scene = std::make_shared<Scene3D>();
        Bitmap earth;
        if (LoadEarth(earth)) m_scene->SetMap(std::move(earth));
        m_view = std::make_shared<GlobeView>(m_scene);
        m_view->SetStyle(style);
        m_view->buildOverlay = [this](GlobeView& view, UIOverlayLayer& overlay) { BuildOverlay(view, overlay); };
        m_view->onSettingsShown = [this](bool shown) { m_viewMenu->SetItemChecked("settings", shown); };

        // The window's own controls: a menu bar on top, a status bar below.
        m_menuBar = std::make_shared<UIMenuBar>(0.0f, 0.0f, 0.0f, 0.0f);
        m_menuBar->SetStyle(style);
        BuildMenus();
        root->AddControl(m_menuBar)->SetHeight({ SizingMode::Fixed, m_menuBar->GetPreferredThickness() });
        root->AddControl(m_view)->SetFlexGrow(1.0f);
        m_statusBar = std::make_shared<UIStatusBar>(0.0f, 0.0f, 0.0f, 0.0f);
        m_statusBar->SetStyle(style);
        m_status = m_statusBar->AddPanel("");
        m_statusCamera = m_statusBar->AddPanel("", 230.0f);
        m_statusPicture = m_statusBar->AddPanel("", 250.0f);
        root->AddControl(m_statusBar)->SetHeight({ SizingMode::Fixed, UIStatusBar::kThickness });

        root->BuildRenderItems();
        SetRootView(root);
        SetStatus(UIGLView::IsAvailable() ? "Drag the globe to turn it. Esc shows or hides the settings."
                                          : "OpenGL 3.3 is not available on this computer.");
        StartSpin();
        // The status bar follows the camera and the picture a few times a second.
        SetInterval(250, [this] { UpdateStatus(); }, m_view);
    }

private:
    // --- The menu bar ---
    void BuildMenus() {
        auto file = m_menuBar->AddMenu("File");
        file->AddCommand("open", "Open map...", "");
        file->AddCommand("earth", "Earth map");
        file->AddSeparator();
        file->AddCommand("save", "Save picture...");
        file->AddSeparator();
        file->AddCommand("exit", "Exit");
        file->ItemClickedEvent.AddListener([this](UIRenderView*, const std::string& command) {
            if (command == "open") OpenMap();
            else if (command == "earth") UseEarth();
            else if (command == "save") SavePicture();
            else if (command == "exit") Quit();
        });

        m_viewMenu = m_menuBar->AddMenu("View");
        m_viewMenu->AddCommand("settings", "Settings", "Esc");
        m_viewMenu->AddCommand("spin", "Spin");
        m_viewMenu->AddSeparator();
        m_viewMenu->AddCommand("reset", "Reset view");
        m_viewMenu->SetItemChecked("settings", true);
        m_viewMenu->SetItemChecked("spin", m_scene->settings.spin);
        m_viewMenu->ItemClickedEvent.AddListener([this](UIRenderView*, const std::string& command) {
            if (command == "settings") m_view->ShowSettings(!m_view->SettingsShown());
            else if (command == "spin") SetSpin(!m_scene->settings.spin);
            else if (command == "reset") ResetView();
        });
    }

    void OpenMap() {
        FileDialogOptions options;
        options.title = "Open a world map";
        options.filters = { { "Pictures", "*.jpg;*.jpeg;*.png;*.webp;*.bmp" } };
        GetHost()->ShowFileDialog(options, [this](const std::vector<std::string>& paths) {
            if (!paths.empty()) UseMap(paths[0]);
        });
    }

    void UseMap(const std::string& path) {
        Bitmap picture;
        if (!picture.Load(path) || !m_scene->SetMap(std::move(picture))) {
            SetStatus("Not a picture: " + path);
            return;
        }
        m_view->Redraw();
        SetStatus("Map: " + path.substr(path.find_last_of("/\\") + 1));
    }

    void UseEarth() {
        Bitmap picture;
        if (!LoadEarth(picture) || !m_scene->SetMap(std::move(picture))) {
            SetStatus("The earth map (earth.jpg) was not found.");
            return;
        }
        m_view->Redraw();
        SetStatus("Map: the earth (Natural Earth II)");
    }

    // The last picture of the view (the overlay layer's controls in it too),
    // as GLSurface read it back.
    void SavePicture() {
        FileDialogOptions options;
        options.mode = FileDialogMode::Save;
        options.title = "Save the picture";
        options.filters = { { "PNG picture", "*.png" } };
        options.defaultExtension = "png";
        options.fileName = "globe.png";
        GetHost()->ShowFileDialog(options, [this](const std::vector<std::string>& paths) {
            if (paths.empty()) return;
            const Bitmap& picture = m_view->Surface().Picture();
            if (!picture.IsEmpty() && picture.Save(paths[0], ImageFileFormat::Png)) SetStatus("Saved " + paths[0]);
            else SetStatus("Could not save " + paths[0]);
        });
    }

    // --- The status bar ---
    void SetStatus(const std::string& text) { m_status->SetText(text); }

    void UpdateStatus() {
        const OrbitCamera& camera = m_scene->camera;
        char text[160];
        std::snprintf(text, sizeof text, "Distance %.1f   Turn %d\xC2\xB0   Up %d\xC2\xB0", camera.distance,
                      static_cast<int>(std::lround(std::fmod(camera.yaw * 180.0f / kPi + 3600.0f, 360.0f))),
                      static_cast<int>(std::lround(camera.pitch * 180.0f / kPi)));
        m_statusCamera->SetText(text);
        const GLSurface& surface = m_view->Surface();
        std::snprintf(text, sizeof text, "%d x %d px   %d samples   %.1f ms", surface.Width(), surface.Height(),
                      m_view->GetSamples(), m_scene->drawMs);
        m_statusPicture->SetText(text);
    }

    // --- The overlay layer's controls, over the 3D picture. They live on its
    // root (a see-through view the size of the picture), placed by position. ---
    void BuildOverlay(GlobeView& view, UIOverlayLayer& overlay) {
        // Top left: what this is and how to move around, on a card so it
        // reads over any background.
        auto card = std::make_shared<UIPanel>(16.0f, 16.0f, 500.0f, 64.0f);
        card->SetOpacity(0.88f);
        card->SetLayoutType(LayoutType::Flex);
        card->GetLayoutNode()->SetFlexDirection(FlexDirection::Column);
        card->GetLayoutNode()->SetPadding(Thickness(12.0f, 8.0f, 12.0f, 8.0f));
        auto title = std::make_shared<UIStatic>(0.0f, 0.0f, 0.0f, 0.0f);
        title->SetText("GLView + UIOverlayLayer");
        card->AddControl(title)->SetHeight({ SizingMode::Fixed, 24.0f });
        auto hint = std::make_shared<UIStatic>(0.0f, 0.0f, 0.0f, 0.0f);
        hint->SetText("Left drag: turn    Right drag: move    Wheel: zoom    Esc: settings");
        card->AddControl(hint)->SetHeight({ SizingMode::Fixed, 22.0f });
        card->OnLayout();
        overlay.Root()->AddChild(card);

        auto toggle = std::make_shared<UIButton>(0.0f, 16.0f, 140.0f, 32.0f);
        toggle->SetText("Hide settings");
        toggle->ClickEvent.AddListener([&view](UIRenderView*, const EventArgs&) { view.ShowSettings(!view.SettingsShown()); });
        overlay.Root()->AddChild(toggle);

        // The settings: a panel laid out in a column.
        auto panel = std::make_shared<UIPanel>(0.0f, 60.0f, 280.0f, 560.0f);
        panel->SetOpacity(0.94f);
        panel->SetLayoutType(LayoutType::Flex);
        panel->GetLayoutNode()->SetFlexDirection(FlexDirection::Column);
        panel->GetLayoutNode()->SetAlignItems(AlignItems::Stretch);
        panel->GetLayoutNode()->SetPadding(Thickness(16.0f));
        panel->GetLayoutNode()->SetGap(6.0f);
        auto add = [&panel](std::shared_ptr<UIRenderView> control, float height) {
            panel->AddControl(control)->SetHeight({ SizingMode::Fixed, height });
        };
        auto label = [&add](const std::string& text) {
            auto l = std::make_shared<UIStatic>(0.0f, 0.0f, 0.0f, 0.0f);
            l->SetText(text);
            add(l, 20.0f);
        };
        const Settings& s = m_scene->settings;

        label("Settings");
        m_spinBox = std::make_shared<UICheckBox>(0.0f, 0.0f, 0.0f, 0.0f);
        m_spinBox->SetText("Spin");
        m_spinBox->SetChecked(s.spin);
        m_spinBox->ToggleEvent.AddListener([this](UIRenderView*, const ToggleEventArgs& args) { SetSpin(args.isChecked); });
        add(m_spinBox, 26.0f);

        label("Spin speed");
        auto speed = std::make_shared<UISlider>(0.0f, 0.0f, 0.0f, 0.0f);
        speed->SetRange(0.1, 4.0);
        speed->SetValue(s.speed);
        speed->ValueChangedEvent.AddListener([this](UIRenderView*, const double& value) {
            m_scene->settings.speed = static_cast<float>(value);
        });
        add(speed, 26.0f);

        label("Sun");
        auto sun = std::make_shared<UISlider>(0.0f, 0.0f, 0.0f, 0.0f);
        sun->SetRange(0.0, 360.0);
        sun->SetValue(s.sunAngle);
        sun->ValueChangedEvent.AddListener([this](UIRenderView*, const double& value) {
            m_scene->settings.sunAngle = static_cast<float>(value);
            m_view->Redraw();
        });
        add(sun, 26.0f);

        auto check = [&](const char* text, bool Settings::*field) {
            auto box = std::make_shared<UICheckBox>(0.0f, 0.0f, 0.0f, 0.0f);
            box->SetText(text);
            box->SetChecked(s.*field);
            box->ToggleEvent.AddListener([this, field](UIRenderView*, const ToggleEventArgs& args) {
                m_scene->settings.*field = args.isChecked;
                m_view->Redraw();
            });
            add(box, 26.0f);
        };
        check("Tilted axis (23.4\xC2\xB0)", &Settings::tilt);
        check("Stand", &Settings::stand);
        check("Floor grid", &Settings::floor);

        label("Background");
        auto backdrop = std::make_shared<UIComboBox>(0.0f, 0.0f, 0.0f, 0.0f);
        for (const Backdrop& b : kBackdrops) backdrop->AddItem(b.name);
        backdrop->SetSelectedIndex(s.backdrop);
        backdrop->SelectionChangedEvent.AddListener([this](UIRenderView*, const SelectionEventArgs& args) {
            if (args.selectedIndex < 0 || args.selectedIndex >= static_cast<int>(std::size(kBackdrops))) return;
            m_scene->settings.backdrop = args.selectedIndex;
            m_view->Redraw();
        });
        add(backdrop, 30.0f);

        label("Smooth edges (samples per pixel)");
        auto samples = std::make_shared<UIComboBox>(0.0f, 0.0f, 0.0f, 0.0f);
        for (const char* name : { "Off", "2", "4", "8" }) samples->AddItem(name);
        samples->SetSelectedIndex(2);   // UIGLView's default: 4
        samples->SelectionChangedEvent.AddListener([this](UIRenderView*, const SelectionEventArgs& args) {
            const int counts[] = { 0, 2, 4, 8 };
            if (args.selectedIndex >= 0 && args.selectedIndex < 4) m_view->SetSamples(counts[args.selectedIndex]);
        });
        add(samples, 30.0f);

        auto reset = std::make_shared<UIButton>(0.0f, 0.0f, 0.0f, 0.0f);
        reset->SetText("Reset view");
        reset->ClickEvent.AddListener([this](UIRenderView*, const EventArgs&) { ResetView(); });
        add(reset, 32.0f);

        auto note = std::make_shared<UIStatic>(0.0f, 0.0f, 0.0f, 0.0f);
        note->SetWordWrap(true);
        note->SetText("UIOverlayLayer draws these controls into the GL view's picture. They get the mouse "
                      "and the keys first; what they leave moves the camera.");
        panel->AddControl(note)->SetFlexGrow(1.0f);
        panel->OnLayout();

        view.SetSettingsPanel(panel, toggle);
    }

    void SetSpin(bool spin) {
        m_scene->settings.spin = spin;
        if (m_spinBox && m_spinBox->IsChecked() != spin) m_spinBox->SetChecked(spin);
        m_viewMenu->SetItemChecked("spin", spin);
        if (spin) StartSpin();
        else ClearTimer(m_spinTimer);
        m_view->Redraw();
    }

    void ResetView() {
        m_scene->camera = OrbitCamera();
        m_view->Redraw();
    }

    void StartSpin() {
        // An animation draws again every frame: Redraw from a timer.
        ClearTimer(m_spinTimer);
        m_spinTimer = SetInterval(16, [this] {
            m_scene->spin += 0.004f * m_scene->settings.speed;
            m_view->Redraw();
        }, m_view);
    }

    std::shared_ptr<Scene3D> m_scene;
    std::shared_ptr<GlobeView> m_view;
    std::shared_ptr<UIMenuBar> m_menuBar;
    std::shared_ptr<UIMenu> m_viewMenu;
    std::shared_ptr<UIStatusBar> m_statusBar;
    std::shared_ptr<UIStatic> m_status, m_statusCamera, m_statusPicture;
    std::shared_ptr<UICheckBox> m_spinBox;
    TimerId m_spinTimer = 0;
};

} // namespace

int main(int, char**) {
    GLView3DApp app;
    return app.Run();
}
