#include<p18f452.h>
#include<timers.h>
#include<delays.h>
#include"xlcd.h"
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<capture.h>
#include<math.h>

#pragma config  OSC=HS
#pragma config  LVP=OFF
#pragma config  WDT=OFF

void DelayFor18TCY( void )
{
    Delay1TCY();
    Delay1TCY();
    Delay1TCY();
    Delay1TCY();
    Delay10TCYx(1);
}

void DelayPORXLCD (void)
{
 Delay10KTCYx(6);
}

void DelayXLCD (void)
{
 Delay10KTCYx(2); 
}

void low_isr(void);
void high_isr(void);
/*
 **********************************************************************************************************
 *                                  GLOBAL VARIABLES
 **********************************************************************************************************
 */
int result=0;
int result_h=0;
int result_l=0;
int overflow_1=0;
int overflow_2=0;




/*
 *This functions sets the variable result to 'value' which is passes when 'decide' is 1.
 *If 'decide' is 0 it acts as a get function and returns the value of result. 
 */
int set_result(int value,int decide)
{
    int result;
    if(decide==1)           //setta function
    {
         result=value;
    }
    return result;
    
}

/*
 **********************************************************************************************************
 *                               LOW PRIORITY INTERRUPT
 **********************************************************************************************************
 */
#pragma code low_vector=0x18
void interrupt_at_low_vector(void)
{
    _asm
    goto low_isr
    _endasm
}
#pragma code        //return to the default section of code

#pragma interruptlow low_isr
void low_isr(void)
{ 
            //CODE FOR LOW PRIOIRITY INTERRUPTS
}

/*
 **********************************************************************************************************
 *                               HIGH PRIORITY INTERRUPT
 **********************************************************************************************************
 */
#pragma code high_vector=0x08
void interrupt_at_high_vector(void)
{
    _asm
    goto high_isr
    _endasm
}
#pragma code

#pragma interrupt high_isr
void high_isr(void)
{ 
    //unsigned int result;
    INTCONbits.GIE=0;
    if(PIR1bits.CCP1IF==1)      //if capture interrupt is set
    {
        overflow_2=overflow_1;
        PIR1bits.CCP1IF=0;      //clear interrupt flag bit
        result_h=CCPR1H;
        result_l=CCPR1L;
        TMR1H=0;
        TMR1L=0;
        overflow_1=0;
        //dummy=set_result(result,1); //set value to result;
    }
    if(PIR1bits.TMR1IF==1)
    {
        PIR1bits.TMR1IF=0;
        overflow_1++;             
    }
    INTCONbits.GIE=1;
}
/*
 ***********************************************************************************************************
 *                                       FUNCTIONS
 ***********************************************************************************************************  
 */
/*
 *******************************************************************************
 *                                  CONFIGERATION
 *******************************************************************************
 */
/*
 ***********************************************************
 *                        XLCD
 ***********************************************************
 */
void config_LCD(void)
{
    OpenXLCD( FOUR_BIT & LINES_5X7 );
    while( BusyXLCD() );
    WriteCmdXLCD( FOUR_BIT & LINES_5X7 );
    while( BusyXLCD() );
    WriteCmdXLCD( BLINK_ON );
    while( BusyXLCD() );
    WriteCmdXLCD( SHIFT_DISP_LEFT );
}
/*
 **************************************************************************************
 *                              DISPLAY READING 
 **************************************************************************************
 */
void display(char bpm_out[4])
{
    while(BusyXLCD());
    WriteCmdXLCD(0b00000001);

    while(BusyXLCD());
    SetDDRamAddr( 0x00 );
    putrsXLCD( "HRV(ms):");

    while(BusyXLCD());
    SetDDRamAddr( 0x08 );
    putsXLCD(bpm_out);
    
}
/*
 **********************************************************************************************************
 *                                          MAIN
 **********************************************************************************************************
 */
void main(void)
{
    
    char display_value[4];
    unsigned int overflow_factor=0;
    unsigned int time=0;
    unsigned int time_2=0;
    TRISCbits.RC2=1;
    config_LCD();
    OpenCapture1(CAPTURE_INT_ON &
                C1_EVERY_RISE_EDGE);
            
    OpenTimer1(TIMER_INT_ON &
               T1_16BIT_RW &
               T1_PS_1_8 &
               T1_OSC1EN_OFF &
               T1_SYNC_EXT_OFF&
               T1_SOURCE_INT &
            T1_SOURCE_CCP);//T1_SOURCE_CCP
    
    TMR1H=0;
    TMR1L=0;
    RCONbits.IPEN = 1;
    INTCONbits.GIE=1;
    

    while(1)
    {
        overflow_factor=overflow_2*526;
        time=result_h*256;//(((result_h*256)+result_l)*0.008);
        time=time+result_l;
        time=time*0.008;
        time=time+overflow_factor;
        itoa(time,display_value);
        display(display_value);
        Delay10KTCYx(26);
        
        
        //result=set_result(0,0); //set the value of result to result
        
    }
            
            
}

