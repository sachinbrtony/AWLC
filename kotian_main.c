#include<Config.h>
#include<stdint.h>
#include "I2C.h"
#include "OLED.h"
#include<stdio.h>
#include<stdlib.h>

#define SSD1306_128_32

uint8_t TANKWAR, WL, SUMPWAR, dryflag = 0;
char* motor;
char* mode;
char* TWL;
char* SWL;

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

void oled_puts(const char* msg, uint8_t msg_size)
{
    while(*msg != '\0')
    {
        SSD1306_PutStretchC(*msg, msg_size);
        msg++;
    }
}

void setupADC()
{
    ADCON1 = 0x80;
    ADCON0 = 0x00;
    __delay_ms(30);
    ADCON0=0b10010011;
    ADCON1=0b10000000;
}

uint16_t calculateADC()
{
    /* Initializes A/D Conversion */
    GO_nDONE = 1;
    /* Wait for A/D Conversion to complete */
    while(GO_nDONE); 
    /* SAVE ADC DATA AND STORE */
    uint16_t adc_val = (ADRESH << 8) + ADRESL;
    return adc_val;
}

void oled_display(char *msg, char *val, uint8_t str_x, 
            uint8_t str_y, uint8_t val_x, uint8_t val_y)
{
    SSD1306_GotoXY(str_x,str_y);
    oled_puts(msg, 1);
    SSD1306_GotoXY(val_x , val_y);
    oled_puts(val, 1);
}

void check_dryflag(float ip_volt)
{
    /* set dryflag based on input voltage */
    if(ip_volt <= 130)
        dryflag = 2;
    else if(ip_volt >= 310)
        dryflag = 3;

    __delay_ms(30);
    
    /* set dryflag based on Sump Water Level */
    if(SUMPWAR == 1)
        dryflag = 4;
    else
        dryflag = 0;
}

const char* get_alm_msg(float ip_volt)
{
    char *alm_msg;

    /* get the dryflag value assigned */
    check_dryflag(ip_volt);

    /* display appropriate message */
    switch(dryflag)
    {
        case 2:
            alm_msg = "Low Mains Volt";
            break;
        case 3:
            alm_msg = "High Mains Volt";
            break;
        case 4:
            alm_msg = "No Water in Sump";
            break;
        default:
            alm_msg = "I'm Working Good";
    }

    return alm_msg;
}

void Display_ADC()
{
    char *acvolt[10];
    uint16_t ACvolt;
    
    setupADC();
    ACvolt = calculateADC();
    __delay_ms(30);
    /* Calculating AC voltage from ADC reading */
    ACvolt = (ACvolt) * 1.19;
    /* Converting integer voltage value to text */
    itoa(acvolt, ACvolt, 10);

    /* OLED display */
    oled_display("MODE: ", mode, 1, 1, 6, 1);
    oled_display("MOTOR:", motor, 1, 2, 6, 2);
    oled_display("AC IN:", acvolt, 16, 1, 17, 2);
    oled_display("TANK:", TWL, 1, 4, 1, 5);
    oled_display("SUMP:", SWL, 12, 4, 10, 5);
    oled_display("ALM:", get_alm_msg(ACvolt), 1, 7, 6, 7);
}

void print_manf_name()
{
    oled_display("ENMAD", "SOLUTIONS", 1, 1, 1, 5);
    LEDchaser();
    LEDchaser();
    __delay_ms(50);
}

void Motor_On()
{
    uint8_t abc = 0;
    ReadSensor();

    if((SUMPWAR == 0) && (dryflag == 0))
    {
        abc = 1;
        RA1 = 0;
    }
    else
    {
        abc = 0;
        RA1 = 1;
        motor = ":OFF    ";
    }

    while(abc==1)
    {
        ReadSensor();
        /* Motor Relay1 On */ 
        RC5 = 1;
        motor=":Running";
        Display_ADC();
        LEDchaser();

        if(SUMPWAR == 1 || TANKWAR == 1 || WL == 100)
        {
            abc = 0;
            RC5 = 0;
            motor = ":OFF    ";
            Display_ADC();
        }
    }
}

void set_tankwl(char *msg, uint8_t twl, uint8_t sensor_err)
{
    TWL = msg;
    WL = twl;
    TANKWAR = sensor_err;
}

void read_tankwl()
{
    /* Read tank water level */
    if(RB1 == 0 && RB0 == 0 && RC7 == 0 && RC6 == 0)
        set_tankwl("FULL", 100, 0);
    else if(RB1 == 0 && RB0 == 0 && RC7 == 0 && RC6 == 1 )
        set_tankwl("75%", 75, 0);
    else if(RB1 == 0 && RB0 == 0 && RC7 == 1 && RC6 == 1)
        set_tankwl("50%", 50, 0);
    else if(RB1 == 0 && RB0 == 1 && RC7 == 1 && RC6 == 1)
        set_tankwl("25%", 25, 0);
    else if(RB1 == 1 && RB0 == 1 && RC7 == 1 && RC6 == 1 )
        set_tankwl("0%", 0, 0);
    else
        set_tankwl("Err", 1, 1);
}

void set_sumpwl(char *msg, uint8_t sensor_err, uint8_t led)
{
    SWL = msg;
    SUMPWAR = sensor_err;
    RA4 = led;
}

void read_sumpwl()
{
    /* Read sump water level */
    if(RB5==0 && RB4==0 && RB3==0)
        set_sumpwl("100%", 0, 0);
    else if(RB5==0 && RB4==0 && RB3==1)
        set_sumpwl("50%", 0, 0);
    else if(RB5==0 && RB4==1 && RB3==1)
        set_sumpwl("25%", 0, 0);
    else if(RB5==1 && RB4==1 && RB3==1)
        set_sumpwl("00%", 1, 1);
    else
        set_sumpwl("Err", 1, 1);
}

void ReadSensor()
{
    /* Read and update tank water level */
    read_tankwl();

    /* Read and update sump water level */
    read_sumpwl();

    __delay_ms(25);
    
    /* Incase or Error */
    if(SUMPWAR == 1 || TANKWAR == 1)
        RA1=1;
    else
        RA1=0;
}

void Init()
{
    ANSELH = 0x00;  //DISABLE ADC IN PORTB
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
    TRISA2 = 0;     //DRY RUN LED
    TRISA1 = 0;     //BUZZER
    TRISA3 = 0;     //HIGH/LOW LED
    TRISA4 = 0;     //S-LOW LED
}

void set_twl(char *msg, uint8_t sensor_val)
{
    TWL = msg;
    Display_ADC();
    if (sensor_val == 25 || sensor_val == 50)
        Motor_On();
    if (sensor_val == 1)
        TANKWAR = 1;
    __delay_ms(40);
}

int main()
{
    Init();
    __delay_ms(50);
    I2C_Initialize(200);
    SSD1306_Init(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);
    SSD1306_ClearDisplay();
    print_manf_name();
    __delay_ms(50);
    SSD1306_ClearDisplay();

    while(1)
    {
        WDTCON = 0b00010111;
        __delay_ms(25);

        if(RC5 == 1)
            motor = ":Running";
        else
            motor = ":OFF    ";

        __delay_ms(25);

        if(RC1 == 0)
        {
            mode = "Manual";
            motor = ":Running";
            LEDchaser();
            Display_ADC();
        }
        else if (RC0 == 0)
            mode = "Auto  ";
        else if(RC0 == 1 && RC1 == 1)
        {
            mode = "OFF   ";
            motor = ":OFF    ";
        }

        Display_ADC();
        /* Get recent WL update on reading sensor */
        ReadSensor();
        switch(WL)
        {
        case 100:
            set_twl("Full", 100);
            break;
        case 75:
            set_twl("75% ", 75);
            break;
        case 50:
            set_twl("50% ", 50);
            break;
        case 25:
            set_twl("25% ", 25);
            break;
        case 0:
            set_twl("00% ", 0);
            break;
        case 1:
            set_twl("Err", 1);
            break;            
        default:
            /* Do NOTHING */
            break;
        }

        CLRWDT();
    }
}
