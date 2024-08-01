/*! @mainpage Blinking
 *
 * \section genDesc General Description
 *
 * This example makes LED_1 blink.
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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 500
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){
    LedsInit();
    while(true){
        printf("Se prende el LED\n");
        LedOn(LED_2);
        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
        printf("Se apaga el LED\n");
        LedOff(LED_2);
        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
    }
}
/*==================[end of file]============================================*/
