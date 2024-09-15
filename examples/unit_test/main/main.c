/* \copyright 2023 Zorxx Software. All rights reserved.
 * \license This file is released under the MIT License. See the LICENSE file for details.
 * \brief ESP32 Neopixel Driver Library Example Application
 */
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "neopixel.h"

#define TAG "neopixel_test"
#define PIXEL_COUNT  256
#if defined(CONFIG_IDF_TARGET_ESP32S3)
   #define NEOPIXEL_PIN GPIO_NUM_48
#elif defined(CONFIG_IDF_TARGET_ESP32C6)
   #define NEOPIXEL_PIN GPIO_NUM_8
#else
   #define NEOPIXEL_PIN GPIO_NUM_27
#endif

#if !defined(MAX)
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

static bool test1(uint32_t iterations)
{
   tNeopixelContext neopixel = neopixel_Init(PIXEL_COUNT, NEOPIXEL_PIN);
   tNeopixel pixel[] =
   {
       { 0, NP_RGB(50, 0,  0) }, /* red */
       { 0, NP_RGB(0,  50, 0) }, /* green */
       { 0, NP_RGB(0,  0, 50) }, /* blue */
       { 0, NP_RGB(0,  0,  0) }, /* off */
   };

   if(NULL == neopixel)
   {
      ESP_LOGE(TAG, "[%s] Initialization failed\n", __func__);
      return false;
   }

   ESP_LOGI(TAG, "[%s] Starting", __func__);
   for(int iter = 0; iter < iterations; ++iter)
   {
      for(int i = 0; i < ARRAY_SIZE(pixel); ++i)
      {
         neopixel_SetPixel(neopixel, &pixel[i], 1);
         vTaskDelay(pdMS_TO_TICKS(200));
      }
   }
   ESP_LOGI(TAG, "[%s] Finished", __func__);

   neopixel_Deinit(neopixel);
   return true;
}

static bool test2(uint32_t iterations)
{
   tNeopixelContext neopixel = neopixel_Init(PIXEL_COUNT, NEOPIXEL_PIN);
   uint32_t refreshRate, taskDelay;

   if(NULL == neopixel)
   {
      ESP_LOGE(TAG, "[%s] Initialization failed\n", __func__);
      return false;
   }

   refreshRate = neopixel_GetRefreshRate(neopixel);
   taskDelay = MAX(1, pdMS_TO_TICKS(1000UL / refreshRate));
   ESP_LOGI(TAG, "[%s] Starting", __func__);
   for(int i = 0; i < iterations * PIXEL_COUNT; ++i)
   {
      tNeopixel pixel[] =
      {
          { (i)   % PIXEL_COUNT, NP_RGB(0, 0,  0) },
          { (i+5) % PIXEL_COUNT, NP_RGB(0, 50, 0) }, /* green */
      };
      neopixel_SetPixel(neopixel, pixel, ARRAY_SIZE(pixel));
      vTaskDelay(taskDelay);
   }
   ESP_LOGI(TAG, "[%s] Finished", __func__);
   neopixel_Deinit(neopixel);
   return true;
}

void app_main(void)
{
   for(;;)
   {
      test1(10);
      test2(10);
   }
}
