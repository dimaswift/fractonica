// mesh_builder.hpp
#pragma once
#include "mesh.hpp"
#include <cmath>

// ─────────────────────────────────────────────
//  Stateful builder — produces a Mesh
// ─────────────────────────────────────────────
class MeshBuilder {
public:
    explicit MeshBuilder(MeshPrimitive default_prim = MeshPrimitive::Triangles)
        : _prim(default_prim) {}

    // ── Primitive selection ────────────────────
    MeshBuilder& primitive(MeshPrimitive p) { _prim = p; return *this; }
    MeshBuilder& begin_group()  { _flush(); return *this; }

    // ── Shared color for next vertices ─────────
    MeshBuilder& color(float r, float g, float b, float a = 1.f)
        { _cr=r; _cg=g; _cb=b; _ca=a; return *this; }
    MeshBuilder& flat_color(float r, float g, float b)
        { _ensure_submesh().flat_color(r,g,b); return *this; }

    // ── Vertex emission ────────────────────────
    MeshBuilder& v2(float x, float y)
        { return _push(Vertex{}.pos(x,y,0).color(_cr,_cg,_cb,_ca)); }

    MeshBuilder& v3(float x, float y, float z)
        { return _push(Vertex{}.pos(x,y,z).color(_cr,_cg,_cb,_ca)); }

    MeshBuilder& v3uv(float x, float y, float z, float u, float v)
        { return _push(Vertex{}.pos(x,y,z).uv(u,v).color(_cr,_cg,_cb,_ca)); }

    // ── Finalize ───────────────────────────────
    Mesh build() {
        _flush();
        return std::move(_mesh);
    }

    // Reset builder for reuse (e.g. dynamic meshes rebuilt each frame)
    MeshBuilder& reset() {
        _mesh.clear();
        _current = nullptr;
        return *this;
    }

private:
    Mesh        _mesh;
    SubMesh*    _current = nullptr;
    MeshPrimitive _prim  = MeshPrimitive::Triangles;
    float _cr=1, _cg=1, _cb=1, _ca=1;

    SubMesh& _ensure_submesh() {
        if (!_current)
            _current = &_mesh.add_submesh(_prim);
        return *_current;
    }

    void _flush() {
        _current = nullptr; // next vertex starts a fresh submesh
    }

    MeshBuilder& _push(const Vertex& v) {
        _ensure_submesh().add_vertex(v);
        return *this;
    }
};