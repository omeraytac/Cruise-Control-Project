#include "mbed.h"
#include "MMA8451Q.h"
#include "IRremote.h"
#include "TextLCD.h"
#include <string>


#if   defined (TARGET_KL25Z) || defined (TARGET_KL46Z)
  PinName const SDA = PTE25;
  PinName const SCL = PTE24;
#elif defined (TARGET_KL05Z)
  PinName const SDA = PTB4;
  PinName const SCL = PTB3;
#elif defined (TARGET_K20D50M)
  PinName const SDA = PTB1;
  PinName const SCL = PTB0;
#else
  #error TARGET NOT DEFINED
#endif

#define MMA8451_I2C_ADDRESS (0x1d<<1)
    MMA8451Q acc(SDA, SCL, MMA8451_I2C_ADDRESS);
    Serial pc(USBTX, USBRX);
    IRremote irInput(D4);
    PwmOut rled(LED1);
    PwmOut gled(LED2);
    PwmOut bled(LED3);
    PwmOut pwm(PTA13);
    TextLCD lcd(PTE20, PTE21, PTE22, PTE23, PTE29, PTD5, TextLCD::LCD16x2); // rs, e, d4-d7
    
union data {
        unsigned long intVal;
        char hexVal[4];
        } data;
void read()
    {
        for (int i = 0; i < 4; ++i)
            data.hexVal[3 - i] = irInput.readclear(i);
    }


int main(void)
{
    lcd.cls();
    //0 , 1,2,3,4,5,6,7,8,9,*,#,up,left,ok,right,down
    /*int received[17] = {16718310, 16729530, 16729785, 16730040, 16729275, 16728255, 16729020, 16713720,
    16717290, 16714230, 16717545, 16715250, 16718055, 16713975,
    16719075, 16734885, 16732845};*/
    int received[17] = {4294967270 , 4294967226 , 4294967225 , 4294967224 , 4294967227 , 4294967231 , 4294967228 , 4294967288 ,
    4294967274 , 4294967286 , 4294967273 , 4294967282 , 4294967271 , 4294967287 ,
    4294967267 , 4294967205 , 4294967213 };
    float angleTable[101] = {-11, -10.4, -9.8, -9.2, -8.6, -8, -7.7, -6.8, -6.2, -5.6, -5, -4.6, -4.2, -3.8, -3.4, -3,
    -2.6, -2.2, -1.8, -1.4, -1, -0.7, -0.4, -0.1, 0.2, 0.5, 0.8, 1.1, 1.4, 1.7, 2, 2.2, 2.4, 2.6, 2.8, 3, 3.2, 3.4, 3.6,
    3.8, 4, 4.2, 4.4, 4.6, 4.8, 5, 5.4, 5.8, 6.2, 6.6, 7, 8, 9, 10, 11, 11.3, 11.7, 12, 12.3, 12.7, 13, 13.6, 14.2, 14.8,
    15.4, 16, 16.4, 16.8, 17.2, 17.6, 18, 18.6, 19.2, 19.8, 20.4, 21, 21.8, 22.6, 23.4, 24.2, 25, 25.6, 26.2, 26.8, 27.4, 
    28, 28.8, 29.6, 30.4, 31.2, 32, 33.6, 35.2, 36.8, 38.4, 40, 42.8, 45.6, 48.4, 51.2, 54};
    pc.printf("MMA8451 ID: %d\n", acc.getWhoAmI());
    pwm.period(0.01f);
    int waitTime=0;
    pwm = 0.00f;
    float yTotal = 0.0f;
    float yAv = 0.0f;
    bool digit = false;
    float speedSet = 0.0f;
    float speedEntered = 0.0f;
    float speedAcc = 0.0f;
    float speed = 0.0f;
    int slopeIndex = 0;
    string lcdLine1 = " ";
    string lcdLine2 = " ";
    int enteredValue = 0;
    int newEnteredValue = 0;
    bool newEntered = false;
    while ( true ) {
        read();
        if(data.intVal != 4294967295){
            if (data.intVal == received[12] && speedEntered < 1.0f)
                speedEntered = speedEntered + 0.01f;
            if (
            data.intVal == received[16] && speedEntered > 0.00f)
                speedEntered = speedEntered - 0.01f;
            if (data.intVal == received[14]){
                if( speedEntered == 0 && speedSet != 0){
                    if ( yAv*100 >= -50 && yAv*100 <= 50){ 
                        slopeIndex = (int)(yAv*100 + 50);
                        speedAcc = angleTable[slopeIndex] / 100.0f;
                    }
                    else{
                        if ( yAv < -0.5)
                            speedAcc = 0.01f;
                        else 
                            speedAcc = 0.54;            
                    }
                    speed = speedAcc + speedSet + 10;
                    if (speed < 0)
                        speed = 0.01f;
                    if ( speed > 1.0f)
                        speed = 1.0f;
                    pwm = speed;   
                    wait(0.02f);
                    speed -= 10;
                    pwm = speed;
                    speedEntered = speedSet;
                 }
                 else
                 speedEntered = speedSet;
                 newEntered = false;
            }
            for ( int i = 0; i < 11 ; i++){
                if ( data.intVal == received[i] && i != 9){
                    if (!digit){
                        speedSet = 0.1f*i;
                        newEnteredValue = i * 10;
                        newEntered = true;
                        digit = true;
                    }
                    else{
                        speedSet += 0.01f*i;
                        newEnteredValue = newEnteredValue + i;
                        digit = false;
                    }
                }
            }
            pc.printf( "IR value %lu \n", data.intVal);
        }
        float x, y, z;
        x = acc.getAccX();
        y = acc.getAccY();
        z = acc.getAccZ();

        rled = 1.0f - abs(x);
        gled = 1.0f - abs(y);
        bled = 1.0f - abs(z);
        
        
        waitTime++;
        if(waitTime == 100){
            y = yTotal/100.0f;
            yAv = y;
            pc.printf("Y: %1.2f\n",y);
            
            if ( y*100 >= -50 && y*100 <= 50){ 
                slopeIndex = (int)(y*100 + 50);
                speedAcc = angleTable[slopeIndex] / 100.0f;
            }
            else{
                if ( y < -0.5)
                    speedAcc = 0.01f;
                else 
                    speedAcc = 0.54;            
            }
            speed = speedAcc + speedEntered;
            if (speed < 0)
                speed = 0.01f;
            if ( speed > 1.0f)
                speed = 1.0f;
            if( speedEntered ==0 )
                speed = 0.0f;       
            pwm = speed;
            waitTime=0;
            yTotal=0.0f;
            
            lcd.cls();
            wait(0.001);
            if ( newEntered == false){
                lcd.printf("SPEED:%d NEW:",(int)(speedEntered * 100));
            }
            else{
                if ( digit == true )
                    lcd.printf("SPEED:%d NEW:%d",(int)(speedEntered * 100), newEnteredValue / 10);
                else{
                    lcd.printf("SPEED:%d NEW:%d",(int)(speedEntered * 100), newEnteredValue);
                    }
            }
            lcd.locate(0,1);
            lcd.printf("PWR:%d DEG:%.1f", (int)(speed * 100), (y * 90));

            pc.printf("speed: %.2f\n",speed);
            pc.printf("slopeIndex: %d\n", slopeIndex);
        }
        else{
            yTotal += y;
        }
            
    }
}
