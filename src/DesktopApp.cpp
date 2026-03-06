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

    void DesktopApp::run() {

        static int year = 1992, day = 5, month = 1, hour = 12;
        static int64_t referenceNow = unixClock.now();
        int64_t now = unixClock.now();
        static int yearNow = 2026, dayNow = 5, monthNow = 1, hourNow = 0;

        static int64_t birthday = 0;

        eclipse_result_t solar = find_closest_solar_eclipse(birthday);


        Gui::UnixDatePicker("Reference (present)", &yearNow, &monthNow, &dayNow, &hourNow, &referenceNow);

        Gui::UnixDatePicker("Birthday", &year, &month, &day, &hour, &birthday);

        if (solar.eclipse.valid) {
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
