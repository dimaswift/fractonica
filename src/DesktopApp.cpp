//
// Created by Dmitry Popov on 30.01.2026.
//

#include "DesktopApp.h"
#include "saros.h"
#include "Utils.h"
#include "saros/solar/eclipse_info_modern.h"
#include "GuiUtils.h"
namespace Fractonica {


    void DesktopApp::setup() {
        matrix.begin();
    }

    void DesktopApp::run() {

        static int year = 1992, day = 5, month = 1;
        int64_t now = unixClock.now();
        static int64_t sarosLookup = 0;
       // ImGuiStyle& style = ImGui::GetStyle();
       // style.FontScaleMain = 1.5;
        eclipse_result_t solar = find_closest_solar_eclipse(sarosLookup);
        eclipse_result_t lunar = find_closest_lunar_eclipse(sarosLookup);
        auto solarWin = find_solar_saros_window(now, solar.eclipse.info.solar.saros_number);
        auto lunarWin = find_lunar_saros_window(now, lunar.eclipse.info.lunar.saros_number);

        ImGui::Begin("Saros");
        Gui::UnixDatePicker("Date", &year, &month, &day, &sarosLookup);
        ImGui::Separator();
        Gui::Timestamp(now, "Now");
        ImGui::Separator();


        Gui::Timestamp(solar.eclipse.unix_time, "Closest Solar");
        Gui::Timestamp(lunar.eclipse.unix_time, "Closest Lunar");
        ImGui::Text("Time till solar:");
        Gui::Duration(sarosLookup - solar.eclipse.unix_time);

        ImGui::Text("Time till lunar:");
        Gui::Duration(sarosLookup - lunar.eclipse.unix_time);

        ImGui::Separator();
        ImGui::TextColored(ImColor::HSV(0.15,1,1),"Solar %d", solar.eclipse.info.solar.saros_number);
        Gui::Timestamp(solarWin.past.unix_time, "Past");
        if (solarWin.future.valid) {
            Gui::Timestamp(solarWin.future.unix_time, "Future");
        }
        else {
            ImGui::Text("Ended");
        }
        ImGui::Separator();

        ImGui::TextColored(ImColor::HSV(0.5,1,1),"Lunar %d", lunar.eclipse.info.lunar.saros_number);
        Gui::Timestamp(lunarWin.past.unix_time, "Past");
        if (lunarWin.future.valid) {
            Gui::Timestamp(lunarWin.future.unix_time, "Future");
            glyph.drawRange(now, solarWin.past.unix_time, solarWin.future.unix_time, &matrix);
        }
        else {
            ImGui::Text("Ended");
        }
        ImGui::End();

    }

    DesktopApp::DesktopApp() : matrix(8, 8, 32) {

    }

    void DesktopApp::shutdown() {

    }

}
