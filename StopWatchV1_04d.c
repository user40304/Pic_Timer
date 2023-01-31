#include <p18f4620.h>

#include <stdio.h>
#include <stdlib.h>
#include <delays.h>
#include <usart.h>
#include <adc.h>
#include <string.h>

#include "../My_Header_Lib/CCS.h"
#include "../My_Header_Lib/IO_Device.h"

#define ISR_CCP1_ENABLE 1
#include "../My_Header_Lib/Interrupt.h"

unsigned int count = 0;

void ISR_CCP1(void)
{
    count++;
}


typedef enum
{
    Taiki,
    Keisoku,
    Keisoku_rap,
    Ichijiteishi_rap
} State;

void main(void)
{
    State state = Taiki;
    char c;
    unsigned int lap = 0;

    Init_RS232C();

    printf("----- Start -----\n");

    setup_ccp1(CCP_COMPARE_RESET_TIMER);

    setup_timer_1(T1_INTERNAL | T1_DIV_BY_8);

    CCP_1(12500- 1);

    disable_interrupts(INT_CCP1);

    set_timer1(0);

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
                    delay_ms(10);
                    while (input(PIN_B0));
                    delay_ms(10);

                    enable_interrupts(INT_CCP1);
                    state = Keisoku;
                }else if(input(PIN_B1)){
                    delay_ms(10);
                    while (input(PIN_B0));
                    delay_ms(10);

                    count = 0;
                }
                break;
            case Keisoku: // åvë™íÜ
                if (input(PIN_B0))
                {
                    delay_ms(10);
                    while (input(PIN_B0));
                    delay_ms(10);

                    disable_interrupts(INT_CCP1);
                    state = Taiki;
                }else if(input(PIN_B1)){    
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
                    delay_ms(10);
                    while (input(PIN_B0));
                    delay_ms(10);

                    disable_interrupts(INT_CCP1);
                    state = Ichijiteishi_rap;
                }else if(input(PIN_B1)){
                    
                    delay_ms(10);
                    while (input(PIN_B1));
                    delay_ms(10);
                    
                    state = Keisoku;
                }

                break;

            case Ichijiteishi_rap: // àÍéûí‚é~íÜ
                if (input(PIN_B0))
                {
                    delay_ms(10);
                    while (input(PIN_B0));
                    delay_ms(10);

                    enable_interrupts(INT_CCP1);
                    state = Keisoku;
                }
                else if (input(PIN_B1))
                {
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

        switch (state){
            case Keisoku_rap:
                printf("state=%d\n", state);
                printf("count=%d.%d\n", lap / 100, lap % 100);

                break;

            case Ichijiteishi_rap: // àÍéûí‚é~íÜ

                printf("state=%d\n", state);
                printf("count=%d.%d\n", lap / 100, lap % 100);

                break;
            default:
                printf("state=%d\n", state);
                printf("count=%d.%d\n", count / 100, count % 100);

                break;
        }

        delay_ms(100);
    }
}

