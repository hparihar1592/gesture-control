#!/usr/bin/env python

import cv2 as cv
from serial.serialutil import SerialException
from utils import CvFpsCalc
from gestures import *
import configargparse
import serial
from motor import MOTOR_SERIAL_PORT, MotorSelection, MotorProfile, MotorDirection, stm_dc_change_direction, stm_dc_speed_command, stm_lcd_send_string, stm_stepper_change_direction, stm_stop_command, stm_stepper_speed_command, stm_lcd_display_dc, stm_lcd_display


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
    dc_speed_percent = 0
    stepper_speed_percent = 0
    delay_counter = 0

    # Debug text
    lower_left_text = ""

    # Set up serial communication stuff
    stm_available = False
    try:
        stm32 = serial.Serial(MOTOR_SERIAL_PORT)
        stm32.baudrate = 115200
        stm_available = True
        stm32.write(b"init\n")
        stm32.write(b"stepperstart\n")
        stm32.write(b"dcstart\n")
    except SerialException:
        print("Error setting up serial com for STM32 board")

    # Default motor profile
    motor_profile = MotorProfile.SPEED
    motor_selection = MotorSelection.STEPPER_MOTOR
    dc_motor_direction = MotorDirection.CLOCKWISE
    stepper_motor_direction = MotorDirection.CLOCKWISE

    while True:
        fps = cv_fps_calc.get()

        # Camera capture
        success, image = cap.read()

        debug_image, gesture_id = gesture_detector.recognize(image, number, mode)
        gesture_buffer.add_gesture(gesture_id)

        if gesture_id == -1:
            pass

        elif gesture_id == 0:
            if motor_profile == MotorProfile.SPEED:
                if motor_selection == MotorSelection.STEPPER_MOTOR and stepper_speed_percent < 100:
                    stepper_speed_percent += 0.25

                elif motor_selection == MotorSelection.DC_MOTOR and dc_speed_percent < 100:
                    dc_speed_percent += 0.25

        elif gesture_id == 1:
            if motor_profile == MotorProfile.SPEED:
                if motor_selection == MotorSelection.STEPPER_MOTOR and stepper_speed_percent > 0:
                    stepper_speed_percent -= 0.25

                elif motor_selection == MotorSelection.DC_MOTOR and dc_speed_percent > 0:
                    dc_speed_percent -= 0.25

        elif gesture_id == 2:
            motor_selection = MotorSelection.STEPPER_MOTOR

        elif gesture_id == 4:
            motor_selection = MotorSelection.DC_MOTOR

        elif gesture_id == 6:
            if motor_selection == MotorSelection.STEPPER_MOTOR:
                stepper_motor_direction = MotorDirection.ANTICLOCKWISE
            elif motor_selection == MotorSelection.DC_MOTOR:
                dc_motor_direction = MotorDirection.ANTICLOCKWISE

        elif gesture_id == 7:
            if motor_selection == MotorSelection.STEPPER_MOTOR:
                stepper_motor_direction = MotorDirection.CLOCKWISE
            elif motor_selection == MotorSelection.DC_MOTOR:
                dc_motor_direction = MotorDirection.CLOCKWISE

        else:
            pass

        # Adjust motor speed
        if stm_available:
            if motor_profile == MotorProfile.SPEED:
                if motor_selection == MotorSelection.STEPPER_MOTOR:
                    stm32.write(stm_stepper_change_direction(stepper_motor_direction))
                elif motor_selection == MotorSelection.DC_MOTOR:
                    stm32.write(stm_dc_change_direction(dc_motor_direction))

                # This order of sending commands is important
                # Stepper command changes ARR register on the stm board
                stm32.write(stm_stepper_speed_command(int(stepper_speed_percent)))
                stm32.write(stm_dc_speed_command(int(dc_speed_percent)))

                if delay_counter > 50:
                    stm32.write(stm_lcd_display())
                    delay_counter = 0
        
        delay_counter += 1

        if motor_profile == MotorProfile.SPEED:
            if motor_selection == MotorSelection.STEPPER_MOTOR:
                lower_left_text = f"Speed: {int(stepper_speed_percent)}%"
            elif motor_selection == MotorSelection.DC_MOTOR:
                lower_left_text = f"Speed: {int(dc_speed_percent)}%"

        debug_image = gesture_detector.draw_info(debug_image, round(fps), mode, number, lower_left_text, motor_selection)

        cv.imshow("Gesture Motor Control", debug_image)

        # Quit?
        key = cv.waitKey(1) & 0xff
        if key == 27:  # ESC
            if stm_available:
                stm32.write(stm_stop_command())
                stm32.close()
            break

    cv.destroyAllWindows()


if __name__ == "__main__":
    main()
