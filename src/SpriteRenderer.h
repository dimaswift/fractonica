//
// Created by Dmitry Popov on 12.02.2026.
//

#ifndef FRACTONICA_QUAD_H
#define FRACTONICA_QUAD_H

namespace Fractonica {

    struct Quad {
        sg_buffer vbuf = {};
        sg_buffer ibuf = {};
        sg_pipeline pip = {};
        sg_sampler smp = {};
        sg_bindings binds = {};

    };


    class SpriteRenderer {
    private:
        Quad quad_;
        bool loaded_ = false;
        sg_shader shader_ = {};
        public:
        void init();
        void render(int index, sg_view view);
        void setup(const sg_shader_desc *desc, const int displayPos = 0, const int displayTexcoord0 = 1);
        void shutdown() const;
    };

    inline void SpriteRenderer::shutdown() const {
        sg_destroy_pipeline(quad_.pip);
        sg_destroy_buffer(quad_.ibuf);
        sg_destroy_buffer(quad_.vbuf);
        sg_destroy_sampler(quad_.smp);
        sg_destroy_shader(shader_);
    }

    inline void SpriteRenderer::render(int index, sg_view view) {

        if (!loaded_) return;
        quad_.binds.views[index] = view;
        sg_apply_pipeline(quad_.pip);
        sg_apply_bindings(&quad_.binds);

        sg_draw(0, 6, 1);
    }

    inline void SpriteRenderer::init() {
        static constexpr float VERTS[] = {
            -1.0f, -1.0f,  0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f,  0.0f, 1.0f,
        };
        static constexpr uint16_t INDICES[] = { 0, 1, 2,  0, 2, 3 };

        sg_buffer_desc vb_desc = {};
        vb_desc.data = SG_RANGE(VERTS);
        vb_desc.label = "quad-vertices";
        quad_.vbuf = sg_make_buffer(&vb_desc);

        sg_buffer_desc ib_desc = {};
        ib_desc.usage.index_buffer = true;
        ib_desc.data = SG_RANGE(INDICES);
        ib_desc.label = "quad-indices";
        quad_.ibuf = sg_make_buffer(&ib_desc);

        sg_sampler_desc smp_desc = {};
        smp_desc.min_filter = SG_FILTER_LINEAR;
        smp_desc.mag_filter = SG_FILTER_LINEAR;
        smp_desc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
        smp_desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
        quad_.smp = sg_make_sampler(&smp_desc);

        quad_.binds.vertex_buffers[0] = quad_.vbuf;
        quad_.binds.index_buffer = quad_.ibuf;
    }

    inline void SpriteRenderer::setup(const sg_shader_desc *desc, const int displayPos, const int displayTexcoord0) {

        sg_pipeline_desc pip_desc = {};
        shader_ = sg_make_shader(desc);
        pip_desc.shader = shader_;
        pip_desc.layout.attrs[displayPos].format = SG_VERTEXFORMAT_FLOAT2;
        pip_desc.layout.attrs[displayTexcoord0].format = SG_VERTEXFORMAT_FLOAT2;
        pip_desc.index_type = SG_INDEXTYPE_UINT16;
        pip_desc.label = "quad-pipeline";
        quad_.pip = sg_make_pipeline(&pip_desc);
        quad_.binds.samplers[0] = quad_.smp;

        loaded_ = true;
    }
}

#endif //FRACTONICA_QUAD_H