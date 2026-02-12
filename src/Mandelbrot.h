//
// Created by Dmitry Popov on 12.02.2026.
//
#ifndef FRACTONICA_MANDELBROT_H
#define FRACTONICA_MANDELBROT_H
#include "ComputeTexture.h"
#include "mandelbrot.glsl.h"

namespace Fractonica {

    class Mandelbrot {
    private:
        ComputeTexture tex = {};
        float zoom = 0.8f;
        float center_x = -0.5f;
        float center_y = 0.0f;
        int width = 512;
        int height = 512;
    public:

        void setup(int w, int h);
        void draw();
        void compute();
        void drawGui();
        [[nodiscard]] int getWidth() const;
        [[nodiscard]] int getHeight() const;
        void shutdown();
    };

    inline void Mandelbrot::shutdown() {
        tex.shutdown();
    }

    inline int Mandelbrot::getHeight() const { return height; }

    inline int Mandelbrot::getWidth() const { return width;}

    inline void Mandelbrot::drawGui() {
        ImGui::SliderFloat("Zoom", &zoom, 0.1f, 10.0f);
        ImGui::SliderFloat("Center X", &center_x, -2.0f, 2.0f);
        ImGui::SliderFloat("Center Y", &center_y, -2.0f, 2.0f);
        if (ImGui::Button("Reset")) {
            zoom = 0.8f;
            center_x = -0.5f;
            center_y = 0.0f;
        }
    }

    inline void Mandelbrot::draw() {
        tex.render();
    }

    inline void Mandelbrot::compute() {
        cs_params_t params = {};
        params.center[0] = center_x;
        params.center[1] = center_y;
        params.zoom = zoom;
        params.width = width;
        params.height = height;
        tex.compute(params);
    }

    inline void Mandelbrot::setup(const int w,  const int h) {
        width = w;
        height = h;
        tex.setup(width,height, mandelbrot_shader_desc(sg_query_backend()), display_shader_desc(sg_query_backend()));
    }
}

#endif //FRACTONICA_MANDELBROT_H