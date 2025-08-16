# BluetoothAudioLib

[简体中文](./README_CN.md) | [English](./README.md)


一个用于ESP32的蓝牙A2DP音频接收与I2S输出的Arduino库。

目前只测试了ESP32-Wrover-E, 其他支持经典蓝牙的模块应该也可以用。

## 功能
- **蓝牙A2DP音频接收**: 轻松将您的ESP32变成一个蓝牙音箱。
- **I2S音频输出**: 支持标准的I2S协议，方便连接外部DAC或功放。
- **音量调节**: 支持远程音量控制。
- **播放控制**: 支持播放、暂停、下一曲、上一曲等控制。
- **歌曲元数据**: 获取当前播放歌曲的详细信息（歌名、歌手、专辑、流派）。
- **连接管理**: 支持自动重连和手动断开连接。

## 安装

1. 下载本仓库的ZIP文件。
2. 在Arduino IDE中, 选择 `项目` -> `加载库` -> `添加.ZIP库`。
3. 选择下载的ZIP文件进行安装。

## 用法示例

```cpp
#include <BluetoothAudio.h>

// 创建一个蓝牙音频对象，并设置蓝牙设备名称
BluetoothAudio audio("MyESP32Speaker");

void setup() {
    // 启动蓝牙音频
    audio.begin();

    // 配置I2S引脚 (BCK, DOUT, WS)
    audio.I2S(27, 26, 25); 

    // 设置初始音量 (0.0 - 1.0)
    audio.volume(0.8);
}

void loop() {
    // 可以在循环中更新歌曲元数据
    // audio.updateMeta();

    // 打印歌曲信息
    if (BluetoothAudio::title != "" && BluetoothAudio::artist != "") {
        Serial.printf("Now playing: %s - %s\n", BluetoothAudio::title.c_str(), BluetoothAudio::artist.c_str());
    }
    delay(2000);
}
```

## API 参考

### `BluetoothAudio(const char* devName)`
构造函数，创建一个蓝牙音频对象。
- `devName`: 蓝牙设备名称。

### `void begin()`
初始化蓝牙音频服务。

### `void end()`
关闭蓝牙音频服务。

### `void reconnect()`
重新连接到上次连接的设备。

### `void disconnect()`
断开当前连接。

### `void setSinkCallback(void (*sinkCallback)(const uint8_t *data, uint32_t len))`
设置音频数据回调函数。
- `sinkCallback`: 指向回调函数的指针。

### `void I2S(int bck, int dout, int ws)`
配置I2S引脚。
- `bck`: Bit Clock (BCLK)
- `dout`: Data Out (DOUT)
- `ws`: Word Select (LRC)

### `void play()`
播放。

### `void pause()`
暂停。

### `void next()`
下一曲。

### `void previous()`
上一曲。

### `void updateMeta()`
请求更新歌曲元数据。

### `void volume(float vol)`
设置音量。
- `vol`: 音量值 (0.0 到 1.0)。

### `float getVolume()`
获取当前音量。

### 静态变量
- `String title`: 歌曲标题
- `String artist`: 歌手
- `String album`: 专辑
- `String genre`: 流派
- `bool isConnected`: 连接状态


## 贡献
欢迎通过Pull Request或Issues来贡献您的代码和建议！

## 许可证
该项目基于 MIT 许可证。