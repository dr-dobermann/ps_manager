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
PSManager::PSManager() :
     step_counts{1, 5, 5, 0, 0, 0} {

    state = psStart;
    timeout = MIN_TIMEOUT;
    blink_timeout = 0;

    step_counter = 0;
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

    PSMState currState = state;
    switch (state) {
        case psStart:
            state = start();
            if (state == psStart)
                timeout = millis() + RESTART_TIMEOUT;
            break;
            
        case psRun:
            state = this->run();
            if (state == psRun)
                timeout = millis() + MIN_TIMEOUT;
            break;

        case psWL_Alarm:
            state = this->close();
            timeout = millis() + MIN_TIMEOUT;
            break;

        case psClosed:
            state = notify();
            timeout = millis() + RESTART_TIMEOUT;
            break;

        case psPS_Alarm:
            state = suspend();
            timeout = millis() + MIN_TIMEOUT;
            break;
           
        default:
            timeout = millis() + MIN_TIMEOUT;
            break;
    }

    if ( currState != state)
        step_counter = 0;
}
//-------------------------------------------------------------

/**
 * start starts the psm and move it into the run state if everythin is ok.
 * Returns false in case of errors 
 * 
 * it opens the water tank valve and powers up the water pump if the water leak sensor is ok
 */
PSMState PSManager::start() {

    if ( step_counter++ >= step_counts[psStart])
        return psRun;
      
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

    if ( step_counter++ >= step_counts[psRun])
        return psWL_Alarm;
        
    return psRun;
}
//-------------------------------------------------------------

/**
 * close closes the water tank valve and powers off the pump
 */
PSMState PSManager::close() {

    if ( step_counter++ >= step_counts[psWL_Alarm])
        return psClosed;

    return psWL_Alarm;
}
//-------------------------------------------------------------

/**
 * notify sends an alarm messages
 */
PSMState PSManager::notify() {
    
    return psClosed;
}
//-------------------------------------------------------------

/*
 * suspend closes the water tank valve and powers off the pump
 * after this it sends arduino to sleep with wakeup interrupt on power supply pin
 * 
 */
PSMState PSManager::suspend() {
    
    return psStart;
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

