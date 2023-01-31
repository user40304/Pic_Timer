#include <p18f4620.h>

#include <stdio.h>
#include <stdlib.h>
#include <delays.h>
#include <usart.h>
#include <adc.h>
#include <string.h>

#include "../My_Header_Lib/CCS.h"
#include "../My_Header_Lib/IO_Device.h"

#define STR_MAX	20
#define ISR_CCP1_ENABLE 1
#define ISR_TIMER0_ENABLE 1
#include "../My_Header_Lib/Interrupt.h"

typedef enum
{
    Taiki,
    Keisoku,
    Keisoku_rap,
    Ichijiteishi_rap
} State;

unsigned int count = 0;
State state = Taiki;

int BCD();
void Beep();

void ISR_CCP1(void)
{
    if (count % 50 == 0 && (state == Keisoku || state == Keisoku_rap || state == Ichijiteishi_rap))
    {
        output_toggle(PIN_C2);
    }
    else if (state == Taiki || state == Ichijiteishi_rap)
    {
        output_bit(PIN_C2, 0);
    }
    count++;
}

int _user_putc(char c)
{
    lcd_data(c);
    return 1;
}

int printf_lcd(auto const far rom char *format, ...)
{
    va_list ap;
    va_start(ap, format);

    vfprintf(_H_USER, format, ap);

    va_end(ap);

    return 1;
}

void ISR_TIMER0(void)
{
    output_toggle(PIN_C0);

    set_timer0(46423);
}

void main(void)
{
    char c;
    char str[STR_MAX];

    unsigned int lap = 0;

    int result = 0;
    int count_a = 0;

    Init_RS232C();

    printf("----- Start -----\n");

    lcd_init();

    setup_ccp1(CCP_COMPARE_RESET_TIMER);
    setup_timer_1(T1_INTERNAL | T1_DIV_BY_8);
    CCP_1(12500 - 1);
    disable_interrupts(INT_CCP1);

    setup_timer_0(RTCC_INTERNAL | RTCC_DIV_1);
    set_timer0(0);
    disable_interrupts(INT_TIMER0);

    enable_interrupts(GLOBAL);

    // èoóÕê›íË
    output_d(0x00);

    // ì¸óÕê›íË
    // input_b();

    while (1)
    {
        switch (state)
        {
            case Taiki: // ë“ã@íÜ
                if (input(PIN_B0))
                {
                    enable_interrupts(INT_CCP1);
                    Beep();

                    delay_ms(10);
                    while (input(PIN_B0));
                    delay_ms(10);

                    state = Keisoku;
                }
                else if (input(PIN_B1))
                {
                    Beep();
                    count = 0;
                    lcd_clear();

                    delay_ms(10);
                    while (input(PIN_B1));
                    delay_ms(10);

                }
                break;
            case Keisoku: // åvë™íÜ
                if (input(PIN_B0))
                {
                    disable_interrupts(INT_CCP1);
                    Beep();

                    delay_ms(10);
                    while (input(PIN_B0));
                    delay_ms(10);

                    state = Taiki;
                }
                else if (input(PIN_B1))
                {
                    Beep();

                    lap = count;

                    delay_ms(10);
                    while (input(PIN_B1));
                    delay_ms(10);


                    state = Keisoku_rap;
                }

                break;
            case Keisoku_rap:
                if (input(PIN_B0))
                {
                    disable_interrupts(INT_CCP1);
                    Beep();

                    delay_ms(10);
                    while (input(PIN_B0));
                    delay_ms(10);

                    state = Ichijiteishi_rap;
                }
                else if (input(PIN_B1))
                {
                    Beep();

                    delay_ms(10);
                    while (input(PIN_B1));
                    delay_ms(10);

                    state = Keisoku;
                }

                break;

            case Ichijiteishi_rap: // àÍéûí‚é~íÜ
                if (input(PIN_B0))
                {
                    enable_interrupts(INT_CCP1);
                    Beep();

                    delay_ms(10);
                    while (input(PIN_B0));
                    delay_ms(10);

                    state = Keisoku;
                }
                else if (input(PIN_B1))
                {
                    Beep();

                    delay_ms(10);
                    while (input(PIN_B1));
                    delay_ms(10);

                    state = Taiki;
                }

                break;
            default:
                printf("*** error ***");
                while (1);
                break;
        }

        switch (state)
        {
            case Keisoku_rap:
            case Ichijiteishi_rap:
                if (result == 0)
                {
                    lcd_clear();

                    result = 1;
                }

                LCD_Move_Cursor(1, 1);
                printf("\rstate=%d\n", state);
                printf("\rlap=%d.%d\n", lap / 100, lap % 100);

                sprintf(str, "lap=%d.%d\n", lap / 100, lap % 100);
                Puts_LCD(str);

                break;
            default:
                LCD_Move_Cursor(1, 1);
                printf("\rstate=%d\n", state);
                printf("\rcount=%d.%d\n", count / 100, count % 100);

                sprintf(str, "count=%d.%d\n", count / 100, count % 100);
                Puts_LCD(str);
                
                result = 0;

                break;
        }

        if (count_a % 50 == 0)
        {
            output_toggle(PIN_C1);
        }

        count_a++;
        output_d(BCD());
        delay_ms(10);
    }
}

void Beep()
{
    enable_interrupts(INT_TIMER0);
    delay_ms(50);
    disable_interrupts(INT_TIMER0);
}

int BCD()
{
    int count_y;
    int count_10;
    int count_a;

    count_y = count / 10 % 100;

    count_10 = count_y / 10;
    count_a = count_y % 10;

    return count_10 << 4 | count_a;
}