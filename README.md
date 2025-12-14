# ESP32-CAM MQTT Image Publisher (Course Project) 📷🔌

This repository is part of a group project for **50.046 Cloud Computing and IoT**. The code captures images from an ESP32-CAM, encodes them in Base64, and publishes them via MQTT over TLS for integration with AWS IoT Core.

---

## Project Overview ✅

- This module captures an image with an ESP32-CAM and uploads it to the cloud.
- The firmware reads frames from the camera, encodes them as Base64, and publishes a JSON payload to the MQTT topic `esp32/camera/pub` every 2s.

---

## Repository layout 🔧

- `esp32_cam/`
  - `esp32_cam.ino` — main Arduino sketch
  - `secrets.h.example` — example config file, copy to `secrets.h` and fill in your values

---

## Setup Instructions (Quickstart) 🚀

1. Copy `esp32_cam/secrets.h.example` to `esp32_cam/secrets.h` and fill the values.

2. Open `esp32_cam/esp32_cam.ino` in the Arduino IDE, ensure that the required packages are installed.

3. Build & Upload.

4. Open the Serial Monitor at 115200 baud to view logs, IP address, and MQTT connection status.
