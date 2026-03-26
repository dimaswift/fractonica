//
// Created by Dmitry Popov on 30.01.2026.
//

#include "DesktopApp.h"

#include "saros.h"
#include "GuiUtils.h"
#include "implot.h"
#include "OctalGlyph.h"

namespace Fractonica {


    void DesktopApp::setup() {
        display.begin();
    }

    static void drawBin(int64_t bin, int64_t timestamp, uint8_t saros_number, IDisplay* display) {
        static OctalGlyphSettings s;
        const uint64_t t = get_solar_rollover_epoch(timestamp, saros_number, bin);
        char lbl[16];
        snprintf(lbl, 16,"%lld",  Utils::DecimaToOctal(bin));
        Gui::Timestamp(t, lbl);
        const ImVec2 pos = ImGui::GetCursorScreenPos();
        OctalGlyph::Draw(bin, display, Vector2(pos.x, pos.y), s);
        ImGui::Separator();
    }

    static void drawFractalView(int64_t base, int64_t current, int64_t timestamp, uint8_t saros_number, IDisplay* display, bool showGlyph) {

        static OctalGlyphSettings s;
        char lbl[16];
        snprintf(lbl, 16,"%lld", Utils::DecimaToOctal(current + base));
        if (ImGui::TreeNode(lbl)) {

            for (int j = 0; j < 8; ++j) {

                const int64_t bin = base + (current  * (j));
                const uint64_t t = get_solar_rollover_epoch(timestamp, saros_number, bin);
                snprintf(lbl, 16,"%d",  j);
                Gui::Timestamp(t, lbl);
                if (showGlyph) {
                    const ImVec2 pos = ImGui::GetCursorScreenPos();
                    OctalGlyph::Draw(bin, display, Vector2(pos.x, pos.y), s);
                }
                if (current > 010000) {
                    drawFractalView(bin, current >> 3, timestamp, saros_number, display, showGlyph);
                }
            }
            ImGui::TreePop();
        }

    }

    void DesktopApp::run() {

        static int year = 1992, day = 5, month = 1, hour = 12;
        static bool customReference = false;
        static int64_t referenceNow = unixClock.now();
        int64_t now = unixClock.now();
        static int yearNow = 2026, dayNow = 5, monthNow = 1, hourNow = 0;

        static int64_t birthday = 0;

        eclipse_result_t solar = find_closest_solar_eclipse(birthday);

        ImGui::Checkbox("Custom Reference", &customReference);
        if (customReference) {
            Gui::UnixDatePicker("Reference (present)", &yearNow, &monthNow, &dayNow, &hourNow, &referenceNow);
        }
        else {
            referenceNow = now;
        }

        Gui::UnixDatePicker("Birthday", &year, &month, &day, &hour, &birthday);

        uint64_t averageBin = get_average_bin(referenceNow, birthday, 1, 2);
        const ImVec2 pos = ImGui::GetCursorScreenPos();
        static OctalGlyphSettings s;
        OctalGlyph::Draw(averageBin, &display, Vector2(pos.x, pos.y), s);

        if (solar.eclipse.valid) {

            if (ImGui::TreeNode("Periods")) {
                for (int i = 0; i < 14; ++i) {
                    uint64_t d = get_solar_saros_period_duration_ms(referenceNow, solar.saros_next.info.solar.saros_number, i);
                    if (d > 1000) {
                        Gui::Duration((float)d / 1000.0f);

                        const ImVec2 pos = ImGui::GetCursorScreenPos();
                        static OctalGlyphSettings s;
                        OctalGlyph::Draw(070000000 >> (3 * i), &display, Vector2(pos.x, pos.y), s);
                        ImGui::Separator();
                    }
                    else {
                        ImGui::Text("%llu ms", d);
                    }
                }
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Fractal View")) {

                drawFractalView(0, 010000000, referenceNow, solar.eclipse.info.solar.saros_number, &display, true);
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Patterns")) {

                std::vector<int64_t> numbers = {

                    011000000,
                    012000000,
                    013000000,
                    014000000,
                    015000000,
                    016000000,
                    017000000,
                    021000000,
                    022000000,
                    023000000,
                    024000000,
                    025000000,
                    026000000,
                    027000000,
                    031000000,
                    032000000,
                    033000000,
                    034000000,
                    035000000,
                    036000000,
                    037000000,
                    041000000,
                    042000000,
                    043000000,
                    044000000,
                    045000000,
                    046000000,
                    047000000,
                    051000000,
                    052000000,
                    053000000,
                    054000000,
                    055000000,
                    056000000,
                    057000000,
                    061000000,
                    062000000,
                    063000000,
                    064000000,
                    065000000,
                    066000000,
                    067000000,
                    071000000,
                    072000000,
                    073000000,
                    074000000,
                    075000000,
                    076000000,
                    077000000,
                     010000000,
                     020000000,
                     030000000,
                     040000000,
                     050000000,
                     060000000,
                     070000000,
                     011111111,
                     022222222,
                     033333333,
                     044444444,
                     055555555,
                     066666666,
                     077777777,
                     010100000,
                     012120000,
                     013130000,
                     014140000,
                     015150000,
                     016160000,
                     017170000,
                     020200000,
                     022220000,
                     023230000,
                     024240000,
                     025250000,
                     026260000,
                     027270000,
                     030300000,
                     032320000,
                     033330000,
                     034340000,
                     035350000,
                     036360000,
                     037370000,
                     040400000,
                     042420000,
                     043430000,
                     044440000,
                     045450000,
                     046460000,
                     047470000,
                     050500000,
                     052520000,
                     053530000,
                     054540000,
                     055550000,
                     056560000,
                     057570000,
                     060600000,
                     062620000,
                     063630000,
                     064640000,
                     065650000,
                     066660000,
                     067670000,
                     070700000,
                     072720000,
                     073730000,
                     074740000,
                     075750000,
                     076760000,
                     077770000,
                     012222222,
                     013333333,
                     014444444,
                     015555555,
                     016666666,
                     017777777,
                     021111111,
                     023333333,
                     024444444,
                     025555555,
                     026666666,
                     027777777,
                     021111111,
                     023333333,
                     024444444,
                     025555555,
                     026666666,
                     027777777,
                     031111111,
                     032222222,
                     034444444,
                     035555555,
                     036666666,
                     037777777,
                     041111111,
                     042222222,
                     043333333,
                     045555555,
                     046666666,
                     047777777,
                     051111111,
                     052222222,
                     053333333,
                     054444444,
                     056666666,
                     057777777,
                     061111111,
                     062222222,
                     063333333,
                     064444444,
                     065555555,
                     067777777,
                     071111111,
                     072222222,
                     073333333,
                     074444444,
                     075555555,
                     076666666,
                };

                static int sortMode = 0;

                ImGui::RadioButton("None", &sortMode, 0); ImGui::SameLine();
                ImGui::RadioButton("Linear", &sortMode, 1); ImGui::SameLine();
                ImGui::RadioButton("Closest Past", &sortMode, 2);ImGui::SameLine();
                ImGui::RadioButton("Closest Future", &sortMode, 3);

                if (sortMode == 1) {
                    std::ranges::sort(numbers);
                }
                else if (sortMode == 2) {
                    int64_t bin = calculate_solar_octal_phase(referenceNow, solar.eclipse.info.solar.saros_number, 2);
                    std::sort(numbers.begin(), numbers.end(), [bin](const int64_t& a, const int64_t& b) {
                        return a < bin && abs(a - bin) < abs(b - bin);
                    });
                }
                else if (sortMode == 3) {
                    int64_t bin = calculate_solar_octal_phase(referenceNow, solar.eclipse.info.solar.saros_number, 2);
                    std::sort(numbers.begin(), numbers.end(), [bin](const int64_t& a, const int64_t& b) {
                        return a >= bin && abs(a - bin) < abs(b - bin);
                    });
                }
                for (int64_t number: numbers) {
                    drawBin(number, referenceNow, solar.eclipse.info.solar.saros_number, &display);
                }


                ImGui::TreePop();
            }

            Gui::DrawSarosCard(referenceNow, solar.eclipse.info.solar.saros_number, &display);
        }



        static bool created = false;

        if (!created) {
            created = true;
            ImPlot::CreateContext();
        }

        static double t_min = birthday;
        static double t_max = 1640995200;

        struct saros {
            int64_t unix_time;
            int number;
        };

        if (ImPlot::BeginPlot("##Time", ImVec2(-1,0))) {
            ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
            ImPlot::SetupAxesLimits(t_min,t_max,0,1);

            ImPlot::Annotation(birthday,0,ImPlot::GetLastItemColor(),ImVec2(10,60),false,"Birth");
            ImPlot::Annotation(solar.eclipse.unix_time,0,ImColor(255,255,0),ImVec2(10,10),false,"Solar");
            int64_t prev = 0;

            int x = 0;
            for (int i = 0; i < 64; i++) {
                int64_t times[SAROS_MAX_ECLIPSES];
                uint8_t sarosCount = 0;
                get_solar_saros_series(i + 110, times, &sarosCount);
                float r,g,b;
                ImGui::ColorConvertHSVtoRGB((float) (i) / 180, 1, 1, r, g, b);

                for (size_t j = 0; j < sarosCount; ++j) {
                    char lbl[16];
                    char dur[128];
                    Gui::Duration(times[j] - prev, dur);
                    prev = times[j];
                    sprintf(lbl, "%d %lu",i + 110, j+1);

                    ImPlot::Annotation(times[j],0, ImColor(r * 255,g * 255,b * 255),ImVec2(10,-10),false,lbl);
                    if (j == 0) {
                        ImPlot::Annotation(times[j],0, ImColor(0,255,0),ImVec2(-20,-30),false,"FIRST");
                    }
                    if (j == sarosCount - 1) {
                        ImPlot::Annotation(times[j],0, ImColor(255,0,0),ImVec2(-20,-30),false,"LAST");
                    }
                }
            }

            ImPlot::EndPlot();
        }

        for (uint8_t i = 0; i < ALIVE_SAROS_COUNT; ++i) {
            char lbl[16];
            uint8_t num = SarosOrderedByBirth[i];
            sprintf(lbl, "%d (%d)", i + 1, num);
            if (ImGui::TreeNode(lbl)) {
                Gui::DrawSarosCard(now, num, &display);
                ImGui::TreePop();
            }
        }
    }

    DesktopApp::DesktopApp() : display(512, 511, 1) {

    }

    void DesktopApp::shutdown() {

    }

}
