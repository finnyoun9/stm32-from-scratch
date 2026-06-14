#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>

#define SSD1306_WIDTH   128
#define SSD1306_HEIGHT  64
#define SSD1306_PAGES   (SSD1306_HEIGHT / 8)   /* 8 pages of 8 pixels */

void ssd1306_init(void);
void ssd1306_clear(void);
void ssd1306_refresh(void);                      /* flush buffer to display */
void ssd1306_draw_pixel(uint8_t x, uint8_t y, uint8_t color);
void ssd1306_draw_char(uint8_t x, uint8_t page, char c);
void ssd1306_draw_big_char(uint8_t x, uint8_t y, char c);
void ssd1306_draw_big_colon(uint8_t x, uint8_t y);
void ssd1306_draw_string(uint8_t x, uint8_t page, const char *s);
void ssd1306_draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);

#endif
