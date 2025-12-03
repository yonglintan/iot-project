#include <PubSubClient.h>
#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <base64.h>
#include "esp_system.h"
#include "esp_psram.h"
// #include <ArduinoJson.h>

// Secrets and config options. Refer to .example file.
#include "secrets.h"

// MQTT client
WiFiClientSecure net;
PubSubClient client(net);

const char* mqtt_topic = "esp32/camera/pub";

void setup() {
  Serial.begin(115200);

  // Connect WiFi
  WiFi.begin(ssid, password);
  Serial.print("Starting WiFi connection");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Set up TLS
  net.setCACert(ca_cert);
  net.setCertificate(client_cert);
  net.setPrivateKey(private_key);

  client.setServer(mqtt_server, mqtt_port);
  client.setBufferSize(100000);
  Serial.print("MQTT send/receive buffer size: ");
  Serial.println(client.getBufferSize());

  if (client.connect(deviceId)) {
    Serial.println("Connected to AWS IoT!");
  } else {
    Serial.print("Failed MQTT connection, rc=");
    Serial.println(client.state());
  }

  if (psramInit()) {
    Serial.println("PSRAM initialized successfully!");
  } else {
    Serial.println("PSRAM not found or failed to initialize!");
  }
  // Initialize camera
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_pwdn = 32;
  config.pin_reset = -1;   // no reset pin
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_VGA;  // 640x480
    config.jpeg_quality = 12;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 15;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return;
  } else {
    Serial.println("Camera init success!");
  }

}

// --- Publish JSON camera data via MQTT ---
void publishCameraMessage(String base64Img) {
  // StaticJsonDocument<256000> doc;
  // doc["device"] = String(deviceId);
  // doc["image"] = base64Img;
  // doc["timestamp"] = millis();

  // char jsonBuffer[256000];
  // serializeJson(doc, jsonBuffer, sizeof(jsonBuffer));

  if (client.connected()) {
    String payload = "{\"device\":\"" + String(deviceId) + "\",\"image\":\"" + base64Img + "\"}";
    bool ok = client.publish(mqtt_topic, payload.c_str());
    // bool ok = client.publish(mqtt_topic, jsonBuffer);
    if (ok) {
      // Serial.println("Published to camera topic: ");
      // Serial.println(jsonBuffer);
      Serial.println("✅ Publish OK");
    } else {
      // Serial.println("Publish to camera topic failed!");
      Serial.println("❌ Publish failed");
    }
  } else {
    Serial.println("MQTT not connected, skip publish to camera topic.");
  }
}

void loop() {
  if (!client.connected()) {
    Serial.println("Client disconnected, retrying");
    client.connect(deviceId);
    delay(1000);
    return;
  }
  
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("❌ Camera capture failed!");
    delay(2000);
    return;
  }
  Serial.printf("✅ Captured image: %d bytes\n", fb->len);

  String encoded = base64::encode(fb->buf, fb->len);
  Serial.printf("✅ Encoded Base64 length: %d bytes\n", encoded.length());

  esp_camera_fb_return(fb);

  publishCameraMessage(encoded);

  // String payload = "{\"deviceId\":\"" + String(deviceId) + "\",\"imageData\":\"" + encoded + "\"}";
  // Serial.printf("✅ Payload size: %d bytes\n", payload.length());

  // boolean result = client.publish(mqtt_topic, payload.c_str());
  // Serial.println(result ? "✅ Publish OK" : "❌ Publish failed");

  delay(5000); // every 5 seconds
}
