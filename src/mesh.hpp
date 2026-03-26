// mesh.hpp
#pragma once
#include <vector>
#include <functional>
#include <cstdint>
#include <cmath>
#include "sokol_gl.h"

// ─────────────────────────────────────────────
//  Vertex (matches all sgl vertex variants)
// ─────────────────────────────────────────────
struct Vertex {
    // Position
    float x = 0, y = 0, z = 0;
    // Texture coords
    float u = 0, v = 0;
    // Color (normalized 0-1)
    float r = 1, g = 1, b = 1, a = 1;

    // ── Builder-style setters ──────────────────
    Vertex& pos(float px, float py, float pz = 0.f)
        { x=px; y=py; z=pz; return *this; }
    Vertex& uv(float pu, float pv)
        { u=pu; v=pv; return *this; }
    Vertex& color(float pr, float pg, float pb, float pa = 1.f)
        { r=pr; g=pg; b=pb; a=pa; return *this; }
    Vertex& color_b(uint8_t pr, uint8_t pg, uint8_t pb, uint8_t pa = 255)
        { r=pr/255.f; g=pg/255.f; b=pb/255.f; a=pa/255.f; return *this; }
};

// ─────────────────────────────────────────────
//  Primitive type (mirrors sgl_begin_* modes)
// ─────────────────────────────────────────────
enum class MeshPrimitive : uint8_t {
    Triangles,
    Quads,
    Lines,
    LineStrip,
    Points,
};

// ─────────────────────────────────────────────
//  A single submesh (one draw call / one color
//  group, matching how sokol-gl batches draws)
// ─────────────────────────────────────────────
struct SubMesh {
    std::vector<Vertex>   vertices;
    MeshPrimitive         primitive = MeshPrimitive::Triangles;

    // Optional flat color override (applied via sgl_c3f before vertices)
    bool  has_flat_color = false;
    float fc_r=1, fc_g=1, fc_b=1;

    SubMesh& set_primitive(MeshPrimitive p) { primitive = p; return *this; }
    SubMesh& flat_color(float r, float g, float b)
        { has_flat_color=true; fc_r=r; fc_g=g; fc_b=b; return *this; }

    SubMesh& add_vertex(const Vertex& v)
        { vertices.push_back(v); return *this; }

    void reserve(size_t n) { vertices.reserve(n); }
    void clear()           { vertices.clear(); }
    bool empty() const     { return vertices.empty(); }
};

// ─────────────────────────────────────────────
//  Mesh — collection of submeshes
//  (one Mesh = one logical object, e.g. "cube")
// ─────────────────────────────────────────────
class Mesh {
public:
    std::vector<SubMesh> submeshes;

    // ── Construction helpers ───────────────────

    SubMesh& add_submesh(MeshPrimitive prim = MeshPrimitive::Triangles) {
        submeshes.emplace_back();
        submeshes.back().primitive = prim;
        return submeshes.back();
    }

    void clear() { submeshes.clear(); }
    bool empty() const { return submeshes.empty(); }

    // ── Draw ──────────────────────────────────
    void draw() const {
        for (const auto& sm : submeshes) {
            if (sm.empty()) continue;

            _begin(sm.primitive);

            if (sm.has_flat_color)
                sgl_c3f(sm.fc_r, sm.fc_g, sm.fc_b);

            for (const auto& v : sm.vertices)
                _emit(v);

            sgl_end();
        }
    }

private:
    static void _begin(MeshPrimitive p) {
        switch (p) {
            case MeshPrimitive::Triangles:  sgl_begin_triangles();   break;
            case MeshPrimitive::Quads:      sgl_begin_quads();       break;
            case MeshPrimitive::Lines:      sgl_begin_lines();       break;
            case MeshPrimitive::LineStrip:  sgl_begin_line_strip();  break;
            case MeshPrimitive::Points:     sgl_begin_points();      break;
        }
    }

    static void _emit(const Vertex& v) {
        sgl_c4f(v.r, v.g, v.b, v.a);
        sgl_v3f_t2f(v.x, v.y, v.z, v.u, v.v);
    }
};