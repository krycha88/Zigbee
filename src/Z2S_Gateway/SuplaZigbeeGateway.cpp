#include "SuplaZigbeeGateway.h"

#include <ZigbeeGateway.h>

#include <supla/events.h>
#include <supla/control/button.h>
#include <supla/storage/storage.h>

#include <esp_coexist.h>

ZigbeeGateway zbGateway = ZigbeeGateway(GATEWAY_ENDPOINT_NUMBER);

namespace Supla {
SuplaZigbeeGateway::SuplaZigbeeGateway(const char* jsonDevices, int factoryResetButtonPin)
    : factoryResetButtonPin(factoryResetButtonPin), zbInit(true), startTime(0), zbInitDelay(5000), gatewayDevice(nullptr), joinedDevice(nullptr) {
  memset(zbdModelName, 0, sizeof(zbdModelName));
  memset(zbdManufName, 0, sizeof(zbdManufName));

  parseDevicesFromJson(jsonDevices);
  loadDevicesFromProgMem(Z2S_DEVICES_LIST);
}

SuplaZigbeeGateway::SuplaZigbeeGateway(const z2s_device_entity_t* devices, int factoryResetButtonPin)
    : factoryResetButtonPin(factoryResetButtonPin), zbInit(true), startTime(0), zbInitDelay(5000), gatewayDevice(nullptr), joinedDevice(nullptr) {
  memset(zbdModelName, 0, sizeof(zbdModelName));
  memset(zbdManufName, 0, sizeof(zbdManufName));

  loadDevicesFromProgMem(devices);
}

SuplaZigbeeGateway::~SuplaZigbeeGateway() {
}

void SuplaZigbeeGateway::onInit() {
  auto buttonCfgRelay = new Supla::Control::Button(factoryResetButtonPin, true, true);
  buttonCfgRelay->addAction(factoryReset, this, Supla::ON_CLICK_10);
  // Initialize Supla storage
  Supla::Storage::Init();

  Z2S_loadDevicesTable();
  Z2S_initSuplaChannels();

  //  Zigbee Gateway notifications
  zbGateway.onTemperatureReceive(Z2S_onTemperatureReceive);
  zbGateway.onHumidityReceive(Z2S_onHumidityReceive);
  zbGateway.onOnOffReceive(Z2S_onOnOffReceive);
  zbGateway.onRMSVoltageReceive(Z2S_onRMSVoltageReceive);
  zbGateway.onRMSCurrentReceive(Z2S_onRMSCurrentReceive);
  zbGateway.onRMSActivePowerReceive(Z2S_onRMSActivePowerReceive);
  zbGateway.onBatteryPercentageReceive(Z2S_onBatteryPercentageReceive);

  zbGateway.onIASzoneStatusChangeNotification(Z2S_onIASzoneStatusChangeNotification);

  zbGateway.onBoundDevice(Z2S_onBoundDevice);
  zbGateway.onBTCBoundDevice(Z2S_onBTCBoundDevice);

  zbGateway.setManufacturerAndModel("Supla", "Z2SGateway");
  zbGateway.allowMultipleBinding(true);

  Zigbee.addEndpoint(&zbGateway);

  // Open network for 180 seconds after boot
  Zigbee.setRebootOpenNetwork(180);

  startTime = millis();
}

void SuplaZigbeeGateway::iterateAlways() {
  // if (SuplaDevice.getCurrentStatus() == Supla::DEVICE_MODE_CONFIG) {
  //   return;
  // }

  if (zbInit && millis() - zbInitDelay > startTime) {
    esp_coex_wifi_i154_enable();
    if (!Zigbee.begin(ZIGBEE_COORDINATOR)) {
      Serial.println("Zigbee initialization failed! Restarting...");
      ESP.restart();
    }
    zbInit = false;
  }

  if (zbGateway.isNewDeviceJoined()) {
    zbGateway.clearNewDeviceJoined();
    zbGateway.printJoinedDevices();

    while (!zbGateway.getJoinedDevices().empty()) {
      joinedDevice = zbGateway.getLastJoinedDevice();

      strcpy(zbdModelName, zbGateway.readManufacturer(joinedDevice->endpoint, joinedDevice->short_addr, joinedDevice->ieee_addr));
      log_i("manufacturer %s ", zbdManufName);
      strcpy(zbdModelName, zbGateway.readModel(joinedDevice->endpoint, joinedDevice->short_addr, joinedDevice->ieee_addr));
      log_i("model %s ", zbdModelName);

      uint16_t devices_list_table_size = sizeof(Z2S_DEVICES) / sizeof(Z2S_DEVICES[0]);
      uint16_t devices_desc_table_size = sizeof(Z2S_DEVICES_DESC) / sizeof(Z2S_DEVICES_DESC[0]);
      bool device_recognized = false;

      for (int i = 0; i < devices_list_table_size; i++) {
        if ((strcmp(zbdModelName, Z2S_DEVICES[i].model_name) == 0) && (strcmp(zbdManufName, Z2S_DEVICES[i].manufacturer_name) == 0)) {
          log_i("LIST matched %s::%s, entry # %d, endpoints # %d, endpoints 0x%x::0x%x,0x%x::0x%x,0x%x::0x%x,0x%x::0x%x",
                Z2S_DEVICES[i].manufacturer_name, Z2S_DEVICES[i].model_name, i, Z2S_DEVICES[i].z2s_device_endpoints_count,
                Z2S_DEVICES[i].z2s_device_endpoints[0].endpoint_id, Z2S_DEVICES[i].z2s_device_endpoints[0].z2s_device_desc_id,
                Z2S_DEVICES[i].z2s_device_endpoints[1].endpoint_id, Z2S_DEVICES[i].z2s_device_endpoints[1].z2s_device_desc_id,
                Z2S_DEVICES[i].z2s_device_endpoints[2].endpoint_id, Z2S_DEVICES[i].z2s_device_endpoints[2].z2s_device_desc_id,
                Z2S_DEVICES[i].z2s_device_endpoints[3].endpoint_id, Z2S_DEVICES[i].z2s_device_endpoints[3].z2s_device_desc_id);

          for (int j = 0; j < Z2S_DEVICES[i].z2s_device_endpoints_count; j++) {
            uint8_t endpoint_id = (Z2S_DEVICES[i].z2s_device_endpoints_count == 1) ? 1 : Z2S_DEVICES[i].z2s_device_endpoints[j].endpoint_id;

            uint32_t z2s_device_desc_id = (Z2S_DEVICES[i].z2s_device_endpoints_count == 1)
                                              ? Z2S_DEVICES[i].z2s_device_desc_id
                                              : Z2S_DEVICES[i].z2s_device_endpoints[j].z2s_device_desc_id;

            for (int k = 0; k < devices_desc_table_size; k++) {
              if (z2s_device_desc_id == Z2S_DEVICES_DESC[k].z2s_device_desc_id) {
                log_i("DESC matched 0x%x, %d, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, endpoint 0x%x ", Z2S_DEVICES_DESC[k].z2s_device_desc_id,
                      Z2S_DEVICES_DESC[k].z2s_device_clusters_count, Z2S_DEVICES_DESC[k].z2s_device_clusters[0],
                      Z2S_DEVICES_DESC[k].z2s_device_clusters[1], Z2S_DEVICES_DESC[k].z2s_device_clusters[2],
                      Z2S_DEVICES_DESC[k].z2s_device_clusters[3], Z2S_DEVICES_DESC[k].z2s_device_clusters[4],
                      Z2S_DEVICES_DESC[k].z2s_device_clusters[5], Z2S_DEVICES_DESC[k].z2s_device_clusters[6],
                      Z2S_DEVICES_DESC[k].z2s_device_clusters[7], endpoint_id);

                device_recognized = true;

                joinedDevice->endpoint = endpoint_id;
                joinedDevice->model_id = Z2S_DEVICES_DESC[k].z2s_device_desc_id;

                Z2S_addZ2SDevice(joinedDevice);

                for (int m = 0; m < Z2S_DEVICES_DESC[k].z2s_device_clusters_count; m++)
                  zbGateway.bindDeviceCluster(joinedDevice, Z2S_DEVICES_DESC[k].z2s_device_clusters[m]);
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
        log_d("Unknown model %s::%s, no binding is possible", zbdManufName, zbdModelName);
    }
  }
}

void SuplaZigbeeGateway::handleAction(int event, int action) {
  if (event == Supla::ON_CLICK_10 && action == factoryReset) {
    Serial.println("Resetting Zigbee to factory settings, reboot.");
    Zigbee.factoryReset();
    Zigbee.openNetwork(180);
  }
}

void SuplaZigbeeGateway::parseDevicesFromJson(const char* json) {
  JsonDocument doc;

  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    Serial.print("Deserialization failed: ");
    Serial.println(error.f_str());
    return;
  }

  JsonArray devicesArray = doc["devices"];
  if (devicesArray.isNull()) {
    Serial.println("Error: 'devices' is null or not found");
    return;
  }

  for (JsonObject device : devicesArray) {
    z2s_device_entity_t newDevice;

    strncpy(newDevice.manufacturer_name, device["manufacturer_name"].as<const char*>(), sizeof(newDevice.manufacturer_name) - 1);
    newDevice.manufacturer_name[sizeof(newDevice.manufacturer_name) - 1] = '\0';

    strncpy(newDevice.model_name, device["model_name"].as<const char*>(), sizeof(newDevice.model_name) - 1);
    newDevice.model_name[sizeof(newDevice.model_name) - 1] = '\0';

    newDevice.z2s_device_desc_id = device["z2s_device_desc_id"];
    newDevice.z2s_device_endpoints_count = device["z2s_device_endpoints_count"];

    Z2S_DEVICES.push_back(newDevice);
  }
}

void SuplaZigbeeGateway::loadDevicesFromProgMem(const z2s_device_entity_t* deviceList) {
  size_t deviceCount = sizeof(deviceList) / sizeof(deviceList[0]);
  for (size_t i = 0; i < deviceCount; ++i) {
    z2s_device_entity_t device;
    memcpy_P(&device, &deviceList[i], sizeof(z2s_device_entity_t));
    Z2S_DEVICES.push_back(device);
  }
}

};  // namespace Supla
