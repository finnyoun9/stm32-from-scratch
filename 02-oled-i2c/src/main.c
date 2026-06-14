#include "stm32f1xx.h"
#include "ssd1306.h"

#define PWR_VCC_PIN   7
#define PWR_GND_PIN   6

static volatile uint32_t g_ticks = 0;
void SysTick_Handler(void) { g_ticks++; }

static void delay_ms(uint32_t ms) {
    uint32_t start = g_ticks;
    while ((g_ticks - start) < ms);
}

static void i2c_init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    GPIOB->CRL &= ~((uint32_t)0xF << (PWR_GND_PIN * 4));
    GPIOB->CRL |=  ((uint32_t)GPIO_OUT_PP_2MHZ << (PWR_GND_PIN * 4));
    GPIOB->BSRR = GPIO_BSRR_BR(PWR_GND_PIN);
    GPIOB->CRL &= ~((uint32_t)0xF << (PWR_VCC_PIN * 4));
    GPIOB->CRL |=  ((uint32_t)GPIO_OUT_PP_2MHZ << (PWR_VCC_PIN * 4));
    GPIOB->BSRR = GPIO_BSRR_BS(PWR_VCC_PIN);

    GPIOB->CRH &= ~((uint32_t)0xFF << 0);
    GPIOB->CRH |=  ((uint32_t)0xFF << 0);

    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    AFIO->MAPR   |= AFIO_MAPR_I2C1_REMAP;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    I2C1->CR2   = 8;
    I2C1->CCR   = 40;
    I2C1->TRISE = 9;
    I2C1->CR1   = I2C_CR1_PE;
}

static const uint8_t g_days_in_month[] = {31,28,31,30,31,30,31,31,30,31,30,31};

/* Draw a 2-digit number with big font at pixel position (x,y) */
static void draw_big2(uint8_t x, uint8_t y, uint8_t n) {
    ssd1306_draw_big_char(x,      y, (char)('0' + n / 10));
    ssd1306_draw_big_char(x + 10, y, (char)('0' + n % 10));
}

/* ── Entry ──────────────────────────────────────────────────── */
int main(void) {
    SystemInit();

    /* SysTick 1 ms */
    SYSTICK->LOAD = 7999;
    SYSTICK->VAL  = 0;
    SYSTICK->CTRL = SYSTICK_CTRL_ENABLE | SYSTICK_CTRL_TICKINT | SYSTICK_CTRL_CLKSOURCE;

    i2c_init();
    delay_ms(100);
    ssd1306_init();

    /* Parse __DATE__ "Mmm dd yyyy" and __TIME__ "hh:mm:ss" */
    static const char compile_date[] = __DATE__;
    static const char compile_time[] = __TIME__;
    static const char months[12][4] = {"Jan","Feb","Mar","Apr","May","Jun",
                                       "Jul","Aug","Sep","Oct","Nov","Dec"};
    uint8_t month = 1;
    for (uint8_t m = 0; m < 12; m++) {
        if (compile_date[0] == months[m][0] &&
            compile_date[1] == months[m][1] &&
            compile_date[2] == months[m][2]) { month = (uint8_t)(m + 1); break; }
    }
    uint8_t  day   = (uint8_t)((compile_date[4] == ' ' ? 0 : (compile_date[4]-'0')*10) + (compile_date[5]-'0'));
    uint16_t year  = (uint16_t)((compile_date[7]-'0')*1000 + (compile_date[8]-'0')*100
                              + (compile_date[9]-'0')*10   + (compile_date[10]-'0'));
    uint8_t  hour  = (uint8_t)((compile_time[0]-'0')*10 + (compile_time[1]-'0'));
    uint8_t  min   = (uint8_t)((compile_time[3]-'0')*10 + (compile_time[4]-'0'));
    uint8_t  sec   = (uint8_t)((compile_time[6]-'0')*10 + (compile_time[7]-'0'));
    uint32_t total_sec = (uint32_t)hour * 3600 + min * 60 + sec;
    uint32_t last_ms = g_ticks;

    while (1) {
        /* Tick once per second */
        if (g_ticks - last_ms >= 1000) {
            last_ms += 1000;
            total_sec++;

            /* Roll over time */
            uint32_t t = total_sec;
            uint8_t sec  = (uint8_t)(t % 60); t /= 60;
            uint8_t min  = (uint8_t)(t % 60); t /= 60;
            uint8_t hour = (uint8_t)(t % 24);
            uint32_t days = t / 24;

            /* Roll over date (lazy: add days one at a time) */
            while (days--) {
                day++;
                uint8_t max_d = g_days_in_month[month - 1];
                /* Leap year Feb */
                if (month == 2 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)))
                    max_d = 29;
                if (day > max_d) { day = 1; month++; }
                if (month > 12) { month = 1; year++; }
            }

            /* ── Render ────────────────────────────────────── */
            ssd1306_clear();

            /* Time: HH:MM:SS in big font, centered */
            /* 8 digits * 10px + 2 colons * 5px + 1px gaps = 80+10+? ≈ 125 */
            /* Layout: HH (20px) : (5px) MM (20px) : (5px) SS (20px) = 70px, centered at 29 */
            draw_big2(2,  4, hour);
            draw_big2(49, 4, min);
            draw_big2(96, 4, sec);
            ssd1306_draw_big_colon(25, 4);
            ssd1306_draw_big_colon(72, 4);

            /* Separator line (page 3 = pixel rows 24-31) */
            for (uint8_t i = 0; i < 128; i++)
                ssd1306_draw_pixel(i, 35, 1);

            /* Date: YYYY-MM-DD (small font, centered on page 5) */
            char date_buf[11];
            date_buf[0]  = (char)('0' + year / 1000);
            date_buf[1]  = (char)('0' + (year / 100) % 10);
            date_buf[2]  = (char)('0' + (year / 10) % 10);
            date_buf[3]  = (char)('0' + year % 10);
            date_buf[4]  = '-';
            date_buf[5]  = (char)('0' + month / 10);
            date_buf[6]  = (char)('0' + month % 10);
            date_buf[7]  = '-';
            date_buf[8]  = (char)('0' + day / 10);
            date_buf[9]  = (char)('0' + day % 10);
            date_buf[10] = '\0';

            /* 10 chars * 6px = 60px, centered at (128-60)/2 = 34 */
            ssd1306_draw_string(34, 5, date_buf);

            /* Weekday (page 7) */
            /* Zeller-like: 2026-06-01 = Monday */
            static const char wd[7][4] = {"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
            /* Compute day-of-week from known anchor */
            const uint16_t month_offsets[12] = {0,3,3,6,1,4,6,2,5,0,3,5};
            uint8_t dow = (uint8_t)((year + year/4 - year/100 + year/400
                          + month_offsets[month-1] + day) % 7);
            /* Adjust: 2026-06-01 = Mon, tweak offset */
            dow = (dow + 4) % 7;
            ssd1306_draw_string(44, 7, wd[dow]);

            ssd1306_refresh();
        }
    }
}
