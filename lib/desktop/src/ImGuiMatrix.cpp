#include "ImGuiMatrix.h"
#include <algorithm>
#include <Utils.h>
namespace Fractonica {

ImGuiMatrix::ImGuiMatrix(uint16_t width,
                         uint16_t height,
                         float scale,
                         LedShape shape,
                         const char* windowName)
    : width_(width),
      height_(height),
      scale_((scale < 1.0f) ? 1.0f : scale),
      shape_(shape),
      windowName_(windowName ? windowName : "Matrix"),
      fb_(static_cast<size_t>(width) * static_cast<size_t>(height), 0u)
{}

uint32_t ImGuiMatrix::getColor(uint8_t r, uint8_t g, uint8_t b) const
{
    return Utils::Color(r,g,b);
}

uint32_t ImGuiMatrix::getColorHSV(uint16_t hue, uint8_t sat, uint8_t val) const {
    return Utils::ColorHSV(hue,sat,val);
}

ImU32 ImGuiMatrix::toImU32(uint32_t rgb)
{
    const int r = (rgb >> 16) & 0xFF;
    const int g = (rgb >> 8)  & 0xFF;
    const int b = (rgb)       & 0xFF;
    return IM_COL32(r, g, b, 255);
}

void ImGuiMatrix::drawPixel(uint16_t x, uint16_t y, uint32_t color)
{
    if (x >= width_ || y >= height_) return;
    fb_[static_cast<size_t>(y) * width_ + x] = color;
}

bool ImGuiMatrix::begin()
{
    begun_ = true;
    return true;
}

void ImGuiMatrix::flush()
{
    if (!begun_) return;

    ImGui::Begin(windowName_);

    // Optional: controls inside the window
    ImGui::SliderFloat("Scale", &scale_, 2.0f, 40.0f, "%.1f");
    int shape = (shape_ == LedShape::Circle) ? 1 : 0;
    if (ImGui::RadioButton("Square", shape == 0)) shape = 0;
    ImGui::SameLine();
    if (ImGui::RadioButton("Circle", shape == 1)) shape = 1;
    shape_ = (shape == 1) ? LedShape::Circle : LedShape::Square;

    const float s = (scale_ < 1.0f) ? 1.0f : scale_;

    // Compute drawing area and create an invisible canvas
    const ImVec2 canvas_size(width_ * s, height_ * s);
    const ImVec2 p0 = ImGui::GetCursorScreenPos();

    ImGui::InvisibleButton("matrix_canvas", canvas_size,
                           ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);

    ImDrawList* dl = ImGui::GetWindowDrawList();



    // Background
    dl->AddRectFilled(p0, ImVec2(p0.x + canvas_size.x, p0.y + canvas_size.y), IM_COL32(10, 10, 10, 255));


    const float pad = std::max(0.0f, s * 0.10f);
    const float r = std::max(1.0f, (s - 2.0f * pad) * 0.5f);

    for (uint16_t y = 0; y < height_; ++y) {
        for (uint16_t x = 0; x < width_; ++x) {
            const uint32_t rgb = fb_[static_cast<size_t>(y) * width_ + x];
            const ImU32 col = toImU32(rgb);

            const float x0 = p0.x + x * s + pad;
            const float y0 = p0.y + y * s + pad;
            const float x1 = p0.x + (x + 1) * s - pad;
            const float y1 = p0.y + (y + 1) * s - pad;

            if (shape_ == LedShape::Square) {
                dl->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), col, 0.0f);
            } else {
                const ImVec2 c((x0 + x1) * 0.5f, (y0 + y1) * 0.5f);
                dl->AddCircleFilled(c, r, col, 16);
            }

        }
    }

    // Outline
    dl->AddRect(p0, ImVec2(p0.x + canvas_size.x, p0.y + canvas_size.y), IM_COL32(80, 80, 80, 255));

    ImGui::End();
}

void ImGuiMatrix::clear() {
    for (auto& pixel : fb_) pixel = 0;
}
} // namespace Fractonica