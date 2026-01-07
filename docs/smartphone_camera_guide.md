# Samsung 스마트폰 카메라를 Raspberry Pi에서 사용하는 방법

이 문서는 Samsung Android 스마트폰의 카메라를 Raspberry Pi에서 영상 처리(자동차 번호판 인식 등)에 활용하는 방법을 설명합니다.

---

## 목차

1. [방법 비교](#방법-비교)
2. [방법 1: IP Webcam (WiFi)](#방법-1-ip-webcam-wifi)
3. [방법 2: DroidCam (WiFi/USB)](#방법-2-droidcam-wifiusb)
4. [방법 3: USB + scrcpy](#방법-3-usb--scrcpy)
5. [방법 4: USB + ADB](#방법-4-usb--adb)
6. [Python/OpenCV 연동](#pythonocv-연동)
7. [번호판 인식 시스템 구성](#번호판-인식-시스템-구성)
8. [권장 방법](#권장-방법)

---

## 방법 비교

| 방법 | 연결 | 지연시간 | 해상도 | 설치 난이도 | 안정성 |
|------|------|----------|--------|-------------|--------|
| IP Webcam (WiFi) | 무선 | 중간 (100-300ms) | 최대 1080p | 쉬움 | 좋음 |
| DroidCam (USB) | 유선 | 낮음 (50-100ms) | 최대 1080p | 중간 | 매우 좋음 |
| scrcpy (USB) | 유선 | 매우 낮음 | 최대 4K | 중간 | 좋음 |
| ADB 직접 (USB) | 유선 | 낮음 | 다양 | 어려움 | 중간 |

**번호판 인식 추천:** USB 연결 (낮은 지연시간이 중요)

---

## 방법 1: IP Webcam (WiFi)

### 장점
- 설치가 가장 간단
- 무선 연결
- MJPEG/RTSP 스트림 지원

### 단점
- WiFi 상태에 따라 지연 발생
- 배터리 소모

### 설치 및 설정

#### 1단계: 스마트폰 앱 설치
1. Google Play Store에서 **"IP Webcam"** 검색
2. "Pavel Khlebovich" 개발자의 앱 설치 (무료)

#### 2단계: 앱 설정
1. IP Webcam 앱 실행
2. 설정 조정:
   - **Video preferences** → **Video resolution**: 1280x720 또는 1920x1080
   - **Video preferences** → **Quality**: 80-90%
   - **Video preferences** → **FPS limit**: 30
   - **Power management** → **Keep screen on during streaming**: On
   - **Audio mode**: Disabled (영상만 필요시)
3. 하단으로 스크롤하여 **"Start server"** 클릭
4. 화면에 표시된 IP 주소 확인 (예: `http://192.168.0.50:8080`)

#### 3단계: Raspberry Pi에서 접속 테스트

```bash
# 브라우저로 확인
firefox http://192.168.0.50:8080

# VLC로 스트림 확인
vlc http://192.168.0.50:8080/video
```

#### 스트림 URL 목록

| URL | 설명 |
|-----|------|
| `http://IP:8080/` | 웹 인터페이스 |
| `http://IP:8080/video` | MJPEG 스트림 |
| `http://IP:8080/videofeed` | MJPEG 피드 |
| `http://IP:8080/shot.jpg` | 단일 이미지 캡처 |
| `http://IP:8080/photo.jpg` | 고해상도 사진 |

#### Python에서 사용

```python
import cv2

# IP Webcam 스트림 연결
cap = cv2.VideoCapture('http://192.168.0.50:8080/video')

while True:
    ret, frame = cap.read()
    if not ret:
        break

    cv2.imshow('IP Webcam', frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
```

---

## 방법 2: DroidCam (WiFi/USB)

### 장점
- WiFi와 USB 모두 지원
- USB 연결 시 낮은 지연시간
- V4L2 디바이스로 인식 (웹캠처럼 사용)

### 단점
- Raspberry Pi에 클라이언트 설치 필요
- ARM 지원이 제한적일 수 있음

### 설치 및 설정

#### 1단계: 스마트폰 앱 설치
1. Play Store에서 **"DroidCam"** 설치

#### 2단계: Raspberry Pi 클라이언트 설치

```bash
# 의존성 설치
sudo apt update
sudo apt install -y libavutil-dev libswscale-dev libjpeg-dev

# DroidCam 클라이언트 다운로드 및 설치
cd /tmp
wget https://files.dev47apps.net/linux/droidcam_1.8.2.zip
unzip droidcam_1.8.2.zip -d droidcam
cd droidcam

# 설치 (ARM용 수동 빌드 필요할 수 있음)
sudo ./install-client
```

#### 3단계: USB 연결 사용

```bash
# ADB 설치
sudo apt install android-tools-adb

# 스마트폰 USB 연결 후
adb devices

# DroidCam USB 모드로 연결
droidcam-cli -v adb 4747
```

#### 4단계: V4L2 디바이스 확인

```bash
# 비디오 디바이스 확인
ls -la /dev/video*

# 보통 /dev/video0 또는 /dev/video1로 인식됨
```

---

## 방법 3: USB + scrcpy

### 장점
- 가장 낮은 지연시간
- 고해상도 지원 (4K)
- 화면 미러링 + 제어 가능

### 단점
- 화면 전체를 미러링 (카메라 앱 실행 필요)
- 추가 처리 필요

### 설치 및 설정

#### 1단계: scrcpy 설치

```bash
# Raspberry Pi에 설치
sudo apt update
sudo apt install -y scrcpy adb
```

#### 2단계: 스마트폰 설정
1. **설정** → **개발자 옵션** 활성화
   - 설정 → 휴대전화 정보 → 소프트웨어 정보 → 빌드번호 7회 탭
2. **개발자 옵션** → **USB 디버깅** 활성화
3. USB 케이블로 Raspberry Pi에 연결
4. "USB 디버깅 허용" 팝업에서 **허용**

#### 3단계: 연결 테스트

```bash
# 연결된 장치 확인
adb devices

# scrcpy 실행 (화면 미러링)
scrcpy

# 비디오만 (소리 없음, 제어 없음)
scrcpy --no-audio --no-control

# 해상도 제한 (성능 향상)
scrcpy --max-size 1280 --no-audio
```

#### 4단계: scrcpy 출력을 가상 카메라로 변환

```bash
# v4l2loopback 설치
sudo apt install v4l2loopback-dkms

# 가상 카메라 디바이스 생성
sudo modprobe v4l2loopback video_nr=10 card_label="scrcpy"

# scrcpy 출력을 가상 카메라로 전송
scrcpy --v4l2-sink=/dev/video10 --no-display
```

#### Python에서 사용

```python
import cv2

# 가상 카메라 디바이스 열기
cap = cv2.VideoCapture('/dev/video10')

while True:
    ret, frame = cap.read()
    if ret:
        cv2.imshow('Phone Camera', frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
```

---

## 방법 4: USB + ADB

### 장점
- 직접 제어 가능
- 유연한 설정

### 단점
- 설정이 복잡
- 추가 스크립트 필요

### 설치 및 설정

#### 1단계: ADB 설치

```bash
sudo apt install android-tools-adb
```

#### 2단계: 스마트폰 연결

```bash
# USB 연결 후
adb devices

# 포트 포워딩 (IP Webcam 앱 사용 시)
adb forward tcp:8080 tcp:8080

# 이제 localhost로 접속 가능
curl http://localhost:8080/shot.jpg -o test.jpg
```

---

## Python/OpenCV 연동

### 공통 코드: 카메라 스트림 캡처

```python
#!/usr/bin/env python3
"""
smartphone_capture.py
스마트폰 카메라 스트림 캡처 모듈
"""

import cv2
import numpy as np

class SmartphoneCamera:
    def __init__(self, source, method='ip_webcam'):
        """
        스마트폰 카메라 초기화

        Args:
            source: IP 주소 또는 디바이스 경로
            method: 'ip_webcam', 'droidcam', 'scrcpy', 'v4l2'
        """
        self.method = method

        if method == 'ip_webcam':
            # IP Webcam MJPEG 스트림
            self.url = f'http://{source}:8080/video'
            self.cap = cv2.VideoCapture(self.url)

        elif method == 'v4l2':
            # V4L2 디바이스 (DroidCam, scrcpy 등)
            self.cap = cv2.VideoCapture(source)

        else:
            self.cap = cv2.VideoCapture(source)

        if not self.cap.isOpened():
            raise Exception(f"카메라 연결 실패: {source}")

        print(f"카메라 연결 성공: {source}")

    def read(self):
        """프레임 읽기"""
        ret, frame = self.cap.read()
        return ret, frame

    def release(self):
        """자원 해제"""
        self.cap.release()

    def get_frame_size(self):
        """프레임 크기 반환"""
        width = int(self.cap.get(cv2.CAP_PROP_FRAME_WIDTH))
        height = int(self.cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
        return width, height


# 사용 예시
if __name__ == '__main__':
    # IP Webcam 사용
    camera = SmartphoneCamera('192.168.0.50', method='ip_webcam')

    # 또는 V4L2 디바이스 사용
    # camera = SmartphoneCamera('/dev/video10', method='v4l2')

    print(f"해상도: {camera.get_frame_size()}")

    while True:
        ret, frame = camera.read()
        if not ret:
            print("프레임 읽기 실패")
            break

        cv2.imshow('Smartphone Camera', frame)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    camera.release()
    cv2.destroyAllWindows()
```

---

## 번호판 인식 시스템 구성

### 시스템 아키텍처

```
[스마트폰 카메라]
       ↓ (WiFi 또는 USB)
[Raspberry Pi]
       ↓
[영상 전처리] → [번호판 검출] → [OCR] → [결과 출력]
```

### 필요 패키지 설치

```bash
# OpenCV 설치
sudo apt install python3-opencv

# 또는 pip으로 설치
pip3 install opencv-python

# 번호판 인식용 추가 패키지
pip3 install pytesseract  # OCR
pip3 install easyocr      # 딥러닝 기반 OCR (더 정확)
pip3 install ultralytics  # YOLOv8 (번호판 검출)

# Tesseract OCR 엔진
sudo apt install tesseract-ocr tesseract-ocr-kor
```

### 간단한 번호판 인식 예시

```python
#!/usr/bin/env python3
"""
license_plate_recognition.py
스마트폰 카메라를 이용한 번호판 인식 예시
"""

import cv2
import numpy as np

# EasyOCR 사용 (설치: pip3 install easyocr)
try:
    import easyocr
    reader = easyocr.Reader(['ko', 'en'])
    USE_EASYOCR = True
except ImportError:
    USE_EASYOCR = False
    print("EasyOCR 미설치. pip3 install easyocr로 설치하세요.")

def preprocess_plate(image):
    """번호판 이미지 전처리"""
    # 그레이스케일 변환
    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)

    # 노이즈 제거
    blur = cv2.GaussianBlur(gray, (5, 5), 0)

    # 이진화
    _, thresh = cv2.threshold(blur, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)

    return thresh

def detect_plate_region(frame):
    """번호판 영역 검출 (간단한 윤곽선 기반)"""
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    blur = cv2.GaussianBlur(gray, (5, 5), 0)
    edges = cv2.Canny(blur, 100, 200)

    contours, _ = cv2.findContours(edges, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

    plates = []
    for contour in contours:
        x, y, w, h = cv2.boundingRect(contour)
        aspect_ratio = w / h

        # 번호판 비율 필터 (한국 번호판: 약 2:1 ~ 4.5:1)
        if 2.0 < aspect_ratio < 5.0 and w > 100 and h > 30:
            plates.append((x, y, w, h))

    return plates

def recognize_plate(plate_image):
    """번호판 문자 인식"""
    if USE_EASYOCR:
        results = reader.readtext(plate_image)
        text = ' '.join([result[1] for result in results])
        return text
    else:
        return "OCR 미설치"

def main():
    # 스마트폰 카메라 연결 (IP Webcam)
    phone_ip = "192.168.0.50"  # 스마트폰 IP 주소로 변경
    cap = cv2.VideoCapture(f'http://{phone_ip}:8080/video')

    if not cap.isOpened():
        print("카메라 연결 실패!")
        return

    print("번호판 인식 시작... (q: 종료)")

    while True:
        ret, frame = cap.read()
        if not ret:
            continue

        # 번호판 영역 검출
        plates = detect_plate_region(frame)

        for (x, y, w, h) in plates:
            # 번호판 영역 표시
            cv2.rectangle(frame, (x, y), (x+w, y+h), (0, 255, 0), 2)

            # 번호판 이미지 추출
            plate_img = frame[y:y+h, x:x+w]

            # 문자 인식
            text = recognize_plate(plate_img)

            # 결과 표시
            cv2.putText(frame, text, (x, y-10),
                       cv2.FONT_HERSHEY_SIMPLEX, 0.9, (0, 255, 0), 2)
            print(f"인식된 번호판: {text}")

        cv2.imshow('License Plate Recognition', frame)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()

if __name__ == '__main__':
    main()
```

### YOLOv8을 이용한 고급 번호판 검출

```python
#!/usr/bin/env python3
"""
yolo_plate_detection.py
YOLOv8을 이용한 번호판 검출
"""

from ultralytics import YOLO
import cv2

# 사전 훈련된 모델 로드 (또는 커스텀 모델)
model = YOLO('yolov8n.pt')  # 기본 모델

def detect_with_yolo(frame):
    """YOLO로 객체 검출"""
    results = model(frame)
    return results

# 번호판 전용 모델은 별도 훈련 또는 다운로드 필요
# 예: https://github.com/Muhammad-Zeerak-Khan/License-Plate-Detection-using-YOLOv8
```

---

## 권장 방법

### 번호판 인식 용도로 추천 순위

| 순위 | 방법 | 이유 |
|------|------|------|
| 1 | **USB + scrcpy + v4l2loopback** | 가장 낮은 지연시간, 고해상도 |
| 2 | **IP Webcam (WiFi)** | 설치 간편, 무선 자유도 |
| 3 | **DroidCam (USB)** | V4L2 직접 지원 |

### 최종 권장 설정

```bash
# 1. 필요 패키지 설치
sudo apt update
sudo apt install -y scrcpy adb v4l2loopback-dkms python3-opencv

# 2. 가상 카메라 모듈 로드
sudo modprobe v4l2loopback video_nr=10 card_label="PhoneCamera"

# 3. 스마트폰 USB 연결 후 scrcpy 실행
scrcpy --v4l2-sink=/dev/video10 --no-display --max-size 1280

# 4. Python에서 /dev/video10 사용
python3 license_plate_recognition.py
```

### 자동 시작 스크립트

```bash
#!/bin/bash
# start_phone_camera.sh

# 가상 카메라 모듈 로드
sudo modprobe v4l2loopback video_nr=10 card_label="PhoneCamera"

# ADB 연결 대기
echo "스마트폰 USB 연결 대기..."
while ! adb devices | grep -q "device$"; do
    sleep 1
done

echo "스마트폰 연결됨!"

# 카메라 앱 실행 (옵션)
adb shell am start -a android.media.action.STILL_IMAGE_CAMERA

# scrcpy 시작
scrcpy --v4l2-sink=/dev/video10 --no-display --max-size 1280 &

echo "카메라 스트림 시작됨: /dev/video10"
```

---

## 문제 해결

### IP Webcam 연결 안됨
- 스마트폰과 Raspberry Pi가 같은 WiFi 네트워크인지 확인
- 방화벽 설정 확인
- 앱에서 "Start server"가 실행 중인지 확인

### scrcpy 연결 실패
```bash
# ADB 서버 재시작
adb kill-server
adb start-server
adb devices
```

### USB 디버깅 활성화 안됨
1. 설정 → 휴대전화 정보 → 소프트웨어 정보
2. "빌드번호" 7회 연속 탭
3. 개발자 옵션 활성화 후 USB 디버깅 켜기

### 영상 지연이 심함
- 해상도를 720p로 낮추기
- WiFi 대신 USB 연결 사용
- IP Webcam에서 FPS 제한 낮추기

---

## 참고 자료

- [IP Webcam 공식 페이지](https://play.google.com/store/apps/details?id=com.pas.webcam)
- [scrcpy GitHub](https://github.com/Genymobile/scrcpy)
- [OpenCV 공식 문서](https://docs.opencv.org/)
- [EasyOCR GitHub](https://github.com/JaidedAI/EasyOCR)
- [YOLOv8 Ultralytics](https://github.com/ultralytics/ultralytics)
