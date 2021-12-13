#!/usr/bin/env python

import cv2 as cv
from utils import CvFpsCalc
from gestures import *
import threading
import configargparse
import serial

SERIAL_PORT = "/dev/ttyACM0"


def get_args():
    print("Reading configuration")
    parser = configargparse.ArgParser(default_config_files=["config.txt"])

    parser.add("-c", "--my-config", required=False, is_config_file=True, help="config file path")
    parser.add("--device", type=int)
    parser.add("--width", help='cap width', type=int)
    parser.add("--height", help='cap height', type=int)
    parser.add("--use_static_image_mode", action="store_true", help="True if running on photos")
    parser.add("--min_detection_confidence",
               help="min_detection_confidence",
               type=float)
    parser.add("--min_tracking_confidence",
               help="min_tracking_confidence",
               type=float)
    parser.add("--buffer_len",
               help="Length of gesture buffer",
               type=int)

    args = parser.parse_args()

    return args


def main():
    # init global vars
    global gesture_buffer
    global gesture_id

    args = get_args()

    cap = cv.VideoCapture(args.device)

    gesture_detector = GestureRecognition(args.use_static_image_mode,
                                        args.min_detection_confidence,
                                        args.min_tracking_confidence)
    gesture_buffer = GestureBuffer(buffer_len=args.buffer_len)

    # FPS measurement
    cv_fps_calc = CvFpsCalc(buffer_len=10)

    mode = 0
    number = -1
    battery_status = -1

    # Set up serial communication stuff
    stm32 = serial.Serial(SERIAL_PORT)
    stm32.baudrate = 115200
    stm32.write(b"init\n")

    while True:
        fps = cv_fps_calc.get()

        # Camera capture
        success, image = cap.read()

        debug_image, gesture_id = gesture_detector.recognize(image, number, mode)
        gesture_buffer.add_gesture(gesture_id)

        debug_image = gesture_detector.draw_info(debug_image, round(fps), mode, number)

        cv.imshow("Gesture Motor Control", debug_image)

        # Quit?
        key = cv.waitKey(1) & 0xff
        if key == 27:  # ESC
            stm32.write(b"stop\n")
            break

    cv.destroyAllWindows()


if __name__ == "__main__":
    main()
