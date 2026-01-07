/*
 * ESP32-CAM Camera Test
 * AI-Thinker ESP32-CAM 모듈용 카메라 테스트 코드
 *
 * 시리얼 모니터에서 카메라 초기화 및 프레임 캡처 결과 확인
 * Baud rate: 115200
 */

#include "esp_camera.h"

// AI-Thinker ESP32-CAM 핀 정의
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

// 내장 LED (플래시)
#define FLASH_LED_PIN     4

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  Serial.println("========================================");
  Serial.println("   ESP32-CAM Camera Test");
  Serial.println("========================================");

  // 플래시 LED 초기화
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);

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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // PSRAM 여부에 따라 프레임 크기 설정
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;  // 1600x1200
    config.jpeg_quality = 10;
    config.fb_count = 2;
    Serial.println("[INFO] PSRAM found - High resolution enabled");
  } else {
    config.frame_size = FRAMESIZE_SVGA;  // 800x600
    config.jpeg_quality = 12;
    config.fb_count = 1;
    Serial.println("[INFO] No PSRAM - Limited resolution");
  }

  // 카메라 초기화
  Serial.println("[INFO] Initializing camera...");
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("[ERROR] Camera init failed with error 0x%x\n", err);
    Serial.println("[ERROR] Check camera connection and power supply");
    return;
  }
  Serial.println("[OK] Camera initialized successfully!");

  // 센서 설정
  sensor_t * s = esp_camera_sensor_get();
  if (s != NULL) {
    Serial.printf("[INFO] Camera PID: 0x%02X\n", s->id.PID);
    // 이미지 품질 조정
    s->set_brightness(s, 0);     // -2 to 2
    s->set_contrast(s, 0);       // -2 to 2
    s->set_saturation(s, 0);     // -2 to 2
    s->set_special_effect(s, 0); // 0=No Effect
    s->set_whitebal(s, 1);       // 0=disable, 1=enable
    s->set_awb_gain(s, 1);       // 0=disable, 1=enable
    s->set_wb_mode(s, 0);        // 0=Auto
    s->set_exposure_ctrl(s, 1);  // 0=disable, 1=enable
    s->set_aec2(s, 0);           // 0=disable, 1=enable
    s->set_gain_ctrl(s, 1);      // 0=disable, 1=enable
    s->set_agc_gain(s, 0);       // 0 to 30
    s->set_gainceiling(s, (gainceiling_t)0);
    s->set_bpc(s, 0);            // 0=disable, 1=enable
    s->set_wpc(s, 1);            // 0=disable, 1=enable
    s->set_raw_gma(s, 1);        // 0=disable, 1=enable
    s->set_lenc(s, 1);           // 0=disable, 1=enable
    s->set_hmirror(s, 0);        // 0=disable, 1=enable
    s->set_vflip(s, 0);          // 0=disable, 1=enable
    s->set_dcw(s, 1);            // 0=disable, 1=enable
    Serial.println("[OK] Camera sensor configured");
  }

  Serial.println("----------------------------------------");
  Serial.println("Camera ready! Starting capture test...");
  Serial.println("----------------------------------------");
}

void loop() {
  static int captureCount = 0;

  // 프레임 캡처
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("[ERROR] Frame capture failed!");
    delay(1000);
    return;
  }

  captureCount++;

  // 캡처 정보 출력
  Serial.printf("[CAPTURE #%d] Size: %d bytes, Resolution: %dx%d, Format: %s\n",
                captureCount,
                fb->len,
                fb->width,
                fb->height,
                (fb->format == PIXFORMAT_JPEG) ? "JPEG" : "RAW");

  // 10번째 캡처마다 플래시 테스트
  if (captureCount % 10 == 0) {
    Serial.println("[TEST] Flash LED ON");
    digitalWrite(FLASH_LED_PIN, HIGH);
    delay(100);
    digitalWrite(FLASH_LED_PIN, LOW);
    Serial.println("[TEST] Flash LED OFF");
  }

  // 프레임 버퍼 반환
  esp_camera_fb_return(fb);

  delay(2000);  // 2초 간격으로 캡처
}
