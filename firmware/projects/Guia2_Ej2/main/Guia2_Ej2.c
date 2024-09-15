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
 * | 05/09/2024 | Document creation		                         |
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
/*==================[macros and definitions]=================================*/
#define ON 1
#define OFF 0
/** @def REFRESCO_LECTURA
 *  @brief se define el delay para la lectura de la distancia
 */
#define REFRESCO_LECTURA 1000 
/** @def REFRESCO_TECLA
 *  @brief se define el delay para la lectura de tecla 
 */
#define REFRESCO_TECLA 200
/** @def REFRESCO_EJECUCION
 *  @brief se define el delay para que se muestre la informacion
 */
#define REFRESCO_MOSTRAR 500
/*==================[internal data definition]===============================*/
TaskHandle_t led1_task_handle = NULL;
TaskHandle_t led2_task_handle = NULL;
TaskHandle_t led3_task_handle = NULL;
/** @def medida
 *  @brief se define la medida obtenida por el sensor
 */
uint16_t medida = 0;
/** @def tecla
 *  @brief se define la tecla presionada
 */
uint8_t tecla = 0;
/** @def tecla1
 *  @brief se define la condicion de la tecla 1
 */
uint8_t tecla1 = 0;
/** @def HOLD
 *  @brief se define el estado de la tecla 2
 */
uint8_t HOLD = 0;
/*==================[internal functions declaration]=========================*/
/**
 * @brief Función invocada en la interrupción del timer A
 */
void FuncTimerA(void* param){
    vTaskNotifyGiveFromISR(led1_task_handle, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_1 */
}

/**
 * @brief Función invocada en la interrupción del timer B
 */
void FuncTimerB(void* param){
    vTaskNotifyGiveFromISR(led2_task_handle, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_2 */
}

/**
 * @brief Función invocada en la interrupción del timer B
 */
void FuncTimerC(void* param){
    vTaskNotifyGiveFromISR(led3_task_handle, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_2 */
}

/** @fn LeerSensor 
 *  @brief se lee la medida del sensor
 *  @return 0
 */
static void LeerSensor(){
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificación */
    
    while (true){
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
static void Mostrar(void *pvParameter){\
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificación */

    while (true){
        if (tecla1 == ON){
            EncenderLed();
            MostrarPantalla();
        }
        else{
            LedsOffAll();
        }
        if (HOLD == ON){
            EncenderLed();
        }   
    }
}

/** @fn PresionarTecla
 *  @brief se lee la tecla medida y cambia su condicion 
 *  @return 0
 */
static void PresionarTecla(void *pvParameter){
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificación */

    while (true){
        tecla = SwitchesRead();
        switch (tecla)
        {
        case (SWITCH_1): tecla1 = ON; 
            HOLD = 0;
            break;
        
        case (SWITCH_2): HOLD = 1;
            tecla1 = OFF;
            break;
        }
   
    }
}

/*==================[external functions definition]==========================*/
void app_main(void){

    /* Inicialización de timers */
    timer_config_t timer_lectura = {
        .timer = TIMER_A,
        .period = REFRESCO_LECTURA,
        .func_p = FuncTimerA,
        .param_p = NULL
    };
    TimerInit(&timer_lectura);

    timer_config_t timer_tecla = {
        .timer = TIMER_B,
        .period = REFRESCO_TECLA,
        .func_p = FuncTimerB,
        .param_p = NULL
    };
    TimerInit(&timer_tecla);

    timer_config_t timer_mostrar = {
        .timer = TIMER_C,
        .period = REFRESCO_MOSTRAR,
        .func_p = FuncTimerC,
        .param_p = NULL
    };
    TimerInit(&timer_mostrar);

    LedsInit(); //Iniciar Leds
    LcdItsE0803Init();  //Iniciar Pantalla
    HcSr04Init(GPIO_3,GPIO_2); //Inicio Sensor
    SwitchesInit(); //Inicio las teclas

    xTaskCreate(&PresionarTecla, "PresionarTecla", 2048, NULL, 5, NULL);
    xTaskCreate(&LeerSensor, "LeerSensor", 2048, NULL, 5, NULL);
    xTaskCreate(&Mostrar, "Mostar", 2048, NULL, 5, NULL);

    /* Inicialización del conteo de timers */
    TimerStart(timer_lectura.timer);
    TimerStart(timer_tecla.timer);
    TimerStart(timer_mostar.timer);
}
