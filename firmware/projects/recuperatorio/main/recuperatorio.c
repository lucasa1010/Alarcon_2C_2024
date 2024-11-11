/*! @mainpage Recuperatorio
 *
 * @section genDesc General Description
 *
 * Se solicita hacer una balanza para un camion, donde se informe el peso y la velocidad maxima que este alcanzo.
 * Aparte se solcita advertir a traves de uso de LEDs la velocidad a la que va
 *
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	ECHO       	| 	GPIO_3		|
 * | 	TRIGGER	 	| 	GPIO_2		|
 * |    PUERTA      |   GPIO_22     |
 * |    UART        |   GPIO16      |
 * |    UART        |   GPIO17      |
 * | 	+5V 	 	| 	+5V   		|
 * | 	GND 	 	| 	GND 		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 11/11/2024 | Document creation		                         |
 *
 * @author Lucas Alarcon (lucas.alarcon@uner.edu.ar)
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
/** @def TIEMPO_INFORMAR
 *  @brief se define el delay para informar sobre las medidad
 */
#define TIEMPO_INFORMAR 250000 // informo al usuario cada 50 muestras
/** @def TIEMPO_BALANZA
 *  @brief se define el delay para la lectura del peso en la balanza
 */
#define TIEMPO_BALANZA 5000       // 200 muestras por segundo
/** @def GPIO_PUERTA
 *  @brief se define el gpio asociado a la puerta 
 */
#define GPIO_PUERTA GPIO_22     // LED de aviso de sensado IR
/*==================[internal data definition]===============================*/
/** @def lectura_task_handle
 *  @brief handle de la tarea asociada a la lectura de la medicion
 */
TaskHandle_t lectura_task_handle = NULL;
/** @def balanza_task_handle
 *  @brief handle de la tarea asociada a la lectura del peso
 */
TaskHandle_t balanza_task_handle = NULL;
/** @def informar_task_handle
 *  @brief handle de la tarea asociada a informar al operador
 */
TaskHandle_t informar_task_handle = NULL;
/** @def medida
 *  @brief se define la medida obtenida por el sensor
 */
uint16_t medida = 0;
/** @def velocidad
 *  @brief se define la velocidad obtenidad
 */
float velocidad = 0;
/** @def velocidad
 *  @brief se define la velocidad maxima obtenidad
 */
float velocidad_maxima = 0;
/** @def peso_delanteroV
 *  @brief se define el peso obtenido en V
 */
uint16_t peso_delanteroV=0;
/** @def peso_traseroV
 *  @brief se define el peso obtenido en V
 */
uint16_t peso_traseroV=0;
/** @def peso_delantero_total
 *  @brief se define el peso acumulado en el tren delantero
 */
uint32_t peso_delantero_total = 0;
/** @def peso_trasero_total
 *  @brief se define el peso acumulado en el tren trasero
 */
uint32_t peso_trasero_total = 0;
/** @def peso_total
 *  @brief se define el peso promedio obtenido
 */
uint16_t peso_total = 0;
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
    vTaskNotifyGiveFromISR(balanza_task_handle, pdFALSE);    
	vTaskNotifyGiveFromISR(informar_task_handle, pdFALSE);    
}
/** @fn calcularVelocidad
 * @brief Función que calcula la velocidad solo si esta dentro de cierto rango
 */
void calcularVelocidad(){
	if(medida<1000){
		velocidad = medida/0.1; //se calcula la velocidad en cm/s ya que tomo 10 medidas por segundo el tiempo sera siempre el mismo
		velocidad = (velocidad)/100;   //se calcula la velocidad en m/s
		if(velocidad>8){ //dependiendo la velocidad enciendo y apago ciertos LEDs
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
		if(velocidad>velocidad_maxima){
			velocidad_maxima=velocidad;
		}
	}
}
/** @fn LeerSensor 
 *  @brief se lee la medida del sensor
 *  @return 0
 */
static void LeerSensor(void* param){    
    while (true){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificación */
        medida = HcSr04ReadDistanceInCentimeters(); 
		calcularVelocidad();
    }
}
/** @fn obtencionPesos 
 *  @brief se obtiene los pesos de cada tren 
 *  @return 0
 */
static void obtencionPesos(void* param){
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificación */
	if(velocidad==0){
		AnalogInputReadSingle(CH1, &peso_delanteroV);
		AnalogInputReadSingle(CH2, &peso_traseroV);
		peso_delanteroV=6060*(peso_delanteroV/1000); //conversion de mV a V y de V a Kg
		peso_traseroV=6060*(peso_traseroV/1000);
		peso_delantero_total=peso_delantero_total+peso_delanteroV; //acumulador
		peso_trasero_total=peso_trasero_total+peso_traseroV; //acumulador
	}
}
/** @fn informar_operario
 *  @brief infromo al operario el peso y la velocidad maxima
 *  @return 0
 */
static void informar_operario(void* param){
	peso_trasero_total=peso_trasero_total/50;
	peso_delantero_total=peso_delantero_total/50;
	peso_total = peso_delantero_total + peso_trasero_total;
	UartSendString(UART_PC,"Peso: ");
	UartSendString(UART_PC, (char*)UartItoa(peso_total, 10)); //se envia la cadena 
	UartSendString(UART_PC," kg");
	UartSendString(UART_PC, "\r\n"); //marcar el final de la transmision
	UartSendString(UART_PC,"Velocidad maxima: ");
	UartSendString(UART_PC, (char*)UartItoa(velocidad_maxima, 10)); //se envia la cadena 
	UartSendString(UART_PC," m/s");
	UartSendString(UART_PC, "\r\n"); //marcar el final de la transmision.
	peso_delantero_total = 0; //reinicio los acumuladores
	peso_trasero_total = 0;
}
/** @fn FuncUart 
 *  @brief se leen las teclas pulsadas y dependiendo esto abro o cierro la puerta
 *  @param param lugar donde se almacena la informacion leida
 *  @return 0
 */
static void FuncUart(void *param){
    uint8_t character;
    UartReadByte(UART_PC, &character);
    if(character=='O'){
        GPIOOn(GPIO_PUERTA);
    }
    if(character=='C'){
        GPIOOff(GPIO_PUERTA);
    }
    if(character=='o'){
        GPIOOn(GPIO_PUERTA);
    }
    if(character=='c'){
        GPIOOff(GPIO_PUERTA);
    }
} 
/*==================[external functions definition]==========================*/
void app_main(void){
	LedsInit(); //Iniciar Leds
    HcSr04Init(GPIO_3,GPIO_2); //Inicio Sensor
	GPIOInit(GPIO_PUERTA, GPIO_OUTPUT); //Inicializo el GPIO de la puerta como que saldran datos

	/* Inicialización de timers */
    timer_config_t timer_lectura = {
        .timer = TIMER_A,
        .period = TIEMPO_LECTURA,
        .func_p = FuncTimerA,
        .param_p = NULL
    };
    TimerInit(&timer_lectura);

	timer_config_t timer_balanza = {
        .timer = TIMER_B,
        .period = TIEMPO_BALANZA,
        .func_p = FuncTimerB,
        .param_p = NULL
    };
    TimerInit(&timer_balanza);

	timer_config_t timer_informar = {
        .timer = TIMER_B,
        .period = TIEMPO_INFORMAR,
        .func_p = FuncTimerB,
        .param_p = NULL
    };
    TimerInit(&timer_informar);

	//Inicializacion de conversores
	analog_input_config_t conversorADdelantero = {
        .input = CH1,
        .mode = ADC_SINGLE,
    };
    AnalogInputInit(&conversorADdelantero);

	analog_input_config_t conversorADtrasero = {
        .input = CH2,
        .mode = ADC_SINGLE,
    };
    AnalogInputInit(&conversorADtrasero);

	serial_config_t my_uart = {  //Incializo la uart
    .port = UART_PC,
    .baud_rate = 9600,
	.func_p = FuncUart,
    .param_p = NULL
    };
    UartInit(&my_uart);

	/* Creación de tareas */
    xTaskCreate(&LeerSensor, "LeerSensor", 2048, NULL, 5, &lectura_task_handle);
	xTaskCreate(&obtencionPesos, "ObtenerPesos", 2048, NULL, 5, &balanza_task_handle);
	xTaskCreate(&informar_operario, "Informar", 2048, NULL, 5, &informar_task_handle);

	/* Inicialización del conteo de timers */
    TimerStart(timer_lectura.timer);
	TimerStart(timer_balanza.timer);
	TimerStart(timer_informar.timer);
}
/*==================[end of file]============================================*/