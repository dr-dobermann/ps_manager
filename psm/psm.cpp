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

    display_deadline = 0;
    check_deadline = 0;
    alarm_deadline = 0;

    pinMode(P_PUMP, OUTPUT);
    
    pinMode(P_WL_ASENSOR, INPUT);
    pinMode(P_WL_DSENSOR, INPUT);

    pinMode(P_PS_SENSOR, INPUT);
    
    pinMode(P_VLV_OPEN, OUTPUT);
    pinMode(P_VLV_CLOSE, OUTPUT);

    pinMode(P_BEEPER, OUTPUT);
}
//-------------------------------------------------------------

/**            
 * executes the main state loop and 
 * dispatches the workflow according the current state
 */
void PSManager::exec() {

    this->display();

    switch ( this->state ) {
        case psStart:
            this->state = this->start();
            break;

        case psRun:
            this->state = this->run();
            break;

        case psValveOperating:
            this->state = this->valveOperating();
            break;
    }
}
//-------------------------------------------------------------

/**
 * starts the psm, closes pump and valve.
*/
PSMState PSManager::start() {
    
    v_state = vsOpened;
    
    closeAll();

    return psValveOperating;
}
//-------------------------------------------------------------

PSMState PSManager::run() {

    if ( check_deadline > millis() )
        return psRun;

    PS_Alarm = checkPSError();
    WL_Alarm = checkWLeak();

    setDeadline(PS_Alarm || WL_Alarm ? ERROR_TIMEOUT : NORMAL_TIMEOUT, dtCheck);
    
    if ( PS_Alarm && closeAll() )
        return psValveOperating;

    if ( WL_Alarm ) {
        if ( closeAll() )
            return psValveOperating;
    } else
        if ( openAll() )
            return psValveOperating;
            
    return psRun;
}
//-------------------------------------------------------------

PSMState PSManager::valveOperating() {
    
    if ( check_deadline > millis() )
        return psValveOperating;

    valveControl(vcPowerOff);
    
    if ( v_state == vsClosing ) {
        v_state = vsClosed;
    }
    else if ( v_state == vsOpening )
        v_state = vsOpened;
    
    return psRun;
}
//-------------------------------------------------------------

void PSManager::display() {
    
    if ( display_timeout > millis() )
        return;

    setDeadline(NORMAL_TIMEOUT, dtDisplay);

    
}
//-------------------------------------------------------------

void PSManager::alarm() {
    
}
//-------------------------------------------------------------

bool PSManager::closeAll() {
    
    if ( v_state == vsClosing || v_state == vsOpening || v_state == vsClosed )
        return false;
    
    pumpControl(psm::OFF);
    valveControl(vcOff);
    
    setDeadline(psm::VALVE_TIMEOUT, dtCheck);

    return true;
}
//-------------------------------------------------------------

bool PSManager::openAll() {
    
    if ( v_state == vsClosing || v_state == vsOpening || v_state == vsOpened )
        return false;

    pumpControl(psm::ON);
    valveControl(vcOn);
    
    setDeadline(psm::VALVE_TIMEOUT, dtCheck);

    return true;
}
//-------------------------------------------------------------

void PSManager::pumpControl(bool ctl) {

    digitalWrite(P_PUMP, ctl);
}
//-------------------------------------------------------------

void PSManager::valveControl(ValveControl ctl) {

    switch ( ctl ) { 
        case vcOn: // open valve
            digitalWrite(P_VLV_CLOSE, false);
            digitalWrite(P_VLV_OPEN, true);
            v_state = vsOpening;
            break;

        case vcOff: // close valve
            digitalWrite(P_VLV_OPEN, false);
            digitalWrite(P_VLV_CLOSE, true);
            v_state = vsClosing;
            break;

        case vcPowerOff:
            digitalWrite(P_VLV_OPEN, false);
            digitalWrite(P_VLV_CLOSE, false);
            break;
    }
}
//-------------------------------------------------------------

bool PSManager::checkWLeak() {
    
    return (analogRead(P_WL_ASENSOR) > 500 || digitalRead(P_WL_DSENSOR) == HIGH);
}
//-------------------------------------------------------------

bool PSManager::checkPSError() {
    
    return (digitalRead(P_PS_SENSOR) == LOW);
}
//-------------------------------------------------------------

void PSManager::setDeadline(uint64_t timeout, DeadlineType dtype) {
    
    switch (dtype) {
        case dtAlarm:
            alarm_deadline = millis() + timeout;
            break;

        case dtDisplay:
            display_deadline = millis() + timeout;
            break;

        case dtCheck:
            check_deadline = millis() + timeout;
            break;
    }
}
//-------------------------------------------------------------
