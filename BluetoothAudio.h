#ifndef __BLUETOOTHAUDIO_H
#define __BLUETOOTHAUDIO_H

#include <Arduino.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_bt_device.h>
#include <esp_gap_bt_api.h>
#include <esp_a2dp_api.h>
#include <esp_avrc_api.h>
#include <driver/i2s.h>
#include <numeric>


class BluetoothAudio {
public:
    BluetoothAudio(const char* devName);

    // 初始化与关闭
    void begin();
    void end();

    // 蓝牙连接控制
    void reconnect();         // 从存储的地址重连
    void disconnect();        // 主动断开连接

    // 音频回调与音频接口
    void setSinkCallback(void (*sinkCallback)(const uint8_t *data, uint32_t len));
    void I2S(int bck, int dout, int ws);

    // 播放控制
    void play();
    void pause();
    void next();
    void previous();

    // 元数据与音量控制
    void updateMeta();
    void volume(float vol);
    float getVolume();

    // 当前播放元信息
    static String title;
    static String artist;
    static String album;
    static String genre;

    static bool isConnected;

private:
    const char* _devName;

    // 静态变量用于回调与配置
    static int32_t _sampleRate;
    static float _vol;
    static uint8_t _address[6];
    static uint8_t _rc_handle;

    // I2S 数据回调
    static void i2sCallback(const uint8_t *data, uint32_t len);

    // 蓝牙事件回调
    static void a2d_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param);
    static void avrc_callback(esp_avrc_ct_cb_event_t event, esp_avrc_ct_cb_param_t *param);
};

#endif  // __BLUETOOTHAUDIO_H
