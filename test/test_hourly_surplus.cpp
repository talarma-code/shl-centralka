extern "C" void setUp(void) {}
extern "C" void tearDown(void) {}
#include <unity.h>
#include "HourlySurplusAlgorithm.h"
#include <stdio.h>

// HourlySurplus tests
void test_hourly_surplus_always_on() {
    HourlySurplusAlgorithm alg;
    DateTime t(2026, 2, 20, 12, 0, 0);
    bool state = false;
    for (int i = 0; i < 20; ++i) {
        uint32_t homeConsumption = (i == 0 || i == 9) ? 50 : 125; // heater off at step 1 and 10, on otherwise
        state = alg.calculatePower(t, 100, 100, homeConsumption);
        printf("Step %d: prod=200 cons=%d state=%d\n", i+1, homeConsumption, state);
        TEST_ASSERT_TRUE(state);
        t = DateTime(2026, 2, 20, 12, 0, 0 + (i+1)*180);
    }
    printf("Accumulated produced Wh: %u\n", alg.getAccumulatedProducedWh());
    printf("Accumulated consumed Wh: %u\n", alg.getAccumulatedConsumedWh());
}

//house consumption is 2000W, production is 20W, so net is -1980W, not enough to run heater, which should stay off
void test_hourly_surplus_always_off() {
    HourlySurplusAlgorithm alg;
    DateTime t(2026, 2, 20, 12, 0, 0);
    bool state = true;
    for (int i = 0; i < 20; ++i) {
        state = alg.calculatePower(t, 10, 10, 200);
        printf("Step %d: prod=20 cons=200 state=%d\n", i+1, state);
        TEST_ASSERT_FALSE(state);
        t = DateTime(2026, 2, 20, 12, 0, 0 + (i+1)*180);
    }
    printf("Accumulated produced Wh: %u\n", alg.getAccumulatedProducedWh());
    printf("Accumulated consumed Wh: %u\n", alg.getAccumulatedConsumedWh());
}

//house consumption is 2000W, production is 20W, so net is -1980W, not enough to run heater, which should stay off
void test_hourly_surplus_production_first_30minutes() {
    HourlySurplusAlgorithm alg;
    DateTime t(2026, 2, 20, 12, 0, 0);
    bool state = false;
    // Oczekiwane stany na podstawie dotychczasowych asercji
    bool expected[20] = {
        1,1,1,1,1,1,1,1,1,1,1,1, // 1-12: TRUE,
        0,0,0,0,0,0,0,0     // 13-20: FALSE

    };
    uint32_t l1Wh[20] = {40,40,40,40,40,40,40,40,40,40,0,0,0,0,0,0,0,0,0,0};
    uint32_t l2Wh[20] = {50,50,50,50,50,50,50,50,50,50,0,0,0,0,0,0,0,0,0,0};
    for (int i = 0; i < 20; ++i) {
        uint32_t homeConsumption = state ? 75 : 0;
        state = alg.calculatePower(t, l1Wh[i], l2Wh[i], homeConsumption);
        printf("Step %d: prod=%d cons=%d state=%d expected=%d\n", i+1, l1Wh[i]+l2Wh[i], homeConsumption, state, expected[i]);
        TEST_ASSERT_EQUAL(expected[i], state);
        t = DateTime(2026, 2, 20, 12, 0, 0 + (i+1)*180);
    }
    printf("Accumulated produced Wh: %u\n", alg.getAccumulatedProducedWh());
    printf("Accumulated consumed Wh: %u\n", alg.getAccumulatedConsumedWh());
}

//house consumption is 2000W, production is 20W, so net is -1980W, not enough to run heater, which should stay off
void test_hourly_surplus_mixed_production() {
    HourlySurplusAlgorithm alg;
    DateTime t(2026, 2, 20, 12, 0, 0);
    bool state = false;
    // Oczekiwane stany na podstawie dotychczasowych asercji
    bool expected[20] = {
        1,1,1,1,1,1,1,1,1,1,1,1, // 1-12: TRUE,
        0,0,0,0,0,0,     // 13-18: FALSE
        1,1             // 19-20: TRUE (hysteresis should allow turning on again)

    };
    uint32_t l1Wh[20] = {40,40,40,40,40,40,40,40,40,40,0,0,0,0,0,0,0,0,35,50};
    uint32_t l2Wh[20] = {50,50,50,50,50,50,50,50,50,50,0,0,0,0,0,0,0,0,45,45};
    for (int i = 0; i < 20; ++i) {
        uint32_t homeConsumption = state ? 75 : 0;
        state = alg.calculatePower(t, l1Wh[i], l2Wh[i], homeConsumption);
        printf("Step %d: prod=%d cons=%d state=%d expected=%d\n", i+1, l1Wh[i]+l2Wh[i], homeConsumption, state, expected[i]);
        TEST_ASSERT_EQUAL(expected[i], state);
        t = DateTime(2026, 2, 20, 12, 0, 0 + (i+1)*180);
    }
    printf("Accumulated produced Wh: %u\n", alg.getAccumulatedProducedWh());
    printf("Accumulated consumed Wh: %u\n", alg.getAccumulatedConsumedWh());
}

void test_hourly_surplus_big_production() {
    HourlySurplusAlgorithm alg;
    DateTime t(2026, 2, 20, 12, 0, 0);
    bool state = false;
    // Oczekiwane stany na podstawie dotychczasowych asercji
    bool expected[20] = {
        0,0, // 1-2: TRUE,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1    // 3-20: FALSE
      

    };
    uint32_t l1Wh[20] = {0,0,100,100,100,100,100,100,100,100,100,100,0,0,0,0,0,0,0,0};
    uint32_t l2Wh[20] = {0,0,100,100,100,100,100,100,100,100,100,100,0,0,0,0,0,0,0,0};
    for (int i = 0; i < 20; ++i) {
        uint32_t homeConsumption = state ? 75 : 0;
        state = alg.calculatePower(t, l1Wh[i], l2Wh[i], homeConsumption);
        printf("Step %d: prod=%d cons=%d state=%d expected=%d\n", i+1, l1Wh[i]+l2Wh[i], homeConsumption, state, expected[i]);
        TEST_ASSERT_EQUAL(expected[i], state);
        t = DateTime(2026, 2, 20, 12, 0, 0 + (i+1)*180);
    }
    printf("Accumulated produced Wh: %u\n", alg.getAccumulatedProducedWh());
    printf("Accumulated consumed Wh: %u\n", alg.getAccumulatedConsumedWh());
}

void test_hourly_surplus_should_not_used_accumulated_power_from_previous_hour() {
    HourlySurplusAlgorithm alg;
    DateTime t(2026, 2, 20, 12, 0, 0);
    bool state = false;
    // Oczekiwane stany na podstawie dotychczasowych asercji
    bool expected[20] = {
        1,1,1,1,1,1,1,1,1,1, // 1-10: TRUE,
        0,0,0,0,0,0,0,0,     // 11-18: FALSE
        1,1             // 19-20: TRUE (hysteresis should allow turning on again)
    };

    //in this case we have consuption equal 75W in first setep in next hour (step 11) so overpordution have to balance taht (we have 100W+60W so is fine)
    uint32_t l1Wh[20] = {100,100,100,100,100,100,100,100,100,100,0,0,0,0,0,0,0,0,100,40};
    uint32_t l2Wh[20] = {100,100,100,100,100,100,100,100,100,100,0,0,0,0,0,0,0,0,60,30};
    //first hour
    for (int i = 0; i < 10; ++i) {
        uint32_t homeConsumption = state ? 75 : 0;
        state = alg.calculatePower(t, l1Wh[i], l2Wh[i], homeConsumption);
        printf("Step %d: prod=%d cons=%d state=%d expected=%d\n", i+1, l1Wh[i]+l2Wh[i], homeConsumption, state, expected[i]);
        TEST_ASSERT_EQUAL(expected[i], state);
        t = DateTime(2026, 2, 20, 12, 0, 0 + (i+1)*180);
    }

    for (int i = 10; i < 20; ++i) {
        t = DateTime(2026, 2, 20, 13, 0, 0 + (i+1)*180);
        uint32_t homeConsumption = state ? 75 : 0;
        state = alg.calculatePower(t, l1Wh[i], l2Wh[i], homeConsumption);
        printf("Step %d: prod=%d cons=%d state=%d expected=%d\n", i+1, l1Wh[i]+l2Wh[i], homeConsumption, state, expected[i]);
        TEST_ASSERT_EQUAL(expected[i], state);
       
    }
    printf("Accumulated produced Wh: %u\n", alg.getAccumulatedProducedWh());
    printf("Accumulated consumed Wh: %u\n", alg.getAccumulatedConsumedWh());
}


void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_hourly_surplus_always_on);
    RUN_TEST(test_hourly_surplus_always_off);
    RUN_TEST(test_hourly_surplus_production_first_30minutes);
    RUN_TEST(test_hourly_surplus_mixed_production);
    RUN_TEST(test_hourly_surplus_big_production);
    RUN_TEST(test_hourly_surplus_should_not_used_accumulated_power_from_previous_hour);
    UNITY_END();

}

void loop() {}
