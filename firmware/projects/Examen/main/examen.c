/*! @mainpage Examen parcial
 *
 * @section genDesc General Description
 *
 * Se disena un dispsitivo que alerta via LEDs, buzzers y notificaciones cuando un 
 * objeto se esta acercando. Ademas tambien alerta si se sufrio una caida
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	ECHO       	| 	GPIO_3		|
 * | 	TRIGGER	 	| 	GPIO_2		|
 * |    BUZZER      |   GPIO_8      |
 * |  ACELEROMETRO  |   CH1         |
 * |  ACELEROMETRO  |   CH2         |
 * |  ACELEROMETRO  |   CH3         |
 * |    UART        |   GPIO16      |
 * |    UART        |   GPIO17      |
 * |   UART_BT      |   GPIO18      |
 * |   UART_BT      |   GPIO19      |
 * | 	+5V 	 	| 	+5V   		|
 * | 	GND 	 	| 	GND 		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 04/11/2023 | Document creation		                         |
 *
 * @author Alarcon Lucas
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
/** @def TIEMPO_MEDICION
 *  @brief se define el delay para la lectura de la distancia
 */
#define TIEMPO_MEDICION 500000  //dos por segundos 

/** @def TIEMPO_ACELEROMETRO
 *  @brief se define el tiempo para obtener datos del acelerometro
 */
#define TIEMPO_ACELEROMETRO 10000  //100 hz en us

/*==================[internal data definition]===============================*/
/** @def medicion_task_handle
 *  @brief handle de la tarea asociada a la lectura de la medicion
 */
TaskHandle_t medicion_task_handle = NULL;
/** @def distancia
 *  @brief se define la distancia obtenida por el sensor
 */
/** @def acelerometro_task_handle
 *  @brief handle de la tarea asociada al acelerometro
 */
TaskHandle_t acelerometrox_task_handle = NULL;
/** @def acelerometro_task_handle
 *  @brief handle de la tarea asociada al acelerometro
 */
TaskHandle_t acelerometroy_task_handle = NULL;
/** @def acelerometro_task_handle
 *  @brief handle de la tarea asociada al acelerometro
 */
TaskHandle_t acelerometroz_task_handle = NULL;
/** @def acelerometro_task_handle
 *  @brief handle de la tarea asociada al acelerometro
 */
TaskHandle_t acelerometro_task_handle = NULL;
/** @def alertar_task_handle
 *  @brief handle de la tarea alertar
 */
TaskHandle_t alertar_task_handle = NULL;
uint16_t distancia = 0;
/** @def voltaje
 *  @brief es el voltaje asociado a la medicion
 */
uint16_t voltajex = 0;
/** @def voltaje
 *  @brief es el voltaje asociado a la medicion
 */
uint16_t voltajey = 0;
/** @def voltaje
 *  @brief es el voltaje asociado a la medicion
 */
uint16_t voltajez = 0;
/** @def gravedad
 *  @brief es el valor de la gravedad 
 */
uint16_t gravedad = 0;
/** @def gravedad
 *  @brief es el valor de la gravedad en x
 */
uint16_t gravedadx = 0;
/** @def gravedad
 *  @brief es el valor de la gravedad en y
 */
uint16_t gravedady = 0;
/** @def gravedad
 *  @brief es el valor de la gravedad en z
 */
uint16_t gravedadz = 0;
/*==================[internal functions declaration]=========================*/
/**
 * @brief Función invocada en la interrupción del timer A
 */
void FuncTimerA(void* param){
    vTaskNotifyGiveFromISR(medicion_task_handle, pdFALSE);   
	vTaskNotifyGiveFromISR(alertar_task_handle, pdFALSE);    
}
/**
 * @brief Función invocada en la interrupción del timer B
 */
void FuncTimerB(void* param){
    vTaskNotifyGiveFromISR(acelerometrox_task_handle, pdFALSE);
	vTaskNotifyGiveFromISR(acelerometroy_task_handle, pdFALSE); 
	vTaskNotifyGiveFromISR(acelerometroz_task_handle, pdFALSE); 
	vTaskNotifyGiveFromISR(acelerometro_task_handle, pdFALSE);   
}
/** @fn obtenerDistancia 
 *  @brief se lee la distancia del sensor
 *  @return 0
 */
static void obtenerDistancia(void *pvParameter){    
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
/** @fn manejoBuzzer
 *  @brief se prenden el buzzer de un tono u otro dependiendo la distancia
 *  @return 0
 */
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

/** @fn alertaBlueetoth 
 *  @brief se alerta via app dependiendo la distancia de un objeto
 *  @return 0
 */
void alertaBlueetoth () {
	if (distancia>300 && distancia<500){ // dependiendo la distancia alerto de una u otra forma
	UartSendString(UART_CONNECTOR,"Precaución, vehículo cerca");
	UartSendString(UART_CONNECTOR,"r\n");
	}
	if (distancia<300){
	UartSendString(UART_CONNECTOR,"Peligro, vehículo cerca”");
	UartSendString(UART_CONNECTOR,"r\n");
	}  
}
/** @fn ConversionADx 
 * @brief Función que convierte datos analogicos a digital
 */
static void ConversionADx(void *pvParameter){
    while (true){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificacion */
        AnalogInputReadSingle(CH1, &voltajex);  //se lee el voltaje en chanel 1 y se almacena
	}
}
/** @fn ConversionADy 
 * @brief Función que convierte datos analogicos a digital
 */
static void ConversionADy(void *pvParameter){
    while (true){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificacion */
        AnalogInputReadSingle(CH2, &voltajey);  //se lee el voltaje en chanel 1 y se almacena
	}
}
/** @fn ConversionADz 
 * @brief Función que convierte datos analogicos a digital
 */
static void ConversionADz(void *pvParameter){
    while (true){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificacion */
        AnalogInputReadSingle(CH3, &voltajez);  //se lee el voltaje en chanel 1 y se almacena
	}
}

/** @fn obtenergravedad
 *  @brief Función que obtiene la gravedad a partir de los distintos valores
 */
void obtenergravedad(){
	while (true){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificacion */
		gravedadx = voltajex*3.33; //obtengo la gravedad medida por el acelerometro en x
		gravedady = voltajey*3.33; //obtengo la gravedad medida por el acelerometro en y
		gravedadz = voltajez*3.33; //obtengo la gravedad medida por el acelerometro en z
		gravedad = gravedadx+gravedady+gravedadz;
		if(gravedad>4){
			UartSendString(UART_PC,"Caída detectada");
	 		UartSendString(UART_PC,"r\n");
		}
	}
} 

/** @fn alertarCiclistas
 *  @brief Función que ejecuta todas las funciones para alertar al ciclista
 */
static void alertarCiclistas(void *pvParameter){
	while (true){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificacion */
		EncenderLed();
		manejoBuzzer();
		alertaBlueetoth();
	}
}

/*==================[external functions definition]==========================*/
void app_main(void){
	LedsInit(); //Iniciar Leds
    LcdItsE0803Init();  //Iniciar Pantalla
    HcSr04Init(GPIO_3,GPIO_2); //Inicio Sensor
	BuzzerInit(GPIO_8); //Inicio de Buzzer

	analog_input_config_t conversorADx = {
        .input = CH1,
        .mode = ADC_SINGLE,
    };
    AnalogInputInit(&conversorADx);

	analog_input_config_t conversorADy = {
        .input = CH2,
        .mode = ADC_SINGLE,
    };
    AnalogInputInit(&conversorADy);

	analog_input_config_t conversorADz = {
        .input = CH3,
        .mode = ADC_SINGLE,
    };
    AnalogInputInit(&conversorADz);

    AnalogOutputInit();

	serial_config_t my_uart = {  //Incializo la uart
    .port = UART_PC,
    .baud_rate = 9600,
    .param_p = NULL
    };
    UartInit(&my_uart);

	serial_config_t my_uart_bt = {  //Incializo la uart que se usa para la app
    .port = UART_CONNECTOR,
    .baud_rate = 9600,
    .param_p = NULL
    };
    UartInit(&my_uart_bt);

    /* Inicialización de timers */
    timer_config_t timer_lectura = {
        .timer = TIMER_A,
        .period = TIEMPO_MEDICION,
        .func_p = FuncTimerA,
        .param_p = NULL
    };
    TimerInit(&timer_lectura);

	timer_config_t timer_acelerometro = {
        .timer = TIMER_B,
        .period = TIEMPO_ACELEROMETRO,
        .func_p = FuncTimerB,
        .param_p = NULL
    };
    TimerInit(&timer_acelerometro);

    /* Creación de tareas */
    xTaskCreate(&obtenerDistancia, "obtener Distancia", 2048, NULL, 5, &medicion_task_handle);
	xTaskCreate(&ConversionADx, "ConversionADx", 2048, NULL, 5, &acelerometrox_task_handle);
	xTaskCreate(&ConversionADy, "ConversionADy", 2048, NULL, 5, &acelerometroy_task_handle);
	xTaskCreate(&ConversionADz, "ConversionADz", 2048, NULL, 5, &acelerometroz_task_handle);
	xTaskCreate(&obtenergravedad, "obtenerGravedad", 2048, NULL, 5, &acelerometro_task_handle);
	xTaskCreate(&alertarCiclistas, "alertar Ciclistas", 2048, NULL, 5, &alertar_task_handle);
  
    /* Inicialización del conteo de timers */
    TimerStart(timer_lectura.timer);
	TimerStart(timer_acelerometro.timer);
}
/*==================[end of file]============================================*/