#include "Joystick.h"

#define AXIS_MAX 1023
#define AXIS_TOP_TRESHOLD 1023
#define AXIS_BOTTOM_TRESHOLD 0

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_JOYSTICK, 1, 0,
        false, false, false,
        true, false, false,
        false, false, false, false, false);

void setup() {
    pinMode(A0, INPUT);
    Serial.begin(9600);
    Joystick.begin();
    Joystick.setRxAxisRange(0, AXIS_MAX);
}

void loop() {
    int axisValue = processAxisValue(analogRead(A0));
    Serial.println(axisValue);
    Joystick.setRxAxis(axisValue);
    //setHandbrakeButton(axisValue);
}

int processAxisValue(int reading) {
    int cappedReading = constrain(reading, AXIS_BOTTOM_TRESHOLD, AXIS_TOP_TRESHOLD);
    return map(cappedReading, AXIS_BOTTOM_TRESHOLD, AXIS_TOP_TRESHOLD, AXIS_MAX, 0);
}

void setHandbrakeButton(int axisValue) {
    if(axisValue > AXIS_MAX/2) {
        Joystick.pressButton(0);
    } else {
        Joystick.releaseButton(0);
    }
}