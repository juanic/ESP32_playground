# BLE HID Consumer Control example (ESP32-S3)

Turns the ESP32-S3 into a Bluetooth LE HID device that sends media keys
(play/pause, next, previous, volume up/down, stop) using the `ble_hid_kb`
driver from `drivers_hal` (built on the ESP-IDF `esp_hid` component, HOGP
profile).

## How to use
1. Build and flash: `idf.py build flash monitor`
2. On the host (PC/phone), pair the device named **ESP32-S3 HID**.
3. Every 2 s the board sends the next media key in a round-robin sequence;
   watch the console and the host media player react.

## Required sdkconfig (already set in `sdkconfig`)
- `CONFIG_HID_TRANSPORT_BLE=y`
- `CONFIG_BT_ENABLED=y`, `CONFIG_BTDM_CTRL_MODE_BLE_ONLY=y`,
  `CONFIG_BT_BLUEDROID_ENABLED=y`, `CONFIG_BT_BLE_ENABLED=y`,
  `CONFIG_BT_GATTS_ENABLE=y`

If you change the transport, run `idf.py menuconfig` →
**Component config → HID Driver (drivers_hal) → Active HID transport**.
