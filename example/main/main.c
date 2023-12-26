/* \copyright 2023 Zorxx Software. All rights reserved.
 * \license This file is released under the MIT License. See the LICENSE file for details.
 * \brief ESP32 Neopixel Driver Library Example Application
 */
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "neopixel.h"

#define PIXEL_COUNT  256
#define NEOPIXEL_PIN GPIO_NUM_27

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

void app_main(void)
{
   tNeopixelContext neopixel;
   uint32_t iteration;
   uint32_t refreshRate;
   uint32_t taskDelay;

   neopixel = neopixel_Init(PIXEL_COUNT, NEOPIXEL_PIN);
   refreshRate = neopixel_GetRefreshRate(neopixel);
   ESP_LOGI(__func__, "Refresh rate: %" PRIu32, refreshRate);

   iteration = 0;
   taskDelay = MAX(1, pdMS_TO_TICKS(1000UL / refreshRate));
   for(;;)
   {
      tNeopixel pixel[] =
      {
          { (iteration)   % PIXEL_COUNT, NP_RGB(0, 0,  0) },
          { (iteration+5) % PIXEL_COUNT, NP_RGB(0, 50, 0) }, /* green */
      };
      neopixel_SetPixel(neopixel, pixel, ARRAY_SIZE(pixel)); 
      ++iteration;
      vTaskDelay(taskDelay);
   }

   neopixel_Deinit(neopixel);
}
