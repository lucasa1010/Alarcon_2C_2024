/*! @mainpage Ejercicio 1
 *
 * @section genDesc General Description
 *
 * A traves de la medicicion de un sensor mostrarla en un lcd
 * y encender distintos leds dependiendo la misma 
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
#define REFRESCO_EJECUCION 500
/*==================[internal data definition]===============================*/
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
uint8_t teclaO = 0;
/** @def HOLD
 *  @brief se define el estado de la tecla 2
 */
uint8_t HOLD = 0;
/*==================[internal functions declaration]=========================*/

/** @fn LeerSensor 
 *  @brief se lee la medida del sensor
 *  @return 0
 */
static void LeerSensor(){
    while (true){
        medida = HcSr04ReadDistanceInCentimeters(); 
        vTaskDelay(REFRESCO_LECTURA / portTICK_PERIOD_MS);
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

/** @fn Mostrar 
 *  @brief se muestra la medida
 *  @return 0
 */
static void Mostrar(void *pvParameter){
    while (true){
        if (teclaO == ON){
            EncenderLed();
            MostrarPantalla();
        }
        else{
            LedsOffAll();
        }
        if (HOLD == ON){
            EncenderLed();
        }
    vTaskDelay(REFRESCO_EJECUCION/ portTICK_PERIOD_MS);     
    }
}

/** @fn PresionarTecla
 *  @brief se lee la tecla medida y cambia su condicion 
 *  @return 0
 */
static void PresionarTecla(void *pvParameter){
    while (true){
        tecla = SwitchesRead();
        switch (tecla)
        {
        case (SWITCH_1): teclaO = ON; 
            HOLD = 0;
            break;
        
        case (SWITCH_2): HOLD = 1;
            teclaO = OFF;
            break;
        }

    vTaskDelay(REFRESCO_TECLA / portTICK_PERIOD_MS);    
    }
}

/*==================[external functions definition]==========================*/
void app_main(void){
    LedsInit(); //Iniciar Leds
    LcdItsE0803Init();  //Iniciar Pantalla
    HcSr04Init(GPIO_3,GPIO_2); //Inicio Sensor
    SwitchesInit(); //Inicio las teclas
    xTaskCreate(&PresionarTecla, "PresionarTecla", 2048, NULL, 5, NULL);
    xTaskCreate(&LeerSensor, "LeerSensor", 2048, NULL, 5, NULL);
    xTaskCreate(&Mostrar, "Mostrar", 2048, NULL, 5, NULL);
}
