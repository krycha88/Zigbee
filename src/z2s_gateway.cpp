#ifdef Z2S_GATEWAY
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <LittleFS.h>
#include "FS.h"
#include <EEPROM.h>
#include <Ticker.h>

#include <SuplaDevice.h>
#include <supla/network/esp_wifi.h>
#include <supla/control/button.h>
#include <supla/device/status_led.h>
#include <supla/storage/littlefs_config.h>
#include <supla/network/esp_web_server.h>
#include <supla/network/html/device_info.h>
#include <supla/network/html/protocol_parameters.h>
#include <supla/network/html/status_led_parameters.h>
#include <supla/network/html/wifi_parameters.h>
#include <supla/device/supla_ca_cert.h>
#include <supla/storage/eeprom.h>
#include <supla/storage/config.h>

#include "Z2S_Gateway/SuplaZigbeeGateway.h"
#include "Z2S_Gateway/Z2S_database.h"

#define GATEWAY_ENDPOINT_NUMBER 1

#define BUTTON_CFG_RELAY_GPIO 9
//#define STATUS_LED_GPIO       8

Supla::ESPWifi wifi;
Supla::LittleFsConfig configSupla;
Supla::Eeprom eeprom;
//Supla::SuplaZigbeeGateway zigbeeGateway(Z2S_DEVICES, BUTTON_CFG_RELAY_GPIO);
Supla::SuplaZigbeeGateway zigbeeGateway(jsonDevices, BUTTON_CFG_RELAY_GPIO);

// Supla::Device::StatusLed statusLed(STATUS_LED_GPIO, true);
Supla::EspWebServer suplaServer;

Supla::Html::DeviceInfo htmlDeviceInfo(&SuplaDevice);
Supla::Html::WifiParameters htmlWifi;
Supla::Html::ProtocolParameters htmlProto;
// Supla::Html::StatusLedParameters htmlStatusLed;

void setup() {
  Serial.begin(115200);

  auto buttonCfgRelay = new Supla::Control::Button(BUTTON_CFG_RELAY_GPIO, true, true);
  buttonCfgRelay->configureAsConfigButton(&SuplaDevice);

  SuplaDevice.setSuplaCACert(suplaCACert);
  SuplaDevice.setSupla3rdPartyCACert(supla3rdCACert);

  SuplaDevice.setName("Zigbee to Supla");
  SuplaDevice.begin();
}

void loop() {
  SuplaDevice.iterate();
}
#endif