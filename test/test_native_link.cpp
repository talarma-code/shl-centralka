#ifdef UNIT_TEST
extern void setup();
extern void loop();
int main() {
	setup();
	// loop() is empty for Unity tests, but call once for compatibility
	loop();
	return 0;
}
#endif
// Force linking of application code for native PlatformIO test
#include "Application/HourlySurplusForecastAlgorithm.cpp"
