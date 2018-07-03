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

#ifndef __PSM_H__
#define __PSM_H__

#include <Arduino.h>

namespace psm {
  
    // States of the Pump Station Manager
    typedef enum {
        psStart,
        psRun,
        psWL_Alarm,  // Water leak alarm
        psPS_Alarm,  // Power Supply alarm
        psClosed,    // State after psWLAlarm
        psSuspended, // State after psPSAlarm
    } PSMState;

    typedef enum {
        bmOn,
        bmOff,
        bmToggle
    } BlinkMode;

    typedef enum {
        vsOpening,
        vsOpened,
        vsClosing,
        vsClosed
    } ValveState;
        
    const int64_t
        MIN_TIMEOUT         = 1000,
        RESTART_TIMEOUT     = 10*1000,

        // the Valve needs up to 15 secs to open or close
        VALVE_MAX_TIMEOUT  = 15*1000,
        BLINK_TIMEOUT      = 500,
        FAST_BLINK_TIMEOUT = 250;

    const uint8_t
        P_PUMP      = 5,    // SS-Relay Water pump Pin
        P_WL_ASENSOR = A2,  // Water leak sensor (Analogue)
        P_WL_DSENSOR = 9,   // Water leak sensor (Digital)
        P_PS_SENSOR = 6,    // Power Supply sensor
        P_VLV_OPEN  = 7,    // Water valve close relay
        P_VLV_CLOSE = 5;    // Water valve open relay
        
    class PSManager {
        public: 
            PSManager();
            
            void exec();
               
        private:
            PSMState state;
            ValveState v_state;

            uint64_t timeout,
                     blink_timeout;

            // state functions
            PSMState start();
            PSMState run();
            PSMState close();
            PSMState notify();
            PSMState suspend();

            // utility functions
            void setTout(uint64_t delay_millis);
            void blink(BlinkMode mode);

            void turnPumpOn();
            void turnPumpOff();
            
    };  // class PSManager
    
}; // namespace psm

#endif // __PSM_H__
