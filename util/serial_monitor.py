#!/usr/bin/env python3
"""
ESP32-CAM Serial Monitor
ESP32-CAM-MB 보드용 시리얼 모니터 (DTR/RTS 비활성화)

사용법:
    python3 serial_monitor.py [포트] [baud rate]

예시:
    python3 serial_monitor.py
    python3 serial_monitor.py /dev/ttyUSB0
    python3 serial_monitor.py /dev/ttyUSB0 115200
"""

import serial
import sys
import time

# 기본 설정
DEFAULT_PORT = '/dev/ttyUSB0'
DEFAULT_BAUDRATE = 115200

def list_ports():
    """사용 가능한 시리얼 포트 목록"""
    import glob
    ports = glob.glob('/dev/ttyUSB*') + glob.glob('/dev/ttyACM*')
    return ports

def serial_monitor(port=DEFAULT_PORT, baudrate=DEFAULT_BAUDRATE, timeout=None):
    """
    시리얼 모니터 실행

    Args:
        port: 시리얼 포트 (기본: /dev/ttyUSB0)
        baudrate: 통신 속도 (기본: 115200)
        timeout: 실행 시간(초), None이면 무한 실행
    """
    print(f"=" * 50)
    print(f"ESP32-CAM Serial Monitor")
    print(f"=" * 50)
    print(f"포트: {port}")
    print(f"Baud Rate: {baudrate}")
    print(f"DTR/RTS: 비활성화 (ESP32-CAM-MB 호환)")
    print(f"-" * 50)
    print("종료: Ctrl+C")
    print(f"=" * 50)
    print()

    try:
        # 시리얼 포트 설정 (DTR/RTS 비활성화)
        ser = serial.Serial()
        ser.port = port
        ser.baudrate = baudrate
        ser.bytesize = serial.EIGHTBITS
        ser.parity = serial.PARITY_NONE
        ser.stopbits = serial.STOPBITS_ONE
        ser.timeout = 0.1
        ser.dtr = False  # DTR 비활성화 (자동 리셋 방지)
        ser.rts = False  # RTS 비활성화 (자동 리셋 방지)

        ser.open()
        print(f"[연결됨] {port}\n")

        start_time = time.time()

        while True:
            # 타임아웃 체크
            if timeout and (time.time() - start_time) > timeout:
                print(f"\n[타임아웃] {timeout}초 경과")
                break

            # 데이터 읽기
            if ser.in_waiting:
                data = ser.read(ser.in_waiting)
                try:
                    text = data.decode('utf-8', errors='ignore')
                    print(text, end='', flush=True)
                except:
                    print(data, end='', flush=True)

            time.sleep(0.01)

    except serial.SerialException as e:
        print(f"\n[오류] 시리얼 포트 오류: {e}")
        ports = list_ports()
        if ports:
            print(f"[정보] 사용 가능한 포트: {', '.join(ports)}")
        else:
            print("[정보] 연결된 시리얼 장치가 없습니다.")
        return False

    except KeyboardInterrupt:
        print("\n\n[종료] 사용자에 의해 종료됨")

    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("[연결 해제]")

    return True

def main():
    # 명령줄 인자 처리
    port = DEFAULT_PORT
    baudrate = DEFAULT_BAUDRATE

    if len(sys.argv) >= 2:
        if sys.argv[1] in ['-h', '--help']:
            print(__doc__)
            print(f"사용 가능한 포트: {list_ports()}")
            return
        port = sys.argv[1]

    if len(sys.argv) >= 3:
        try:
            baudrate = int(sys.argv[2])
        except ValueError:
            print(f"[오류] 잘못된 baud rate: {sys.argv[2]}")
            return

    serial_monitor(port, baudrate)

if __name__ == '__main__':
    main()
