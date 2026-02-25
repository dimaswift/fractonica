#include "ImGuiDisplay.h"
#include <Utils.h>

namespace Fractonica {

ImGuiDisplay::ImGuiDisplay(uint16_t width,
                         uint16_t height,
                         float scale,

                         const char* windowName)
    : width_(width),
      height_(height),
      scale_((scale < 1.0f) ? 1.0f : scale),
      windowName_(windowName ? windowName : "Matrix"),
      fb_(static_cast<size_t>(width) * static_cast<size_t>(height), 0u)
{}

uint32_t ImGuiDisplay::getColor(uint8_t r, uint8_t g, uint8_t b) const
{
    return Utils::Color(r,g,b);
}

uint32_t ImGuiDisplay::getColorHSV(uint16_t hue, uint8_t sat, uint8_t val) const {
    return Utils::ColorHSV(hue,sat,val);
}

void ImGuiDisplay::print(const char *msg, int16_t x, int16_t y, uint8_t size) {

}

void ImGuiDisplay::log(const char *msg) {
}

void ImGuiDisplay::logError(const char *msg) {
}

void ImGuiDisplay::drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t thickness, uint32_t color) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), toImU32(color), thickness);
}

void ImGuiDisplay::drawLine(const Vector2 &p1, const Vector2 &p2, int16_t thickness, uint32_t color) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddLine(ImVec2(p1.x, p1.y), ImVec2(p2.x, p2.y), toImU32(color), thickness);
}

void ImGuiDisplay::drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), toImU32(color));
}

void ImGuiDisplay::drawFillRect(const Vector2 &min, const Vector2 &max, uint32_t color) {
    drawRect(min.x, min.y, max.x - min.x, max.y - min.y, toImU32(color));
}

void ImGuiDisplay::drawNGonFilled(const Vector2 &center, float radius, uint32_t color, int num_segments) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddNgonFilled(ImVec2(center.x, center.y), radius, toImU32(color), num_segments);
}

void ImGuiDisplay::drawRect(const Vector2 &min, const Vector2 &max, uint32_t color) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRect(ImVec2(min.x, min.y), ImVec2(max.x, max.y), toImU32(color));
}

void ImGuiDisplay::drawBitmap(int16_t x, int16_t y, uint16_t width, uint16_t height, const uint16_t *bitmap) {
}

void ImGuiDisplay::setCursor(const Vector2 &pos) {
    ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y));
}

void ImGuiDisplay::printF(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    ImGui::TextV(fmt, args);
    va_end(args);
}

void ImGuiDisplay::expand(int16_t w, int16_t h) {
    ImGui::Dummy(ImVec2(w, h));
}

void ImGuiDisplay::update() {
}

bool ImGuiDisplay::isOpen() {
    return true;
}

Vector2 ImGuiDisplay::size() {
    return Vector2(width_, height_);
}

ImU32 ImGuiDisplay::toImU32(uint32_t rgb)
{
    const int r = (rgb >> 16) & 0xFF;
    const int g = (rgb >> 8)  & 0xFF;
    const int b = (rgb)       & 0xFF;
    return IM_COL32(r, g, b, 255);
}

void ImGuiDisplay::drawPixel(uint16_t x, uint16_t y, uint32_t color)
{
    if (x >= width_ || y >= height_) return;
    fb_[static_cast<size_t>(y) * width_ + x] = toImU32(color);
}

bool ImGuiDisplay::begin()
{
    begun_ = true;
    return true;
}

void ImGuiDisplay::flush()
{
    if (!begun_) return;

}

void ImGuiDisplay::clear() {
    for (auto& pixel : fb_) pixel = 0;
}
} // namespace Fractonica