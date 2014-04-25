// This test script is included by /src/test/_host.c:main()

// === TEST: SRC/WGEN/GEOCOORDINATES ===

// === TEST: SRC/CLOCK ===
T(test_clock_error,     "clock error handling")
T(test_clock_time_warp, "clocks if time jumps backwards")

// === TEST: SRC/UTF8 ===
T(test_utf8_strlen,     "multibyte string parsing")
