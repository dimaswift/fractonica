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

	#define PI 3.141592653589793
	#define PI2 (PI+PI)
	static double gd, gcosearthtilt, gsinearthtilt;
	static double starang = 0;
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

	static void dcossin(double angle, double *cos_out, double *sin_out) {
		*cos_out = cos(angle);
		*sin_out = sin(angle);
	}

	static double tokeplerang (double ia, double k)
	{
		double oa, ooa, dacos, dasin;
		long cnt;

		//Make sure 0 < ia < PI*2
		ia -= (double)((long)(ia/PI2))*PI2;
		while (ia < 0) ia += PI2;
		while (ia >= PI2) ia -= PI2;

		dcossin(ia,&dacos,&dasin);
		oa = ia - k*dasin * (1 - k*dacos);
		for(cnt=7;cnt>=0;cnt--) //WARNING! Doesn't work when k = -1
		{
			dcossin(oa,&dacos,&dasin);
			ooa = oa; oa += (ia-oa - k*dasin) / (1 + k*dacos);
			if (ooa == oa) return(oa);
		}
		return(oa);
	}

	static void initdatetime (long dastat)
	{
		double w, M, dayfrac;
		long i;
		SYSTEMTIME SysTime;

		if (dastat)
		{
			long ogsecond, ogyear;
			ogsecond = ((gday*24+ghour)*60+gminute)*60+gsecond; ogyear = gyear;
			GetLocalTime(&SysTime);
			gyear = SysTime.wYear;
			gmonth = SysTime.wMonth;
			gday = SysTime.wDay;
			ghour = SysTime.wHour;
			gminute = SysTime.wMinute;
			gsecond = SysTime.wSecond;
			//? = SysTime.wMilliseconds;
			if (gyear == ogyear)
				ototalclock += (((gday*24+ghour)*60+gminute)*60+gsecond - ogsecond)*1000;
			else
				ototalclock += 1000; //hope that system clock incremented by 1 second at the year crossing
		}
		else
		{
			//Fix date&time
			if (gsecond < 0) { gsecond += 60; gminute--; }
			if (gsecond >= 60) { gsecond -= 60; gminute++; }
			if (gminute < 0) { gminute += 60; ghour--; }
			if (gminute >= 60) { gminute -= 60; ghour++; }
			if (ghour < 0) { ghour += 24; gday--; }
			if (ghour >= 24) { ghour -= 24; gday++; }
			getdate((long)getday(gyear,gmonth,gday),&gyear,&gmonth,&gday);
		}

		if (!isdaylight(gyear,gmonth,gday,ghour)) i = tzinfo.Bias+tzinfo.StandardBias;
		else i = tzinfo.Bias+tzinfo.DaylightBias;
		dayfrac = ((double)ghour+((double)gminute+(double)gsecond/60.0+i)/60.0)/24.0;
		if (dastat) dayfrac += ((double)(totalclock-ototalclock))/(1000.0*60.0*60.0*24.0);

		gd = getday(gyear,gmonth,gday)-getday(1999L,12L,31L) + dayfrac;

		w = (282.9404 + 4.70935E-5*gd)*PI/180.0;    //Arg. of perigee
		M = (356.0470 + 0.9856002585*gd)*PI/180.0;  //Mean anomaly
		starang = dayfrac*PI*2 + w + M + PI;

		dcossin((23.4393 - 3.563E-7*gd)*(PI/180.0),&gcosearthtilt,&gsinearthtilt);

		//Precession (Earth's axis is wobbling)
		//eclipticlongitudeoffs = 3.82394E-5*(365.2422*(epoch-2000.0)-d)*PI/180.0;

		if (dastat) angmessed = 0;
	}

    static void getmoonpos (double *retx, double *rety, double *retz)
	{
		double t, dist, ox, oy, oz;
		double N, i, w, a, e, M, E, x, y, z, dacos, dasin;
		double Ls, Lm, Ms, Mm, D, F, Sw, SM;
		double lonperturb, latperturb, radperturb;

		N = (125.1228 - 0.0529538083*gd)*PI/180.0;  //Long asc. node
		i = 5.1454*PI/180.0;         //Inclination
		w = (318.0634 + 0.1643573223*gd)*PI/180.0;  //Arg. of perigee
		a = 60.2666;            //Mean distance
		e = 0.054900;                //Eccentricity
		M = (115.3654 + 13.0649929509*gd)*PI/180.0; //Mean anomaly

		E = tokeplerang(M,-e);

		Sw = (282.9404 + 4.70935E-5*gd)*PI/180.0;   //sun's longitude of perihelion
		SM = (356.0470 + 0.9856002585*gd)*PI/180.0; //sun's mean anomaly
		Ls = Sw+SM;
		Lm = N+w+M;
		Ms = SM;
		Mm = M;
		D = Lm-Ls;
		F = Lm-N;

		//Ls -= (double)((long)(Ls/PI2))*PI2; while (Ls < 0) Ls += PI2; while (Ls >= PI2) Ls -= PI2;
		//Lm -= (double)((long)(Lm/PI2))*PI2; while (Lm < 0) Lm += PI2; while (Lm >= PI2) Lm -= PI2;
		//Ms -= (double)((long)(Ms/PI2))*PI2; while (Ms < 0) Ms += PI2; while (Ms >= PI2) Ms -= PI2;
		//Mm -= (double)((long)(Mm/PI2))*PI2; while (Mm < 0) Mm += PI2; while (Mm >= PI2) Mm -= PI2;
		//D  -= (double)((long)(D /PI2))*PI2; while (D  < 0) D  += PI2; while (D  >= PI2) D  -= PI2;
		//F  -= (double)((long)(F /PI2))*PI2; while (F  < 0) F  += PI2; while (F  >= PI2) F  -= PI2;
		//print4x6(0L,32L,31,-1,"%lf %lf %lf %lf %lf %lf\n",Ms*180/PI,Mm*180/PI,Ls*180/PI,Lm*180/PI,D*180/PI,F*180/PI);

			//Perturbations in longitude (degrees):
		lonperturb = -1.274*sin(Mm-2*D)   +0.658*sin(2*D)       -0.186*sin(Ms)
						 -0.059*sin(2*Mm-2*D) -0.057*sin(Mm-2*D+Ms) +0.053*sin(Mm+2*D)
						 +0.046*sin(2*D-Ms)   +0.041*sin(Mm-Ms)     -0.035*sin(D)
						 -0.031*sin(Mm+Ms)    -0.015*sin(2*F-2*D)   +0.011*sin(Mm-4*D);

			//Perturbations in latitude (degrees):
		latperturb = -0.173*sin(F-2*D)    -0.055*sin(Mm-F-2*D)  -0.046*sin(Mm+F-2*D)
						 +0.033*sin(F+2*D)    +0.017*sin(2*Mm+F);

			//Perturbations in lunar distance (Earth radii):
		radperturb = -0.58*cos(Mm-2*D)    -0.46*cos(2*D);

		//print4x6(0L,32L,31,-1,"%lf %lf %lf\n",lonperturb, latperturb, radperturb);
		a += radperturb;

		x = a*(cos(E)-e);
		y = a*sqrt(1-e*e)*sin(E);

		dacos = x*cos(w) - y*sin(w);
		dasin = x*sin(w) + y*cos(w);
		x = (cos(N)*dacos - sin(N)*dasin*cos(i));
		y = (sin(N)*dacos + cos(N)*dasin*cos(i));
		z = dasin * sin(i);

		dcossin(lonperturb*(PI/180.0),&dacos,&dasin);
		ox = x*dacos - y*dasin;
		oy = y*dacos + x*dasin;
		oz = z;

		dcossin(latperturb*(PI/180.0),&dacos,&dasin);
		x = ox*dacos - oz*dasin*(ox/sqrt(ox*ox+oy*oy));
		y = oy*dacos - oz*dasin*(oy/sqrt(ox*ox+oy*oy));
		z = oz*dacos + sqrt(ox*ox+oy*oy)*dasin;

		//print4x6(0L,32L,31,-1,"%lf %lf %lf\n",atan2(y,x)*(180.0/PI)+360.0,atan2(z,sqrt(x*x+y*y))*(180.0/PI),sqrt(x*x+y*y+z*z));

		//x = r * cos(RA) * cos(Decl)
		//y = r * sin(RA) * cos(Decl)
		//z = r * sin(Decl)
		//
		//r    = sqrt( x*x + y*y + z*z )
		//RA   = atan2( y, x )
		//Decl = asin( z / r ) = atan2( z, sqrt( x*x + y*y ) )

		ox = y;
		oy = -z;
		oz = -x;

		x = ox*gcosearthtilt + oy*gsinearthtilt;
		y = oy*gcosearthtilt - ox*gsinearthtilt;
		z = oz;

			//Geocentric -> Topocentric conversion (don't assume you're at center of earth)
			//Since ipos is earth radii, must do this when x,y,z are in Earth radii
		dcossin(starang,&dacos,&dasin);
		x -= (ipos.x*dacos-ipos.z*dasin);
		y -= ipos.y;
		z -= (ipos.z*dacos+ipos.x*dasin);

			//Must convert distance from Earth radii to millions of miles
		t = 0.238857 / 60.2666;
		*retx = x*t;
		*rety = y*t;
		*retz = z*t;
	}

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