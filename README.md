# arduino_TM1638_DRO
a simple Arduino DRO with a single TM1638 display and rotary encoder

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

