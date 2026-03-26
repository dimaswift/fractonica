//
// Created by Dmitry Popov on 23.03.2026.
//

#ifndef FRACTONICA_SOLIDEXPLORER_H
#define FRACTONICA_SOLIDEXPLORER_H
#include "mesh.hpp"
#include "mesh_builder.hpp"

namespace Fractonica {
    class SolidExplorer {
        Mesh g_cube;
        sgl_pipeline pip = {};

    public:
        void init();

        void draw();
        void shutdown() const;
    };

    inline Mesh cube() {
        MeshBuilder b(MeshPrimitive::Lines);

        // Back face   — red
        b.begin_group().flat_color(1, 0, 0)
                .v3uv(-1, 1, -1, -1, 1).v3uv(1, 1, -1, 1, 1)
                .v3uv(1, -1, -1, 1, -1).v3uv(-1, -1, -1, -1, -1);

        // Front face  — green
        b.begin_group().flat_color(0, 1, 0)
                .v3uv(-1, -1, 1, -1, 1).v3uv(1, -1, 1, 1, 1)
                .v3uv(1, 1, 1, 1, -1).v3uv(-1, 1, 1, -1, -1);

        // Left face   — blue
        b.begin_group().flat_color(0, 0, 1)
                .v3uv(-1, -1, 1, -1, 1).v3uv(-1, 1, 1, 1, 1)
                .v3uv(-1, 1, -1, 1, -1).v3uv(-1, -1, -1, -1, -1);

        // Right face  — orange
        b.begin_group().flat_color(1, .5f, 0)
                .v3uv(1, -1, 1, -1, 1).v3uv(1, -1, -1, 1, 1)
                .v3uv(1, 1, -1, 1, -1).v3uv(1, 1, 1, -1, -1);

        // Bottom face — teal
        b.begin_group().flat_color(0, .5f, 1)
                .v3uv(1, -1, -1, -1, 1).v3uv(1, -1, 1, 1, 1)
                .v3uv(-1, -1, 1, 1, -1).v3uv(-1, -1, -1, -1, -1);

        // Top face    — pink
        b.begin_group().flat_color(1, 0, .5f)
                .v3uv(-1, 1, -1, -1, 1).v3uv(-1, 1, 1, 1, 1)
                .v3uv(1, 1, 1, 1, -1).v3uv(1, 1, -1, -1, -1);

        return b.build();
    }

    inline void SolidExplorer::init() {

        pip = sgl_make_pipeline({
            .depth = {
                .write_enabled = true,
                .compare = SG_COMPAREFUNC_LESS_EQUAL,
            },
            .cull_mode = SG_CULLMODE_BACK,
        });
        g_cube = cube();
    }

    inline void SolidExplorer::draw() {


        sgl_defaults();
        sgl_load_pipeline(pip);
        sgl_matrix_mode_projection();
        float aspect = (float)sapp_width() / (float)sapp_height();

        sgl_ortho(-aspect, aspect, -1, 1, -100.0f, 100.0f);

        sgl_matrix_mode_modelview();
        sgl_translate(0.0f, 0.0f, -12.0f);
        sgl_rotate(sgl_rad(45), 1.0f, 0.0f, 0.0f);
        sgl_rotate(sgl_rad(45), 0.0f, 1.0f, 0.0f);

        sgl_begin_lines();

        float s = 0.5;



        sgl_v3f_t2f(s, s, s, 0, 0);
        sgl_v3f_t2f(-s, s, s, 0, 0);

        sgl_v3f_t2f(-s, s, s, 0, 0);
        sgl_v3f_t2f(-s, -s, s, 0, 0);

        sgl_v3f_t2f(-s, -s, s, 0, 0);
        sgl_v3f_t2f(s, -s, s, 0, 0);

        sgl_v3f_t2f(s, -s, s, 0, 0);
        sgl_v3f_t2f(s, s, s, 0, 0);



        sgl_v3f_t2f(s, s, s, 0, 0);
        sgl_v3f_t2f(s, -s, s, 0, 0);

        sgl_v3f_t2f(s, -s, s, 0, 0);
        sgl_v3f_t2f(s, -s, -s, 0, 0);

        sgl_v3f_t2f(s, -s, -s, 0, 0);
        sgl_v3f_t2f(s, s, -s, 0, 0);

        sgl_v3f_t2f(s, s, -s, 0, 0);
        sgl_v3f_t2f(s, s, s, 0, 0);



        sgl_v3f_t2f(s, s, s, 0, 0);
        sgl_v3f_t2f(-s, s, s, 0, 0);

        sgl_v3f_t2f(-s, s, s, 0, 0);
        sgl_v3f_t2f(-s, s, -s, 0, 0);

        sgl_v3f_t2f(-s, s, -s, 0, 0);
        sgl_v3f_t2f(s, s, -s, 0, 0);

        sgl_v3f_t2f(s, s, -s, 0, 0);
        sgl_v3f_t2f(s, s, s, 0, 0);

        sgl_end();

        sgl_draw();
    }

    inline void SolidExplorer::shutdown() const {

        sgl_destroy_pipeline(pip);
    }
}

#endif //FRACTONICA_SOLIDEXPLORER_H
