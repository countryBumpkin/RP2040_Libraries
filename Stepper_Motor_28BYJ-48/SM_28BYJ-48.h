/*
 * Stepper motor(SM) driver for generic 28BYJ-48.
 * Author: Garrett Wells
 * Date: 02/23/2022
 */
#ifndef SM_28BYJ_48_H
#define SM_28BYJ_48_H

#include <pico/stdlib.h>

enum Direction {CW=true, CCW=false}; // spindle rotation directions, true when looking at rear of motor housing

/*
 * Object to control a stepper motor on the raspberry pi pico rp2040.
 *  Directions: true == clockwise as the user looks at the back of the motor housing
 *              false == counter-clockwise when looking at the back of the motor housing
 */
class SM_28BYJ_48 {
    private:
        int state;      // number of the step that was taken last so we know what step to take next
        bool direction; // the direction of our next step
        const uint8_t STATE[8] = {
                                0x08,
                                0x0C,
                                0x04,
                                0x06,
                                0x02,
                                0x03,
                                0x01,
                                0x09
                             };

        int IN1, IN2, IN3, IN4; // pins used to control the SM
        int step_size=1;        // set to be either 1 (normal), 2 (fast-ish)
        int offset_since_epoch; // how many steps and direction since start

    public:
        static const int HALF_REVOLUTION=2048;
        static const int FULL_REVOLUTION=4096; // number of steps in one revolution

        SM_28BYJ_48(int in1, int in2, int in3, int in4);
        void step(void);        // take the next step with the motor, always steps clockwise
        void step(Direction);   // take one step in the direction, don't change speed
        void turtle_speed(Direction);       // set the direction and set speed to 1 (half step)
        void warp_speed_mr_sulu(Direction); // set the direction and set speed to 2 (full step)
        int get_state(void);  // get the state of the motor, ie phase 1-8
};
#endif
