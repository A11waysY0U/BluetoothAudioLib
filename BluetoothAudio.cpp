#include "BluetoothAudio.h"
#include <Preferences.h>

int32_t BluetoothAudio::_sampleRate = 44100;
float BluetoothAudio::_vol = 0.95f;
uint8_t BluetoothAudio::_address[6] = {0};
uint8_t BluetoothAudio::_rc_handle = 1;
String BluetoothAudio::title = "";
String BluetoothAudio::artist = "";
String BluetoothAudio::album = "";
String BluetoothAudio::genre = "";
bool BluetoothAudio::isConnected = false;

Preferences preferences;

BluetoothAudio::BluetoothAudio(const char* devName) : _devName(devName) {}

void BluetoothAudio::begin() {
    
    btStart();
    esp_bluedroid_init();
    esp_bluedroid_enable();
    esp_bt_dev_set_device_name(_devName);

    esp_avrc_ct_init();  // 先初始化 AVRCP 控制器
    esp_avrc_ct_register_callback(avrc_callback);

    esp_a2d_register_callback(a2d_cb);  // 再初始化 A2DP
    esp_a2d_sink_init();

#if ESP_IDF_VERSION_MAJOR > 3
    esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
#else
    esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);
#endif
}

void BluetoothAudio::end() {
    esp_a2d_sink_deinit();
    esp_bluedroid_disable();
    esp_bluedroid_deinit();
    btStop();
}

void BluetoothAudio::reconnect() {
    preferences.begin("btAudio", false);
    for (int i = 0; i < 6; i++) {
        _address[i] = preferences.getUChar(String("btaddr" + String(i)).c_str(), 0);
    }
    preferences.end();
    if (std::accumulate(_address, _address + 6, 0) != 0) {
        esp_a2d_sink_connect(_address);
    }
}

void BluetoothAudio::disconnect() {
    esp_a2d_sink_disconnect(_address);
}

void BluetoothAudio::setSinkCallback(void (*sinkCallback)(const uint8_t *data, uint32_t len)) {
    esp_a2d_sink_register_data_callback(sinkCallback);
}

void BluetoothAudio::I2S(int bck, int dout, int ws) {
    i2s_config_t i2s_config = {
        .mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = static_cast<uint32_t>(_sampleRate),
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
#if ESP_IDF_VERSION_MAJOR > 3
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
#else
        .communication_format = static_cast<i2s_comm_format_t>(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
#endif
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2 | ESP_INTR_FLAG_IRAM,
        .dma_buf_count = 3,
        .dma_buf_len = 300,
        .use_apll = true,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .mck_io_num = -1,
        .bck_io_num = bck,
        .ws_io_num = ws,
        .data_out_num = dout,
        .data_in_num = -1
    };

    if (i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL) != ESP_OK) {
        Serial.println("I2S driver install failed");
    }
    i2s_set_pin(I2S_NUM_0, &pin_config);
    esp_a2d_sink_register_data_callback(i2sCallback);
}

void BluetoothAudio::i2sCallback(const uint8_t *data, uint32_t len) {
    size_t i2s_bytes_write = 0;
    int16_t *in = (int16_t *)data;
    int16_t *buffer = (int16_t *)malloc(len);
    if (!buffer) return;

    for (uint32_t i = 0; i < len / 2; i++) {
        buffer[i] = (int16_t)(in[i] * _vol);
    }

    i2s_write(I2S_NUM_0, buffer, len, &i2s_bytes_write, portMAX_DELAY);
    free(buffer);
}

void BluetoothAudio::volume(float vol) {
    _vol = constrain(vol, 0, 1);
}

float BluetoothAudio::getVolume() {
    return _vol;
}

void BluetoothAudio::a2d_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t* param) {
    esp_a2d_cb_param_t* a2d = param;
    switch (event) {
        case ESP_A2D_CONNECTION_STATE_EVT: {
            uint8_t *temp = a2d->conn_stat.remote_bda;
            if (a2d->conn_stat.state == ESP_A2D_CONNECTION_STATE_CONNECTED) {
                memcpy(_address, temp, 6);
                preferences.begin("btAudio", false);
                for (int i = 0; i < 6; i++) {
                    preferences.putUChar(String("btaddr" + String(i)).c_str(), _address[i]);
                }
                preferences.end();
                isConnected = true;
                Serial.println("Bluetooth connected");
            } else if (a2d->conn_stat.state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
                Serial.println("Bluetooth disconnected");
            }
            break;
        }

        case ESP_A2D_AUDIO_CFG_EVT: {
            if (a2d->audio_cfg.mcc.type == ESP_A2D_MCT_SBC) {
                _sampleRate = 16000;
                char oct0 = a2d->audio_cfg.mcc.cie.sbc[0];
                if (oct0 & (1 << 6)) _sampleRate = 32000;
                else if (oct0 & (1 << 5)) _sampleRate = 44100;
                else if (oct0 & (1 << 4)) _sampleRate = 48000;

                i2s_set_sample_rates(I2S_NUM_0, _sampleRate);
            }
            break;
        }

        default:
            break;
    }
}

void BluetoothAudio::updateMeta() {
    uint8_t attr_mask = ESP_AVRC_MD_ATTR_TITLE | ESP_AVRC_MD_ATTR_ARTIST | ESP_AVRC_MD_ATTR_ALBUM | ESP_AVRC_MD_ATTR_GENRE;
    esp_avrc_ct_send_metadata_cmd(_rc_handle, attr_mask);
}

void BluetoothAudio::avrc_callback(esp_avrc_ct_cb_event_t event, esp_avrc_ct_cb_param_t *param) {
    esp_avrc_ct_cb_param_t *rc = param;
    char *attr_text;
    String mystr;

    switch (event) {
        case ESP_AVRC_CT_CONNECTION_STATE_EVT:
            Serial.printf("AVRCP connection state: %s\n", rc->conn_stat.connected ? "CONNECTED" : "DISCONNECTED");
            if (rc->conn_stat.connected) {
                _rc_handle = 1;  // 必须设置！
                Serial.println("AVRCP connected");
            }
            break;


        case ESP_AVRC_CT_METADATA_RSP_EVT: {
            attr_text = (char *) malloc(rc->meta_rsp.attr_length + 1);
            memcpy(attr_text, rc->meta_rsp.attr_text, rc->meta_rsp.attr_length);
            attr_text[rc->meta_rsp.attr_length] = 0;
            mystr = String(attr_text);
            free(attr_text);

            switch (rc->meta_rsp.attr_id) {
                case ESP_AVRC_MD_ATTR_TITLE: title = mystr; break;
                case ESP_AVRC_MD_ATTR_ARTIST: artist = mystr; break;
                case ESP_AVRC_MD_ATTR_ALBUM: album = mystr; break;
                case ESP_AVRC_MD_ATTR_GENRE: genre = mystr; break;
            }
            break;
        }

        default:
            break;
    }
}

void BluetoothAudio::play() {
    esp_avrc_ct_send_passthrough_cmd(_rc_handle, ESP_AVRC_PT_CMD_PLAY, ESP_AVRC_PT_CMD_STATE_PRESSED);
    esp_avrc_ct_send_passthrough_cmd(_rc_handle, ESP_AVRC_PT_CMD_PLAY, ESP_AVRC_PT_CMD_STATE_RELEASED);
}

void BluetoothAudio::pause() {
    esp_avrc_ct_send_passthrough_cmd(_rc_handle, ESP_AVRC_PT_CMD_PAUSE, ESP_AVRC_PT_CMD_STATE_PRESSED);
    esp_avrc_ct_send_passthrough_cmd(_rc_handle, ESP_AVRC_PT_CMD_PAUSE, ESP_AVRC_PT_CMD_STATE_RELEASED);
}

void BluetoothAudio::next() {
    esp_avrc_ct_send_passthrough_cmd(_rc_handle, ESP_AVRC_PT_CMD_FORWARD, ESP_AVRC_PT_CMD_STATE_PRESSED);
    esp_avrc_ct_send_passthrough_cmd(_rc_handle, ESP_AVRC_PT_CMD_FORWARD, ESP_AVRC_PT_CMD_STATE_RELEASED);
}

void BluetoothAudio::previous() {
    esp_avrc_ct_send_passthrough_cmd(_rc_handle, ESP_AVRC_PT_CMD_BACKWARD, ESP_AVRC_PT_CMD_STATE_PRESSED);
    esp_avrc_ct_send_passthrough_cmd(_rc_handle, ESP_AVRC_PT_CMD_BACKWARD, ESP_AVRC_PT_CMD_STATE_RELEASED);
}
