![logo](images/Freezy%20Arena%20EStops%20logo.png)
# Freezy Estops

Freezy Estops is an application designed to manage emergency stop systems efficiently. This application ensures that all emergency stop mechanisms are monitored and controlled effectively to enhance safety and reliability.


## Board Types / Build Enviorments 

- **ESP32-S3-DevKitM-1**
  - [ESP32-S3 ETH Development Board w/ POE](https://www.amazon.com/ESP32-S3-Ethernet-Development-Processor-Communication/dp/B0DKXFB7PW/ref=sr_1_3?crid=1L2Z0CGR54FHU&dib=eyJ2IjoiMSJ9.UefYGZXbDey6UFv4YsbhNxskNttFl8s9nGFclmjvl9U9fHZN8czEXfbN5aZnUTA7yx2L9qz8ZMAuRYZKQqMXRXAwJYT3bybECpT9NrKEqH9NaQ6Aw_kVLN3HTad1Wk7wn4A42ch_y0oK8E76IgsZocKAqC4twgTc6pTBtCTQBqWJ1jjCOuei6QaViupWRpQ1Z9kxDIeJFblM3zB5BZhZNqrj43zmrMXRS32pJsaL80U.UAjUaJCDwETiQ7RGK9MvWZO2R9tGdLXIJ6uy08mdCZc&dib_tag=se&keywords=esp32+poe&qid=1735571720&sprefix=esp32+poe%2Caps%2C190&sr=8-3)
  - This board is used for Ethernet connectivity.
  - Pins connected to the stop buttons: 
    - 33  // Field stop
    - 34  // 1E stop
    - 35  // 1A stop
    - 36  // 2E stop
    - 37  // 2A stop
    - 38  // 3E stop
    - 39  // 3A stop
  - Start match button pin: 40
  - LED strip pin: 17
  - ~~ONBOARD_LED~~ Board does not have this option
  - ONBOARD_RGB 21

- **ESP32dev**
    -[ESP32-S3-DevKitM-1 on Amazon](https://www.amazon.com/ESP-WROOM-32-Development-Microcontroller-Integrated-Compatible/dp/B08D5ZD528/ref=sr_1_3?crid=1BH3BAGLCLPR&dib=eyJ2IjoiMSJ9.XBINg-sjhfF_gUtnMiKGjhCiGW31sXyKLVbL01ZB_eTAv6nJuLtxLYF73ReJRPW1_2Dkc5tT8U7R4Mj3lUXideu8IC_M2NuE5i72-Z6dyDPiToma8un9pUOxEvTdo3YWM1TqHYk89fS2fph4NjHJJI6Vm9B4wR3vTx6njmpae68pk34QTGDdAitnvB8MC73P3nyoX88WTIa1-5LNC1yu3gy5VwFetQGBE68NKv_Amsg.FhBxCRGXHEoQ3rz-NuNdTxv0LFx1RpvhzgUoUu9mP8c&dib_tag=se&keywords=esp32&qid=1735571566&sprefix=esp32%2Caps%2C147&sr=8-3)
  - This board is used for WiFi connectivity.
  - Pins connected to the stop buttons: 
    - 21  // Field stop
    - 22  // 1E stop
    - 23  // 1A stop
    - 25  // 2E stop
    - 26  // 2A stop
    - 27  // 3E stop
    - 32  // 3A stop
  - Start match button pin: 33
  - LED strip pin: 4
  - ONBOARD_LED 2

## Features

-- Work In Prgress
- **Web-Based Configuration**: Easily configure the device settings through a web interface.
  - **Alliance Color Selection**: Choose between Red, Blue, and Field alliance colors.
  - **Network Configuration**: Set up the device IP, arena IP, and port.
  - **DHCP Support**: Currently only DHCP

- **Real-Time Status Updates**: Monitor and update the status of the device in real-time.
  - **LED Indicators**: Visual feedback of filed status through LED indicators for different statuses.
  - **Stop Button Monitoring**: Monitor multiple stop buttons and update their status.

- **Ethernet Connectivity**: Reliable network connection using Ethernet.

- **SPIFFS File System**: Store and serve static files such as images from the ESP32's file system.
  - **Image Serving**: Serve images for the web interface directly from the ESP32.

- **REST API Integration**: Communicate with the arena system using REST API.
  - **Start Match**: Send a request to start the match.
  - **Stop Status Update**: Update the stop status through API calls.

- **User-Friendly Interface**: Intuitive and easy-to-use web interface for configuration and monitoring.

- **Preferences Storage**: Save and retrieve configuration settings using non-volatile storage.
  - **Persistent Settings**: Ensure settings are retained across device reboots.


## Contributing

We welcome contributions! Please follow these steps to contribute:

1. Fork the repository.
2. Create a new branch (`git checkout -b feature-branch`).
3. Make your changes.
4. Commit your changes (`git commit -m 'Add some feature'`).
5. Push to the branch (`git push origin feature-branch`).
6. Open a pull request.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
