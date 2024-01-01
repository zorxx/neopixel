/* \copyright 2023 Zorxx Software. All rights reserved.
 * \license This file is released under the MIT License. See the LICENSE file for details.
 * \brief ESP32 Neopixel Driver
 */
#include <stdint.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/i2s_std.h"
#include "driver/i2s_common.h"
#include "ws2182b_protocol.h"
#include "neopixel.h"

#define TAG "neopixel"
#define I2S_TIMEOUT_TICKS 1000

typedef struct sNpContext
{
   portMUX_TYPE lock;
   SemaphoreHandle_t signal;
   i2s_chan_handle_t i2s;
   uint32_t pixels;
   bool terminate;
   bool update;

   uint8_t *buffer;
   uint32_t bufferSize;
}  tNpContext;

static void neopixel_task(void *arg);
static void setpixel(uint8_t *buffer, uint32_t index, uint32_t rgb);

/* -------------------------------------------------------------------------------------------------------------
 * Exported Functions
 */

tNeopixelContext *neopixel_Init(uint32_t pixels, int dout_pin)
{
   tNpContext *c = NULL; 
   i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
   i2s_std_config_t std_cfg = {
      .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(WS2812B_BITRATE / 16 / 2), /* 16-bit, 2 channels (stereo) per slot */
      .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
      .gpio_cfg = {
         .mclk = I2S_GPIO_UNUSED,
         .bclk = I2S_GPIO_UNUSED,
         .ws =  I2S_GPIO_UNUSED,
         .dout = dout_pin, 
         .din = I2S_GPIO_UNUSED,
         .invert_flags = {
            .mclk_inv = false,
            .bclk_inv = false,
            .ws_inv = false,
         },
      },
   };

   c = (tNpContext *) malloc(sizeof(*c));
   if(NULL == c)
   {
      ESP_LOGE(TAG, "Failed to allocate context");
      return NULL;
   }
   memset(c, 0, sizeof(*c));
   c->pixels = pixels;
   c->bufferSize = (c->pixels * WS2182B_BYTES_PER_PIXEL) + WS2812B_RESET_BYTES;
   portMUX_INITIALIZE(&c->lock);
   c->signal = xSemaphoreCreateBinary();
   c->terminate = false;
   c->update = false;

   c->buffer = (uint8_t *)malloc(c->bufferSize);
   memset(c->buffer, 0, c->bufferSize); /* initializes the reset bytes to zero */
   for(int i = 0; i < c->pixels; ++i)
      setpixel(c->buffer, i, NP_RGB(0, 0, 0));  /* turn off all pixels */

   i2s_new_channel(&chan_cfg, &c->i2s, NULL);  /* Tx channel only (no Rx) */
   i2s_channel_init_std_mode(c->i2s, &std_cfg);

   xTaskCreate(&neopixel_task, TAG, 1024, (void *)c, 5, NULL);

   return (tNeopixelContext) c;
}

void neopixel_Deinit(tNeopixelContext ctx)
{
   tNpContext *c = (tNpContext*) ctx;
   if(NULL == c)
      return;
   c->terminate = true;
   xSemaphoreGive(c->signal); /* thread does cleanup */
}

bool neopixel_SetPixel(tNeopixelContext ctx, tNeopixel *pixel, uint32_t pixelCount)
{
   tNpContext *c = (tNpContext*) ctx;
   bool success = true;
      
   taskENTER_CRITICAL(&c->lock);
   for(uint32_t i = 0; i < pixelCount; ++i)
   {
      tNeopixel *p = &pixel[i];
      if(p->index >= c->pixels)
      {
         ESP_LOGI(TAG, "Invalid pixel (%" PRIu32 ")", p->index);
         success = false;
      }
      else
         setpixel(c->buffer, p->index, p->rgb);
   }
   c->update = true;
   taskEXIT_CRITICAL(&c->lock);
   xSemaphoreGive(c->signal);
   return success;
}

uint32_t neopixel_GetRefreshRate(tNeopixelContext ctx)
{
   tNpContext *c = (tNpContext*) ctx;
   return WS2812B_BITRATE / (c->bufferSize * 8); 
}

/* -------------------------------------------------------------------------------------------------------------
 * Helper Functions
 */

static void neopixel_task(void *arg)
{
   tNpContext *c = (tNpContext*) arg;
   uint8_t *buffer;
   bool done;

   buffer = (uint8_t *)malloc(c->bufferSize);
   if(NULL == buffer)
   {
      ESP_LOGE(TAG, "[%s] Failed to allocate buffer", __func__);
      return;
   }

   ESP_LOGD(TAG, "[%s] Started", __func__);
   while(!c->terminate) 
   {
      /* block task, waiting for an update */
      if(xSemaphoreTake(c->signal, portMAX_DELAY) != pdTRUE)
      {
         vTaskDelay(pdMS_TO_TICKS(10)); /* prevent tight loops */
         continue;
      }

      done = false;
      while(!done)
      {
         taskENTER_CRITICAL(&c->lock);
         if(c->update)
         {
            /* Make a local copy of the current pixel buffer to be sent to the hardware */
            memcpy(buffer, c->buffer, c->bufferSize);
            c->update = false;
         }
         else
         {
            /* Even though the buffer didn't change, send the most recent buffer
               one more time before blocking to wait for an update. This
               seems to be necessary based on the behavior of i2s_channel_write */
            done = true;
         }
         taskEXIT_CRITICAL(&c->lock);

         i2s_channel_enable(c->i2s);
         i2s_channel_write(c->i2s, buffer, c->bufferSize, NULL, I2S_TIMEOUT_TICKS);
         i2s_channel_disable(c->i2s);
      }
   }
   ESP_LOGD(TAG, "[%s] Finished", __func__);

   free(buffer);

   /* Destroy context */
   free(c->buffer);
   i2s_del_channel(c->i2s);
   free(c);
   vTaskDelete(NULL);
}

static void setpixel(uint8_t *buffer, uint32_t index, uint32_t rgb)
{
   uint32_t offset = index * WS2182B_BYTES_PER_PIXEL;
   const uint8_t *sequence = ws2812b_color_map[NP_RGB2GREEN(rgb)];
   for(int i = 0; i < WS2182B_BYTES_PER_PIXEL; ++i, ++offset)
   {
      if(i == 3)
         sequence = ws2812b_color_map[NP_RGB2RED(rgb)];
      if(i == 6)
         sequence = ws2812b_color_map[NP_RGB2BLUE(rgb)];
      buffer[offset ^ 1] = sequence[i % WS2182B_BYTES_PER_COLOR];  /* Fill buffer in 16-bit little-endian format */
   }
}
