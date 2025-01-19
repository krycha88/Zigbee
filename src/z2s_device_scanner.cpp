#ifdef Z2S_DEVICE_SCANNER
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <LittleFS.h>
#include "FS.h"
#include <EEPROM.h>
#include <Ticker.h>

#include <ZigbeeCore.h>
#define EP_CLUSTERS_ATTRIBUTES_SCANNER 1

#include <ZigbeeGateway.h>
#ifdef ZIGBEE_MODE_ZCZR
zigbee_role_t role = ZIGBEE_ROUTER;  // or can be ZIGBEE_COORDINATOR, but it won't scan itself
#else
zigbee_role_t role = ZIGBEE_END_DEVICE;
#endif

ZigbeeGateway zbGateway = ZigbeeGateway(ZIGBEE_ROUTER);

void printScannedNetworks(uint16_t networksFound) {
  if (networksFound == 0) {
    Serial.println("No networks found");
  }
  else {
    zigbee_scan_result_t *scan_result = Zigbee.getScanResult();
    Serial.println("\nScan done");
    Serial.print(networksFound);
    Serial.println(" networks found:");
    Serial.println("Nr | PAN ID | CH | Permit Joining | Router Capacity | End Device Capacity | Extended PAN ID");
    for (int i = 0; i < networksFound; ++i) {
      Serial.printf("%2d", i + 1);
      Serial.print(" | ");
      Serial.printf("0x%04hx", scan_result[i].short_pan_id);
      Serial.print(" | ");
      Serial.printf("%2d", scan_result[i].logic_channel);
      Serial.print(" | ");
      Serial.printf("%-14.14s", scan_result[i].permit_joining ? "Yes" : "No");
      Serial.print(" | ");
      Serial.printf("%-15.15s", scan_result[i].router_capacity ? "Yes" : "No");
      Serial.print(" | ");
      Serial.printf("%-19.19s", scan_result[i].end_device_capacity ? "Yes" : "No");
      Serial.print(" | ");
      Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", scan_result[i].extended_pan_id[7], scan_result[i].extended_pan_id[6],
                    scan_result[i].extended_pan_id[5], scan_result[i].extended_pan_id[4], scan_result[i].extended_pan_id[3],
                    scan_result[i].extended_pan_id[2], scan_result[i].extended_pan_id[1], scan_result[i].extended_pan_id[0]);
      Serial.println();
      delay(10);
    }
    Serial.println("");
    Zigbee.scanDelete();  // Free memory
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Zigbee start!");

  zbGateway.setManufacturerAndModel("Zigbee2Supla", "Z2S_Device_Scanner");
  zbGateway.allowMultipleBinding(true);

  Zigbee.addEndpoint(&zbGateway);

  // Open network for 180 seconds after boot
  Zigbee.setRebootOpenNetwork(180);
  Zigbee.openNetwork(180);

  if (!Zigbee.begin(ZIGBEE_COORDINATOR)) {
    Serial.println("Zigbee failed to start!");
    Serial.println("Rebooting...");
    ESP.restart();
  }
}
void loop() {
  if (zbGateway.isNewDeviceJoined()) {
    zbGateway.clearNewDeviceJoined();
    zbGateway.printJoinedDevices();

    while (!zbGateway.getJoinedDevices().empty()) {
      zb_device_params_t *joined_device = zbGateway.getLastJoinedDevice();

      log_i("manufacturer %s ", zbGateway.readManufacturer(joined_device->endpoint, joined_device->short_addr, joined_device->ieee_addr));
      log_i("model %s ", zbGateway.readModel(joined_device->endpoint, joined_device->short_addr, joined_device->ieee_addr));
      char *manufacturer = zbGateway.readManufacturer(joined_device->endpoint, joined_device->short_addr, joined_device->ieee_addr);
      char *model = zbGateway.readModel(joined_device->endpoint, joined_device->short_addr, joined_device->ieee_addr);

      Serial.printf("New Device: Short Addr: 0x%04X, Endpoint: %d\n", joined_device->short_addr, joined_device->endpoint);
      Serial.printf("Manufacturer: %s, Model: %s\n", manufacturer ? manufacturer : "Unknown", model ? model : "Unknown");

      zbGateway.zbPrintDeviceDiscovery(joined_device);
    }
  }
}
// #endif //EP_CLUSTERS_ATTRIBUTES_SCANNER
// // Skanowanie sieci Zigbee
// int16_t ZigbeeScanStatus = Zigbee.scanComplete();
// if (ZigbeeScanStatus < 0) {  // Skanowanie w toku lub błąd
//   if (ZigbeeScanStatus == ZB_SCAN_FAILED) {
//     Serial.println("Zigbee scan has failed. Restarting scan.");
//     Zigbee.scanNetworks();  // Restartuj skanowanie w przypadku błędu
//   }
//   // ZB_SCAN_RUNNING - oczekiwanie
// }
// else {                                     // Skanowanie zakończone
//   printScannedNetworks(ZigbeeScanStatus);  // Wyświetlenie wyników

//   // Obsługa nowych urządzeń
//   if (zbGateway.isNewDeviceJoined()) {
//     zbGateway.clearNewDeviceJoined();
//     zbGateway.printJoinedDevices();  // Opcjonalne logowanie

//     while (!zbGateway.getJoinedDevices().empty()) {
//       zb_device_params_t *joined_device = zbGateway.getLastJoinedDevice();

//       // Pobierz dane producenta i model
//       char *manufacturer = zbGateway.readManufacturer(joined_device->endpoint, joined_device->short_addr, joined_device->ieee_addr);
//       char *model = zbGateway.readModel(joined_device->endpoint, joined_device->short_addr, joined_device->ieee_addr);

//       // Wyświetl szczegóły urządzenia
//       Serial.printf("Nowe urządzenie: Short Addr: 0x%04X, Endpoint: %d\n", joined_device->short_addr, joined_device->endpoint);
//       Serial.printf("Manufacturer: %s, Model: %s\n", manufacturer ? manufacturer : "Unknown", model ? model : "Unknown");

//       // Wywołanie funkcji debugującej
//       zbGateway.zbPrintDeviceDiscovery(joined_device);

//       // Usuń urządzenie z kolejki po przetworzeniu
//       // zbGateway.popJoinedDevice();
//     }
//   }

//   Zigbee.scanNetworks();  // Rozpocznij nowe skanowanie
// }

// #define EP_CLUSTERS_ATTRIBUTES_SCANNER 1

// #include <ZigbeeGateway.h>

// #define GATEWAY_ENDPOINT_NUMBER 1

// #define BUTTON_PIN 9  // Boot button for C6/H2

// ZigbeeGateway zbGateway = ZigbeeGateway(GATEWAY_ENDPOINT_NUMBER);

// void sz_ias_zone_notification(int status, uint8_t *ieee_addr_64) {
// }

// uint32_t startTime = 0;
// uint32_t printTime = 0;
// uint32_t zbInit_delay = 0;

// bool zbInit = true;

// void setup() {
//   pinMode(BUTTON_PIN, INPUT_PULLUP);
//   Serial.println("Zigbee start!");

//   //  Zigbee

//   zbGateway.setManufacturerAndModel("Zigbee2Supla", "Z2S_Device_Scanner");
//   zbGateway.allowMultipleBinding(true);

//   Zigbee.addEndpoint(&zbGateway);

//   //Open network for 180 seconds after boot
//   Zigbee.setRebootOpenNetwork(180);

//   if (!Zigbee.begin(ZIGBEE_COORDINATOR)) {
//       Serial.println("Zigbee failed to start!");
//       Serial.println("Rebooting...");
//       ESP.restart();
//   }

//   startTime = millis();
//   printTime = millis();
//   zbInit_delay = millis();
// }

// zb_device_params_t *gateway_device;
// zb_device_params_t *joined_device;

// char zbd_model_name[64];

// uint16_t gateway_devices_list_size = 0;

// void loop() {

//   if (digitalRead(BUTTON_PIN) == LOW) {  // Push button pressed
//     // Key debounce handling
//     delay(100);

//     while (digitalRead(BUTTON_PIN) == LOW) {
//       delay(50);
//       if ((millis() - startTime) > 5000) {
//         // If key pressed for more than 5 secs, factory reset Zigbee and reboot
//         Serial.printf("Resetting Zigbee to factory settings, reboot.\n");
//         Zigbee.factoryReset();
//       }
//     }
//     Zigbee.openNetwork(180);
//   }
//   delay(100);

//   if (zbGateway.isNewDeviceJoined()) {

//     zbGateway.clearNewDeviceJoined();
//     zbGateway.printJoinedDevices();

//     while (!zbGateway.getJoinedDevices().empty())
//     {
//       joined_device = zbGateway.getLastJoinedDevice();

//       log_i("manufacturer %s ", zbGateway.readManufacturer(joined_device->endpoint, joined_device->short_addr, joined_device->ieee_addr));
//       log_i("model %s ", zbGateway.readModel(joined_device->endpoint, joined_device->short_addr, joined_device->ieee_addr));

// #ifdef EP_CLUSTERS_ATTRIBUTES_SCANNER

//     zbGateway.zbPrintDeviceDiscovery(joined_device);

// #endif //EP_CLUSTERS_ATTRIBUTES_SCANNER
//     }
//   }
// }
#endif