# ESP32-CAM + ESP32-CAM-MB 카메라 동작 확인 가이드

## 하드웨어 정보

- **모듈**: ESP32-CAM (AI-Thinker)
- **프로그래머**: ESP32-CAM-MB (CH340 USB-to-Serial)
- **연결 포트**: `/dev/ttyUSB0`

---

## 1. 개발 환경 설정

### 1.1 Arduino IDE 설치 (Raspberry Pi)

```bash
# Arduino IDE 설치
sudo apt update
sudo apt install -y arduino

# 또는 최신 버전 다운로드
# https://www.arduino.cc/en/software (Linux ARM 32-bit)
```

### 1.2 ESP32 보드 매니저 추가

1. Arduino IDE 실행
2. **File > Preferences** 이동
3. **Additional Board Manager URLs**에 추가:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. **Tools > Board > Boards Manager** 이동
5. "esp32" 검색 후 **ESP32 by Espressif Systems** 설치

### 1.3 보드 설정

| 설정 항목 | 값 |
|----------|-----|
| Board | AI Thinker ESP32-CAM |
| Upload Speed | 115200 |
| CPU Frequency | 240MHz |
| Flash Frequency | 80MHz |
| Flash Mode | QIO |
| Partition Scheme | Huge APP (3MB No OTA) |
| Port | /dev/ttyUSB0 |

---

## 2. 카메라 테스트 코드 (CameraWebServer)

### 2.1 예제 열기

1. **File > Examples > ESP32 > Camera > CameraWebServer** 선택

### 2.2 카메라 모델 설정

`CameraWebServer.ino` 파일 상단에서:

```cpp
// 다른 카메라 모델은 주석 처리하고 아래만 활성화
#define CAMERA_MODEL_AI_THINKER

#include "camera_pins.h"
```

### 2.3 WiFi 설정

```cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
```

---

## 3. 업로드 방법

### 3.1 ESP32-CAM-MB 사용 시 (자동 리셋)

ESP32-CAM-MB 보드는 자동으로 프로그래밍 모드로 진입합니다.

```bash
# 포트 권한 확인
sudo chmod 666 /dev/ttyUSB0

# 또는 dialout 그룹에 사용자 추가
sudo usermod -aG dialout $USER
```

Arduino IDE에서 **Upload** 버튼 클릭

### 3.2 업로드 실패 시

1. ESP32-CAM-MB의 **IO0** 버튼을 누른 상태에서
2. **RST** 버튼을 눌렀다 떼기
3. **IO0** 버튼 떼기
4. 다시 업로드 시도

---

## 4. 동작 확인

### 4.1 시리얼 모니터 확인

1. **Tools > Serial Monitor** 열기
2. Baud rate: **115200** 설정
3. RST 버튼 누르기
4. 출력 확인:

```
WiFi connected
Camera Ready! Use 'http://192.168.x.x' to connect
```

### 4.2 웹 브라우저 접속

1. 시리얼 모니터에 표시된 IP 주소로 접속
2. 웹 인터페이스에서 **Start Stream** 클릭
3. 카메라 영상 확인

---

## 5. 커맨드라인 테스트 (esptool)

### 5.1 esptool 설치

```bash
pip3 install esptool
```

### 5.2 ESP32 연결 확인

```bash
esptool.py --port /dev/ttyUSB0 chip_id
```

예상 출력:
```
Chip is ESP32-D0WDQ6 (revision 1)
Features: WiFi, BT, Dual Core, 240MHz, VRef calibration in efuse
```

### 5.3 플래시 정보 확인

```bash
esptool.py --port /dev/ttyUSB0 flash_id
```

---

## 6. 간단한 카메라 테스트 코드

직접 업로드할 수 있는 최소 테스트 코드:

```cpp
#include "esp_camera.h"

// AI Thinker ESP32-CAM 핀 설정
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

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32-CAM 카메라 테스트");

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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // 카메라 초기화
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("카메라 초기화 실패: 0x%x\n", err);
    return;
  }
  Serial.println("카메라 초기화 성공!");
}

void loop() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("프레임 캡처 실패");
    delay(1000);
    return;
  }

  Serial.printf("캡처 성공! 크기: %d bytes, 해상도: %dx%d\n",
                fb->len, fb->width, fb->height);

  esp_camera_fb_return(fb);
  delay(2000);
}
```

---

## 7. 문제 해결

| 증상 | 해결 방법 |
|------|----------|
| 업로드 실패 | IO0 + RST 버튼으로 수동 부팅 모드 진입 |
| 카메라 초기화 실패 | 전원 공급 확인 (5V 안정적인 전원 필요) |
| Brownout detector triggered | USB 허브 대신 직접 연결, 또는 외부 5V 전원 사용 |
| WiFi 연결 안됨 | SSID/비밀번호 확인, 2.4GHz 네트워크만 지원 |
| 이미지 품질 저하 | 카메라 렌즈 초점 조절 (렌즈 회전) |

---

## 8. 참고 자료

- [ESP32-CAM 공식 문서](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/camera.html)
- [Arduino-ESP32 GitHub](https://github.com/espressif/arduino-esp32)
- [AI-Thinker ESP32-CAM 스펙](https://docs.ai-thinker.com/esp32-cam)
