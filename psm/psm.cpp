/**
 * Water pump station manager 
 * 
 * Controls water leak sensors, power supply sensor and 
 * manages the water pump and water tank filling valve.
 * 
 * dr-dobermann, 2018
 * 
 * https://github.com/dr-dobermann/ps_manager.git
 */

#include "psm.h"

using namespace psm;

/**
 * PSManager initializes the Pump Station Manager
 */
PSManager::PSManager() {

    state = psStart;
    v_state = vsClosed;
    
    timeout = 0;
    blink_timeout = 0;

    pinMode(P_PUMP, OUTPUT);
    pinMode(P_WL_ASENSOR, INPUT);
    pinMode(P_WL_DSENSOR, INPUT);
    pinMode(P_PS_SENSOR, INPUT);
    pinMode(P_VLV_OPEN, OUTPUT);
    pinMode(P_VLV_CLOSE, OUTPUT);
}
//-------------------------------------------------------------

/**            
 * exec executes the main state loop and 
 *   dispatches the workflow according the current state
 */
void PSManager::exec() {

    // display the state over the blink
    switch (state) {
        case psStart:
            this->blink(bmOn);
            break;
            
        case psRun:
        case psWL_Alarm:
            if (blink_timeout < millis()) {
                this->blink(bmToggle);
                blink_timeout = millis() + (state == psRun ? BLINK_TIMEOUT : FAST_BLINK_TIMEOUT);
            }
            break;

        default:
            this->blink(bmOff);
            break;            
    };
    
    if (millis() < timeout)
        return;

    Serial.println(state);
    
    switch (state) {
        case psStart:
            state = start();
            break;
            
        case psRun:
            state = this->run();
            break;

        case psWL_Alarm:
            state = this->close();
            break;

        case psClosed:
            state = notify();
            break;

        case psPS_Alarm:
            state = suspend();
            break;
           
        default:
            setTout(MIN_TIMEOUT);
            break;
    }
}
//-------------------------------------------------------------

/**
 * start starts the psm and move it into the run state if everythin is ok.
 * Returns false in case of errors 
 * 
 * it opens the water tank valve and powers up the water pump if the water leak sensor is ok
 */
PSMState PSManager::start() {

    Serial.println("start");
    
    if ( analogRead(P_WL_ASENSOR) < 500 || digitalRead(P_WL_DSENSOR) == LOW ) { // wait for WL sensor to dry  ) { // || 
        Serial.println("could not open the valve. Water leak is present");
        setTout(RESTART_TIMEOUT);
        turnPumpOff();
        
        return psStart;
    }

    if ( v_state == vsClosed ) {
        Serial.println("opening...");
        v_state = vsOpening;
        digitalWrite(P_VLV_OPEN, false); // Low state relay need's low level to contact
        digitalWrite(P_VLV_CLOSE, true);
        setTout(VALVE_MAX_TIMEOUT);

        return psStart;
    }
    
    if ( v_state == vsOpening ) {
        Serial.println("opened");
        v_state = vsOpened;
        digitalWrite(P_VLV_OPEN, true); // Turn off the opening relay
        turnPumpOn();
        setTout(0);

        return psRun;
    }
        
    return psStart;
}
//-------------------------------------------------------------

/**
 * run runs the main application loop
 * 
 * checks following sensors:
 *   the water leak sensor. If it fires, the state changes to psWL_Alarm
 *   the power supply sensor. If it fires, the state changes to psPS_Alarm
 */
PSMState PSManager::run() {

    Serial.println("run");

    if ( analogRead(P_WL_ASENSOR) < 500 || digitalRead(P_WL_DSENSOR) == LOW ) {
        setTout(0);
        return psWL_Alarm;
    }

//    if ( digitalRead(P_PS_SENSOR) == LOW ) {
//        setTout(0);
//        return psPS_Alarm;
//    }
        
    setTout(MIN_TIMEOUT);
        
    return psRun;
}
//-------------------------------------------------------------

/**
 * close closes the water tank valve and powers off the pump
 */
PSMState PSManager::close() {

    Serial.println("close");

    if ( v_state == vsOpened ) {
        turnPumpOff();
        Serial.println("closing...");
        v_state = vsClosing;
        digitalWrite(P_VLV_CLOSE, false); // Low state relay need's low level to contact
        digitalWrite(P_VLV_OPEN, true);
        setTout(VALVE_MAX_TIMEOUT);

        return psWL_Alarm;
    }

    if ( v_state == vsClosing ) {
        Serial.println("closed");
        v_state = vsClosed;
        digitalWrite(P_VLV_CLOSE, true); // Turn off the closing relay
        setTout(0);
            
        return psClosed;
    }

    return psWL_Alarm;
}
//-------------------------------------------------------------

/**
 * notify sends an alarm messages
 */
PSMState PSManager::notify() {
    
    Serial.println("notify");

    setTout(RESTART_TIMEOUT);
    
    return psClosed;
}
//-------------------------------------------------------------

/*
 * suspend closes the water tank valve and powers off the pump
 * after this it sends arduino to sleep with wakeup interrupt on power supply pin
 * 
 */
PSMState PSManager::suspend() {

    Serial.println("suspend");

    return psStart;
}
//-------------------------------------------------------------

void PSManager::setTout(uint64_t delay_millis) {
    
    timeout = millis() + delay_millis;
    Serial.print("new timeout: "); 
    Serial.println((long)timeout);
}
//-------------------------------------------------------------

/*
 * blink turns on or off the Arduino builtin led
 */
void PSManager::blink(BlinkMode mode) {
    
    bool newValue = false;
    
    switch (mode) {
        case bmOn:
            newValue = HIGH;
            break;

        case bmOff:
            newValue = LOW;
            break;

        default: // toggle
            newValue = !digitalRead(LED_BUILTIN);
            break;
    }

    if ( digitalRead(LED_BUILTIN) != newValue)
        digitalWrite(LED_BUILTIN, newValue);
}
//-------------------------------------------------------------

void PSManager::turnPumpOn() {
    
    if ( digitalRead(P_PUMP) != HIGH )
        digitalWrite(P_PUMP, HIGH);
}
//-------------------------------------------------------------

void PSManager::turnPumpOff() {
    
    if ( digitalRead(P_PUMP) != LOW )
        digitalWrite(P_PUMP, LOW);
}
//-------------------------------------------------------------

