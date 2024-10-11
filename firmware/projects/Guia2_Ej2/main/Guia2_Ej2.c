/*! @mainpage Ejercicio 2
 *
 * @section genDesc General Description
 *
 * Modificacion de la actividad del punto 1 de manera de utilizar interrupciones 
 * para el control de las teclas y el control de tiempos (Timers).
 *
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	ECHO       	| 	GPIO_3		|
 * | 	TRIGGER	 	| 	GPIO_2		|
 * | 	+5V 	 	| 	+5V   		|
 * | 	GND 	 	| 	GND 		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 19/09/2024 | Document creation		                         |
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
/*==================[macros and definitions]=================================*/
#define ON 1
#define OFF 0
/** @def TIEMPO_LECTURA
 *  @brief se define el delay para la lectura de la distancia
 */
#define TIEMPO_LECTURA 1000000
/** @def TIEMPO_MOSTRAR
 *  @brief se define el delay para mostar la medida por pantalla 
 */
#define TIEMPO_MOSTRAR 500000
/*==================[internal data definition]===============================*/
/** @def lectura_task_handle
 *  @brief handle de la tarea asociada a la lectura de la medicion
 */
TaskHandle_t lectura_task_handle = NULL;
/** @def mostar_task_handle
 *  @brief handle de la tarea asociada a la lectura de la tecla
 */
TaskHandle_t mostrar_task_handle = NULL;
/** @def medida
 *  @brief se define la medida obtenida por el sensor
 */
uint16_t medida;
/** @def tecla1
 *  @brief se define la condicion de la tecla 1
 */
uint8_t tecla1 = 0;
/** @def tecla2
 *  @brief se define el estado de la tecla 2
 */
uint8_t tecla2 = 0;
/*==================[internal functions declaration]=========================*/
/**
 * @brief Función invocada en la interrupción del timer A
 */
void FuncTimerA(void* param){
    vTaskNotifyGiveFromISR(lectura_task_handle, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_1 */
}

/**
 * @brief Función invocada en la interrupción del timer B
 */
void FuncTimerB(void* param){
    vTaskNotifyGiveFromISR(mostrar_task_handle, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_2 */
}

/**
 * @brief Función asociado al cambio de la tecla 1
 */
static void estado_tecla1(){
    tecla1 =! tecla1;
}
/**
 * @brief Función asociado al cambio de la tecla 1
 */
static void estado_tecla2(){
    tecla2 =! tecla2;
}

/** @fn LeerSensor 
 *  @brief se lee la medida del sensor
 *  @return 0
 */
static void LeerSensor(){    
    while (true){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificación */
        medida = HcSr04ReadDistanceInCentimeters(); 
    }
}

/** @fn EncenderLed 
 *  @brief se prenden los Leds dependiendo la condicion
 *  @return 0
 */
void EncenderLed(){
    if (medida<10){
        LedsOffAll();        
    }
    if (medida>10 && medida<20){
        LedOn(LED_1);
    }
    if (medida>20 && medida<30){
        LedOn(LED_1);
        LedOn(LED_2);
    } 
    if (medida>30){
        LedOn(LED_1);
        LedOn(LED_2);
        LedOn(LED_3);
    }
}

/** @fn MostrarPantalla 
 *  @brief Se muestra por pantalla la medida
 *  @return 0
 */
void MostrarPantalla(){
    LcdItsE0803Write(medida);
}

/** @fn Ejecutar 
 *  @brief se muestra la medida
 *  @return 0
 */
static void Mostrar(void *pvParameter){
    while (true){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificación */
        if (tecla1 == ON){
            EncenderLed();
            MostrarPantalla();
        }
        else{
            LedsOffAll();
        }
        if (tecla2 == ON){
            EncenderLed();
        }   
    }
}


/*==================[external functions definition]==========================*/
void app_main(void){
    LedsInit(); //Iniciar Leds
    LcdItsE0803Init();  //Iniciar Pantalla
    HcSr04Init(GPIO_3,GPIO_2); //Inicio Sensor
    SwitchesInit(); //Inicio las teclas

    /* Inicialización de timers */
    timer_config_t timer_lectura = {
        .timer = TIMER_A,
        .period = TIEMPO_LECTURA,
        .func_p = FuncTimerA,
        .param_p = NULL
    };
    TimerInit(&timer_lectura);

    timer_config_t timer_mostrar = {
        .timer = TIMER_B,
        .period = TIEMPO_MOSTRAR,
        .func_p = FuncTimerB,
        .param_p = NULL
    };
    TimerInit(&timer_mostrar);

    /* Creación de tareas */
    xTaskCreate(&LeerSensor, "LeerSensor", 2048, NULL, 5, &lectura_task_handle);
    xTaskCreate(&Mostrar, "Mostrar", 2048, NULL, 5, &mostrar_task_handle);

    /* se analiza si se presiona una tecla */
    SwitchActivInt(SWITCH_1, *estado_tecla1, NULL);
    SwitchActivInt(SWITCH_2, *estado_tecla2, NULL);
  
    /* Inicialización del conteo de timers */
    TimerStart(timer_lectura.timer);
    TimerStart(timer_mostrar.timer);

}
