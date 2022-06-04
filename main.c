#include<Config.h>
#include "I2C.h"
#include "OLED.h"
#define SSD1306_128_32

int dry,SUMPWAR,WL;
int dryflag=0;
char* motor;
char* mode;
char* TWL;
char* SWL;

void oled_puts(const char* c, uint8_t size) 
{
    while(*c != '\0') 
    {
        SSD1306_PutStretchC(*c, size);
        c++;
    }
}

void Display_ADC()
{
    //SSD1306_ClearDisplay();
    SSD1306_GotoXY(1,1);
    oled_puts("MODE ", 1);
    SSD1306_GotoXY(6,1);
    oled_puts(mode, 1);
    
    SSD1306_GotoXY(1,2);
    oled_puts("MOTOR:", 1);
    SSD1306_GotoXY(6,2);
    oled_puts(motor, 1);
    
    SSD1306_GotoXY(16,1);
    oled_puts("AC IN", 1);
    SSD1306_GotoXY(17,2);
    oled_puts("000", 1);    
    
    SSD1306_GotoXY(1,4);
    oled_puts("TANK:", 1);
    SSD1306_GotoXY(1,5);
    oled_puts(TWL, 2);
    
    SSD1306_GotoXY(12,4);
    oled_puts("SUMP:", 1);
    SSD1306_GotoXY(10,5);
    oled_puts(SWL, 2);    
    
    SSD1306_GotoXY(1,7);
    oled_puts("ALM:", 1);
    SSD1306_GotoXY(6,7);
    oled_puts("NONE", 1); 


    //SSD1306_GotoXY(1,3);
    //oled_puts("a25:", 2);
    //SSD1306_GotoXY(10,3);
    //oled_puts(a25, 2);    
    
    //__delay_ms(500);
     
  
}

void Call_display()
{
        SSD1306_GotoXY(1,1);
        oled_puts("ENMAD", 2);
        SSD1306_GotoXY(1,5);
        oled_puts("SOLUTIONS", 2);
      //  SSD1306_GotoXY(5,2);
     //   oled_puts("very", 3);
      //  SSD1306_GotoXY(2,5);
      //  oled_puts("BEST", 4);
        __delay_ms(5000); 
}



void Motor_On()
{
  int flag=0;
  int abc=0;
  ReadSensor();
  
if(dry==0)
{
   dryflag=0;
}

  if((SUMPWAR==0) && (dryflag<1))
  {
    abc=1;
  }
  else
  {
    abc=0;
  }
  
  while(abc==1)
  {
    //Serial.println("inside While");
    RC2 = 1;              //Motor Relay1 On
    motor=":Running";
    //SSD1306_ClearDisplay();
    Display_ADC();

    flag=RB1;
    if(flag == 1 )
    {
        for(int z=0;z<=10 ;z++)
        {
          //drawPercentbar( 10, 8, 90, 8,p1);
            flag=RB1;
            ReadSensor();
            if(dry==0 || SUMPWAR==1 || WL== 100)
            {
                z=0;
                break;
            }
            //Serial.println(z);
            //Serial.println(dryflag);
            //Serial.println(dry);
            __delay_ms(380);
        }
        dryflag=5;
        RC2 = 0;
        motor=":OFF     ";
        //SSD1306_ClearDisplay();
        abc=0;
    }
    ReadSensor();

    if(SUMPWAR==1 || WL== 100)
    {
      abc=0;
      RC2 = 0;
      motor=":OFF";
      //SSD1306_ClearDisplay();
    }

  }
}




int ReadSensor()
{
    dry = RB1;
 
  if(RB1==0 && RB0==0 && RC7==0 && RC6==0 )
  {
    TWL="FULL";
    WL= 100;
  }
 
    else if(RB1==0 && RB0==0 && RC7==0 && RC6==1 )
  {
    TWL="75%";
    WL= 75;
  }
 
    else if(RB1==0 && RB0==0 && RC7==1 && RC6==1 )
  {
    TWL="50%";
    WL= 50;
  }
    else if(RB1==0 && RB0==1 && RC7==1 && RC6==1 )
  {
    TWL="25%";
    WL= 25;
  }
    else if(RB1==1 && RB0==1 && RC7==1 && RC6==1 )
  {
    TWL="00%";
    WL= 0;
  }
  else
  {
    TWL="Error/Wire Cut";
  }

  
  if(RB5==1 && RB4==1 && RB3==1)
  {
    SWL="000%";
    SUMPWAR=0;
  }
  else if(RB5==0 && RB4==0 && RB3==1)
  {
    SWL="50%";
    SUMPWAR=0;
  }
  else if(RB5==1 && RB4==0 && RB3==0)
  {
    SWL="25%";
    SUMPWAR=0;
  }
  else if(RB5==1 && RB4==1 && RB3==1)
  {
    SWL="00%";
    SUMPWAR=1;
  }
  else
  {
    SWL="no i/p";
  }
  
 return(WL); 
}

int main()
{
    TRISB1 = 1;     //T25%          //INPUTS
    TRISB0 = 1;     //T50%
    TRISC7 = 1;     //T75%
    TRISC6 = 1;     //T100%
    
    TRISB5 = 1;     //S25%
    TRISB4 = 1;     //S50%
    TRISB3 = 1;     //S100%
    
    TRISB2 = 1;     //DRY RUN
    
    TRISC1 = 1;     //MANUAL
    TRISC0 = 1;     //AUTO
    
    TRISC5 = 0;     //RELAY1        //OUTPUTS
    TRISC2 = 0;     //RELAY2
    TRISA4 = 0;     //S LOW LED
    TRISA2 = 0;     //DRY RUN LED
    TRISA1 = 0;     // BUZZER
    TRISA3 = 0;     //HI/LO LED

    
    __delay_ms(1000);
    I2C_Initialize(2000);
    SSD1306_Init(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);
    SSD1306_ClearDisplay();
    Call_display();
    __delay_ms(1000);
    SSD1306_ClearDisplay();
    
    while(1)
    {   
        if(RC1==1)
        {
            mode="Manual";
        }
        else if (RC0==1)
        {
            mode="Auto";
        }
        else
        {
            mode="No mode";
        }
        Display_ADC();
     ReadSensor();       
     int wl=ReadSensor();
     switch(wl)
        {
            case 100:
                TWL="Full";
                motor=":OFF      ";
                //SSD1306_ClearDisplay();
                Display_ADC();
                 __delay_ms(380);
                break;
            case 75:
                TWL="75% ";
                Display_ADC();
            __delay_ms(380);
                break;  
            case 50:
                TWL="50%  ";
                //SSD1306_ClearDisplay();
                Display_ADC();
                Motor_On();
                __delay_ms(380);
                break;
            case 25:
                TWL="25%  ";
                //SSD1306_ClearDisplay();
                Display_ADC();
                Motor_On();
                __delay_ms(380);
                break; 
            case 0:
                TWL="00%  ";
                //SSD1306_ClearDisplay();
                Display_ADC();
                __delay_ms(380);
                break;   
                
            default:

                break;
        }
        
    }
}