/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Función que reciba un dato de 32 bits,  la cantidad de dígitos de salida 
 * y un puntero a un arreglo donde se almacene los n dígitos.
 * La función deberá convertir el dato recibido a BCD, guardando cada uno de los dígitos 
 * de salida en el arreglo pasado como puntero.
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
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
{
	for(int i=0; i<digits;i++){
		bcd_number[i]=data%10;    //almacena cada componente del numero en una posicion del vector
		data=data/10;
	}
return 1;
}

/*==================[external functions definition]==========================*/
void app_main(void){
	uint8_t vector[3];
	uint32_t numero=123;
	uint8_t numeros=3;
	convertToBcdArray(numero,numeros,&vector);	
		printf("\nEl numero: ");
		printf ("%d",vector[0]);
			printf("\nEl numero: ");
		printf ("%d" ,vector[1]);
			printf("\nEl numero: ");
		printf ("%d",vector[2]);
}
/*==================[end of file]============================================*/