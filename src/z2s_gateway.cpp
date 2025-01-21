#ifdef Z2S_GATEWAY
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <LittleFS.h>
#include "FS.h"
#include <EEPROM.h>
#include <Ticker.h>

#include <ZigbeeGateway.h>

#include "Z2S_Gateway/Z2S_database.h"
#include <Z2S_Devices_Table.h>
#include <Z2S_devices_database.h>
#include "esp_coexist.h"

#include <SuplaDevice.h>
#include <supla/network/esp_wifi.h>
#include <supla/control/roller_shutter.h>
#include <supla/control/relay.h>
#include <supla/control/button.h>
#include <supla/control/action_trigger.h>
#include <supla/device/status_led.h>
#include <supla/storage/littlefs_config.h>
#include <supla/network/esp_web_server.h>
#include <supla/network/html/device_info.h>
#include <supla/network/html/protocol_parameters.h>
#include <supla/network/html/status_led_parameters.h>
#include <supla/network/html/wifi_parameters.h>
#include <supla/device/supla_ca_cert.h>
#include <supla/events.h>
#include <supla/storage/eeprom.h>
#include <supla/storage/config.h>

#define GATEWAY_ENDPOINT_NUMBER 1

#define BUTTON_CFG_RELAY_GPIO 9
#define STATUS_LED_GPIO       8

Supla::ESPWifi wifi;
Supla::LittleFsConfig configSupla;
Supla::Eeprom eeprom;

Supla::Device::StatusLed statusLed(STATUS_LED_GPIO, true);
Supla::EspWebServer suplaServer;

Supla::Html::DeviceInfo htmlDeviceInfo(&SuplaDevice);
Supla::Html::WifiParameters htmlWifi;
Supla::Html::ProtocolParameters htmlProto;
Supla::Html::StatusLedParameters htmlStatusLed;

ZigbeeGateway zbGateway = ZigbeeGateway(GATEWAY_ENDPOINT_NUMBER);

uint32_t startTime = 0;
uint32_t zbInit_delay = 0;
bool zbInit = true;

enum addedActions
{
  factoryReset,
};

class CustomActionHandler : public Supla::ActionHandler {
 public:
  CustomActionHandler() {};
  void handleAction(int event, int action) {
    if (event == Supla::ON_CLICK_10 && action == factoryReset) {
      Serial.printf("Resetting Zigbee to factory settings, reboot.\n");
      Zigbee.factoryReset();
      Zigbee.openNetwork(180);
    }
  }
};

CustomActionHandler *customActionHandler = new CustomActionHandler;

void setup() {
  Serial.begin(115200);

  Supla::Storage::Init();
  eeprom.setStateSavePeriod(5000);

  auto buttonCfgRelay = new Supla::Control::Button(BUTTON_CFG_RELAY_GPIO, true, true);
  buttonCfgRelay->addAction(factoryReset, customActionHandler, Supla::ON_CLICK_10);
  buttonCfgRelay->configureAsConfigButton(&SuplaDevice);

  Z2S_loadDevicesTable();

  Z2S_initSuplaChannels();

  //  Zigbee Gateway notifications

  zbGateway.onTemperatureReceive(Z2S_onTemperatureReceive);
  zbGateway.onHumidityReceive(Z2S_onHumidityReceive);
  zbGateway.onOnOffReceive(Z2S_onOnOffReceive);
  zbGateway.onRMSVoltageReceive(Z2S_onRMSVoltageReceive);
  zbGateway.onRMSCurrentReceive(Z2S_onRMSCurrentReceive);
  zbGateway.onRMSActivePowerReceive(Z2S_onRMSActivePowerReceive);

  zbGateway.onIASzoneStatusChangeNotification(Z2S_onIASzoneStatusChangeNotification);

  zbGateway.onBoundDevice(Z2S_onBoundDevice);
  zbGateway.onBTCBoundDevice(Z2S_onBTCBoundDevice);

  zbGateway.setManufacturerAndModel("Supla", "Z2SGateway");
  zbGateway.allowMultipleBinding(true);

  Zigbee.addEndpoint(&zbGateway);

  // Open network for 180 seconds after boot
  Zigbee.setRebootOpenNetwork(180);

  // Supla

  SuplaDevice.setSuplaCACert(suplaCACert);
  SuplaDevice.setSupla3rdPartyCACert(supla3rdCACert);

  SuplaDevice.setName("Zigbee to Supla");
  SuplaDevice.begin();

  startTime = millis();
  zbInit_delay = millis();
}

zb_device_params_t *gateway_device;
zb_device_params_t *joined_device;

char zbd_model_name[32];
char zbd_manuf_name[32];

void loop() {
  SuplaDevice.iterate();

  // if (SuplaDevice.getCurrentStatus()  == Supla::DEVICE_MODE_CONFIG) {
  //   return;
  // }

  if (zbInit && SuplaDevice.getCurrentStatus() == STATUS_REGISTERED_AND_READY) {
    Serial.println("zbInit");

    esp_coex_wifi_i154_enable();

    if (!Zigbee.begin(ZIGBEE_COORDINATOR)) {
      Serial.println("Zigbee failed to start!");
      Serial.println("Rebooting...");
      ESP.restart();
    }
    zbInit = false;
    startTime = millis();
  }

  if (zbGateway.isNewDeviceJoined()) {
    zbGateway.clearNewDeviceJoined();
    zbGateway.printJoinedDevices();

    while (!zbGateway.getJoinedDevices().empty()) {
      joined_device = zbGateway.getLastJoinedDevice();

      strcpy(zbd_manuf_name, zbGateway.readManufacturer(joined_device->endpoint, joined_device->short_addr, joined_device->ieee_addr));
      log_i("manufacturer %s ", zbd_manuf_name);
      strcpy(zbd_model_name, zbGateway.readModel(joined_device->endpoint, joined_device->short_addr, joined_device->ieee_addr));
      log_i("model %s ", zbd_model_name);

      uint16_t devices_list_table_size = sizeof(Z2S_DEVICES) / sizeof(Z2S_DEVICES[0]);
      uint16_t devices_desc_table_size = sizeof(Z2S_DEVICES_DESC) / sizeof(Z2S_DEVICES_DESC[0]);
      bool device_recognized = false;

      for (int i = 0; i < devices_list_table_size; i++) {
        if ((strcmp(zbd_model_name, Z2S_DEVICES[i].model_name) == 0) && (strcmp(zbd_manuf_name, Z2S_DEVICES[i].manufacturer_name) == 0)) {
          log_i("LIST matched %s::%s, entry # %d, endpoints # %d, endpoints %d,%d,%d,%d", Z2S_DEVICES[i].manufacturer_name,
                Z2S_DEVICES[i].model_name, i, Z2S_DEVICES[i].z2s_device_endpoints_count, Z2S_DEVICES[i].z2s_device_endpoints[0],
                Z2S_DEVICES[i].z2s_device_endpoints[1], Z2S_DEVICES[i].z2s_device_endpoints[2],
                Z2S_DEVICES[i].z2s_device_endpoints[3]);

          for (int j = 0; j < Z2S_DEVICES[i].z2s_device_endpoints_count; j++) {
            uint8_t endpoint_id = (Z2S_DEVICES[i].z2s_device_endpoints_count == 1) ? 1 : Z2S_DEVICES[i].z2s_device_endpoints[j];

            for (int k = 0; k < devices_desc_table_size; k++) {
              if (Z2S_DEVICES[i].z2s_device_desc_id == Z2S_DEVICES_DESC[k].z2s_device_desc_id) {
                log_i("DESC matched 0x%x, %d, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, endpoint 0x%x ", Z2S_DEVICES_DESC[k].z2s_device_desc_id,
                      Z2S_DEVICES_DESC[k].z2s_device_clusters_count, Z2S_DEVICES_DESC[k].z2s_device_clusters[0],
                      Z2S_DEVICES_DESC[k].z2s_device_clusters[1], Z2S_DEVICES_DESC[k].z2s_device_clusters[2],
                      Z2S_DEVICES_DESC[k].z2s_device_clusters[3], Z2S_DEVICES_DESC[k].z2s_device_clusters[4],
                      Z2S_DEVICES_DESC[k].z2s_device_clusters[5], Z2S_DEVICES_DESC[k].z2s_device_clusters[6],
                      Z2S_DEVICES_DESC[k].z2s_device_clusters[7], endpoint_id);

                device_recognized = true;

                joined_device->endpoint = endpoint_id;
                joined_device->model_id = Z2S_DEVICES_DESC[k].z2s_device_desc_id;

                Z2S_addZ2SDevice(joined_device);

                for (int m = 0; m < Z2S_DEVICES_DESC[k].z2s_device_clusters_count; m++)
                  zbGateway.bindDeviceCluster(joined_device, Z2S_DEVICES_DESC[k].z2s_device_clusters[m]);
              }
              else
                log_i("DESC checking 0x%x, %d, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, endpoint %d ", Z2S_DEVICES_DESC[k].z2s_device_desc_id,
                      Z2S_DEVICES_DESC[k].z2s_device_clusters_count, Z2S_DEVICES_DESC[k].z2s_device_clusters[0],
                      Z2S_DEVICES_DESC[k].z2s_device_clusters[1], Z2S_DEVICES_DESC[k].z2s_device_clusters[2],
                      Z2S_DEVICES_DESC[k].z2s_device_clusters[3], Z2S_DEVICES_DESC[k].z2s_device_clusters[4],
                      Z2S_DEVICES_DESC[k].z2s_device_clusters[5], Z2S_DEVICES_DESC[k].z2s_device_clusters[6],
                      Z2S_DEVICES_DESC[k].z2s_device_clusters[7], endpoint_id);
            }
          }
        }
        else
          log_i("LIST checking %s::%s, entry # %d", Z2S_DEVICES[i].manufacturer_name, Z2S_DEVICES[i].model_name, i);
      }
      if (!device_recognized)
        log_d("Unknown model %s::%s, no binding is possible", zbd_manuf_name, zbd_model_name);
    }
  }
}
#endif