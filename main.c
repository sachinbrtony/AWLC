#include<Config.h>
#include<stdint.h>
#include "I2C.h"
#include "OLED.h"
#include<stdio.h>
#include<stdlib.h>

#define SSD1306_128_32

int TANKWAR,SUMPWAR,WL,dryflag=0;
char* motor;
char* mode;
char* TWL;
char* SWL;
//char* ALM;
void LEDchaser()
{
    RA2=1;
    __delay_ms(40);
    RA2=0;
    __delay_ms(40);
    RA3=1;
    __delay_ms(40);
    RA3=0;
    __delay_ms(40);
    RA4=1;
    __delay_ms(40);
    RA4=0;
    __delay_ms(40);
}

void oled_puts(const char* c, uint8_t size)
{
    while(*c != '\0')
    {
        SSD1306_PutStretchC(*c, size);
        c++;
    }
}

void setupADC()
{
    ADCON1 = 0x80;
    ADCON0 = 0x00;
    __delay_ms(30);
    ADCON0=0b10010011;          //10010011
    ADCON1=0b10000000;
}
int calculateADC()
{
    GO_nDONE = 1; //Initializes A/D Conversion
    while(GO_nDONE); //Wait for A/D Conversion to complete;
    int val = (ADRESH<<8)+ADRESL;  //SAVE ADC DATA AND STORE
    return val;
}

void Display_ADC()
{
    char* ALM;
    char* acvolt[10];
    float ACvolt;
    
    setupADC();
    ACvolt=calculateADC();
    __delay_ms(30);
    
    ACvolt=(ACvolt)*1.19;      //Calculating AC voltage from ADC reading
    itoa(acvolt,ACvolt,10); //Converting integer voltage value to text
    
    //ACvolt=ACvolt*3;
    
    if(ACvolt<=130)
    {
        dryflag=2;
    }
    else if(ACvolt>=310)
    {
        dryflag=3;
    }
    __delay_ms(30);
    
    if(SUMPWAR==1)
    {
        dryflag=4;
    }
    else
    {
        dryflag=0;
    }

    SSD1306_GotoXY(1,1);
    oled_puts("MODE: ", 1);
    SSD1306_GotoXY(6,1);
    oled_puts(mode, 1);

    SSD1306_GotoXY(1,2);
    oled_puts("MOTOR:", 1);
    SSD1306_GotoXY(6,2);
    oled_puts(motor, 1);
    
    SSD1306_GotoXY(16,1);
    oled_puts("AC IN", 1);
    SSD1306_GotoXY(17,2);
    oled_puts(acvolt, 1);

    SSD1306_GotoXY(1,4);
    oled_puts("TANK:", 1);
    SSD1306_GotoXY(1,5);
    oled_puts(TWL, 2);

    SSD1306_GotoXY(12,4);
    oled_puts("SUMP:", 1);
    SSD1306_GotoXY(10,5);
    oled_puts(SWL, 2);

    if(dryflag==2)
    {
        ALM="Low Mains Volt";
    }
    else if(dryflag==3)
    {
        ALM="High Mains Volt";
    }
    else if(dryflag==4)
    {
        ALM="No Water in Sump";
    }
    else
    {
        ALM="I'm Working Good";
    }
    
    SSD1306_GotoXY(1,7);
    oled_puts("ALM:", 1);
    SSD1306_GotoXY(6,7);
    oled_puts(ALM, 1);

    /*  char* mtrtime[15];

      itoa(mtrtime,mtimer,10);

      SSD1306_GotoXY(1,8);
      oled_puts("Time:", 1);
      SSD1306_GotoXY(6,8);
      oled_puts(mtrtime, 1);
      */

}

void Call_display()
{
    SSD1306_GotoXY(1,1);
    oled_puts("ENMAD", 2);
    SSD1306_GotoXY(1,5);
    oled_puts("SOLUTIONS", 2);
    LEDchaser();
    LEDchaser();
    __delay_ms(50);
}


void Motor_On()
{
    int abc=0;
    ReadSensor();

    if( (SUMPWAR==0) && (dryflag==0))
    {
        abc=1;
        RA1=0;
    }
    else
    {
        abc=0;
        RA1=1;
        motor=":OFF    ";
    }

    while(abc==1)
    {
        ReadSensor();
        RC5 = 1;              //Motor Relay1 On
        motor=":Running";
        Display_ADC();
        LEDchaser();

        if(SUMPWAR==1 || TANKWAR==1 || WL== 100)
        {
            abc=0;
            RC5 = 0;
            motor=":OFF    ";
            Display_ADC();
        }
    }
}


int ReadSensor()
{
    //dry = RB2;    //DryRun Status
//over Head tank Sensor/Water level Reading Start---------------------------------------------

    if(RB1==0 && RB0==0 && RC7==0 && RC6==0 )
    {
        TWL="FULL";
        WL= 100;
        TANKWAR=0;
    }
    else if(RB1==0 && RB0==0 && RC7==0 && RC6==1 )
    {
        TWL="75%";
        WL= 75;
        TANKWAR=0;
    }
    else if(RB1==0 && RB0==0 && RC7==1 && RC6==1 )
    {
        TWL="50%";
        WL= 50;
        TANKWAR=0;
    }
    else if(RB1==0 && RB0==1 && RC7==1 && RC6==1 )
    {
        TWL="25%";
        WL= 25;
        TANKWAR=0;
    }
    else if(RB1==1 && RB0==1 && RC7==1 && RC6==1 )
    {
        TWL="00%";
        WL= 0;
        TANKWAR=0;
    }
    else
    {
        
        TWL="Err";
        TANKWAR=1;
        WL=1;
    }
//OverHead Tank Sensor/Water level Reading End---------------------------------------------

//Sump Sensor/Water level Reading ---------------------------------------------
    if(RB5==0 && RB4==0 && RB3==0)
    {
        SWL="100%";
        SUMPWAR=0;
        RA4=0;
    }
    else if(RB5==0 && RB4==0 && RB3==1)
    {
        SWL="50% ";
        SUMPWAR=0;
        RA4=0;
    }
    else if(RB5==0 && RB4==1 && RB3==1)
    {
        SWL="25% ";
        SUMPWAR=0;
        RA4=0;
    }
    else if(RB5==1 && RB4==1 && RB3==1)
    {
        SWL="00% ";
        SUMPWAR=1;
        RA4=1;
        
    }
    else
    {
        SWL="Err";
        SUMPWAR=1;
    }
    
    __delay_ms(25);
    
    if(SUMPWAR==1 || TANKWAR==1)
    {
        RA1=1;
    }
    else
    {
        RA1=0;
    }

//Sump Sensor/Water level Reading End---------------------------------------------

    return(WL);
}


void Init()
{
    //ANSEL= 0x00;         //DISABLE ADC IN PORTA
    ANSELH = 0x00;       //DISABLE ADC IN PORTB
    TRISB5 = 1;     //S25%
    TRISB4 = 1;     //S50%
    TRISB3 = 1;     //S100%------------------------
    TRISB1 = 1;     //T25%          //INPUTS
    TRISB0 = 1;     //T50%
    TRISC7 = 1;     //T75%
    TRISC6 = 1;     //T100%------------------------
    TRISA5 = 1;     //AC volt reaD-----------------
    TRISB2 = 1;     //DRY RUN----------------------
    TRISC1 = 1;     //MANUAL
    TRISC0 = 1;     //AUTO-------------------------
    TRISC5 = 0;     //RELAY1        //OUTPUTS
    TRISC2 = 0;     //RELAY2
    TRISA4 = 0;     //S LOW LED
    TRISA2 = 0;     //DRY RUN LED
    TRISA1 = 0;     // BUZZER
    TRISA3 = 0;     //HIGH/LOW LED
    TRISA4 = 0;     //S-LOW LED
}

int main()
{
    
    Init();
    __delay_ms(50);
    I2C_Initialize(200);
    SSD1306_Init(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);
    SSD1306_ClearDisplay();
    Call_display();
    __delay_ms(50);
    SSD1306_ClearDisplay();

    while(1)
    {
        WDTCON=0b00010111;
        __delay_ms(25);

        if(RC5==1)
        {
            motor=":Running";
        }
        else
        {
            motor=":OFF    ";
        }

        __delay_ms(25);
        if(RC1==0)
        {
            mode="Manual";
            motor=":Running";
            LEDchaser();
            Display_ADC();
        }
        else if (RC0==0)
        {
            mode="Auto  ";
        }
        else if(RC0==1 && RC1==1)
        {
            mode="OFF   ";
            motor=":OFF    ";
        }

        Display_ADC();
        //ReadSensor();
        int wl=ReadSensor();
        switch(wl)
        {
        case 100:
            TWL="Full";
            //motor=":OFF      ";
            Display_ADC();
            __delay_ms(40);
            break;
        case 75:
            TWL="75% ";
            Display_ADC();
            __delay_ms(40);
            break;
        case 50:
            TWL="50%  ";
            Display_ADC();
            //if(TANKWAR==1)
            //{
            //    break;
            //}
            Motor_On();
            __delay_ms(40);
            break;
        case 25:
            TWL="25%  ";
            Display_ADC();
            Motor_On();
            __delay_ms(40);
            break;
        case 0:
            TWL="00%  ";
            Display_ADC();
            __delay_ms(40);
            break;
        case 1:
            TWL="Err";
            Display_ADC();
            TANKWAR=1;
            __delay_ms(40);
            break;            
                

        default:

            break;
        }
        CLRWDT();
        //SSD1306_ClearDisplay();
    }
}