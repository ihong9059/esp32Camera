# ESP32 Camera Project

ESP32-CAM 모듈을 활용한 카메라 프로젝트입니다.

## 하드웨어 요구사항

- ESP32-CAM 모듈 (AI-Thinker 또는 호환 보드)
- USB-TTL 변환기 (프로그래밍용)
- 5V 전원 공급 장치

## 주요 기능

- 웹 서버를 통한 실시간 스트리밍
- 사진 캡처 및 저장
- WiFi 연결 설정
- OTA (무선 펌웨어 업데이트) 지원

## 핀 연결 (프로그래밍 시)

| ESP32-CAM | USB-TTL |
|-----------|---------|
| 5V        | 5V      |
| GND       | GND     |
| U0R       | TX      |
| U0T       | RX      |
| IO0       | GND (업로드 모드) |

## 설치 방법

### 1. Arduino IDE 설정

1. Arduino IDE에서 `파일 > 환경설정`으로 이동
2. 추가 보드 관리자 URL에 다음 추가:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. `도구 > 보드 > 보드 매니저`에서 "esp32" 검색 후 설치
4. 보드 선택: `AI Thinker ESP32-CAM`

### 2. 코드 업로드

1. `src/main.ino` 파일에서 WiFi 정보 수정
2. IO0 핀을 GND에 연결
3. RST 버튼을 눌러 업로드 모드 진입
4. 코드 업로드
5. IO0 핀 연결 해제 후 RST 버튼으로 재시작

## 프로젝트 구조

```
esp32Camera/
├── README.md
├── src/
│   └── main.ino          # 메인 소스 코드
├── docs/
│   └── wiring.md         # 배선 가이드
└── examples/
    └── basic_stream.ino  # 기본 스트리밍 예제
```

## 웹 인터페이스 접속

업로드 완료 후 시리얼 모니터에서 IP 주소를 확인하고 웹 브라우저에서 접속합니다.

```
http://<ESP32-IP-주소>
```

## 라이선스

MIT License

## 참고 자료

- [ESP32-CAM 공식 문서](https://docs.espressif.com/)
- [Arduino-ESP32 GitHub](https://github.com/espressif/arduino-esp32)
