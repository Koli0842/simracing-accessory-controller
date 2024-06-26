#include "Joystick.h"

#define HANDBRAKE true
#define SHIFTER true
#define DEBUG false
#define DEBUG_HALL false
#define DEBUG_SHIFTER false

#define HANDBRAKE_AXIS_MAX 1023
#define HANDBRAKE_AXIS_TOP_TRESHOLD 700
#define HANDBRAKE_AXIS_BOTTOM_TRESHOLD 550
#define HANDBRAKE_POLARITY true
#define HANDBRAKE_GAMMA 1.6

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
    int normalizedReading, handbrakeValue, shifterHorizontalValue, shifterVerticalValue;
    bool shifterPressedDown;
    Gear gear;
    if (HANDBRAKE || DEBUG) {
        normalizedReading = normalizeReading(analogRead(PIN_HANDBRAKE));
        handbrakeValue = applyGammaFilter(normalizedReading);
        Joystick.setRxAxis(handbrakeValue);
    }

    if (SHIFTER || DEBUG) {
        shifterHorizontalValue = analogRead(PIN_G29_SHIFTER_X);
        shifterVerticalValue = analogRead(PIN_G29_SHIFTER_Y);
        shifterPressedDown = digitalRead(PIN_G29_DOWN);
        gear = !shifterPressedDown ?
            findGear(shifterHorizontalValue, shifterVerticalValue) :
            findGearDown(shifterHorizontalValue, shifterVerticalValue);
        updateShifter(gear);
    }

    
    if (DEBUG) {
        char buffer[100];
        sprintf(buffer, "Handbrake [ %d ] Shifter [ %d | X %d Y %d PRESSED %x ]",
            handbrakeValue, gear, shifterHorizontalValue, shifterVerticalValue, shifterPressedDown);
        Serial.println(buffer);
    }
}

int applyGammaFilter(const int& reading) {
    return pow(reading / (double)HANDBRAKE_AXIS_MAX, 1.0 / HANDBRAKE_GAMMA) * HANDBRAKE_AXIS_MAX;
}

int normalizeReading(const int& reading) {
    if (DEBUG_HALL) {
        char buffer[100];
        sprintf(buffer, "Handbrake [ %d ]", reading);
        Serial.println(buffer);
    }
    int polarizedValue;
    if (HANDBRAKE_POLARITY) {
        polarizedValue = constrain(reading, HANDBRAKE_AXIS_BOTTOM_TRESHOLD, HANDBRAKE_AXIS_TOP_TRESHOLD);
    } else {
        polarizedValue = constrain(HANDBRAKE_AXIS_MAX - reading, HANDBRAKE_AXIS_MAX - HANDBRAKE_AXIS_TOP_TRESHOLD, HANDBRAKE_AXIS_MAX - HANDBRAKE_AXIS_BOTTOM_TRESHOLD);
    }
    return map(polarizedValue, HANDBRAKE_AXIS_BOTTOM_TRESHOLD, HANDBRAKE_AXIS_TOP_TRESHOLD, 0, HANDBRAKE_AXIS_MAX);
}

void setHandbrakeButton(const int& axisValue) {
    if (axisValue > HANDBRAKE_AXIS_MAX/2) {
        Joystick.pressButton(0);
    } else {
        Joystick.releaseButton(0);
    }
}

void updateShifter(const Gear& gear) {
    if (gear == SELECTED_GEAR) return;
    if (gear != Gear::NEUTRAL && SELECTED_GEAR != Gear::NEUTRAL) return; // Only allow gearshifts through neutral, fix twitchy sensors & overlap between gears

    if (DEBUG_SHIFTER) {
        char buffer[100];
        sprintf(buffer, "Shifter [ %d ] -> [ %d ]", SELECTED_GEAR, gear);
        Serial.println(buffer);
    }
    
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
    case Gear::NEUTRAL:
        Joystick.releaseButton(SELECTED_GEAR);
        break;
    }
    SELECTED_GEAR = gear;
}

Gear findGear(const int& x, const int& y) {
    if (y > SHIFTER_AXIS_TOP) {
    if (x < SHIFTER_AXIS_LEFT) {
            return Gear::FIRST;
        } else if (x > SHIFTER_AXIS_RIGHT) {
            return Gear::FIFTH;
        } else {
            return Gear::THIRD;
        }
        } else if (y < SHIFTER_AXIS_BOTTOM) {
        if (x < SHIFTER_AXIS_LEFT) {
            return Gear::SECOND;
    } else if (x > SHIFTER_AXIS_RIGHT) {
            return Gear::SIXTH;
    } else {
            return Gear::FOURTH;
        }
    }
    return Gear::NEUTRAL;
}

Gear findGearDown(const int& x, const int& y) {
    if (x > SHIFTER_AXIS_RIGHT) {
        if (y < SHIFTER_AXIS_BOTTOM) {
            return Gear::REVERSE;
        }
    }
    return Gear::NEUTRAL;
}