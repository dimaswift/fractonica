//
// Created by Dmitry Popov on 12.02.2026.
//

#ifndef FRACTONICA_COMPUTETEXTURE_H
#define FRACTONICA_COMPUTETEXTURE_H
#include "SpriteRenderer.h"
#include "ComputeShader.h"

namespace Fractonica {

    class ComputeTexture {

        sg_image target_img_ = {};
        sg_view storage_view_ = {};
        sg_view texture_view_ = {};
        int width_ = 512;
        int height_ = 512;
        ComputeShader compute_shader_;
        SpriteRenderer sprite_;

    public:
        void setup(int width, int height, const sg_shader_desc *computeShader, const sg_shader_desc *fragShader, int computeSlot = 0) ;
        void shutdown() const;
        void render();
        template<typename T> void compute(const T& uniforms);
    };

    inline void ComputeTexture::shutdown() const {
        sg_destroy_image(target_img_);
        sg_destroy_view(storage_view_);
        sg_destroy_view(texture_view_);
        compute_shader_.shutdown();
        sprite_.shutdown();
    }

    inline void ComputeTexture::render() {
        sprite_.render(0, texture_view_);
    }

    template<typename T>
    void ComputeTexture::compute(const T& uniforms) {

        const int groups_x = (width_ + 15) / 16;
        const int groups_y = (height_ + 15) / 16;
        compute_shader_.dispatch(groups_x, groups_y, 1, 0, uniforms);
    }

    inline void ComputeTexture::setup(int width, int height, const sg_shader_desc *computeShader, const sg_shader_desc *fragShader, int computeSlot) {

        width_ = width;
        height_ = height;
        sprite_.init();
        sprite_.setup(fragShader);
        compute_shader_.setup(computeShader);

        sg_image_desc img_desc = {};
        img_desc.type = SG_IMAGETYPE_2D;
        img_desc.width = width;
        img_desc.height = height;
        img_desc.num_slices = 1;
        img_desc.num_mipmaps = 1;
        img_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
        img_desc.sample_count = 1;
        img_desc.usage.storage_image = true;
        img_desc.label = "compute-image";
        target_img_ = sg_make_image(&img_desc);

        // Storage image view (for compute write)
        sg_view_desc storage_desc = {};
        storage_desc.storage_image.image = target_img_;
        storage_desc.label = "compute-storage-view";
        storage_view_ = sg_make_view(&storage_desc);

        // Texture view (for fragment sample)
        sg_view_desc texture_desc = {};
        texture_desc.texture.image = target_img_;
        texture_desc.label = "compute-texture-view";
        texture_view_ = sg_make_view(&texture_desc);

        compute_shader_.bind(computeSlot, storage_view_);
    }
}

#endif //FRACTONICA_COMPUTETEXTURE_H