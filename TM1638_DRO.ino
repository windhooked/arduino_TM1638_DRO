#include <Encoder.h>
#include <TM1638.h>

/*
[] [] [] [] [] [] [] []

X Y Z S CLR PUSH POP CALC ( Buttons )

Normal mode 
( x,y,z,s leds remain green when selected; clear, push , pop flash green, calc goes red and stays red for calc operation )
Pressing X,Y,Z,S will show the current encoder value for x,y,z axis and spindle speed
Pressing clear while axis is selected, will clear current position
Pressing push will push the current number to stack for calculation mode, or other axis access

Calc Mode ( leds flash red when pressed) 
Pressing calc will enter the RPN calculator mode
X,Y,Z,S become math operators * / - +
Use the rotary knob to enter numbers in 99 blocks, pressing the knob button will advance the number field to the next 99 block,  use clear to clear
Use push to push the number to stack, followed by more numbers to stack, followed by the operator, which will pop the numbers from stack and display the result.
Last result will remain on stack.
pressing calc again will exit calculation and resume normal operation, where pop can be used to fill the selected axis.

*/

#define bit_get(p,m) ((p) & (m))
#define bit_set(p,m) ((p) |= (m))
#define bit_clear(p,m) ((p) &= ~(m))
#define bit_flip(p,m) ((p) ^= (m))
#define bit_write(c,p,m) (c ? bit_set(p,m) : bit_clear(p,m))
#define BIT(x) (0x01 << (x))
#define LONGBIT(x) ((unsigned long)0x00000001 << (x))


// Change these pin numbers to the pins connected to your encoder.
//   Best Performance: both pins have interrupt capability
//   Good Performance: only the first pin has interrupt capability
//   Low Performance:  neither pin has interrupt capability
Encoder encInp(2, 3);
#define TM1638_DIO 6
#define TM1638_CLK 7
#define TM1638_STB1 8

TM1638 module(TM1638_DIO,TM1638_CLK, TM1638_STB1); //

enum run_mode {  x,y,z,s, clr, push, assign ,calc };
enum calc_mode { calc_mul, calc_div, calc_add, calc_sub, calc_clr, calc_push, calc_pop, calc_exit };
enum modes {run_mode, calc_mode };

long dro[3];
unsigned int stack[10];
unsigned char sp = 0;
unsigned char dp = 255;
unsigned char state = 0;
unsigned char buttons = 0;
unsigned char leds = 0;
long positionEnc  = 0;
//   avoid using pins with LEDs attached

void setup() {
//  Serial.begin(115200);
//  Serial.println("TwoKnobs Encoder Test:");

module.clearDisplay();
module.setupDisplay(true, 2 );//here 7 is intensity (from 0~7)
module.setDisplayToString("dro boot", 0); delay(1000);

state = run_mode;
dro[x] = 100;
dro[y] = 200;
dro[z] = 300;

stack[0] = 0;
stack[1] = 0;
stack[2] = 0;
 // default x mode
 display_num(dro[x]);
 module.setLEDs(0x0000);
 module.setLED(  TM1638_COLOR_GREEN , x);
 dp = x;
 sp=0;
    
}


void display_num(long num) {
  module.clearDisplay(); 
  if ( num ) 
    module.setDisplayToDecNumber(num,0,false);
  else
   module.setDisplayDigit(dro[dp], 7, false);
}
void led_flash_green(unsigned char led) {
     module.setLED(  TM1638_COLOR_GREEN , led);
     delay (2);
     module.setLED(  TM1638_COLOR_NONE , led );  
}
void led_flash_red(unsigned char led) {
     module.setLED(  TM1638_COLOR_RED , led);
     delay (2);
     module.setLED(  TM1638_COLOR_NONE , led );  
}
void led_set_green(unsigned char led) {
  module.setLED(  TM1638_COLOR_GREEN , led);
}
  
void loop() {
  
  long newInp;
  newInp = encInp.read();
 
  if (newInp != positionEnc ) {
    
    //module.setDisplayToDecNumber(positionEnc,0,true);
  
   // Serial.print("Left = ");
   // Serial.print(positionEnc);

  //  Serial.println();
    positionEnc = newInp;

  }
  
  buttons = 0;
  buttons = module.getButtons();
  delay(150); //
  leds = 0;
 

  
  // RUN Mode
  if ( state == run_mode ) {
  
  if( buttons & BIT(x) ) { //X
    display_num(dro[x]);
    module.setLEDs(0x0000);
    led_set_green(x);
    dp = x;
  } 
 
  if( buttons & BIT(y) ) { //Y
    display_num(dro[y]);
    module.setLEDs(0x0000);
    led_set_green(  y );
    dp = y;
  }
 
 if( buttons & BIT(z) ) { //Z
    
    display_num(dro[z]);
    module.setLEDs(0x0000);
    led_set_green( z);
    dp = z;
  }
 
 
  if( buttons & BIT(s) ) { //S
    display_num(dro[s]);
    module.setLEDs(0x0000);
    led_set_green( s);
    dp = s;
  }
 
 // CLR
  if( buttons & BIT(clr)  ) { // clear
    if ( 0 < dp < 4) { 
      module.setDisplayToString("clear", 0); delay(10);
      dro[dp] = 0L;
      
     led_flash_green( clr );
       
      display_num(dro[dp]);
    } else {
      module.setDisplayToString("CLR ERR", 0);
    }
  }
  
  if( buttons & BIT(push) ) { // push
    module.setDisplayToString("push", 0); delay(10);
    if ( sp < 3 ) {
      sp++;
      stack[3] = stack[2];
      stack[2] = stack[1];
      stack[1] = stack[0];
      stack[0] = dro[dp];
    
    led_flash_green( push );  
    display_num(dro[dp]);
    } else {
       module.setDisplayToString("ST OFL ERR", 0);
    }
  }
  
   if( buttons & BIT(assign) ) { // assign
    module.setDisplayToString("assign", 0); delay(10);
    sp--;
    dro[dp] = stack[0];
    led_flash_green( assign );  
    display_num(dro[dp]); 
   }
  
   if( buttons & BIT(calc) ) { // calc
    
      module.setLEDs(0x0000);
      module.setLED(  TM1638_COLOR_RED , calc);
  
      state = calc_mode;
   }
   
 // CALC Mode
 
  } else if ( state == calc_mode ) {
    
    //if ( sp < 1 ) {  
    //   module.setDisplayToString("ST UFL ERR", 0); delay(10);
    //} else {
    
    if( buttons & BIT(calc_mul) ) { // Multiply
  
      stack[0] *= stack[1];
      stack[1] = stack[2];
      stack[2] = 0L;
      sp--;
      
     led_flash_red( calc_mul);
    
     display_num(stack[0]);
    }
    
    if ( buttons & BIT(calc_div) ) { // Divide
       
        if ( stack[0] > 0 ) {
         stack[0] = stack[1] / stack[0];
         stack[1] = stack[2];
         stack[2] = 0L;
         sp--; 
        } else {
           module.setDisplayToString("div by 0 err", 0); delay(10);
        }
        led_flash_red(calc_div);
        display_num(stack[0]);

    }    
    if ( buttons & BIT(calc_add) ) { // Add
  
        stack[0] += stack[1];
        stack[1] = stack[2];
        stack[2] = 0L;
        sp--;
        led_flash_red(calc_add);
        display_num(stack[0]);
    }
    if ( buttons & BIT(calc_sub) ) { // Sub
        
        stack[0] = stack[1] - stack[0];
        stack[1] = stack[2];
        stack[2] = 0L;
        sp--;
        led_flash_red(calc_sub);
        display_num(stack[0]);
    }
    
   // } // end of stack check for calc ops
    
    if( buttons & BIT(calc_clr)  ) { // clear
     led_flash_red(calc_clr);
     stack[0] = 0L;
     stack[1] = 0L;
     stack[2] = 0L;
     stack[3] = 0L;
    }
    
    if( buttons & BIT(calc_push)  ) { // push
     stack[3] = stack[2];
     stack[2] = stack[1];
     stack[1] = stack[0];
     stack[0] = 0L; 
     sp++;
     
     led_flash_red(calc_push);
     // module.setDisplayToString("sp" , 0); delay(10);
    }
    
    if( buttons & BIT(calc_pop)  ) { // pop
      stack[0] = stack[1];
      stack[1] = stack[2];
      stack[2] = stack[3];
      stack[3] = 0L;
      sp--;
      led_flash_red(calc_pop);
    } 
    
    if( buttons & BIT(calc_exit) ) { // calc exit
    
       module.setLEDs(0x0000);
       
       if ( !dp ) dp = x; 
       module.setLED(  TM1638_COLOR_GREEN , dp );
       state = run_mode;
   }
   
  }
    
  // if a character is sent from the serial monitor,
  // reset both back to zero.
  if (Serial.available()) {
    Serial.read();
    Serial.println("Reset both knobs to zero");
    encInp.write(0);
   ;
  }
}
