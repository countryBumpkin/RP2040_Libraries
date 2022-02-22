#ifndef VANDALUINO_7SEGMENT_H
#define VANDALUINO_7SEGMENT_H

#include <pico/stdlib.h>

#define HIGH 1
#define LOW 0
#define CC1 11  // left segment common cathode
#define CC2 10  // right CC
#define LED_BUILTIN 13
#define ALL_SEGMENTS 0b0111111000001000000000010000000

#define RIGHT_ON 0b0000000000000000000010000000000 /*turn on the left segment display*/
#define LEFT_ON 0b0000000000000000000100000000000 /*turn on the right segment display*/
#define CCX 0b0000000000000000000110000000000 /*set both segment displays to same state*/

const int SEGMENT_NUM[] = {
  //0b0CGBAEP00000D0000000000F0000000
    0b0101110000001000000000010000000, // 0
    0b0101000000000000000000000000000, // 1
    0b0011110000001000000000000000000, // 2
    0b0111100000001000000000000000000, // 3
    0b0111000000000000000000010000000, // 4
    0b0110100000001000000000010000000, // 5
    0b0110110000001000000000010000000, // 6
    0b0101100000000000000000000000000, // 7
    0b0111110000001000000000010000000, // 8
    0b0111100000001000000000010000000, // 9
};

const int SEGMENT_HEX[] = {
  //0b0CGBAEP00000D0000000000F0000000
    0b0101110000001000000000010000000, // 0
    0b0101000000000000000000000000000, // 1
    0b0011110000001000000000000000000, // 2
    0b0111100000001000000000000000000, // 3
    0b0111000000000000000000010000000, // 4
    0b0110100000001000000000010000000, // 5
    0b0110110000001000000000010000000, // 6
    0b0101100000000000000000000000000, // 7
    0b0111110000001000000000010000000, // 8
    0b0111100000001000000000010000000, // 9
    0b0111110000000000000000010000000, // A
    0b0110010000001000000000010000000, // b or 8
    0b0000110000001000000000010000000, // C
    0b0111010000001000000000000000000, // d
    0b0010110000001000000000010000000, // E
    0b0010110000000000000000010000000, // E
};

/*
 *  Initialize the GPIO pins for the 7 segment display to be outputs and set LOW
 */
void init_7_segment(){
    // init the common cathodes as outputs, HIGH initially so nothing shows
    gpio_init_mask(CCX);
    gpio_set_dir_out_masked(CCX);
    gpio_set_mask(CCX); // raise both so that neither is displaying

    // init letter segments and set them as outputs
    // don't have to drive low because that is done by default on init
    gpio_init_mask(ALL_SEGMENTS);
    gpio_set_dir_out_masked(ALL_SEGMENTS);

    // init built-in LED
    gpio_init(LED_BUILTIN);
    gpio_set_dir(LED_BUILTIN, GPIO_OUT);
}

/*
 * Light right digit, can be used for testing or simple projects
 */
void show_on_right(uint32_t bitmask){
    gpio_set_mask(CCX);
    gpio_clr_mask(ALL_SEGMENTS);

    gpio_set_mask(bitmask);
    gpio_clr_mask(RIGHT_ON);
}

/*
 * Light left digit
 */
void show_on_left(uint32_t bitmask){
    gpio_set_mask(CCX);
    gpio_clr_mask(ALL_SEGMENTS);

    gpio_set_mask(bitmask);
    gpio_clr_mask(LEFT_ON);
}
#endif
