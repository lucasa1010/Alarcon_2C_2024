/** @mainpage Ejercio numero 6
 * Mostrar a traves de un LCD un numero que nosotros seleccionemos
 *
 * @section genDesc General Description
 *
 * El programa recibe un numero y su longitud, a partir de aqui entra a la funcion
 * que convierte un bcd a un lcd que a su vez llama dos funciones, una primera que separa 
 * al BCD en sus digitos y una segunda que manda sus digitos a una posicion del LCD
 *
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	D1       	| 	GPIO_20		|
 * | 	D2  	 	| 	GPIO_21		|
 * | 	D3  	 	| 	GPIO_22		|
 * | 	D4  	 	| 	GPIO_23		|
 * | 	SEL_1	 	| 	GPIO_19		|
 * | 	SEL_2	 	| 	GPIO_18		|
 * | 	SEL_3	 	| 	GPIO_9		|
 * | 	+5V 	 	| 	+5V   		|
 * | 	GND 	 	| 	GND 		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 29/08/2024 | Document creation		                         |
 *
 * @author Lucas Alarcon (lucasalarcon872@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <gpio_mcu.h>
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/** @def gpioConf_t
 *  @brief configuracion de cada pin
 */
typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;


/** @def pines
 *  @brief se definen la posicion de los pines para el 7 segmentos
 */
gpioConf_t pines [4] = {{GPIO_20,GPIO_OUTPUT},{GPIO_21,GPIO_OUTPUT},{GPIO_22,GPIO_OUTPUT},{GPIO_23,GPIO_OUTPUT}};

/** @def mux
 *  @brief se definen la posicion donde se encuentra cada 7 segmento
 */
gpioConf_t mux [3] = {{GPIO_19,GPIO_OUTPUT},{GPIO_18,GPIO_OUTPUT},{GPIO_9,GPIO_OUTPUT}};

/*==================[internal functions declaration]=========================*/

/** @fn convertToBcdArray(uint32_t data, uint8_t digits, uint8_t * bcd_number)
 *  @brief fragmenta un numero en la cantidad de digitos que tiene y asigna cada numero a un arreglo
 *  @param data numero que recibe
 *  @param digits cantidad de digitos que tiene
 *  @param bcd_number vector donde se almacenan los digitos
 *  @return 1
 */
int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
{
	for(int i=0; i<digits;i++){
		bcd_number[digits-1-i]=data%10;  //convierto el numero a un vector ordenado 
		data=data/10;
	}
return 1;
}

/** @fn convertbcdtopin(uint8_t bcd, gpioConf_t *vector)
 *  @brief permite que un 7 segmentos interprete un numero y lo grafique
 *  @param bcd numero a graficar
 *  @param vector vector donde se almacena la informacion para que el 7 segmentos grafique
 *  @return 
 */
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

/** @fn convertbcdtopin(uint32_t numero, uint8_t digitos,gpioConf_t *pins, gpioConf_t *mux)
 *  @brief permite que un LCD interprete un numero y lo grafique
 *  @param numero numero a graficar
 *  @param digitos cantidad de digitos que tiene el numero
 *  @param pins pines de cada 7 segmentos
 *  @param mux pin donde se encuentra cada 7 segmento 
 *  @return 
 */
void convertbcdtolcd(uint32_t numero, uint8_t digitos,gpioConf_t *pins, gpioConf_t *mux){
	uint8_t vec[3];
	convertToBcdArray(numero, digitos, vec);  //obtengo el numero por partes
	for(uint8_t i=0; i<digitos; i++){
		GPIOInit(mux[i].pin, mux[i].dir);   //inicio los gpio
	}
	for(uint8_t i=0; i<digitos; i++){
		convertbcdtopin(vec[i], pins); //convierto un solo numero a pin 
		GPIOOn(mux[i].pin); //mando ese numero al 7 segmetnos correspondiente
		GPIOOff(mux[i].pin);
	}
}

/*==================[external functions definition]==========================*/

void app_main(void){
	uint8_t digitos = 3;
	uint32_t numero =777;
	convertbcdtolcd(numero, digitos, pines, mux);
}

/*==================[end of file]============================================*/