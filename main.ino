#include <LiquidCrystal.h>

/* Pins definitions */
#define PUSH_BUTTON 2
#define TRACK_RELAY 3     /* Relay */
#define TRACK_MOS   4     /* MOSFET M1 */
#define V_PLUS      A0
#define V_MINUS     A1
#define RS_LCD      12
#define RW_LCD      11
#define ENABLE_LCD  10
#define D7_LCD      9
#define D6_LCD      8
#define D5_LCD      7
#define D4_LCD      6

/* Resistor divider */
#define RA          15    /* MΩ */
#define RB          10    /* MΩ */

#define TRACK_ON  1
#define TRACK_OFF 0

/* Compute average of voltage measured */
#define NUM_SAMPLES 20

LiquidCrystal lcd(RS_LCD, RW_LCD, ENABLE_LCD, 
                  D4_LCD, D5_LCD, D6_LCD, D7_LCD);


uint8_t track_supply;

/* Push_button Interrupt service routine */
void button_ISR()
{
        int state;
        track_supply = !track_supply;
        state = (track_supply == TRACK_ON) ? HIGH : LOW;
        digitalWrite(LED_BUILTIN, state);
        digitalWrite(TRACK_RELAY, state);
        digitalWrite(TRACK_MOS, !state);      /* Inverse logic */
        lcd.clear();
}

void lcd_show(float p_voltage, float m_voltage)
{
        lcd.clear();
        lcd.print("+V:");
        lcd.print(p_voltage);
        lcd.print("V TRACK:");
        lcd.setCursor(0, 1);
        lcd.print("-V:");
        lcd.print(m_voltage);
        lcd.print("V ");
        if(track_supply == TRACK_ON)
                lcd.print("ON");
        else
                lcd.print("OFF");
}


void setup()
{

        /* Init lcd */
        lcd.begin(16, 2);
        lcd.setCursor(0, 0);

        /* Init variables */
        track_supply = TRACK_OFF;

        /* Init pins */
        pinMode(PUSH_BUTTON, INPUT);
        pinMode(TRACK_RELAY, OUTPUT);
        pinMode(TRACK_MOS, OUTPUT);
        pinMode(LED_BUILTIN, OUTPUT);
        pinMode(V_PLUS, INPUT);
        pinMode(V_MINUS, INPUT);

        digitalWrite(TRACK_RELAY, TRACK_OFF);
        digitalWrite(TRACK_MOS, !TRACK_OFF);    /* Inverse logic */
        
        /* Interrupts */
        attachInterrupt(digitalPinToInterrupt(PUSH_BUTTON), button_ISR, RISING); 

}

void loop() 
{
        int k;
        int p_voltage[NUM_SAMPLES];
        int m_voltage[NUM_SAMPLES];
        int p_voltage_mean = 0;
        int m_voltage_mean = 0;
        float real_pvoltage, real_mvoltage;
        for(k = 0; k < NUM_SAMPLES; k++){
                p_voltage[k] = analogRead(V_PLUS);
                delay(10);                              /* Arduino reference */
                m_voltage[k] = analogRead(V_MINUS);
                p_voltage_mean += p_voltage[k];
                m_voltage_mean += m_voltage[k];
        }
        p_voltage_mean /= NUM_SAMPLES;
        m_voltage_mean /= NUM_SAMPLES;

        real_pvoltage = p_voltage_mean*(RA+RB)/RB*(5.0/1024.0);    /* Offset */
        real_mvoltage = -m_voltage_mean*(RA+RB)/RB*(5.0/1024.0);   /* Offset */ 
        lcd_show(real_pvoltage, real_mvoltage);
}
