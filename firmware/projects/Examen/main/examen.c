/*! @mainpage Examen parcial
 *
 * @section genDesc General Description
 *
 * Se pretende diseñar un dispositivo basado en la ESP-EDU que permita detectar
 * eventos peligrosos para ciclistas.
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
/*==================[macros and definitions]=================================*/
/** @def TIEMPO_MEDICION
 *  @brief se define el delay para la lectura de la distancia
 */
#define TIEMPO_MEDICION 50000  //dos por segundos 

/*==================[internal data definition]===============================*/
/** @def medicion_task_handle
 *  @brief handle de la tarea asociada a la lectura de la medicion
 */
TaskHandle_t medicion_task_handle = NULL;
/** @def distancia
 *  @brief se define la distancia obtenida por el sensor
 */
uint16_t distancia;
/*==================[internal functions declaration]=========================*/
/**
 * @brief Función invocada en la interrupción del timer A
 */
void FuncTimerA(void* param){
    vTaskNotifyGiveFromISR(medicion_task_handle, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_1 */
}

/** @fn LeerSensor 
 *  @brief se lee la medida del sensor
 *  @return 0
 */
static void obtenerDistancia(){    
    while (true){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificación */
        distancia = HcSr04ReadDistanceInCentimeters(); 
    }
}

/** @fn EncenderLed 
 *  @brief se prenden los Leds dependiendo la distancia
 *  @return 0
 */
void EncenderLed(){
    if (distancia<300){
        LedOn(LED_1);
        LedOn(LED_2);
        LedOn(LED_3);       //se prenden todos los LEDs si la distancia es menor a tres metros
    }
    if (distancia>300 && distancia<500){
        LedOn(LED_1); // se prende LED ver y amarillo si la distancia esta entre 3 y 5 metros
		LedOn(LED_2);
    }
    if (distancia>500){
        LedOn(LED_1);  //se prende el LED verde si la distancia es mayor a 5 metros
    }
}

void manejoBuzzer(){  //La alarma sonará con una frecuencia de 1 segundo en el caso de precaución y cada 0.5 segundos en el caso de peligro.
    if (distancia<300){
		BuzzerSetFrec(0.5);  //seteo la frecuencia para peligro
		BuzzerOn(); //enciendo el buzzer
    }
    if (distancia>300 && distancia<500){
		BuzzerSetFrec(1);  //seteo la frecuencia para precaucion
		BuzzerOn(); //enciendo el buzzer
    }
	if (distancia>500){
		BuzzerOff(); // Apago el buzzer a una distancia segura
    }
}

/** @fn UartTask 
 *  @brief se muestra la lectura por el puerto serie
 *  @param pvParameter numero que se recibe para mostrar
 *  @return 0
 */
void alertaBlueetoth (void *pvParameter) {
	if (distancia>300 && distancia<500){ // dependiendo la distancia alerto de una u otra forma
	UartSendString(UART_CONNECTOR,"Precaución, vehículo cerca");
	UartSendString(UART_CONNECTOR,"r\n");
	}
	if (distancia<300){
	UartSendString(UART_CONNECTOR,"Peligro, vehículo cerca”");
	UartSendString(UART_CONNECTOR,"r\n");
	}  
}

/*==================[external functions definition]==========================*/
void app_main(void){
	LedsInit(); //Iniciar Leds
    LcdItsE0803Init();  //Iniciar Pantalla
    HcSr04Init(GPIO_3,GPIO_2); //Inicio Sensor
	BuzzerInit(GPIO_2); //Inicio de Buzzer

	    serial_config_t my_uart = {
    .port = UART_PC,
    .baud_rate = 9600,
    .param_p = NULL
    };
    UartInit(&my_uart);

    /* Inicialización de timers */
    timer_config_t timer_lectura = {
        .timer = TIMER_A,
        .period = TIEMPO_MEDICION,
        .func_p = FuncTimerA,
        .param_p = NULL
    };
    TimerInit(&timer_lectura);


    /* Creación de tareas */
    xTaskCreate(&obtenerDistancia, "obtener Distancia", 2048, NULL, 5, &medicion_task_handle);

  
    /* Inicialización del conteo de timers */
    TimerStart(timer_lectura.timer);


}
/*==================[end of file]============================================*/