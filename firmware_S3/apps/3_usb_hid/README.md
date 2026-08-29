# USB HID Consumer Control example (ESP32-S3)

Turns the ESP32-S3 native USB-OTG peripheral into a USB HID device that sends
media keys (play/pause, next, previous, volume up/down, stop) using the
`usb_hid_kb` driver from `drivers_hal` (built on the ESP-IDF `esp_tinyusb`
/ TinyUSB component, Consumer Control report descriptor).

## How to use
1. Build and flash: `idf.py build flash monitor`
2. Connect the board to a PC with a **USB data cable** (the native USB
   connector on an ESP32-S3-DevKitC).
3. The host enumerates it as a Consumer Control HID device.
4. Every 2 s the board sends the next media key in a round-robin sequence;
   watch the console and the host media player react.

## Required sdkconfig (already set in `sdkconfig`)
- `CONFIG_HID_TRANSPORT_USB=y`
- `CONFIG_TINYUSB_ENABLED=y`, `CONFIG_TINYUSB_HID_ENABLED=y`

If you change the transport, run `idf.py menuconfig` →
**Component config → HID Driver (drivers_hal) → Active HID transport**.
