#include "Joystick.h"

#define DEBUG 1

#define HANDBRAKE_AXIS_MAX 1023
#define HANDBRAKE_AXIS_TOP_TRESHOLD 1023
#define HANDBRAKE_AXIS_BOTTOM_TRESHOLD 0

#define SHIFTER_AXIS_LEFT 400
#define SHIFTER_AXIS_RIGHT 650
#define SHIFTER_AXIS_TOP 800
#define SHIFTER_AXIS_BOTTOM 200

#define JOYSTICK_BUTTON_GR 6
#define JOYSTICK_BUTTON_G1 0
#define JOYSTICK_BUTTON_G2 1
#define JOYSTICK_BUTTON_G3 2
#define JOYSTICK_BUTTON_G4 3
#define JOYSTICK_BUTTON_G5 4
#define JOYSTICK_BUTTON_G6 5

#define PIN_HANDBRAKE A0
#define PIN_G29_SHIFTER_X A2
#define PIN_G29_SHIFTER_Y A3
#define PIN_G29_DOWN 14

enum Gear {
    FIRST, SECOND, THIRD, FOURTH, FIFTH, SIXTH, REVERSE, NEUTRAL
};

Gear SELECTED_GEAR = Gear::NEUTRAL;

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_JOYSTICK, 7, 0,
        false, false, false,
        true, false, false,
        false, false, false, false, false);

void setup() {
    pinMode(PIN_HANDBRAKE, INPUT_PULLUP);
    pinMode(PIN_G29_SHIFTER_X, INPUT_PULLUP);
    pinMode(PIN_G29_SHIFTER_Y, INPUT_PULLUP);
    pinMode(PIN_G29_DOWN, INPUT);
    Serial.begin(9600);
    Joystick.begin();
    Joystick.setRxAxisRange(0, HANDBRAKE_AXIS_MAX);
}

void loop() {
    int handbrakeValue = processAxisValue(analogRead(PIN_HANDBRAKE));
    Joystick.setRxAxis(handbrakeValue);

    int shifterHorizontalValue = analogRead(PIN_G29_SHIFTER_X);
    int shifterVerticalValue = analogRead(PIN_G29_SHIFTER_Y);
    bool shifterPressedDown = digitalRead(PIN_G29_DOWN);
    Gear gear = !shifterPressedDown ?
        findGear(shifterHorizontalValue, shifterVerticalValue) :
        findGearDown(shifterHorizontalValue, shifterVerticalValue);
    updateShifter(gear);
    if (DEBUG) {
        char buffer[100];
        sprintf(buffer, "Handbrake [ %d ] Shifter [ %d | X %d Y %d PRESSED %x ]",
            handbrakeValue, gear, shifterHorizontalValue, shifterVerticalValue, shifterPressedDown);
        Serial.println(buffer);
    }
}

int processAxisValue(int reading) {
    int cappedReading = constrain(reading, HANDBRAKE_AXIS_BOTTOM_TRESHOLD, HANDBRAKE_AXIS_TOP_TRESHOLD);
    return map(cappedReading, HANDBRAKE_AXIS_BOTTOM_TRESHOLD, HANDBRAKE_AXIS_TOP_TRESHOLD, HANDBRAKE_AXIS_MAX, 0);
}

void setHandbrakeButton(int axisValue) {
    if (axisValue > HANDBRAKE_AXIS_MAX/2) {
        Joystick.pressButton(0);
    } else {
        Joystick.releaseButton(0);
    }
}

void updateShifter(Gear gear) {
    if (SELECTED_GEAR != gear) {
        Joystick.releaseButton(SELECTED_GEAR);
    }
    SELECTED_GEAR = gear;
    
    switch (gear) {
    case Gear::FIRST:
        Joystick.pressButton(JOYSTICK_BUTTON_G1);
        break;
    case Gear::SECOND:
        Joystick.pressButton(JOYSTICK_BUTTON_G2);
        break;
    case Gear::THIRD:
        Joystick.pressButton(JOYSTICK_BUTTON_G3);
        break;
    case Gear::FOURTH:
        Joystick.pressButton(JOYSTICK_BUTTON_G4);
        break;
    case Gear::FIFTH:
        Joystick.pressButton(JOYSTICK_BUTTON_G5);
        break;
    case Gear::SIXTH:
        Joystick.pressButton(JOYSTICK_BUTTON_G6);
        break;
    case Gear::REVERSE:
        Joystick.pressButton(JOYSTICK_BUTTON_GR);
        break;
    }
}

Gear findGear(int x, int y) {
    if (x < SHIFTER_AXIS_LEFT) {
        if (y > SHIFTER_AXIS_TOP) {
            return Gear::FIRST;
        } else if (y < SHIFTER_AXIS_BOTTOM) {
            return Gear::SECOND;
        }
    } else if (x > SHIFTER_AXIS_RIGHT) {
        if (y > SHIFTER_AXIS_TOP) {
            return Gear::FIFTH;
        } else if (y < SHIFTER_AXIS_BOTTOM) {
            return Gear::SIXTH;
        }
    } else {
        if (y > SHIFTER_AXIS_TOP) {
            return Gear::THIRD;
        } else if (y < SHIFTER_AXIS_BOTTOM) {
            return Gear::FOURTH;
        }
    }
    return Gear::NEUTRAL;
}

Gear findGearDown(int x, int y) {
    if (x > SHIFTER_AXIS_RIGHT) {
        if (y < SHIFTER_AXIS_BOTTOM) {
            return Gear::REVERSE;
        }
    }
    return Gear::NEUTRAL;
}