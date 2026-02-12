//
// Created by Dmitry Popov on 12.02.2026.
//

#ifndef FRACTONICA_COMPUTE_H
#define FRACTONICA_COMPUTE_H
#include "sokol_gfx.h"

namespace Fractonica {

    class ComputeShader {
        sg_pipeline pip_ = {};
        sg_bindings bindings_ = {};
        sg_shader shader_ = {};
    public:

        void setup(const sg_shader_desc* shader_desc);
        void shutdown() const;
        void bind(int slot, sg_view view);
        void bind_sampler(int slot, sg_sampler sampler);
        template<typename T>
        void dispatch(int dim_x, int dim_y, int dim_z, int ub_slot, const T& uniforms) const;
        void dispatch(int dim_x, int dim_y, int dim_z) const;
    };

    inline void ComputeShader::setup(const sg_shader_desc* shader_desc) {
        sg_pipeline_desc desc = {};
        shader_ = sg_make_shader(shader_desc);
        desc.compute = true;
        desc.shader = shader_;
        desc.label = "compute";
        pip_ = sg_make_pipeline(&desc);
    }

    inline void ComputeShader::shutdown() const {
        sg_destroy_pipeline(pip_);
        sg_dealloc_shader(shader_);
    }

    inline void ComputeShader::bind(int slot, sg_view view) {
        if (slot >= 0 && slot < SG_MAX_VIEW_BINDSLOTS) {
            bindings_.views[slot] = view;
        }
    }

    inline void ComputeShader::bind_sampler(int slot, sg_sampler sampler) {
        if (slot >= 0 && slot < SG_MAX_SAMPLER_BINDSLOTS) {
            bindings_.samplers[slot] = sampler;
        }
    }

    template<typename T> void ComputeShader::dispatch(int dim_x, int dim_y, int dim_z, int ub_slot, const T& uniforms) const {
        sg_pass pass = {};
        pass.compute = true;
        sg_begin_pass(&pass);
        sg_apply_pipeline(pip_);
        sg_apply_bindings(&bindings_);

        sg_range range = { &uniforms, sizeof(T) };
        sg_apply_uniforms(ub_slot, range);

        sg_dispatch(dim_x, dim_y, dim_z);
        sg_end_pass();
    }

    inline void ComputeShader::dispatch(int dim_x, int dim_y, int dim_z) const {
        sg_pass pass = {};
        pass.compute = true;
        sg_begin_pass(&pass);
        sg_apply_pipeline(pip_);
        sg_apply_bindings(&bindings_);
        sg_dispatch(dim_x, dim_y, dim_z);
        sg_end_pass();
    }
}

#endif //FRACTONICA_COMPUTE_H