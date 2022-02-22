# Stepper Motor 28BYJ-48 Driver
This is a driver for controlling the 28BYJ-48 stepper motor.

## How to Use
1. Initialize the motor object and provide a pinout.
2. Call `step(direction)` or `step()` to rotate the spindle one tick.
3. To move faster, call `warp_speed_mr_sulu(direction)` and then call just `step()`.

## Example Usage
```C++
    #include <SM_28BYJ-48.h>
    SM_28BYJ_48 stepper(0, 1, 6, 13); // init with pin numbers IN1-4
    stepper.warp_speed_mr_sulu(stepper.CW); // tell stepper to move clockwise, two steps at a time

    while(true){
        stepper.step(); // tell stepper to move one step clockwise
        //sleep_ms(1000);
    }
```
