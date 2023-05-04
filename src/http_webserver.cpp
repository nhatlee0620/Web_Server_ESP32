#include <ArduinoJson.h>
#include <HTTPUpdate.h>

#include "Arduino.h"
#include "Preferences.h"
#include "app_config.h"
#include "esp_http_client.h"
#include "esp_http_server.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "index_html.h"


httpd_handle_t web_server = NULL;

static esp_err_t wifi_handler(httpd_req_t *req) {
    char *value;
    // const char *SSID, *PWD, *MODE, *TOKEN;
    const char *SSID, *PWD, *MODE;
    int remaining = req->content_len;
    int count = 0;
    char buf[remaining];
    static char info_response[64];
    char *p = info_response;

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_type(req, "application/json");

    if (remaining == 0) {
        httpd_resp_send_500(req);
    }

    while (remaining > 0) {
        /* Read the data for the request */
        int ret = httpd_req_recv(req, buf, remaining);

        StaticJsonDocument<200> doc;
        DeserializationError err = deserializeJson(doc, buf);
        if (err) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(err.c_str());
            return httpd_resp_send_500(req);
        }
        MODE = doc["mode"];
        SSID = doc["ssid"];
        PWD = doc["pass"];
        // TOKEN = doc["token"];

        if (MODE != nullptr) {
            if (String(MODE) == "AP") {
                IPAddress apIP = IPAddress(192, 168, 4, 1);
                const char *hostpass = productCode.c_str();
                bool result = WiFi.softAP(hostname.c_str(), hostpass, 1, 0);
                WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
                if (!result) {
                    Serial.println("AP Config failed.");
                    inAPmode = false;
                    WiFi.softAPdisconnect();
                    return httpd_resp_send(req, String(INVALID).c_str(), 1);
                } else {
                    Serial.println("AP Config Success.");
                    *p++ = '{';
                    p += sprintf(p, "\"ssid\":\"%s\",\"pass\":\"%s\"", hostname.c_str(), hostpass);
                    *p++ = '}';
                    *p++ = 0;
                    httpd_resp_send(req, info_response, strlen(info_response));
                    inAPmode = true;
                    // allowConnectDB = false;
                    delay(5000);
                    WiFi.disconnect();
                }
            } else if (String(MODE) == "STA") {
                // if (TOKEN != nullptr) {
                //     devicePreferences.begin("device", false);
                //     devicePreferences.remove("token");
                //     devicePreferences.putString("token", TOKEN);
                //     devicePreferences.putBool("CheckModeDFlower", CheckModeDFlower);
                //     devicePreferences.end();
                //     gotToken = true;
                // }
                if ((SSID != nullptr) && (PWD != nullptr)) {
                    WiFi.begin(SSID, PWD);
                    inAPmode = false;
                    while (WiFi.status() != WL_CONNECTED) {
                        delay(500);
                        Serial.print(F("."));
                        count++;
                        if (count >= 30) {
                            httpd_resp_send(req, String(INVALID).c_str(), 1);
                            inAPmode = true;
                            break;
                        }
                    }
                    if (!inAPmode) {
                        wifiPreferences.begin("WiFi credential", false);
                        wifiPreferences.putString("ssid", SSID);
                        wifiPreferences.putString("password", PWD);
                        wifiPreferences.end();
                    }
                } else {
                    WiFi.begin();
                    inAPmode = false;
                    while (WiFi.status() != WL_CONNECTED) {
                        delay(500);
                        Serial.print(F("."));
                        count++;
                        if (count >= 30) {
                            httpd_resp_send(req, String(INVALID).c_str(), 1);
                            count = 0;
                            inAPmode = true;
                            break;
                        }
                    }
                }
                if (!inAPmode) {
                    // allowConnectDB = true;
                    // gotToken = true;
                    String ip = "http://" + WiFi.localIP().toString();
                    int n;
                    n = sprintf(p, "%s", ip.c_str());
                    httpd_resp_send(req, info_response, strlen(info_response));
                    Serial.println(ip);
                    delay(7000);
                    WiFi.softAPdisconnect();
                } else {
                    WiFi.disconnect();
                }
                count = 0;
            } else {
                return httpd_resp_send_500(req);
            }
        }
        remaining -= ret;
    }
}

unsigned char h2int(char c) {
    if (c >= '0' && c <= '9') {
        return ((unsigned char)c - '0');
    }
    if (c >= 'a' && c <= 'f') {
        return ((unsigned char)c - 'a' + 10);
    }
    if (c >= 'A' && c <= 'F') {
        return ((unsigned char)c - 'A' + 10);
    }
    return (0);
}

String urldecode(String str) {
    String encodedString = "";
    char c;
    char code0;
    char code1;
    for (int i = 0; i < str.length(); i++) {
        c = str.charAt(i);
        if (c == '+') {
            encodedString += ' ';
        } else if (c == '%') {
            i++;
            code0 = str.charAt(i);
            i++;
            code1 = str.charAt(i);
            c = (h2int(code0) << 4) | h2int(code1);
            encodedString += c;
        } else {
            encodedString += c;
        }
        yield();
    }
    return encodedString;
}

static esp_err_t cmd_handler(httpd_req_t *req) {
    char *buf;
    const char *response;
    size_t buf_len;
    int count = 0;
    char mode[6];
    char ssid[30];
    char pass[30];
    httpd_resp_set_hdr(req, "Content-Type", "text/event-stream;charset=UTF-8");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = (char *)malloc(buf_len);
        if (!buf) {
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            // if (httpd_query_key_value(buf, "auto", mode, sizeof(mode)) == ESP_OK) {
            //     /* Insert the code here */
            //     Serial.printf("Auto mode: %s\n", mode);
            //     if (String(mode) == "true") {
            //         CheckModeDFlower = true;
            //     } else
            //         CheckModeDFlower = false;
            //     devicePreferences.begin("device", false);
            //     devicePreferences.putBool("CheckModeDFlower", CheckModeDFlower);
            //     devicePreferences.end();
            //     lightModeStatusAtLocal = true;
            //     /*  */
            // } else {
            if (httpd_query_key_value(buf, "ssid", ssid, sizeof(ssid)) == ESP_OK && urldecode(String(ssid)).length() != 0) {
            } else {
                response = "WIFI name is empty";
                httpd_resp_send(req, response, strlen(response));
            }
            if (httpd_query_key_value(buf, "pass", pass, sizeof(pass)) == ESP_OK && urldecode(String(pass)).length() >= 8) {
            } else {
                response = "Password not enough 8 characters";
                httpd_resp_send(req, response, strlen(response));
            }
            if (urldecode(String(ssid)).length() != 0 && urldecode(String(pass)).length() >= 8) {
                char wifiRes[60];

                Serial.printf("SSID: %s\n", urldecode(String(ssid)).c_str());
                Serial.printf("PASS: %s\n", urldecode(String(pass)).c_str());

                /*  */
                WiFi.begin(urldecode(String(ssid)).c_str(), urldecode(String(pass)).c_str());
                inAPmode = false;
                while (WiFi.status() != WL_CONNECTED) {
                    delay(500);
                    Serial.print(F("."));
                    count++;
                    if (count >= 30) {
                        sprintf(wifiRes, "Connect to wifi %s failed", ssid);
                        Serial.printf("\n%s\n", wifiRes);
                        httpd_resp_send(req, wifiRes, strlen(wifiRes));
                        inAPmode = true;
                        break;
                    }
                }
                if (!inAPmode) {
                    wifiPreferences.begin("WiFi credential", false);
                    wifiPreferences.putString("ssid", urldecode(String(ssid)).c_str());
                    wifiPreferences.putString("password", urldecode(String(pass)).c_str());
                    wifiPreferences.end();
                    // allowConnectDB = true;
                    String ip = "Connect to wifi " + urldecode(String(ssid)) + " successfully\n" + "http://" + WiFi.localIP().toString();
                    response = ip.c_str();
                    httpd_resp_send(req, response, strlen(response));
                    Serial.println(ip);
                    delay(7000);
                    WiFi.softAPdisconnect();
                } else {
                    WiFi.disconnect();
                }
                count = 0;
                /*  */
                sprintf(wifiRes, "Connect to wifi %s successfully", ssid);
                httpd_resp_send(req, wifiRes, strlen(wifiRes));
            }
            response = "Mode is empty";
            httpd_resp_send(req, response, strlen(response));
            // }
        } else {
            free(buf);
            httpd_resp_send_404(req);
            return ESP_FAIL;
        }
        free(buf);
    } else {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    return httpd_resp_send(req, NULL, 0);
}

// static esp_err_t state_handler(httpd_req_t *req) {
//     char *buf;
//     httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
//     // Serial.println(CheckModeDFlower);
//     return httpd_resp_send(req, String(CheckModeDFlower).c_str(), String(CheckModeDFlower).length());
// }

static esp_err_t index_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    Serial.println("Webpage loading!!");
    return httpd_resp_send(req, (const char *)INDEX2_HTML, strlen(INDEX2_HTML));
}

void startWebServer() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_uri_t index_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = index_handler,
        .user_ctx = NULL};

    httpd_uri_t cmd_uri = {
        .uri = "/control",
        .method = HTTP_GET,
        .handler = cmd_handler,
        .user_ctx = NULL};

    // httpd_uri_t state_uri = {
    //     .uri = "/getState",
    //     .method = HTTP_GET,
    //     .handler = state_handler,
    //     .user_ctx = NULL};

    httpd_uri_t wifi_uri = {
        .uri = "/wifi",
        .method = HTTP_POST,
        .handler = wifi_handler,
        .user_ctx = NULL};

    Serial.printf("Starting web server on port: '%d'\n", config.server_port);
    if (httpd_start(&web_server, &config) == ESP_OK) {
        httpd_register_uri_handler(web_server, &index_uri);
        httpd_register_uri_handler(web_server, &wifi_uri);
        httpd_register_uri_handler(web_server, &cmd_uri);
        // httpd_register_uri_handler(web_server, &state_uri);
    }
}