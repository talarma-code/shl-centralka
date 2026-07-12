#pragma once

// Home Assistant MQTT topic definitions using the YAML unique_id values as macro names
#define energia_wyprodukowana_l1          "lacko/shl_c1/telemetry/power/l1"
#define energia_wyprodukowana_l2         "lacko/shl_c1/telemetry/power/l2"
#define energia_pobrana_grzalka          "lacko/shl_c1/telemetry/power/heater"
#define energia_pobrana_dom              "lacko/shl_c1/telemetry/power/home_total"

#define moc_chwilowa_wyprodukowana_l1    "lacko/shl_c1/telemetry/power/L1PowerW"
#define moc_chwilowa_wyprodukowana_l2    "lacko/shl_c1/telemetry/power/L2PowerW"
#define moc_chwilowa_pobrana_dom         "lacko/shl_c1/telemetry/power/HomePowerW"

#define napiecie_faza_l1                "lacko/shl_c1/telemetry/voltage/l1"
#define napiecie_faza_l2                "lacko/shl_c1/telemetry/voltage/l2"
#define centrala_last_seen              "lacko/shl_c1/status/online"
#define centrala_reset_reason           "lacko/shl_c1/status/reason"
#define centrala_reset_code             "lacko/shl_c1/status/code"
#define status_komunikacji_grzalka      "lacko/shl_c1/status/espnow"
#define stan_grzalki                    "lacko/shl_c1/telemetry/heater/requested_status"
