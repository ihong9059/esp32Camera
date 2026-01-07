/*
 * ESP32-CAM 웹 스트리밍 서버
 *
 * 이 코드는 ESP32-CAM을 사용하여 웹 브라우저에서
 * 실시간 카메라 스트리밍을 제공합니다.
 */

#include "esp_camera.h"
#include <WiFi.h>
#include "esp_http_server.h"

// ===========================================
// WiFi 설정 - 사용자 환경에 맞게 수정하세요
// ===========================================
const char* ssid = "ihong";
const char* password = "hongks@6063";

// ===========================================
// AI-Thinker ESP32-CAM 핀 정의
// ===========================================
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// LED 플래시 핀
#define LED_GPIO_NUM       4

httpd_handle_t stream_httpd = NULL;
httpd_handle_t camera_httpd = NULL;

// MJPEG 스트림 boundary
#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

// 스트림 핸들러
static esp_err_t stream_handler(httpd_req_t *req) {
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t * _jpg_buf = NULL;
    char * part_buf[64];

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if(res != ESP_OK) {
        return res;
    }

    while(true) {
        fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("카메라 캡처 실패");
            res = ESP_FAIL;
        } else {
            if(fb->format != PIXFORMAT_JPEG) {
                bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
                esp_camera_fb_return(fb);
                fb = NULL;
                if(!jpeg_converted) {
                    Serial.println("JPEG 변환 실패");
                    res = ESP_FAIL;
                }
            } else {
                _jpg_buf_len = fb->len;
                _jpg_buf = fb->buf;
            }
        }

        if(res == ESP_OK) {
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }
        if(res == ESP_OK) {
            size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if(res == ESP_OK) {
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }

        if(fb) {
            esp_camera_fb_return(fb);
            fb = NULL;
            _jpg_buf = NULL;
        } else if(_jpg_buf) {
            free(_jpg_buf);
            _jpg_buf = NULL;
        }

        if(res != ESP_OK) {
            break;
        }
    }
    return res;
}

// 단일 이미지 캡처 핸들러
static esp_err_t capture_handler(httpd_req_t *req) {
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;

    fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("카메라 캡처 실패");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");

    res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
    esp_camera_fb_return(fb);
    return res;
}

// 메인 페이지 핸들러
static esp_err_t index_handler(httpd_req_t *req) {
    const char* html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32-CAM 스트리밍</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            background: #1a1a2e;
            color: #eee;
            margin: 0;
            padding: 20px;
        }
        h1 { color: #00d4ff; }
        .container {
            max-width: 800px;
            margin: 0 auto;
        }
        img {
            max-width: 100%;
            border: 3px solid #00d4ff;
            border-radius: 10px;
        }
        .btn {
            background: #00d4ff;
            color: #1a1a2e;
            border: none;
            padding: 15px 30px;
            margin: 10px;
            border-radius: 5px;
            cursor: pointer;
            font-size: 16px;
            font-weight: bold;
        }
        .btn:hover { background: #00a8cc; }
        .status {
            margin: 20px 0;
            padding: 10px;
            background: #16213e;
            border-radius: 5px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>ESP32-CAM 스트리밍</h1>
        <div class="status" id="status">연결됨</div>
        <img id="stream" src="">
        <div>
            <button class="btn" onclick="captureImage()">사진 캡처</button>
            <button class="btn" onclick="toggleFlash()">플래시 토글</button>
        </div>
    </div>
    <script>
        // 스트림 URL을 포트 81로 설정
        window.onload = function() {
            var streamUrl = 'http://' + window.location.hostname + ':81/stream';
            document.getElementById('stream').src = streamUrl;
        }
        function captureImage() {
            window.open('/capture', '_blank');
        }
        function toggleFlash() {
            fetch('/flash').then(r => r.text()).then(t => {
                document.getElementById('status').innerText = t;
            });
        }
    </script>
</body>
</html>
)rawliteral";

    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, html, strlen(html));
}

// 플래시 제어 핸들러
bool flashState = false;
static esp_err_t flash_handler(httpd_req_t *req) {
    flashState = !flashState;
    digitalWrite(LED_GPIO_NUM, flashState ? HIGH : LOW);

    const char* response = flashState ? "플래시 ON" : "플래시 OFF";
    httpd_resp_set_type(req, "text/plain");
    return httpd_resp_send(req, response, strlen(response));
}

// 웹 서버 시작
void startCameraServer() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;

    httpd_uri_t index_uri = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = index_handler,
        .user_ctx  = NULL
    };

    httpd_uri_t capture_uri = {
        .uri       = "/capture",
        .method    = HTTP_GET,
        .handler   = capture_handler,
        .user_ctx  = NULL
    };

    httpd_uri_t stream_uri = {
        .uri       = "/stream",
        .method    = HTTP_GET,
        .handler   = stream_handler,
        .user_ctx  = NULL
    };

    httpd_uri_t flash_uri = {
        .uri       = "/flash",
        .method    = HTTP_GET,
        .handler   = flash_handler,
        .user_ctx  = NULL
    };

    Serial.printf("카메라 서버 시작 (포트: %d)\n", config.server_port);
    if (httpd_start(&camera_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(camera_httpd, &index_uri);
        httpd_register_uri_handler(camera_httpd, &capture_uri);
        httpd_register_uri_handler(camera_httpd, &flash_uri);
    }

    config.server_port += 1;
    config.ctrl_port += 1;
    Serial.printf("스트림 서버 시작 (포트: %d)\n", config.server_port);
    if (httpd_start(&stream_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(stream_httpd, &stream_uri);
    }
}

void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println();

    // LED 플래시 핀 설정
    pinMode(LED_GPIO_NUM, OUTPUT);
    digitalWrite(LED_GPIO_NUM, LOW);

    // 카메라 설정
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_VGA;  // 640x480
    config.jpeg_quality = 12;
    config.fb_count = 2;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_LATEST;

    // PSRAM이 있으면 더 높은 해상도 사용
    if(psramFound()) {
        config.frame_size = FRAMESIZE_UXGA;  // 1600x1200
        config.jpeg_quality = 10;
        config.fb_count = 2;
        Serial.println("PSRAM 발견 - 고해상도 모드");
    } else {
        config.frame_size = FRAMESIZE_SVGA;  // 800x600
        config.jpeg_quality = 12;
        config.fb_count = 1;
        Serial.println("PSRAM 없음 - 저해상도 모드");
    }

    // 카메라 초기화
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("카메라 초기화 실패: 0x%x\n", err);
        return;
    }
    Serial.println("카메라 초기화 완료");

    // 카메라 센서 설정
    sensor_t * s = esp_camera_sensor_get();
    s->set_brightness(s, 0);     // -2 ~ 2
    s->set_contrast(s, 0);       // -2 ~ 2
    s->set_saturation(s, 0);     // -2 ~ 2
    s->set_whitebal(s, 1);       // 0 = disable, 1 = enable
    s->set_awb_gain(s, 1);       // 0 = disable, 1 = enable
    s->set_wb_mode(s, 0);        // 0 ~ 4
    s->set_exposure_ctrl(s, 1);  // 0 = disable, 1 = enable
    s->set_aec2(s, 0);           // 0 = disable, 1 = enable
    s->set_gain_ctrl(s, 1);      // 0 = disable, 1 = enable
    s->set_agc_gain(s, 0);       // 0 ~ 30
    s->set_gainceiling(s, (gainceiling_t)0);  // 0 ~ 6
    s->set_bpc(s, 0);            // 0 = disable, 1 = enable
    s->set_wpc(s, 1);            // 0 = disable, 1 = enable
    s->set_raw_gma(s, 1);        // 0 = disable, 1 = enable
    s->set_lenc(s, 1);           // 0 = disable, 1 = enable
    s->set_hmirror(s, 0);        // 0 = disable, 1 = enable
    s->set_vflip(s, 0);          // 0 = disable, 1 = enable
    s->set_dcw(s, 1);            // 0 = disable, 1 = enable

    // WiFi 연결
    WiFi.begin(ssid, password);
    WiFi.setSleep(false);

    Serial.print("WiFi 연결 중");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.println("WiFi 연결됨!");
    Serial.print("IP 주소: ");
    Serial.println(WiFi.localIP());

    // 웹 서버 시작
    startCameraServer();

    Serial.println("========================================");
    Serial.println("ESP32-CAM 웹 서버 시작!");
    Serial.print("웹 브라우저에서 접속: http://");
    Serial.println(WiFi.localIP());
    Serial.println("========================================");
}

void loop() {
    delay(10000);

    // WiFi 연결 상태 확인 및 재연결
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi 연결 끊김. 재연결 시도...");
        WiFi.disconnect();
        WiFi.begin(ssid, password);

        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            Serial.print(".");
            attempts++;
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nWiFi 재연결 성공!");
            Serial.print("IP 주소: ");
            Serial.println(WiFi.localIP());
        }
    }
}
