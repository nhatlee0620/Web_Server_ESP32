#include <WiFi.h>
#include <WiFiClient.h>
#include <esp_wifi.h>
#include <sys/time.h>

#include "Preferences.h"
#include "src/app_config.h"

Preferences wifiPreferences;
Preferences devicePreferences;

WiFiClient wifiClient;

int count2 = 0, countCheckMode = 0, timeOut = 0, countFR = 0;  // count factory reset
// bool inAPmode, gotToken = false, allowConnectDB;
// String hostname, productCode, TOKEN;
bool inAPmode;
String hostname, productCode;
// bool CheckModeDFlower = true, otaBegin = false;
// bool lightModeStatusAtLocal = false;
unsigned long previousMillis = 0, interval = 15000;
char macAddress[12];

void startWebServer();

// begin WiFi AP mode
void beginAP() {
    Serial.println("Enter AP mode");
    IPAddress apIP = IPAddress(192, 168, 4, 1);
    hostname = PRODUCT_NAME + String("_AP_") + WiFi.macAddress();
    const char *hostpass = productCode.c_str();
    bool result = WiFi.softAP(hostname.c_str(), hostpass, 1, 0);
    delay(500);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    if (!result) {
        Serial.println("AP Config failed.");
        inAPmode = false;
        ESP.restart();
        return;
    } else {
        // allowConnectDB = false;
        Serial.println(String("AP Config Success. Stream link: ") + WiFi.softAPIP().toString());
        inAPmode = true;
        WiFi.disconnect();
    }
}

// check Wifi connection to make sure STA or AP mode is activating
void checkWifiConnection(void *pvParameters) {
    while (true) {
        unsigned long currentMillis = millis();
        if ((WiFi.status() != WL_CONNECTED) &&
            (currentMillis - previousMillis >= interval) && (!inAPmode) &&
            (WiFi.softAPgetStationNum() < 1)) {
            Serial.println("Reconnecting to WiFi...");
            WiFi.disconnect();
            WiFi.reconnect();
            count2++;
            if (count2 >= 5) {
                countCheckMode = 0;
                beginAP();
                // break;
            }
            previousMillis = currentMillis;
        }
        if (inAPmode && count2 != 0)
            count2 = 0;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
    countCheckMode = 0;
}

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    devicePreferences.begin("device", false);
    countFR = devicePreferences.getInt("countFR", 0) + 1;
    devicePreferences.putInt("countFR", countFR);
    if (devicePreferences.getInt("countFR", 0) >= 4) {
        devicePreferences.clear();
        wifiPreferences.begin("WiFi credential", false);
        wifiPreferences.clear();
        wifiPreferences.end();
        Serial.println("Factory reset!");
    }
    devicePreferences.end();

    // Product_Code
    sprintf(macAddress, "%04X%08X", (uint32_t)(ESP.getEfuseMac() >> 32), (uint32_t)ESP.getEfuseMac());
    productCode = String(macAddress);
    Serial.printf("Mac address: %s\n", productCode.c_str());
    Serial.printf("Firmware version: %s\n", PRODUCT_VERSION);

    // Reset factory reset counter
    delay(3000);
    devicePreferences.begin("device", false);
    devicePreferences.remove("countFR");
    devicePreferences.end();

    // setting WiFi
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAPdisconnect();
    wifiPreferences.begin("WiFi credential", true);
    String SSID = wifiPreferences.getString("ssid", "");
    String PWD = wifiPreferences.getString("password", "");
    wifiPreferences.end();
    if (SSID == "") {
        beginAP();
    } else {
        Serial.print("Connecting to WiFi");
        WiFi.begin(SSID.c_str(), PWD.c_str());
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(F("."));
            countCheckMode++;
            if (countCheckMode >= 30) {
                countCheckMode = 0;
                beginAP();
                break;
            }
            if (String(WiFi.localIP()) != "0") {
                hostname = PRODUCT_NAME + String("_AP_") + WiFi.macAddress();
                Serial.println(String("Connected to WiFi. Stream link: ") + WiFi.localIP().toString());
            }
        }
        countCheckMode = 0;
    }
    startWebServer();  // start Web server
    disableCore0WDT();

    xTaskCreatePinnedToCore(
        checkWifiConnection,
        "checkWifiConnection",
        8192,
        NULL,
        1,
        NULL,
        0);
}

void loop() {
    // put your main code here, to run repeatedly:
    Serial.println("Hello");
    delay(1000);
}