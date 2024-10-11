/*! @mainpage Ejercicio 4
 *
 * @section genDesc General Description
 *
 * Programa que permitie digitalizar una señal analógica y 
 * transmitirla a un graficador de puerto serie de la PC. Ademas puede
 * convertir un ECG digital a analogico
 * 
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	+5V 	 	| 	+5V   		|
 * | 	GND 	 	| 	GND 		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 03/10/2024 | Document creation		                         |
 *
 * @author Lucas Alarcon (lucasalarcon872@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
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
#include "analog_io_mcu.h"
#include "uart_mcu.h"
/*==================[macros and definitions]=================================*/
/** @def TIEMPO_LECTURA
 *  @brief se define el delay para la lectura de datos
 */
#define TIEMPO_LECTURA 20000
/** @def BUFFER_SIZE
 *  @brief se define el tamanIo del buffer a utilizar
 */
#define BUFFER_SIZE 231
/** @def REFRESCO_ECG
 *  @brief se define el delay para la lectura de datos del ECG
 */
#define REFRESCO_ECG 40000
/** @def cuentas_ECG
 *  @brief constante que lleva dato sobre la cantidad de ceuntas del ECG
 */
uint8_t cuentas_ECG = 0;
/*==================[internal data definition]===============================*/

/** @def lectura_task_handle
 *  @brief handle de la tarea asociada a la lectura de la medicion
 */
TaskHandle_t lectura_task_handle = NULL;

/** @def ecg_task_handle
 *  @brief handle de la tarea asociada al ecg
 */
TaskHandle_t ecg_task_handle = NULL;

/** @def voltaje
 *  @brief es el voltaje asociado a la medicion
 */
uint16_t voltaje = 0;

/** @def ecg
 *  @brief electrocardiograma a analizar
 */
const char ecg[BUFFER_SIZE] = {
    76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77, 77, 76, 76,
};

/*==================[internal functions declaration]=========================*/
/**
 * @brief Función invocada en la interrupción del timer A
 */
void FuncTimerA(void* param){
    vTaskNotifyGiveFromISR(lectura_task_handle, pdFALSE);    
}

/**
 * @brief Función invocada en la interrupción del timer B
 */
void FuncTimerB(void* param){
    vTaskNotifyGiveFromISR(ecg_task_handle, pdFALSE); 
}

/**
 * @brief Función que convierte datos analogicos a digital
 * @param pvParameter parametro interno
 */
static void ConversionAD(void *pvParameter){
    while (true){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificación */
        AnalogInputReadSingle(CH1, &voltaje);
		UartSendString(UART_PC, (char*)UartItoa(voltaje, 10));
		UartSendString(UART_PC, "\r\n");
    }
}

/**
 * @brief Tarea que envía los datos analógicos 
 * @param pvParameter parametro interno
 */
static void enviarDatos(void *pvParameter){
	while(true){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificación */
		AnalogOutputWrite(ecg[cuentas_ECG]);
		if(cuentas_ECG<(BUFFER_SIZE-1))
			cuentas_ECG++;
		else
			cuentas_ECG=0;
	}
}

/*==================[external functions definition]==========================*/
void app_main(void){

	analog_input_config_t conversorAD = {
        .input = CH1,
        .mode = ADC_SINGLE,
    };
    AnalogInputInit(&conversorAD);
    AnalogOutputInit();

    /* Inicialización de timers */
    timer_config_t timer_lectura = {
        .timer = TIMER_A,
        .period = TIEMPO_LECTURA,
        .func_p = FuncTimerA,
        .param_p = NULL
    };
    TimerInit(&timer_lectura);

    timer_config_t timer_ecg = {
        .timer = TIMER_B,
        .period = REFRESCO_ECG,
        .func_p = FuncTimerB,
        .param_p = NULL
    };
    TimerInit(&timer_ecg);

    serial_config_t my_uart = {
    .port = UART_PC,
    .baud_rate = 57600,
    .func_p = NULL,
    .param_p = NULL
    };
    UartInit(&my_uart);

    /* Creación de tareas */
    xTaskCreate(&ConversionAD, "ConversionAD", 2048, NULL, 5, &lectura_task_handle);
    xTaskCreate(&enviarDatos, "enviarDatos", 2048, NULL, 5, &ecg_task_handle);

  
    /* Inicialización del conteo de timers */
    TimerStart(timer_lectura.timer);
    TimerStart(timer_ecg.timer);

	serial_config_t puertoSerie = {
		.port = UART_PC,
		.baud_rate = 115200,
		.func_p = NULL,
		.param_p = NULL
	};
	UartInit(&puertoSerie);
}
/*==================[end of file]============================================*/