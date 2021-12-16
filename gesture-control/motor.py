from enum import Enum, auto

MOTOR_SERIAL_PORT = "/dev/ttyACM0"


class MotorSelection(Enum):
    STEPPER_MOTOR = auto()
    DC_MOTOR = auto()


class MotorProfile(Enum):
    SPEED = auto()
    POSITION = auto()


class MotorDirection(Enum):
    CLOCKWISE = auto()
    ANTICLOCKWISE = auto()


def stm_stop_command():
    return f"stop\n".encode("ascii")

# Stepper methods
def stm_stepper_speed_command(speed_percentage: int):
    return f"stepperchangespeed {speed_percentage}\n".encode("ascii")


def stm_stepper_change_direction(direction: MotorDirection):
    return f"stepperchangedirection {1 if direction == MotorDirection.ANTICLOCKWISE else 0}\n".encode("ascii")


# DC motor methods
def stm_dc_speed_command(speed_percentage: int):
    return f"dcchangespeed {speed_percentage}\n".encode("ascii")


def stm_dc_change_direction(direction: MotorDirection):
    return f"dcchangedirection {1 if direction == MotorDirection.ANTICLOCKWISE else 0}\n".encode("ascii")


# LCD methods
def stm_lcd_send_string(message: str, line_number: int):
    return f"lcd {line_number} {message}\n".encode("ascii")


def stm_lcd_send_clear():
    return f"lcdclear\n".encode("ascii")


def stm_lcd_display_dc():
    return f"lcddisplaydc\n".encode("ascii")


def stm_lcd_display():
    return f"lcddisplay\n".encode("ascii")
