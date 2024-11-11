/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "hc_sr04.h"
#include "lcditse0803.h"
#include "switch.h"
#include "timer_mcu.h"
#include "buzzer.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
/*==================[macros and definitions]=================================*/
/** @def TIEMPO_LECTURA
 *  @brief se define el delay para la lectura de la distancia
 */
#define TIEMPO_LECTURA 100000 //10 muestras por segundo


#define TIEMPO_uS 1000       // cada cuanto cuenta el clock en us
/*==================[internal data definition]===============================*/
/** @def lectura_task_handle
 *  @brief handle de la tarea asociada a la lectura de la medicion
 */
TaskHandle_t lectura_task_handle = NULL;
/** @def medida
 *  @brief se define la medida obtenida por el sensor
 */
uint16_t medida = 0;

uint64_t conteo_en_ms = 0;

float velocidad = 0;
/*==================[internal functions declaration]=========================*/
/**
 * @brief Función invocada en la interrupción del timer A
 */
void FuncTimerA(void* param){
    vTaskNotifyGiveFromISR(lectura_task_handle, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_1 */
}

/** @fn LeerSensor 
 *  @brief se lee la medida del sensor
 *  @return 0
 */
static void LeerSensor(void* param){    
    while (true){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificación */
        medida = HcSr04ReadDistanceInCentimeters(); 
    }
}

void calcularVelocidad(){
	if(medida<1000){
		velocidad = medida/0.1; //se calcula la velocidad en cm/s
		velocidad = (velocidad)/100;   //se calcula la velocidad en m/s
		if(velocidad>8){
			LedOn(LED_3);
			LedOff(LED_2);
			LedOff(LED_1);  
		}if(velocidad>0 && velocidad<8){
			LedOff(LED_3);
			LedOn(LED_2);
			LedOff(LED_1);
		}
		if(velocidad==0){
			LedOff(LED_3);
			LedOff(LED_2);
			LedOn(LED_1);
		}
	}
}
/*==================[external functions definition]==========================*/
void app_main(void){
	LedsInit(); //Iniciar Leds
    HcSr04Init(GPIO_3,GPIO_2); //Inicio Sensor

	/* Inicialización de timers */
    timer_config_t timer_lectura = {
        .timer = TIMER_A,
        .period = TIEMPO_LECTURA,
        .func_p = FuncTimerA,
        .param_p = NULL
    };
    TimerInit(&timer_lectura);


	/* Creación de tareas */
    xTaskCreate(&LeerSensor, "LeerSensor", 2048, NULL, 5, &lectura_task_handle);

	/* Inicialización del conteo de timers */
    TimerStart(timer_lectura.timer);
}
/*==================[end of file]============================================*/