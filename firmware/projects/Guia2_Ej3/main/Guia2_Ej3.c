/*! @mainpage Ejercicio 3
 *
 * @section genDesc General Description
 *
 * Se modifica la actividad del punto 2 agregando ahora el puerto serie. 
 * Se envían los datos de las mediciones y se pueden observar en un terminal en la PC
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
#include "uart_mcu.h"
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
/** @def mostrar_task_handle
 *  @brief handle de la tarea asociada a la muestra de la medida
 */
TaskHandle_t mostrar_task_handle = NULL;
/** @def uart_task_handle
 *  @brief handle de la tarea asociada al UART
 */
TaskHandle_t uart_task_handle = NULL;
/** @def medida
 *  @brief se define la medida obtenida por el sensor
 */
uint16_t medida;
/** @def teclaO
 *  @brief se define la condicion de la tecla O
 */
uint8_t teclaO = 0;
/** @def teclaH
 *  @brief se define el estado de la tecla H
 */
uint8_t teclaH = 0;

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
    vTaskNotifyGiveFromISR(uart_task_handle, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_2 */
}


/** @fn LeerSensor 
 *  @brief se lee la medida del sensor
 *  @return 0
 */
static void LeerSensor(){    
    while (true){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificación */
        if(teclaH == ON){

        }
        else{
            medida = HcSr04ReadDistanceInCentimeters(); 
        }
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
        if (teclaO == ON){
            
            MostrarPantalla();
            EncenderLed();
        }
        else{
            LedsOffAll();
        }
        if (teclaH == ON){
            EncenderLed();
        }   
    }
}

/** @fn UartTask 
 *  @brief se muestra la lectura por el puerto serie
 *  @param pvParameter numero que se recibe para mostrar
 *  @return 0
 */
static void UartTask (void *pvParameter) {
    while (true){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificación */
        UartSendString(UART_PC,(char*)UartItoa(medida, 10));
        UartSendString(UART_PC," ");
        UartSendString(UART_PC,"cm");
        UartSendString(UART_PC,"r\n");
    }    
}

/** @fn FuncUart 
 *  @brief se leen las teclas pulsadas
 *  @param param lugar donde se almacena la informacion leida
 *  @return 0
 */
static void FuncUart(void *param){
    uint8_t character;
    UartReadByte(UART_PC, &character);
    if(character=='O'){
        teclaO =! teclaO;
    }
    if(character=='H'){
        teclaH =! teclaH;
    }
    if(character=='o'){
        teclaO =! teclaO;
    }
    if(character=='h'){
        teclaH =! teclaH;
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

    timer_config_t timer_mostrar_y_uart = {
        .timer = TIMER_B,
        .period = TIEMPO_MOSTRAR,
        .func_p = FuncTimerB,
        .param_p = NULL
    };
    TimerInit(&timer_mostrar_y_uart);

    serial_config_t my_uart = {
    .port = UART_PC,
    .baud_rate = 9600,
    .func_p = FuncUart,
    .param_p = NULL
    };
    UartInit(&my_uart);

    /* Creación de tareas */
    xTaskCreate(&LeerSensor, "LeerSensor", 2048, NULL, 5, &lectura_task_handle);
    xTaskCreate(&Mostrar, "Mostrar", 2048, NULL, 5, &mostrar_task_handle);
    xTaskCreate(&UartTask, "UART", 2048, &my_uart, 5, &uart_task_handle);
  
    /* Inicialización del conteo de timers */
    TimerStart(timer_lectura.timer);
    TimerStart(timer_mostrar_y_uart.timer);
}
