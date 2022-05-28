#include <stdio.h>
#include "tremo_regs.h"
#include "tremo_lcd.h"
#include "tremo_rcc.h"
#include "tremo_gpio.h"


/* LCD connect to chip */
/*********************
LCD pin      Chip pin
COM1         LCD_COM0
COM2         LCD_COM1
COM3         LCD_COM2
COM4         LCD_COM3
2            LCD_SEG5
3            LCD_SEG7
4            LCD_SEG3
5            LCD_SEG4
6            LCD_SEG2
7            LCD_SEG8
8            LCD_SEG10
9            LCD_SEG11
10           LCD_SEG6
11           LCD_SEG22
12           LCD_SEG23
13           LCD_SEG17
14           LCD_SEG14
15           LCD_SEG15
16           LCD_SEG16
17           LCD_SEG18
***********************/

/**
================================================================================
                              VN-2646 GLASS LCD MAPPING
================================================================================
						A
          ------
          |    |
         F|    |B
          |    |
          --G-- 
          |    |
         E|    |C
          |    |
          ------
						D

0 LCD number coding is based on the following matrix:
             COM0    COM1    COM2    COM3
  SEG(n)    { D ,     E ,     G ,     F }
  SEG(n+1)  { 0 ,     C ,     B ,     A }

The number 0 for example is:
-----------------------------------------------------------
             COM0    COM1    COM2    COM3
  SEG(n)    { 1 ,     1 ,     0 ,     1 }
  SEG(n+1)  { 0 ,     1 ,     1 ,     1 }
   --------------------------------------------------------
           =  1       3       2       3 hex

   => '0' = 0x1323

*/

const uint32_t g_mask[] =
{
    0xF000, 0x0F00, 0x00F0, 0x000F
};
const uint8_t g_shift[4] =
{
    12, 8, 4, 0
};

uint8_t g_digit[4];     /* Digit frame buffer */

/* number map of the custom LCD */
uint32_t g_number_map[10] =
{
    /*  0       1       2       3       4   */
    0x1323, 0x0220, 0x1132, 0x1232, 0x0231,
    /*  5       6       7       8       9   */
    0x1213, 0x1313, 0x0222, 0x1333, 0x1233
};

gpio_t *lcd_gpiox;
uint32_t lcd_pin;

void convert(uint8_t * c)
{
    uint32_t ch = 0, tmp = 0;
    uint16_t i;

    if ((*c < (uint8_t)0x3a) && (*c > (uint8_t)0x2f))
    {
        ch = g_number_map[*c - (uint8_t)0x30];
    }
    for (i = 0; i < 4; i++)
    {
        tmp = ch & g_mask[i];
        g_digit[i] = (uint8_t)(tmp >> (uint8_t)g_shift[i]);
    }
}

void lcd_write_char(uint8_t * ch, uint8_t position)
{
    convert(ch);

    switch (position)
    {
        case 0:
            lcd_write_com_seg_bit(0, 5, g_digit[0] & 0x1);

            lcd_write_com_seg_bit(1, 5, g_digit[1] & 0x1);
            lcd_write_com_seg_bit(1, 7, (g_digit[1] >> 1) & 0x1);

            lcd_write_com_seg_bit(2, 5, g_digit[2] & 0x1);
            lcd_write_com_seg_bit(2, 7, (g_digit[2] >> 1) & 0x1);

            lcd_write_com_seg_bit(3, 5, g_digit[3] & 0x1);
            lcd_write_com_seg_bit(3, 7, (g_digit[3] >> 1) & 0x1);
            break;

        case 1:
            lcd_write_com_seg_bit(0, 3, g_digit[0] & 0x1);

            lcd_write_com_seg_bit(1, 3, g_digit[1] & 0x1);
            lcd_write_com_seg_bit(1, 4, (g_digit[1] >> 1) & 0x1);

            lcd_write_com_seg_bit(2, 3, g_digit[2] & 0x1);
            lcd_write_com_seg_bit(2, 4, (g_digit[2] >> 1) & 0x1);

            lcd_write_com_seg_bit(3, 3, g_digit[3] & 0x1);
            lcd_write_com_seg_bit(3, 4, (g_digit[3] >> 1) & 0x1);
            break;

        case 2:
            lcd_write_com_seg_bit(0, 2, g_digit[0] & 0x1);

            lcd_write_com_seg_bit(1, 2, g_digit[1] & 0x1);
            lcd_write_com_seg_bit(1, 8, (g_digit[1] >> 1) & 0x1);

            lcd_write_com_seg_bit(2, 2, g_digit[2] & 0x1);
            lcd_write_com_seg_bit(2, 8, (g_digit[2] >> 1) & 0x1);

            lcd_write_com_seg_bit(3, 2, g_digit[3] & 0x1);
            lcd_write_com_seg_bit(3, 8, (g_digit[3] >> 1) & 0x1);
            break;

        case 3:
            lcd_write_com_seg_bit(0, 10, g_digit[0] & 0x1);

            lcd_write_com_seg_bit(1, 10, g_digit[1] & 0x1);
            lcd_write_com_seg_bit(1, 11, (g_digit[1] >> 1) & 0x1);

            lcd_write_com_seg_bit(2, 10, g_digit[2] & 0x1);
            lcd_write_com_seg_bit(2, 11, (g_digit[2] >> 1) & 0x1);

            lcd_write_com_seg_bit(3, 10, g_digit[3] & 0x1);
            lcd_write_com_seg_bit(3, 11, (g_digit[3] >> 1) & 0x1);
            break;

        case 4:
            lcd_write_com_seg_bit(0, 6, g_digit[0] & 0x1);

            lcd_write_com_seg_bit(1, 6, g_digit[1] & 0x1);
            lcd_write_com_seg_bit(1, 22, (g_digit[1] >> 1) & 0x1);

            lcd_write_com_seg_bit(2, 6, g_digit[2] & 0x1);
            lcd_write_com_seg_bit(2, 22, (g_digit[2] >> 1) & 0x1);

            lcd_write_com_seg_bit(3, 6, g_digit[3] & 0x1);
            lcd_write_com_seg_bit(3, 22, (g_digit[3] >> 1) & 0x1);
            break;

        case 5:
            lcd_write_com_seg_bit(0, 23, g_digit[0] & 0x1);

            lcd_write_com_seg_bit(1, 23, g_digit[1] & 0x1);
            lcd_write_com_seg_bit(1, 17, (g_digit[1] >> 1) & 0x1);

            lcd_write_com_seg_bit(2, 23, g_digit[2] & 0x1);
            lcd_write_com_seg_bit(2, 17, (g_digit[2] >> 1) & 0x1);

            lcd_write_com_seg_bit(3, 23, g_digit[3] & 0x1);
            lcd_write_com_seg_bit(3, 17, (g_digit[3] >> 1) & 0x1);
            break;

        case 6:
            lcd_write_com_seg_bit(0, 14, g_digit[0] & 0x1);

            lcd_write_com_seg_bit(1, 14, g_digit[1] & 0x1);
            lcd_write_com_seg_bit(1, 15, (g_digit[1] >> 1) & 0x1);

            lcd_write_com_seg_bit(2, 14, g_digit[2] & 0x1);
            lcd_write_com_seg_bit(2, 15, (g_digit[2] >> 1) & 0x1);

            lcd_write_com_seg_bit(3, 14, g_digit[3] & 0x1);
            lcd_write_com_seg_bit(3, 15, (g_digit[3] >> 1) & 0x1);
            break;

        case 7:
            lcd_write_com_seg_bit(0, 16, g_digit[0] & 0x1);

            lcd_write_com_seg_bit(1, 16, g_digit[1] & 0x1);
            lcd_write_com_seg_bit(1, 18, (g_digit[1] >> 1) & 0x1);

            lcd_write_com_seg_bit(2, 16, g_digit[2] & 0x1);
            lcd_write_com_seg_bit(2, 18, (g_digit[2] >> 1) & 0x1);

            lcd_write_com_seg_bit(3, 16, g_digit[3] & 0x1);
            lcd_write_com_seg_bit(3, 18, (g_digit[3] >> 1) & 0x1);
            break;

        default:
            break;
    }
}

void lcd_display_string(uint8_t* ptr)
{
    uint8_t i = 0x3;

    while ((*ptr != 0) & (i < 7))
    {
        lcd_write_char(ptr, i);
        ptr++;

        i++;
    }
}

void lcd_set_gpio(void)
{
    lcd_gpiox = GPIOA;
    lcd_pin = GPIO_PIN_4;
    gpio_init(lcd_gpiox, lcd_pin, GPIO_MODE_ANALOG);
    lcd_pin = GPIO_PIN_5;
    gpio_init(lcd_gpiox, lcd_pin, GPIO_MODE_ANALOG);
    lcd_pin = GPIO_PIN_8;
    gpio_init(lcd_gpiox, lcd_pin, GPIO_MODE_ANALOG);
    lcd_pin = GPIO_PIN_9;
    gpio_init(lcd_gpiox, lcd_pin, GPIO_MODE_ANALOG);
    lcd_pin = GPIO_PIN_10;
    gpio_init(lcd_gpiox, lcd_pin, GPIO_MODE_ANALOG);

    lcd_gpiox = GPIOB;
    lcd_pin = GPIO_PIN_7;
    gpio_init(lcd_gpiox, lcd_pin, GPIO_MODE_ANALOG);
    lcd_pin = GPIO_PIN_8;
    gpio_init(lcd_gpiox, lcd_pin, GPIO_MODE_ANALOG);
    lcd_pin = GPIO_PIN_9;
    gpio_init(lcd_gpiox, lcd_pin, GPIO_MODE_ANALOG);
    lcd_pin = GPIO_PIN_10;
    gpio_init(lcd_gpiox, lcd_pin, GPIO_MODE_ANALOG);
    lcd_pin = GPIO_PIN_11;
    gpio_init(lcd_gpiox, lcd_pin, GPIO_MODE_ANALOG);
    lcd_pin = GPIO_PIN_12;
    gpio_init(lcd_gpiox, lcd_pin, GPIO_MODE_ANALOG);
    lcd_pin = GPIO_PIN_14;
    gpio_init(lcd_gpiox, lcd_pin, GPIO_MODE_ANALOG);
    lcd_pin = GPIO_PIN_15;
    gpio_init(lcd_gpiox, lcd_pin, GPIO_MODE_ANALOG);

    lcd_gpiox = GPIOC;
    lcd_pin = GPIO_PIN_3;
    gpio_init(lcd_gpiox, lcd_pin, GPIO_MODE_ANALOG);
    lcd_pin = GPIO_PIN_4;
    gpio_init(lcd_gpiox, lcd_pin, GPIO_MODE_ANALOG);
    lcd_pin = GPIO_PIN_8;
    gpio_init(lcd_gpiox, lcd_pin, GPIO_MODE_ANALOG);
    lcd_pin = GPIO_PIN_9;
    gpio_init(lcd_gpiox, lcd_pin, GPIO_MODE_ANALOG);
    lcd_pin = GPIO_PIN_10;
    gpio_init(lcd_gpiox, lcd_pin, GPIO_MODE_ANALOG);
    lcd_pin = GPIO_PIN_12;
    gpio_init(lcd_gpiox, lcd_pin, GPIO_MODE_ANALOG);
    lcd_pin = GPIO_PIN_13;
    gpio_init(lcd_gpiox, lcd_pin, GPIO_MODE_ANALOG);
}


void lcd_display_test()
{
    uint8_t str_array[8] = {'6', '6', '0', '1'};

    lcd_set_gpio();

    lcd_init(LCD_1_4_DUTY, LCD_1_3_BIAS, LCD_1_8_PRESCALER, LCD_1_23_DIVISION);

    lcd_config_blink_sel(LCD_DISABLE_BLINK);

    lcd_config_dead_cycle(LCD_0_DEAD_CYCLE);
		
    lcd_enable_switch(true);
    lcd_config_large_current_num(LCD_1_LARGE_CURRENT_NUM);

    for (int i = 0; i <= LCD_COM_3; i++)
    {
        lcd_clear_com_seg_state(i);
    }

    lcd_enable(true);

    //display 'A'
    lcd_write_com_seg_bit(LCD_COM_0, 5, 0);

    lcd_write_com_seg_bit(LCD_COM_1, 5, 1);
    lcd_write_com_seg_bit(LCD_COM_1, 7, 1);

    lcd_write_com_seg_bit(LCD_COM_2, 5, 1);
    lcd_write_com_seg_bit(LCD_COM_2, 7, 1);

    lcd_write_com_seg_bit(LCD_COM_3, 5, 1);
    lcd_write_com_seg_bit(LCD_COM_3, 7, 1);


    //display 'S'
    lcd_write_com_seg_bit(LCD_COM_0, 3, 1);

    lcd_write_com_seg_bit(LCD_COM_1, 3, 0);
    lcd_write_com_seg_bit(LCD_COM_1, 4, 1);

    lcd_write_com_seg_bit(LCD_COM_2, 3, 1);
    lcd_write_com_seg_bit(LCD_COM_2, 4, 0);

    lcd_write_com_seg_bit(LCD_COM_3, 3, 1);
    lcd_write_com_seg_bit(LCD_COM_3, 4, 1);


    //display 'R'
    lcd_write_com_seg_bit(LCD_COM_0, 2, 0);

    lcd_write_com_seg_bit(LCD_COM_1, 2, 1);
    lcd_write_com_seg_bit(LCD_COM_1, 8, 1);

    lcd_write_com_seg_bit(LCD_COM_2, 2, 1);
    lcd_write_com_seg_bit(LCD_COM_2, 8, 1);

    lcd_write_com_seg_bit(LCD_COM_3, 2, 1);
    lcd_write_com_seg_bit(LCD_COM_3, 8, 1);


    lcd_display_string(str_array);

    lcd_enable_com(LCD_USE_4_COM);
    lcd_enable_seg(2, 7);
    lcd_enable_seg(10, 2);
    lcd_enable_seg(14, 5);
    lcd_enable_seg(22, 2);
    lcd_enable_analog();
}

int main(void)
{
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_AFEC, true);

    // enable the clk
    rcc_enable_oscillator(RCC_OSC_XO32K, true);

    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOA, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOB, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOC, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOD, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_LCD, true);

    lcd_display_test();

    /* Infinite loop */
    while (1) { }
}

#ifdef USE_FULL_ASSERT
void assert_failed(void* file, uint32_t line)
{
    (void)file;
    (void)line;

    while (1) { }
}
#endif
