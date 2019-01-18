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

    const bool 
        ON = true,
        OFF = false;
  
    // States of the Pump Station Manager
    typedef enum {
        psStart,
        psRun,
        psValveOperating,   // State while valve closing/opening
    } PSMState;

    typedef enum {
        vsOpening,
        vsOpened,
        vsClosing,
        vsClosed
    } ValveState;

    typedef enum {
        vcOff,
        vcOn,
        vcPowerOff,
    } ValveControl;

    const int64_t
        NORMAL_TIMEOUT = 1000,
        ERROR_TIMEOUT  = 10*1000,   // timeout while error state detected
        ALARM_TIMEOUT  = 30*1000,
        
        // the Valve needs up to 15 secs to open or close
        VALVE_TIMEOUT  = 15*1000;

    typedef enum {
        dtDisplay,
        dtAlarm,
        dtCheck,
    } DeadlineType;

    const uint8_t
        P_PUMP       = 4,    // SSR Water pump Pin
        P_WL_ASENSOR = A2,   // Water leak sensor (Analogue)
        P_WL_DSENSOR = 12,   // Water leak sensor (Digital)
        P_BEEPER     = 7,    // Bepeer
        P_PS_SENSOR  = 8,    // Power Supply sensor
        P_VLV_OPEN   = 5,    // Water valve closing SSR
        P_VLV_CLOSE  = 6;    // Water valve opening SSR
        
    class PSManager {
        public: 
            PSManager();
            
            void exec();
               
        private:
            PSMState state;
            ValveState v_state;

            bool PS_Alarm,
                 WL_Alarm;

            uint64_t check_deadline,
                     display_deadline,
                     alarm_deadline;

            // state functions
            PSMState start();
            PSMState run();
            PSMState valveOperating();

            // utility functions
            void setDeadline(uint64_t timeout, DeadlineType dtype);
            bool closeAll();
            bool openAll();

            void pumpControl(bool ctl);
            void valveControl(ValveControl ctl);

            void display();
            void alarm();

            // checking functions
            // return true if Water Leak or Power Supply Interrupt are detected
            bool checkWLeak();
            bool checkPSError();
            
    };  // class PSManager
    
}; // namespace psm

#endif // __PSM_H__
