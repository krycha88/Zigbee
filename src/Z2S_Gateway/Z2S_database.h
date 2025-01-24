#ifndef Z2S_DATABASE_H_
#define Z2S_DATABASE_H_
#include <Arduino.h>
#include <Z2S_devices_database.h>

const char* jsonDevices = R"({"devices": [
    {
      "manufacturer_name": "_TZ3000_akqdg6g7",
      "model_name": "TS0201",
      "z2s_device_desc_id": "TEMPHUMIDITY_SENSOR",
      "z2s_device_endpoints_count": 1
    },
    {
      "manufacturer_name": "Xiaomi",
      "model_name": "LYWSD03MMC-z",
      "z2s_device_desc_id": "TEMPHUMIDITY_SENSOR_1",
      "z2s_device_endpoints_count": 1
    },
    {
      "manufacturer_name": "_TZ3000_0hkmcrza",
      "model_name": "TS0203",
      "z2s_device_desc_id": "IAS_ZONE_SENSOR",
      "z2s_device_endpoints_count": 1
    },
    {
      "manufacturer_name": "_TZ3000_cehuw1lw",
      "model_name": "TS011F",
      "z2s_device_desc_id": "RELAY_ELECTRICITY_METER",
      "z2s_device_endpoints_count": 1
    },
    {
      "manufacturer_name": "_TZ3000_wkai4ga5",
      "model_name": "TS0044",
      "z2s_device_desc_id": "ON_OFF",
      "z2s_device_endpoints_count": 4,
      "endpoints": [1, 2, 3, 4]
    }
  ]
}
})";

static const z2s_device_entity_t Z2S_DEVICES[] PROGMEM = {
    // Temperatury i wilgotności
    {"_TZ3000_akqdg6g7", "TS0201", Z2S_DEVICE_DESC_TEMPHUMIDITY_SENSOR, 1},
    {"Xiaomi", "LYWSD03MMC-z", Z2S_DEVICE_DESC_TEMPHUMIDITY_SENSOR_1, 1},
    {"_TZ3000_rdhukkmi", "TS0201", Z2S_DEVICE_DESC_TEMPHUMIDITY_SENSOR, 1},
    {"_TZ3000_fllyghyj", "TS0201", Z2S_DEVICE_DESC_TEMPHUMIDITY_SENSOR, 1},
    {"_TZ3000_v1w2k9dd", "TS0201", Z2S_DEVICE_DESC_TEMPHUMIDITY_SENSOR, 1},
    {"_TZE204_upagmta9", "TS0601", Z2S_DEVICE_DESC_TEMPHUMIDITY_SENSOR, 1},
    {"_TZ3000_xr3htd96", "TS0201", Z2S_DEVICE_DESC_TEMPHUMIDITY_SENSOR, 1},
    {"SONOFF", "SNZB-02D", Z2S_DEVICE_DESC_TEMPHUMIDITY_SENSOR, 1},
    {"_TZE200_vvmbj46n", "TS0601", Z2S_DEVICE_DESC_TEMPHUMIDITY_SENSOR, 1},

    // Czujniki strefowe (IAS Zone)
    {"_TZ3000_0hkmcrza", "TS0203", Z2S_DEVICE_DESC_IAS_ZONE_SENSOR, 1},
    {"TEST1", "TS0202", Z2S_DEVICE_DESC_IAS_ZONE_SENSOR, 1},
    {"_TZ3000_mcxw5ehu", "TS0202", Z2S_DEVICE_DESC_IAS_ZONE_SENSOR, 1},
    {"_TZ3000_26fmupbb", "TS0203", Z2S_DEVICE_DESC_IAS_ZONE_SENSOR, 1},
    {"_TZ3000_rcuyhwe3", "TS0203", Z2S_DEVICE_DESC_IAS_ZONE_SENSOR, 1},
    {"_TZ3000_2mbfxlzr", "TS0203", Z2S_DEVICE_DESC_IAS_ZONE_SENSOR, 1},
    {"_TZE200_bh3n6gk8", "TS0601", Z2S_DEVICE_DESC_IAS_ZONE_SENSOR, 1},
    {"_TZ3000_k4ej3ww2", "TS0207", Z2S_DEVICE_DESC_IAS_ZONE_SENSOR, 1},
    {"_TZ3000_85czd6fy", "TS0207", Z2S_DEVICE_DESC_IAS_ZONE_SENSOR, 1},
    {"_TZ3000_upgcbody", "TS0207", Z2S_DEVICE_DESC_IAS_ZONE_SENSOR, 1},
    {"TEST1", "TS0205", Z2S_DEVICE_DESC_IAS_ZONE_SENSOR, 1},
    {"_TZ3000_lf56vpxj", "TS0202", Z2S_DEVICE_DESC_IAS_ZONE_SENSOR, 1},

    // Przekaźniki z licznikiem energii
    {"_TZ3000_cehuw1lw", "TS011F", Z2S_DEVICE_DESC_RELAY_ELECTRICITY_METER, 1},
    {"_TZ3000_5f43h46b", "TS011F", Z2S_DEVICE_DESC_RELAY_ELECTRICITY_METER, 1},
    {"_TZ3000_w0qqde0g", "TS011F", Z2S_DEVICE_DESC_RELAY_ELECTRICITY_METER, 1},

    // Przełączniki (ON/OFF)
    {"_TZ3000_wkai4ga5", "TS0044", Z2S_DEVICE_DESC_ON_OFF, 4, {1, 2, 3, 4}},
    {"_TZ3000_yj6k7vfo", "TS0041", Z2S_DEVICE_DESC_ON_OFF, 4, {1, 2, 3, 4}},
    {"_TZ3000_4fjiwweb", "TS004F", Z2S_DEVICE_DESC_ON_OFF, 1},
    {"_TZ3000_uri7ongn", "TS004F", Z2S_DEVICE_DESC_ON_OFF, 1},
    {"_TZ3000_ixla93vd", "TS004F", Z2S_DEVICE_DESC_ON_OFF, 1},
    {"_TZ3000_qja6nq5z", "TS004F", Z2S_DEVICE_DESC_ON_OFF, 1},
    {"_TZ3000_abrsvsou", "TS004F", Z2S_DEVICE_DESC_ON_OFF, 1},
    {"IKEA of Sweden", "Remote Control N2", Z2S_DEVICE_DESC_ON_OFF, 1},
};

#endif
