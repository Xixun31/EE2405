#include "mbed.h"
#include "PwmIn.h"

#define CENTER_BASE 1500
#define unitsFC 360                          // Units in a full circle
#define dutyScale 1000                       // Scale duty cycle to 1/1000ths
#define dcMin 29                             // Minimum duty cycle
#define dcMax 971                            // Maximum duty cycle
#define q2min unitsFC/4                      // For checking if in 1st quadrant
#define q3max q2min * 3                      // For checking if in 4th quadrant

PwmOut servo_c(D11);                             //servo control signal
PwmIn servo_f(D10);                               //servo feedback signal

EventQueue servo_queue;
Thread t;

volatile int flag_control360=0;
volatile int angle, targetAngle=0;              // Global shared variables
volatile int Kp = 1;                          // Proportional constant
volatile float tCycle;
volatile int theta;
volatile int thetaP;
volatile int turns = 0;

void servo_control(int speed) {
   if (speed > 200)       speed = 200;
   else if (speed < -200) speed = -200;

   servo_c = (CENTER_BASE + speed)/20000.0f;
}

void init_thetaP(){
   ThisThread::sleep_for(100ms); //feedback signal need time to read dutycycle
   int dc = dutyScale * servo_f.dutycycle();
   thetaP = (unitsFC - 1) - ((dc - dcMin) * unitsFC) / (dcMax - dcMin + 1);
   theta = thetaP;
   angle = thetaP;
}

void distance_control(float distance){
   targetAngle = (int)(distance*360/(6.5*3.14)) + angle;
   flag_control360 = 1;
}

void angle_control(int setangle){
   targetAngle = setangle;
   flag_control360 = 1;
}

void feedback360()                          // Position monitoring
{

   tCycle = servo_f.period();
   while(1){                                 // Keep checking
      if((tCycle > 1000) && (tCycle < 1200))// If cycle time valid
            break;                            // break from loop
   }
   int dc = dutyScale * servo_f.dutycycle();
   theta = (unitsFC - 1) -                   // Calculate angle
            ((dc - dcMin) * unitsFC)
                  / (dcMax - dcMin + 1);
   if(theta < 0)                             // Keep theta valid
      theta = 0;
   else if(theta > (unitsFC - 1))
      theta = unitsFC - 1;

      // If transition from quadrant 4 to
   // quadrant 1, increase turns count.
   if((theta < q2min) && (thetaP > q3max))
      turns++;
   // If transition from quadrant 1 to
   // quadrant 4, decrease turns count.
   else if((thetaP < q2min) && (theta > q3max))
      turns --;

      // Construct the angle measurement from the turns count and
   // current theta value.
   if(turns >= 0)
      angle = (turns * unitsFC) + theta;
   else if(turns <  0)
      angle = ((turns + 1) * unitsFC) - (unitsFC - theta);

   thetaP = theta;                           // Theta previous for next rep
}

void control360()                           // Position control
{
   if(flag_control360){
      int errorAngle, output, offset;             // Control system variables
      errorAngle = targetAngle - angle;         // Calculate error
      output = errorAngle * Kp;                 // Calculate proportional
      if(output > 200) output = 200;            // Clamp output
      if(output < -200) output = -200;

      if(errorAngle > 0)                        // Add offset
            offset = 30;
      else if(errorAngle < 0)
            offset = -30;
      else
            offset = 0;
      servo_control(output + offset);
   }
   else
      servo_control(0);
}

int main() {
   init_thetaP();                               //initial thetaP
   servo_c.period_ms(20);
   t.start(callback(&servo_queue, &EventQueue::dispatch_forever));
   servo_queue.call_every(5ms,feedback360);
   servo_queue.call_every(20ms,control360);

   int i=0;
   // float distance[3];

   // distance[0] = 20.41; // 6.5* 3.14
   // distance[1] = 40.82; // 2* 6.5* 3.14
   // distance[2] = 51.03; // 2.5* 6.5* 3.14
   int setangle[3];

   setangle[0] = 0;
   setangle[1] = 180;
   setangle[2] = 270;


   while (i<3) {
      // printf("Target distance: %.2f\n", distance[i]);
      // distance_control(distance[i]);
      angle_control(setangle[i]);

      while(abs(targetAngle - angle) > 3)         // Display until close to finish
         {
            // printf("target distance = %.2f, target Angle = %d , angle = %d\r\n", // Display target & measured
            //       distance[i], targetAngle, angle);
            printf("target Angle = %d , angle = %d\r\n", // Display target & measured
                  targetAngle, angle);
            ThisThread::sleep_for(500ms);
         }
      flag_control360 = 0;          // make servo stay still
      printf(">> 成功抵達目標！當前位置驗證: %d 度\n", angle);
      ThisThread::sleep_for(1500ms);
      i++;
   }
}