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

int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
{
	for(int i=0; i<digits;i++){
		bcd_number[i]=data%10;
		data=data/10;
	
	}
return 1;
}

void convertbcdtolcd(uint8_t bcd, gpioConf_t *vector, uint8_t digits, gpioConf_t *LCD){

}
/*==================[external functions definition]==========================*/
void app_main(void){
	gpioConf_t pines1 [4] = {{GPIO_20,GPIO_OUTPUT},{GPIO_21,GPIO_OUTPUT},{GPIO_22,GPIO_OUTPUT},{GPIO_23,GPIO_OUTPUT}};
	gpioConf_t pines2 [3] = {{GPIO_19,GPIO_OUTPUT},{GPIO_18,GPIO_OUTPUT},{GPIO_9,GPIO_OUTPUT}};

}
/*==================[end of file]============================================*/