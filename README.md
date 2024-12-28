![logo](images/Freezy%20Arena%20EStops%20logo.png)
# Freezy Estops

Freezy Estops is an application designed to manage emergency stop systems efficiently. This application ensures that all emergency stop mechanisms are monitored and controlled effectively to enhance safety and reliability.

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
