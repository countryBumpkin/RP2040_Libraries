#include <pico/stdlib.h>

class SM_28BYJ_48 {
    private:
        int state; // number of the step that was taken last so we know what step to take next
        bool direction; // the direction of our next step
        const uint8_t STATE[8] = {0x08,
                                0x0C,
                                0x04,
                                0x06,
                                0x02,
                                0x03,
                                0x01,
                                0x09
                             };

        int IN1, IN2, IN3, IN4;
        int step_size=1;  // set to be either 1 (normal), 2 (fast-ish)

    public:
        enum Direction {CW=false, CCW=true};

        SM_28BYJ_48(int in1, int in2, int in3, int in4);
        void step(void);        // take the next step with the motor, always steps clockwise
        void step(Direction);   // take one step in the direction, don't change speed
        void turtle_speed(Direction);       // set the direction and set speed to 1
        void warp_speed_mr_sulu(Direction); // set the direction and set speed to 2

};
