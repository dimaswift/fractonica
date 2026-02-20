//
// Created by Dmitry Popov on 30.01.2026.
//

#include "DesktopApp.h"
#include "saros.h"
#include "GuiUtils.h"


namespace Fractonica {


    void DesktopApp::setup() {
        matrix.begin();
    }

    void DesktopApp::run() {

        static int year = 1992, day = 5, month = 1;
        static int64_t now = unixClock.now();

        static int yearNow = 1992, dayNow = 5, monthNow = 1;
       // ImGui::Begin("Now");

        static int64_t sarosLookup = 0;
       // ImGuiStyle& style = ImGui::GetStyle();
       // style.FontScaleMain = 1.5;
        eclipse_result_t solar = find_closest_solar_eclipse(sarosLookup);
        eclipse_result_t lunar = find_closest_lunar_eclipse(sarosLookup);
        auto solarWin = find_solar_saros_window(now, solar.eclipse.info.solar.saros_number);
        auto lunarWin = find_lunar_saros_window(now, lunar.eclipse.info.lunar.saros_number);

        Gui::UnixDatePicker("Reference (present)", &yearNow, &monthNow, &dayNow, &now);

        Gui::UnixDatePicker("Birthday", &year, &month, &day, &sarosLookup);


        //ImGui::Separator();
        //Gui::Timestamp(now, "Now");
        //ImGui::Separator();


        Gui::Timestamp(solar.eclipse.unix_time, "Closest Solar");
        Gui::Timestamp(lunar.eclipse.unix_time, "Closest Lunar");
        ImGui::Text("Time till solar:");
        Gui::Duration(sarosLookup - solar.eclipse.unix_time);

        ImGui::Text("Time till lunar:");
        Gui::Duration(sarosLookup - lunar.eclipse.unix_time);

        ImGui::Separator();
        ImGui::TextColored(ImColor::HSV(0.15,1,1),"Solar %d", solar.eclipse.info.solar.saros_number);
        Gui::Timestamp(solarWin.past.unix_time, "Past Eclipse");



        if (solarWin.future.valid) {
           // auto d = static_cast<double>(solarWin.future.unix_time - solarWin.past.unix_time);
            Gui::Timestamp(solarWin.future.unix_time, "Future Eclipse");
            //ImGui::Text("Duration seconds %f", d);
           // ImGui::Text("Octal Bin %lld", Utils::DecimaToOctal(floor(( (now - solarWin.past.unix_time) / d) * pow(8, 12))));
            // for (int i = 0; i < 32; ++i) {
            //    // ImGui::InputScalar("Delta",ImGuiDataType_Double, &d);
            //
            //     auto f = 1.0 / d;
            //     char freqBuf[32];
            //     Utils::FormatFrequency(static_cast<int64_t>(f), freqBuf, sizeof(freqBuf));
            //     ImGui::Text("Harmonic: %d Frequency: %s", i + 1, freqBuf);
            //     Gui::Duration(static_cast<int64_t>(d));
            //     d /= 8;
            //     ImGui::InputScalar("N", ImGuiDataType_Double, &f);
            //
            // }
            // double df = 531187804423397.0;
            // ImGui::InputScalar("Base", ImGuiDataType_Double, &df);
            // for (int i = 0; i < 32; ++i) {
            //     // ImGui::InputScalar("Delta",ImGuiDataType_Double, &d);
            //
            //     auto p = 1.0 / df;
            //     char freqBuf[32];
            //     Gui::Duration(static_cast<int64_t>(p));
            //    // Utils::FormatFrequency(static_cast<int64_t>(p), freqBuf, sizeof(freqBuf));
            //   //  ImGui::Text("Harmonic: %d Frequency: %s", i + 1, freqBuf);
            //     df /= 8;
            // }
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
          //  glyph.drawRange(now, solarWin.past.unix_time, solarWin.future.unix_time, &matrix);
        }
        else {
            ImGui::Text("Ended");
        }

    }

    DesktopApp::DesktopApp() : matrix(8, 8, 32) {

    }

    void DesktopApp::shutdown() {

    }

}
