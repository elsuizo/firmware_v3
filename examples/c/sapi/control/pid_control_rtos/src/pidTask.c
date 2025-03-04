/*=============================================================================
 * Copyright (c) 2019, Eric Pernia <ericpernia@gmail.com>
 * All rights reserved.
 * License: bsd-3-clause (see LICENSE.txt)
 * Date: 2019/09/16
 * Version: 1.0.0
 *===========================================================================*/

/*=====[Inclusion of own header]=============================================*/

#include "pidTask.h"
 
/*=====[Inclusions of private function dependencies]=========================*/

#include "sapi.h"
#include "pid_controller.h"

/*=====[Definition macros of private constants]==============================*/

#define SCALE_OUT       100.0f
#define SCALE_REF       100.0f
#define SCALE_PID_OUT   100.0f
        
/*=====[Private function-like macros]========================================*/

/*=====[Definitions of private data types]===================================*/

/*=====[Definitions of external public global variables]=====================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

/*=====[Prototypes (declarations) of private functions]======================*/

/*=====[Implementations of public functions]=================================*/

// Para calcular el tiempo que tarda el algoritmo y establecer un h minimo
//#define COUNT_CYCLES

// Task implementation
void pidControlTask( void* taskParmPtr )
{
   // Controller signals
   float r = 0.0f; // Measure of reference r[k]
   float y = 0.0f; // Measure of system output y[k]
   float u = 0.0f; // Calculated controller's output u[k]

   // h no puede ser menor ni al tiempo del algoritmo, y con va a ser un
   // multiplo del periodo de tick del RTOS
   uint32_t h_ms = 4; // Task periodicity (sample rate)

   // PID Controller structure (like an object instance)
   PIDController_t PsPIDController;

   // PID Controller Initialization
   pidInit( &PsPIDController,
            2.8,                   // Kp
            1.2,                   // Ki
            0.1,                   // Kd
            ((float)h_ms)/1000.0f, // h en [s]
            20.0f,                 // N
            1.0f,                  // b
            30.0f,                 // u_min
            60.0f                  // u_max
          );

   // Peridodic task each h_ms
   portTickType xPeriodicity =  h_ms / portTICK_RATE_MS;
   portTickType xLastWakeTime = xTaskGetTickCount();

   #ifdef COUNT_CYCLES
      // Configura el contador de ciclos con el clock de la EDU-CIAA NXP
      cyclesCounterConfig(EDU_CIAA_NXP_CLOCK_SPEED);
      volatile uint32_t cyclesElapsed = 0;
   #endif

   while(true) {

      #ifdef COUNT_CYCLES
         // Resetea el contador de ciclos
         cyclesCounterReset();
      #endif

      // Leer salida y[k] y refererencia r[k]
      y = adcRead( CH1 ) * SCALE_OUT;
      r = adcRead( CH2 ) * SCALE_REF;

      // Calculate PID controller output u[k]
      u = pidCalculateControllerOutput( &PsPIDController, y, r );

      // Actualizar la salida u[k]
      dacWrite( DAC, u * SCALE_PID_OUT );

      // Update PID controller for next iteration
      pidUpdateController( &PsPIDController, y, r );

      #ifdef COUNT_CYCLES
         // Leer conteco actual de ciclos
         cyclesElapsed = DWT_CYCCNT;
         volatile float us = cyclesCounterToUs(cyclesElapsed);
      #endif

      // Send the task to the locked state during xPeriodicity
      // (periodical delay)
      vTaskDelayUntil( &xLastWakeTime, xPeriodicity );
   }
}

/*=====[Implementations of interrupt functions]==============================*/

/*=====[Implementations of private functions]================================*/

