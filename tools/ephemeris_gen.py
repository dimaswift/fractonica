#!/usr/bin/env python3
"""
Generates ephemeris files with raw int64 timestamps.

Binary format (little-endian):
  [uint32 magic]           - "FRAC" (0x43415246)
  [uint32 entry_count]     - Number of timestamps
  [uint64 reserved]        - Reserved
  [int64  timestamps...]   - Absolute timestamps
"""

import argparse
import struct
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import List, Tuple, Callable
import numpy as np

try:
    from astropy.time import Time
    from astropy.coordinates import get_sun, get_body, GeocentricTrueEcliptic
    from astropy import units as u
    from scipy.optimize import brentq
except ImportError:
    print("Required packages: pip install astropy scipy numpy")
    sys.exit(1)


# Constants
UNIX_EPOCH = Time('1970-01-01T00:00:00', scale='utc')
DEFAULT_COUNT = 512
FRACTONICA_MAGIC = 0x43415246

# Average periods in seconds (approximate, for initial search steps)
AVG_SYNODIC_MONTH = 29.530589 * 86400  # ~29.53 days
AVG_ANOMALISTIC_MONTH = 27.554550 * 86400  # ~27.55 days
AVG_TROPICAL_YEAR = 365.24219 * 86400  # ~365.24 days
AVG_NODICAL_MONTH = 27.212221 * 86400  # ~27.21 days


def time_to_unix(t: Time) -> int:
    # ensure UTC on output
    t = t.utc
    # seconds since unix epoch as float
    dt = (t - UNIX_EPOCH).to_value('s')
    # choose a rule: floor/round/ceil. Round-to-nearest is usually best.
    return int(np.rint(dt))


def unix_to_time(ts: int) -> Time:
    """Convert Unix timestamp to astropy Time."""
    return UNIX_EPOCH + ts * u.s

def find_roots_by_stepping(func, t0, t1, step=6*3600):
    roots = []
    a = t0
    fa = func(a)
    b = a + step
    while b <= t1:
        fb = func(b)
        if fa == 0:
            roots.append(a)
        elif fa * fb < 0:
            roots.append(brentq(func, a, b))
        a, fa = b, fb
        b += step
    return roots


# =============================================================================
# Lunar Phase (New Moon / Full Moon) Calculations
# =============================================================================

def moon_sun_elongation(unix_ts: float) -> float:
    """
    Calculate the elongation of the Moon from the Sun.
    Returns value in range [-180, 180].
    Crosses 0 at new moon, ±180 at full moon.
    """
    t = unix_to_time(unix_ts)

    sun = get_sun(t)
    moon = get_body('moon', t)

    sun_ecl = sun.transform_to(GeocentricTrueEcliptic(equinox=t))
    moon_ecl = moon.transform_to(GeocentricTrueEcliptic(equinox=t))

    diff = (moon_ecl.lon - sun_ecl.lon).wrap_at(180*u.deg).deg

    # Normalize to [-180, 180]
    while diff > 180:
        diff -= 360
    while diff < -180:
        diff += 360

    return diff


def moon_sun_elongation_shifted(unix_ts: float) -> float:
    """
    Elongation shifted by 180 degrees.
    Crosses 0 at full moon.
    """
    elong = moon_sun_elongation(unix_ts)
    # Shift so full moon (±180) becomes 0
    if elong > 0:
        return elong - 180
    else:
        return elong + 180


def calculate_new_moons(start_unix: int, count: int) -> List[int]:
    """Calculate timestamps of new moons (elongation = 0, crossing from negative to positive)."""
    print(f"Calculating {count} new moons...")

    events = []
    current = start_unix
    last_ts = None

    while len(events) < count:
        # Search window: one synodic month
        end = current + int(AVG_SYNODIC_MONTH * 1.1)

        roots = find_roots_by_stepping(moon_sun_elongation, current, end)

        for root in roots:
            # New moon: elongation crosses 0 from negative to positive
            elong_before = moon_sun_elongation(root - 3600)
            elong_after = moon_sun_elongation(root + 3600)

            if elong_before < 0 < elong_after:  # Crossing upward = new moon
                ts = int(root)
                # Skip if too close to last event (< 10 days)
                if last_ts is None or ts - last_ts > 86400 * 10:
                    if len(events) < count:
                        events.append(ts)
                        last_ts = ts
                        if len(events) % 100 == 0:
                            print(f"  Found {len(events)} new moons...")

        current = end - int(AVG_SYNODIC_MONTH * 0.1)

    return events


def calculate_full_moons(start_unix: int, count: int) -> List[int]:
    """Calculate timestamps of full moons (elongation = ±180)."""
    print(f"Calculating {count} full moons...")

    events = []
    current = start_unix
    last_ts = None

    while len(events) < count:
        end = current + int(AVG_SYNODIC_MONTH * 1.1)

        # Use shifted function that crosses 0 at full moon
        roots = find_roots_by_stepping(moon_sun_elongation_shifted, current, end)

        for root in roots:
            # Verify it's actually near ±180 degrees elongation
            elong = abs(moon_sun_elongation(root))
            if elong > 170:  # Close to 180 = full moon
                ts = int(root)
                if last_ts is None or ts - last_ts > 86400 * 10:
                    if len(events) < count:
                        events.append(ts)
                        last_ts = ts
                        if len(events) % 100 == 0:
                            print(f"  Found {len(events)} full moons...")

        current = end - int(AVG_SYNODIC_MONTH * 0.1)

    return events


# =============================================================================
# Anomalistic Month (Perigee/Apogee) Calculations
# =============================================================================

def moon_distance(unix_ts: float) -> float:
    """Get Moon's distance from Earth in km."""
    t = unix_to_time(unix_ts)
    moon = get_body('moon', t)
    return moon.distance.to(u.km).value


def moon_distance_derivative(unix_ts: float, dt: float = 60.0) -> float:
    """Numerical derivative of moon distance (for finding extrema)."""
    return (moon_distance(unix_ts + dt) - moon_distance(unix_ts - dt)) / (2 * dt)


def calculate_perigees(start_unix: int, count: int) -> List[int]:
    """Calculate timestamps of lunar perigees (closest approach)."""
    print(f"Calculating {count} perigees...")

    events = []
    current = start_unix
    last_ts = None

    while len(events) < count:
        end = current + int(AVG_ANOMALISTIC_MONTH * 1.1)

        # Find where derivative crosses zero (extrema)
        roots = find_roots_by_stepping(moon_distance_derivative, current, end)

        for root in roots:
            # Check if it's a minimum (perigee) by checking second derivative
            d1 = moon_distance(root - 3600)
            d2 = moon_distance(root)
            d3 = moon_distance(root + 3600)

            if d1 > d2 < d3:  # Local minimum = perigee
                ts = int(root)
                if last_ts is None or ts - last_ts > 86400 * 10:
                    if len(events) < count:
                        events.append(ts)
                        last_ts = ts
                        if len(events) % 100 == 0:
                            print(f"  Found {len(events)} perigees...")

        current = end - int(AVG_ANOMALISTIC_MONTH * 0.1)

    return events


def calculate_apogees(start_unix: int, count: int) -> List[int]:
    """Calculate timestamps of lunar apogees (farthest point)."""
    print(f"Calculating {count} apogees...")

    events = []
    current = start_unix
    last_ts = None

    while len(events) < count:
        end = current + int(AVG_ANOMALISTIC_MONTH * 1.1)

        roots = find_roots_by_stepping(moon_distance_derivative, current, end)

        for root in roots:
            d1 = moon_distance(root - 3600)
            d2 = moon_distance(root)
            d3 = moon_distance(root + 3600)

            if d1 < d2 > d3:  # Local maximum = apogee
                ts = int(root)
                if last_ts is None or ts - last_ts > 86400 * 10:
                    if len(events) < count:
                        events.append(ts)
                        last_ts = ts
                        if len(events) % 100 == 0:
                            print(f"  Found {len(events)} apogees...")

        current = end - int(AVG_ANOMALISTIC_MONTH * 0.1)

    return events


# =============================================================================
# Solstice/Equinox Calculations
# =============================================================================

def sun_declination(unix_ts: float) -> float:
    """Get Sun's declination in degrees."""
    t = unix_to_time(unix_ts)
    sun = get_sun(t)
    return sun.dec.deg


def sun_declination_derivative(unix_ts: float, dt: float = 3600.0) -> float:
    """Numerical derivative of sun declination."""
    return (sun_declination(unix_ts + dt) - sun_declination(unix_ts - dt)) / (2 * dt)


def calculate_solstices_summer(start_unix: int, count: int) -> List[int]:
    """Calculate timestamps of summer solstices (max declination, ~June 21)."""
    print(f"Calculating {count} summer solstices...")

    events = []
    current = start_unix
    last_ts = None

    while len(events) < count:
        end = current + int(AVG_TROPICAL_YEAR * 1.1)

        roots = find_roots_by_stepping(sun_declination_derivative, current, end)

        for root in roots:
            dec = sun_declination(root)
            if dec > 20:  # Summer solstice has high positive declination
                ts = int(root)
                if last_ts is None or ts - last_ts > 86400 * 100:
                    if len(events) < count:
                        events.append(ts)
                        last_ts = ts
                        print(f"  Found summer solstice {len(events)}: {datetime.fromtimestamp(root, tz=timezone.utc)}")

        current = end - int(AVG_TROPICAL_YEAR * 0.1)

    return events


def calculate_solstices_winter(start_unix: int, count: int) -> List[int]:
    """Calculate timestamps of winter solstices (min declination, ~Dec 21)."""
    print(f"Calculating {count} winter solstices...")

    events = []
    current = start_unix
    last_ts = None

    while len(events) < count:
        end = current + int(AVG_TROPICAL_YEAR * 1.1)

        roots = find_roots_by_stepping(sun_declination_derivative, current, end)

        for root in roots:
            dec = sun_declination(root)
            if dec < -20:  # Winter solstice has high negative declination
                ts = int(root)
                if last_ts is None or ts - last_ts > 86400 * 100:
                    if len(events) < count:
                        events.append(ts)
                        last_ts = ts
                        print(f"  Found winter solstice {len(events)}: {datetime.fromtimestamp(root, tz=timezone.utc)}")

        current = end - int(AVG_TROPICAL_YEAR * 0.1)

    return events


def calculate_equinoxes_vernal(start_unix: int, count: int) -> List[int]:
    """Calculate timestamps of vernal equinoxes (~March 20)."""
    print(f"Calculating {count} vernal equinoxes...")

    events = []
    current = start_unix
    last_ts = None

    while len(events) < count:
        end = current + int(AVG_TROPICAL_YEAR * 1.1)

        roots = find_roots_by_stepping(sun_declination, current, end)

        for root in roots:
            dec_before = sun_declination(root - 86400)
            dec_after = sun_declination(root + 86400)

            if dec_before < 0 < dec_after:  # Crossing upward
                ts = int(root)
                if last_ts is None or ts - last_ts > 86400 * 100:
                    if len(events) < count:
                        events.append(ts)
                        last_ts = ts
                        print(f"  Found vernal equinox {len(events)}: {datetime.fromtimestamp(root, tz=timezone.utc)}")

        current = end - int(AVG_TROPICAL_YEAR * 0.1)

    return events


def calculate_equinoxes_autumnal(start_unix: int, count: int) -> List[int]:
    """Calculate timestamps of autumnal equinoxes (~Sept 22)."""
    print(f"Calculating {count} autumnal equinoxes...")

    events = []
    current = start_unix
    last_ts = None

    while len(events) < count:
        end = current + int(AVG_TROPICAL_YEAR * 1.1)

        roots = find_roots_by_stepping(sun_declination, current, end)

        for root in roots:
            dec_before = sun_declination(root - 86400)
            dec_after = sun_declination(root + 86400)

            if dec_before > 0 > dec_after:  # Crossing downward
                ts = int(root)
                if last_ts is None or ts - last_ts > 86400 * 100:
                    if len(events) < count:
                        events.append(ts)
                        last_ts = ts
                        print(f"  Found autumnal equinox {len(events)}: {datetime.fromtimestamp(root, tz=timezone.utc)}")

        current = end - int(AVG_TROPICAL_YEAR * 0.1)

    return events


# =============================================================================
# Nodal Crossing Calculations
# =============================================================================

def moon_ecliptic_latitude(unix_ts: float) -> float:
    """Get Moon's ecliptic latitude in degrees."""
    t = unix_to_time(unix_ts)
    moon = get_body('moon', t)
    moon_ecl = moon.transform_to(GeocentricTrueEcliptic(equinox=t))
    return moon_ecl.lat.deg


def calculate_nodal_ascending(start_unix: int, count: int) -> List[int]:
    """Calculate timestamps of ascending node crossings."""
    print(f"Calculating {count} ascending node crossings...")

    events = []
    current = start_unix
    last_ts = None

    while len(events) < count:
        end = current + int(AVG_NODICAL_MONTH * 1.1)

        roots = find_roots_by_stepping(moon_ecliptic_latitude, current, end)

        for root in roots:
            lat_before = moon_ecliptic_latitude(root - 3600)
            lat_after = moon_ecliptic_latitude(root + 3600)

            if lat_before < 0 < lat_after:  # Crossing upward = ascending
                ts = int(root)
                if last_ts is None or ts - last_ts > 86400 * 10:
                    if len(events) < count:
                        events.append(ts)
                        last_ts = ts
                        if len(events) % 100 == 0:
                            print(f"  Found {len(events)} ascending crossings...")

        current = end - int(AVG_NODICAL_MONTH * 0.1)

    return events


def calculate_nodal_descending(start_unix: int, count: int) -> List[int]:
    """Calculate timestamps of descending node crossings."""
    print(f"Calculating {count} descending node crossings...")

    events = []
    current = start_unix
    last_ts = None

    while len(events) < count:
        end = current + int(AVG_NODICAL_MONTH * 1.1)

        roots = find_roots_by_stepping(moon_ecliptic_latitude, current, end)

        for root in roots:
            lat_before = moon_ecliptic_latitude(root - 3600)
            lat_after = moon_ecliptic_latitude(root + 3600)

            if lat_before > 0 > lat_after:  # Crossing downward = descending
                ts = int(root)
                if last_ts is None or ts - last_ts > 86400 * 10:
                    if len(events) < count:
                        events.append(ts)
                        last_ts = ts
                        if len(events) % 100 == 0:
                            print(f"  Found {len(events)} descending crossings...")

        current = end - int(AVG_NODICAL_MONTH * 0.1)

    return events


# =============================================================================
# Binary File I/O
# =============================================================================

def analyze_timestamps(timestamps: List[int]) -> Tuple[int, int]:
    """
    Analyze timestamps and return average period.
    """
    if len(timestamps) < 2:
        return timestamps[0], 0

    # Calculate actual periods
    periods = [timestamps[i+1] - timestamps[i] for i in range(len(timestamps) - 1)]

    # Calculate average
    avg_period = int(round(sum(periods) / len(periods)))

    print(f"  Start: {timestamps[0]} ({datetime.fromtimestamp(timestamps[0], tz=timezone.utc)})")
    print(f"  Average period: {avg_period} seconds ({avg_period/86400:.4f} days)")

    return timestamps[0], avg_period


def write_binary_file(filepath: Path, timestamps: List[int]):
    """
    Write compact binary ephemeris file.

    Format (little-endian):
        uint32  magic           (4 bytes)
        uint32  entry_count     (4 bytes)
        uint64  reserved        (8 bytes)
        int64[] timestamps      (8 * entry_count bytes)
    """
    with open(filepath, 'wb') as f:
        # Header
        f.write(struct.pack('<I', FRACTONICA_MAGIC))         # uint32 magic
        f.write(struct.pack('<I', len(timestamps)))          # uint32 entry_count
        f.write(struct.pack('<Q', 0))                        # uint64 reserved

        # Timestamps
        for ts in timestamps:
            f.write(struct.pack('<q', ts))

    size = 16 + 8 * len(timestamps)
    print(f"  Written: {filepath} ({size} bytes)")


# =============================================================================
# C Header Generation
# =============================================================================

def generate_c_header(filepath: Path, name: str, timestamps: List[int],
                      avg_period: int, storage_type: str = "PROGMEM"):
    """Generate C header file with static int64 arrays."""

    upper_name = name.upper()
    epoch = timestamps[0] if timestamps else 0

    header = f"""/**
 * Fractonica Ephemeris Data: {name}
 *
 * Auto-generated - do not edit
 *
 * Epoch: {epoch} ({datetime.fromtimestamp(epoch, tz=timezone.utc).isoformat()})
 * Average period: {avg_period} seconds
 * Entry count: {len(timestamps)}
 */

#ifndef FRACTONICA_{upper_name}_H
#define FRACTONICA_{upper_name}_H

#include <stdint.h>

#if defined(ARDUINO_ARCH_AVR)
  #include <avr/pgmspace.h>
  #define FRACTONICA_STORAGE {storage_type}
#elif defined(ESP_PLATFORM) || defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
  #include <pgmspace.h>
  #define FRACTONICA_STORAGE {storage_type}
#else
  #define FRACTONICA_STORAGE
#endif

#define FRACTONICA_{upper_name}_COUNT {len(timestamps)}U

static const int64_t FRACTONICA_STORAGE fractonica_{name}_timestamps[{len(timestamps)}] = {{
"""

    # Write timestamps in rows of 8
    for i in range(0, len(timestamps), 8):
        row = timestamps[i:i+8]
        header += "    " + ", ".join(f"{ts}LL" for ts in row) + ",\n"

    header = header.rstrip(",\n") + "\n};\n\n#endif /* FRACTONICA_" + upper_name + "_H */\n"

    with open(filepath, 'w') as f:
        f.write(header)

    print(f"  Written: {filepath}")


# =============================================================================
# Main Entry Point
# =============================================================================

PERIOD_CALCULATORS = {
    'new_moon': ('New Moon', calculate_new_moons),
    'full_moon': ('Full Moon', calculate_full_moons),
    'perigee': ('Lunar Perigee', calculate_perigees),
    'apogee': ('Lunar Apogee', calculate_apogees),
    'solstice_summer': ('Summer Solstice', calculate_solstices_summer),
    'solstice_winter': ('Winter Solstice', calculate_solstices_winter),
    'equinox_vernal': ('Vernal Equinox', calculate_equinoxes_vernal),
    'equinox_autumnal': ('Autumnal Equinox', calculate_equinoxes_autumnal),
    'nodal_ascending': ('Ascending Node', calculate_nodal_ascending),
    'nodal_descending': ('Descending Node', calculate_nodal_descending),
}


def main():
    parser = argparse.ArgumentParser(
        description='Generate Fractonica celestial ephemeris data (int64)',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Period types:
  new_moon          New Moon timestamps
  full_moon         Full Moon timestamps
  perigee           Lunar perigee (closest to Earth)
  apogee            Lunar apogee (farthest from Earth)
  solstice_summer   Summer solstice (~June 21)
  solstice_winter   Winter solstice (~Dec 21)
  equinox_vernal    Vernal equinox (~March 20)
  equinox_autumnal  Autumnal equinox (~Sept 22)
  nodal_ascending   Moon ascending node crossing
  nodal_descending  Moon descending node crossing
  all               Generate all period types

Examples:
  %(prog)s new_moon -n 1024 -o ./data
  %(prog)s all -n 512 --header-only
  %(prog)s perigee -n 100 --storage PSRAM
        """
    )

    parser.add_argument('period', choices=list(PERIOD_CALCULATORS.keys()) + ['all'],
                        help='Period type to calculate')
    parser.add_argument('-n', '--count', type=int, default=DEFAULT_COUNT,
                        help=f'Number of periods (default: {DEFAULT_COUNT})')
    parser.add_argument('-o', '--output', type=Path, default=Path('.'),
                        help='Output directory (default: current)')
    parser.add_argument('--start', type=int, default=0,
                        help='Start Unix timestamp (default: 0 = 1970-01-01)')
    parser.add_argument('--binary-only', action='store_true',
                        help='Generate only binary files')
    parser.add_argument('--header-only', action='store_true',
                        help='Generate only C header files')
    parser.add_argument('--storage', choices=['PROGMEM', 'PSRAM', ''],
                        default='PROGMEM', help='Storage type for C headers')

    args = parser.parse_args()

    # Create output directory
    args.output.mkdir(parents=True, exist_ok=True)

    # Determine which periods to generate
    if args.period == 'all':
        periods = list(PERIOD_CALCULATORS.keys())
    else:
        periods = [args.period]

    for period in periods:
        print(f"\n{'='*60}")
        print(f"Generating: {period}")
        print('='*60)

        # Regular period calculation
        name, calc_func = PERIOD_CALCULATORS[period]

        # Calculate timestamps
        timestamps = calc_func(args.start, args.count)

        # Analyze
        epoch, avg_period = analyze_timestamps(timestamps)

        if not args.header_only:
            write_binary_file(
                args.output / f'{period}.bin',
                timestamps
            )

        if not args.binary_only:
            generate_c_header(
                args.output / f'{period}.h',
                period, timestamps, avg_period, args.storage
            )

    print(f"\n{'='*60}")
    print("Generation complete!")
    print('='*60)


if __name__ == '__main__':
    main()
