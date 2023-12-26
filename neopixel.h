/* \copyright 2023 Zorxx Software. All rights reserved.
 * \license This file is released under the MIT License. See the LICENSE file for details.
 * \brief ESP32 Neopixel Driver Library API
 */
#ifndef _ESP32_NEOPIXEL_H
#define _ESP32_NEOPIXEL_H

#include <stdint.h>
#include <stdbool.h>

typedef void *tNeopixelContext;

#define NP_RGB2RED(rgb)   (((rgb) & 0xFF0000UL) >> 16)
#define NP_RGB2GREEN(rgb) (((rgb) & 0x00FF00UL) >> 8)
#define NP_RGB2BLUE(rgb)  ((rgb) & 0x0000FFUL)
#define NP_RGB(r, g, b)   ( ((uint32_t)(r) & 0xFF) << 16  \
                       | ((uint32_t)(g) & 0xFF) << 8   \
                       | ((uint32_t)(b) & 0xFF) )

typedef struct sNeopixel 
{
   uint32_t index;
   uint32_t rgb;
} tNeopixel;

/*! \brief Create a neopixel context
  * \param pixels Number of pixels
  * \param dout_pin Physical pin to send neopixel data (e.g. GPIO_NUM_27) 
  * \returns Pointer to neopixel context, used as the first parameter
  *          to subsequent neopixel function calls
  */
tNeopixelContext *neopixel_Init(uint32_t pixels, int dout_pin);

/*! \brief Get minimum number of ticks between neopixel_SetPixel calls
 *  \param ctx Neopixel context received from successful neopixel_Init calls
 *  \returns Minimum number of ticks to wait between neopixel_SetPixel calls
 *           to ensure each neopixel_SetPixel call is displayed. If the time
 *           delta between neopixel_SetPixel calls is less than this, some
 *           neopixel_SetPixel data will simply not be displayed; no other
 *           ill-effects will result.
 */
uint32_t neopixel_GetRefreshRate(tNeopixelContext ctx);

/*! \brief Destroy an existing neopixel context and all associated resources 
 *  \param ctx Neopixel context received from successful neopixel_Init calls
 */ 
void neopixel_Deinit(tNeopixelContext ctx);

/*! \brief Set one or more pixels 
 *  \param ctx Neopixel context received from successful neopixel_Init calls
 *  \param pixel Pointer to array of tNeopixel, pixels to set
 *  \param pixelCount Number of pixels in tNeopixel array
 *  \returns true on success, false on failure
 */ 
bool neopixel_SetPixel(tNeopixelContext ctx, tNeopixel *pixel, uint32_t pixelCount);

#endif /* _ESP32_NEOPIXEL_H */
