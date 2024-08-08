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
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
/*==================[macros and definitions]=================================*/

#define OFF 0 //defino las variables para las distintas opciones
#define ON 1
#define TOOGLE 2

/*==================[internal data definition]===============================*/
struct leds
{
    uint8_t mode;        // ON, OFF, TOGGLE
	uint8_t n_led;       // indica el número de led a controlar
	uint8_t n_ciclos;    // indica la cantidad de ciclos de ncendido/apagado
	uint16_t periodo;    // indica el tiempo de cada ciclo
} my_leds;

/*==================[internal functions declaration]=========================*/

void EncenderApagarToogle(struct leds *led){
	int retardo = 100;

switch (led->mode)
{
case (ON):  //prendo todos los led si el caso es afirmativo 
switch (led->n_led)
{
case 1:
LedOn(LED_1);
	break;
case 2:
LedOn(LED_2);
	break;
	case 3:
LedOn(LED_3);
	break;
}
break;

case (OFF): //apago todos los leds si el caso es negativo
switch (led->n_led)
{
case 1:
LedOff(LED_1);
	break;
case 2:
LedOff(LED_2);
	break;
	case 3:
LedOff(LED_3);
	break;
}
break;

case (TOOGLE):
for(int i=0; i<led->n_ciclos; i++){ //se repite la cantidad de ciclos que se informa
		if(led->n_led==1){ //segun el led seleccionado
			LedToggle(LED_1); //cambia el estado del led
			for(int j=0; j<(led->periodo/retardo); j++){ //hago un delay 
			vTaskDelay(100 / portTICK_PERIOD_MS);
			}
		}
		if(led->n_led==2){
			LedToggle(LED_2);
			for(int j=0; j<(led->periodo/retardo); j++){
			vTaskDelay(100/ portTICK_PERIOD_MS);
			}
		}
		if(led->n_led==3){
		LedToggle(LED_3);
		for(int j=0; j<(led->periodo/retardo); j++){
		vTaskDelay(100 / portTICK_PERIOD_MS);
			}
		}
	}
	break;
}
}

/*==================[external functions definition]==========================*/
void app_main(void){

struct leds *led;
led = &my_leds;

LedsInit();

led->mode=TOOGLE;
led->n_led=3;
led->n_ciclos=100;
led->periodo=500;

EncenderApagarToogle(led);
}
/*==================[end of file]============================================*/