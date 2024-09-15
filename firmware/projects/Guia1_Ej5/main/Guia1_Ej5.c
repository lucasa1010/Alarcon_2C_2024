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
#include <gpio_mcu.h>

/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;



/*==================[internal functions declaration]=========================*/
void convertbcdtopin(uint8_t bcd, gpioConf_t *vector){
	uint8_t mascara=1;
	for(int i=0;i<4;i++){
		GPIOInit(vector[i].pin,vector[i].dir);
	}
	for(int i=0; i<4;i++){
		if((mascara & bcd) != 0){
			GPIOOn(vector[i].pin);
		}
		else{
			GPIOOff(vector[i].pin);
		}
		mascara=mascara<<1;	
	}
}



/*==================[external functions definition]==========================*/
void app_main(void){
	uint8_t numero = 2;
	gpioConf_t pines [4] = {{GPIO_20,GPIO_OUTPUT},{GPIO_21,GPIO_OUTPUT},{GPIO_22,GPIO_OUTPUT},{GPIO_23,GPIO_OUTPUT}};
	convertbcdtopin(numero, pines);
}
/*==================[end of file]============================================*/