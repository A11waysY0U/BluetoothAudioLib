# BluetoothAudioLib

[简体中文](./README_CN.md) | [English](./README.md)

An Arduino library for ESP32 that enables Bluetooth A2DP audio reception and I2S output.

This library has been tested on ESP32-Wrover-E, but it should also be compatible with other modules that support classic Bluetooth.

## Features
- **Bluetooth A2DP Audio Sink**: Easily turn your ESP32 into a Bluetooth speaker.
- **I2S Audio Output**: Supports the standard I2S protocol for connecting external DACs or amplifiers.
- **Volume Control**: Supports remote volume control.
- **Playback Control**: Supports play, pause, next, and previous track controls.
- **Song Metadata**: Retrieves detailed information about the currently playing song (title, artist, album, genre).
- **Connection Management**: Supports automatic reconnection and manual disconnection.

## Installation

1. Download the ZIP file of this repository.
2. In the Arduino IDE, select `Sketch` -> `Include Library` -> `Add .ZIP Library`.
3. Select the downloaded ZIP file to install.

## Usage Example

```cpp
#include <BluetoothAudio.h>

// Create a BluetoothAudio object and set the Bluetooth device name
BluetoothAudio audio("MyESP32Speaker");

void setup() {
    // Start the Bluetooth audio
    audio.begin();

    // Configure the I2S pins (BCK, DOUT, WS)
    audio.I2S(27, 26, 25);

    // Set the initial volume (0.0 - 1.0)
    audio.volume(0.8);
}

void loop() {
    // You can update the song metadata in the loop
    // audio.updateMeta();

    // Print song information
    if (BluetoothAudio::title != "" && BluetoothAudio::artist != "") {
        Serial.printf("Now playing: %s - %s\n", BluetoothAudio::title.c_str(), BluetoothAudio::artist.c_str());
    }
    delay(2000);
}
```

## API Reference

### `BluetoothAudio(const char* devName)`
Constructor, creates a Bluetooth audio object.
- `devName`: The name of the Bluetooth device.

### `void begin()`
Initializes the Bluetooth audio service.

### `void end()`
Stops the Bluetooth audio service.

### `void reconnect()`
Reconnects to the last connected device.

### `void disconnect()`
Disconnects the current connection.

### `void setSinkCallback(void (*sinkCallback)(const uint8_t *data, uint32_t len))`
Sets the audio data callback function.
- `sinkCallback`: A pointer to the callback function.

### `void I2S(int bck, int dout, int ws)`
Configures the I2S pins.
- `bck`: Bit Clock (BCLK)
- `dout`: Data Out (DOUT)
- `ws`: Word Select (LRC)

### `void play()`
Plays the audio.

### `void pause()`
Pauses the audio.

### `void next()`
Skips to the next track.

### `void previous()`
Goes back to the previous track.

### `void updateMeta()`
Requests an update of the song metadata.

### `void volume(float vol)`
Sets the volume.
- `vol`: The volume level (from 0.0 to 1.0).

### `float getVolume()`
Gets the current volume.

### Static Variables
- `String title`: Song title
- `String artist`: Artist
- `String album`: Album
- `String genre`: Genre
- `bool isConnected`: Connection status


## Contributing
Contributions are welcome through Pull Requests or Issues!

## License
This project is licensed under the Apache 2.0 License.
