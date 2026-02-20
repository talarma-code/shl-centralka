#ifdef __cplusplus
extern "C" {
#endif
void setUp(void) {}
void tearDown(void) {}
#ifdef __cplusplus
}
#endif
#ifdef UNIT_TEST
#include "DateTime_stub.h"
#include <stdint.h>
#else
#include <Arduino.h>
#endif
#include <unity.h>
#include "HourlySurplusForecastAlgorithm.h"

void test_heater_enable_on_surplus() {
    HourlySurplusForecastAlgorithm alg;
    DateTime t(2026, 2, 20, 12, 0, 0);
    // symuluj nadwyżkę energii
    alg.calculatePower(t, 100, 100, 50); // 200 Wh produkcji, 50 Wh zużycia
    // Dodaj asercje, np. sprawdź stan heaterOnState przez getter lub publiczne pole (jeśli dodasz)
    // TEST_ASSERT_TRUE(alg.isHeaterOn());
    TEST_ASSERT_TRUE(true); // Placeholder, zastąp prawdziwym testem stanu grzejnika
}




void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_heater_enable_on_surplus);
    UNITY_END();
}

void loop() {}