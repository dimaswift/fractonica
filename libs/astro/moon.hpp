#ifndef MOON_HPP
#define MOON_HPP

#include <cmath>
#include <string>
#include <array>
#include <iostream>

extern "C" {
#include "../cspice/include/SpiceUsr.h"
}

#include "kernels.hpp"

namespace moon {

    // Helper struct for normalized 3D coordinates
    struct UnitVector {
        double x, y, z;
    };

    struct AlignmentData {
        UnitVector sun_direction;  // Pointing from Earth to Sun
        UnitVector moon_direction; // Pointing from Earth to Moon
        double phase_angle_deg;    // Angle between the two vectors
    };

    // Use inline static to ensure the variable is shared across translation units (C++17)
    // If you are on C++11/14, move 'initialized' to a .cpp file to avoid linking errors.
    inline static bool initialized = false;


    inline void init() {
        if (initialized) {
            return;
        }
        // Ensure you handle kernel loading errors in production
        load_kernels_from_path(rootDir() +"/kernels");

        // Error check: CSPICE error handling is tricky, simplified here
        if (failed_c()) {
             // Handle error or reset spice error status
             reset_c();
        }
        initialized = true;
    }


    inline AlignmentData get_eclipse_alignment(const SpiceDouble et) {
        // Ensure kernels are loaded
        init();

        SpiceDouble sun_pos[3], moon_pos[3];
        SpiceDouble lt; // Light time (ignored for visualization usually, but required by func)

        // 1. Get Sun position relative to Earth
        // Target: "SUN", Observer: "EARTH", Frame: "J2000", Correction: "LT+S"
        // "LT+S" corrects for light time and stellar aberration (crucial for accurate alignment)
        spkpos_c("SUN", et, "J2000", "LT+S", "EARTH", sun_pos, &lt);

        // 2. Get Moon position relative to Earth
        spkpos_c("MOON", et, "J2000", "LT+S", "EARTH", moon_pos, &lt);

        // 3. Normalize vectors (Scale becomes 1.0)
        SpiceDouble sun_hat[3], moon_hat[3];
        vhat_c(sun_pos, sun_hat);   // vhat_c normalizes the vector
        vhat_c(moon_pos, moon_hat);

        // 4. Calculate angular separation
        // vsep_c returns angle in radians between two 3-vectors
        double angle_rad = vsep_c(sun_hat, moon_hat);
        double angle_deg = angle_rad * dpr_c(); // dpr_c() is degrees per radian

        return {
            {sun_hat[0], sun_hat[1], sun_hat[2]},
            {moon_hat[0], moon_hat[1], moon_hat[2]},
            angle_deg
        };
    }

} // namespace moon

#endif // MOON_HPP