# ESP32-CAM 프로젝트 설명서

이 프로젝트는 AI-Thinker ESP32-CAM 모듈을 위한 두 가지 애플리케이션을 포함합니다.

---

## 하드웨어 요구사항

- **보드**: AI-Thinker ESP32-CAM (또는 ESP32-CAM-MB)
- **카메라 센서**: OV2640
- **PSRAM**: 4MB (고해상도 지원)

### ESP32-CAM-MB 사용 시

| 버튼 | 기능 |
|------|------|
| BOOT | 업로드 모드 진입 (누른 상태로 RST) |
| RST | 리셋 |

---

## 1. CameraTest (카메라 테스트)

**파일 위치**: `CameraTest/CameraTest.ino`

### 기능 설명

카메라 하드웨어가 정상 작동하는지 확인하는 테스트 프로그램입니다.

- 카메라 초기화 및 상태 확인
- PSRAM 감지 및 해상도 자동 설정
- 2초 간격으로 JPEG 이미지 캡처
- 캡처 정보를 시리얼 모니터로 출력
- 10회 캡처마다 플래시 LED 테스트

### 시리얼 출력 예시

```
========================================
   ESP32-CAM Camera Test
========================================
[INFO] PSRAM found - High resolution enabled
[INFO] Initializing camera...
[OK] Camera initialized successfully!
[INFO] Camera PID: 0x26
[OK] Camera sensor configured
----------------------------------------
Camera ready! Starting capture test...
----------------------------------------
[CAPTURE #1] Size: 67780 bytes, Resolution: 1600x1200, Format: JPEG
[CAPTURE #2] Size: 281754 bytes, Resolution: 1600x1200, Format: JPEG
...
[CAPTURE #10] Size: 85407 bytes, Resolution: 1600x1200, Format: JPEG
[TEST] Flash LED ON
[TEST] Flash LED OFF
```

### 해상도 설정

| PSRAM | 해상도 | 프레임 크기 |
|-------|--------|-------------|
| 있음 | 1600x1200 | UXGA |
| 없음 | 800x600 | SVGA |

### 사용 방법

1. Arduino IDE에서 `CameraTest.ino` 열기
2. 보드 선택: **AI Thinker ESP32-CAM**
3. 업로드 (BOOT + RST 사용)
4. 시리얼 모니터 열기 (115200 baud)
5. RST 버튼으로 리셋

---

## 2. Web Streaming Server (웹 스트리밍 서버)

**파일 위치**: `src/main.ino`

### 기능 설명

WiFi를 통해 웹 브라우저에서 실시간 카메라 영상을 볼 수 있는 스트리밍 서버입니다.

- MJPEG 실시간 스트리밍
- 단일 이미지 캡처
- 플래시 LED 원격 제어
- 반응형 웹 UI 제공
- WiFi 자동 재연결

### WiFi 설정 (필수)

업로드 전 코드에서 WiFi 정보를 수정하세요:

```cpp
const char* ssid = "YOUR_WIFI_SSID";      // WiFi 이름
const char* password = "YOUR_WIFI_PASSWORD"; // WiFi 비밀번호
```

### 웹 서버 엔드포인트

| URL | 기능 |
|-----|------|
| `http://IP주소/` | 메인 페이지 (스트리밍 UI) |
| `http://IP주소/stream` | MJPEG 스트림 |
| `http://IP주소/capture` | 단일 이미지 캡처 (JPEG) |
| `http://IP주소/flash` | 플래시 LED 토글 |

### 시리얼 출력 예시

```
PSRAM 발견 - 고해상도 모드
카메라 초기화 완료
WiFi 연결 중.....
WiFi 연결됨!
IP 주소: 192.168.1.100
카메라 서버 시작 (포트: 80)
스트림 서버 시작 (포트: 81)
========================================
ESP32-CAM 웹 서버 시작!
웹 브라우저에서 접속: http://192.168.1.100
========================================
```

### 사용 방법

1. 코드에서 WiFi SSID/비밀번호 수정
2. Arduino IDE에서 업로드
3. 시리얼 모니터에서 IP 주소 확인
4. 웹 브라우저에서 해당 IP 주소 접속
5. 실시간 스트리밍 확인

### 웹 UI 기능

- **실시간 스트리밍**: 메인 화면에 카메라 영상 표시
- **사진 캡처**: 버튼 클릭 시 새 탭에서 JPEG 이미지 열기
- **플래시 토글**: 플래시 LED On/Off 제어

---

## 업로드 방법 (ESP32-CAM-MB)

### Arduino IDE 설정

1. **보드 매니저 URL 추가**:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```

2. **ESP32 보드 패키지 설치**: Boards Manager → "esp32" 검색 → 설치

3. **보드 선택**: Tools → Board → AI Thinker ESP32-CAM

4. **포트 선택**: Tools → Port → COM 포트

### 업로드 순서

1. **BOOT 버튼 누른 상태** 유지
2. **RST 버튼** 눌렀다 떼기
3. BOOT 버튼 떼기
4. **Upload** 버튼 클릭
5. 업로드 완료 후 **RST 버튼** 눌러 실행

---

## 시리얼 모니터 설정

| 항목 | 값 |
|------|-----|
| Baud Rate | 115200 |
| Data Bits | 8 |
| Stop Bits | 1 |
| Parity | None |
| Flow Control | None |

---

## 문제 해결

### boot:0x12 (다운로드 모드로 부팅)
- BOOT 버튼이 눌린 상태인지 확인
- RST 버튼만 눌러서 재시작

### boot:0x13 (정상 부팅)
- 정상 상태입니다

### 카메라 초기화 실패
- 카메라 연결 상태 확인
- 전원 공급 확인 (5V 권장)

### WiFi 연결 안됨
- SSID/비밀번호 확인
- 2.4GHz WiFi인지 확인 (5GHz 미지원)

---

## 핀 배치 (AI-Thinker ESP32-CAM)

| 기능 | GPIO |
|------|------|
| PWDN | 32 |
| XCLK | 0 |
| SIOD (SDA) | 26 |
| SIOC (SCL) | 27 |
| Y9 | 35 |
| Y8 | 34 |
| Y7 | 39 |
| Y6 | 36 |
| Y5 | 21 |
| Y4 | 19 |
| Y3 | 18 |
| Y2 | 5 |
| VSYNC | 25 |
| HREF | 23 |
| PCLK | 22 |
| Flash LED | 4 |

---

## 유틸리티 (util)

### serial_monitor.py

**파일 위치**: `util/serial_monitor.py`

Raspberry Pi에서 ESP32-CAM-MB 보드의 시리얼 출력을 읽기 위한 Python 스크립트입니다.

> **참고**: ESP32-CAM-MB 보드는 시리얼 연결 시 DTR/RTS 신호로 인해 자동 리셋됩니다.
> 이 스크립트는 DTR/RTS를 비활성화하여 이 문제를 해결합니다.

#### 설치 요구사항

```bash
pip3 install pyserial
```

#### 사용법

```bash
# 기본 실행 (/dev/ttyUSB0, 115200 baud)
python3 util/serial_monitor.py

# 포트 지정
python3 util/serial_monitor.py /dev/ttyUSB0

# 포트 + baud rate 지정
python3 util/serial_monitor.py /dev/ttyUSB0 115200

# 도움말
python3 util/serial_monitor.py --help
```

#### 실행 예시

```
==================================================
ESP32-CAM Serial Monitor
==================================================
포트: /dev/ttyUSB0
Baud Rate: 115200
DTR/RTS: 비활성화 (ESP32-CAM-MB 호환)
--------------------------------------------------
종료: Ctrl+C
==================================================

[연결됨] /dev/ttyUSB0

========================================
   ESP32-CAM Camera Test by hong
========================================
[INFO] PSRAM found - High resolution enabled
[OK] Camera initialized successfully!
[CAPTURE #1] Size: 75674 bytes, Resolution: 1600x1200, Format: JPEG
```

#### 종료

`Ctrl+C`를 눌러 종료합니다.

---

## 프로젝트 구조

```
esp32Camera/
├── README.md                 # 프로젝트 설명서
├── CameraTest/
│   └── CameraTest.ino        # 카메라 테스트 프로그램
├── src/
│   └── main.ino              # 웹 스트리밍 서버
├── util/
│   └── serial_monitor.py     # 시리얼 모니터 (Raspberry Pi용)
└── 작업보고서/
    └── 2026-01-07_작업보고서.md
```

---

## 라이선스

MIT License

## 참고 자료

- [ESP32-CAM 공식 문서](https://docs.espressif.com/)
- [Arduino-ESP32 GitHub](https://github.com/espressif/arduino-esp32)
