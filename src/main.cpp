
#if !defined(ZIGBEE_MODE_ED) && !defined(ZIGBEE_MODE_ZCZR) 
#error "Zigbee device mode is not selected in Tools->Zigbee mode"
#endif

#include "esp_zigbee_attribute.h"
#include "ZigbeeCore.h"

unsigned long previousMillis = 0;
const unsigned long interval = 5000;

bool getTemperature(uint8_t endpoint, float *temperature)
{
  uint16_t cluster_id = 0x0402;                          // Cluster temperatury Zigbee
  uint16_t attr_id = 0x0000;                             // Atrybut temperatury w tym klastrze
  uint8_t cluster_role = ESP_ZB_ZCL_CLUSTER_SERVER_ROLE; // Rola serwera dla klastra

  esp_zb_zcl_attr_t *attr = esp_zb_zcl_get_attribute(endpoint, cluster_id, cluster_role, attr_id);

  if (attr == nullptr)
  {
    return false;
  }
  if (attr->data_p != nullptr)
  {
    int16_t raw_temp = *((int16_t *)attr->data_p);
    *temperature = (float)raw_temp / 100.0;
    return true;
  }
  return false;
}

bool getTemperatureAndHumidity(uint8_t endpoint, float *temperature, float *humidity)
{
  const uint16_t temperature_cluster_id = 0x0402;              // Identyfikator klastra temperatury
  const uint16_t humidity_cluster_id = 0x0405;                 // Identyfikator klastra wilgotności
  const uint16_t temperature_attr_id = 0x0000;                 // Identyfikator atrybutu temperatury
  const uint16_t humidity_attr_id = 0x0000;                    // Identyfikator atrybutu wilgotności
  const uint8_t cluster_role = ESP_ZB_ZCL_CLUSTER_SERVER_ROLE; // Rola serwera dla klastrów

  bool success = true;

  esp_zb_zcl_attr_t *temperature_attr = esp_zb_zcl_get_attribute(endpoint, temperature_cluster_id, cluster_role, temperature_attr_id);
  if (temperature_attr != nullptr)
  {
    int16_t raw_temperature = *(int16_t *)temperature_attr->data_p;
    *temperature = raw_temperature / 100.0;
  }
  else
  {
    success = false;
    ESP_LOGE("Zigbee", "Failed to read temperature from endpoint %d.", endpoint);
  }

  esp_zb_zcl_attr_t *humidity_attr = esp_zb_zcl_get_attribute(endpoint, humidity_cluster_id, cluster_role, humidity_attr_id);
  if (humidity_attr != nullptr)
  {
    uint16_t raw_humidity = *(uint16_t *)humidity_attr->data_p;
    *humidity = raw_humidity / 100.0;
  }
  else
  {
    success = false;
    ESP_LOGE("Zigbee", "Failed to read humidity from endpoint %d.", endpoint);
  }

  return success;
}

void setup()
{
  Serial.begin(115200);
  if (!Zigbee.begin(ZIGBEE_COORDINATOR))
  {
    Serial.println("Zigbee initialization failed!");
    ESP.restart();
  }

  Serial.println("Zigbee setup complete. Starting network scan...");

  Zigbee.scanNetworks();
}

void loop()
{
  unsigned long currentMillis = millis();
  static int16_t scan_status = 0;
  uint8_t sensor_endpoint = 0;

  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;

    scan_status = Zigbee.scanComplete();
    if (scan_status > 0)
    {
      Serial.println("Networks found, reading sensor data...");
      sensor_endpoint = 10;
      float temperature1 = 0.0, humidity1 = 0.0;

      if (getTemperature(sensor_endpoint, &temperature1))
      {
        Serial.printf("Sensor 1 Temperature: %.2f°C\n", temperature1);
      }
      else
      {
        Serial.println("Failed to read temperature from Sensor 1.");
      }

      sensor_endpoint = 10;
      float temperature2 = 0.0, humidity2 = 0.0;
      if (getTemperatureAndHumidity(sensor_endpoint, &temperature2, &humidity2))
      {
        Serial.printf("Sensor 2 - Temperature: %.2f°C, Humidity: %.2f%%\n", temperature2, humidity2);
      }
      else
      {
        Serial.println("Failed to read data from Sensor 1.");
      }
    }
    else if (scan_status == 0)
    {
      Serial.println("No networks found. Retrying...");
    }
    else
    {
      Serial.println("Scan error. Retrying...");
    }
  }
}
