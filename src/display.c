#include "includes.h"

/* ************************************************************************** */

const uint8_t swrThreshDisplay[] = { 0x08,0x10,0x20,0x40 };
const uint8_t ledBarTable[] = { 0x00,0x01,0x03,0x07,0x0f,0x1f,0x3f,0x7f,0xff, };
const uint8_t ledSingleTable[] = { 0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80, };

/* ************************************************************************** */

void display_init(void)
{
    spi_init();

    clear_status_LEDs();

    // shell command
    shell_register(shell_show_bargraphs, "bar");
}

/* -------------------------------------------------------------------------- */

void clear_status_LEDs(void)
{
    POWER_LED_PIN = 0;
    ANT_LED_PIN = 0;
    BYPASS_LED_PIN = 0;
}

void update_status_LEDs(void)
{
    POWER_LED_PIN = system_flags.powerStatus;
    ANT_LED_PIN = ~system_flags.antenna;
    BYPASS_LED_PIN = bypassStatus[system_flags.antenna];
}

void update_antenna_LED(void)
{
    ANT_LED_PIN = ~system_flags.antenna;
}

void update_bypass_LED(void)
{
    BYPASS_LED_PIN = bypassStatus[system_flags.antenna];
}

void update_power_LED(void)
{
    POWER_LED_PIN = system_flags.powerStatus;
}

/* ************************************************************************** */

// Publishes a raw frame to the display
void FP_update(uint16_t data)
{
    spi_tx_word(data);
}

// Clears the display by turning off both bargraphs
void display_clear(void)
{
    FP_update(0x0000);
}

// Display a single from an animation
void display_single_frame(const animation_s *animation, uint8_t frame_number)
{
    FP_update(animation[frame_number].image);
}

// Play an animation from animations.h
void play_animation(const animation_s *animation)
{
    uint8_t i = 0;

    while(1)
    {
        FP_update(animation[i].image);
        if (animation[i].frame_delay == NULL) break;
        delay_ms(animation[i].frame_delay);

        i++;
    }
}

// Play an animation from animations.h, and repeat it n times
void repeat_animation(const animation_s *animation, uint8_t repeats)
{
    uint8_t i;

    for (i = 0; i < repeats; i++)
    {
        play_animation(animation);
    }
}

// Play an animation from animations.h and return early if a button is pressed
void play_interruptable_animation(const animation_s *animation)
{
    uint8_t i = 0;
    uint16_t j = 0;

    while(1)
    {
        FP_update(animation[i].image);
        if (animation[i].frame_delay == NULL) break;
        
        for(j = animation[i].frame_delay; j != 0; j--) {
            if (get_buttons() != 0) return;
            delay_ms(1);
        }

        i++;
    }
}

/* -------------------------------------------------------------------------- */

// function-related display functions

void show_peak(void)
{
    if (system_flags.peakMode == 0) {
        play_animation(&peak_off);
    } else {
        play_animation(&peak_on);
    }
}

// function-related, blinky display functions
// THESE HAVE BLOCKING DELAYS
void blink_bypass(void)
{
    if (bypassStatus[system_flags.antenna] == 1) {
        repeat_animation(&blink_both_bars, 3);
    } else {
        show_relays();
        delay_ms(150);
        display_clear();
    }
}

void blink_antenna(void)
{
    if (system_flags.antenna == 1) {
        play_animation(&right_wave);
    } else {
        play_animation(&left_wave);
    }
}

void blink_auto(uint8_t blinks)
{
    if (system_flags.autoMode == 0) {
        repeat_animation(&auto_off, blinks);
    } else {
        repeat_animation(&auto_on, blinks);
    }
}

void blink_HiLoZ(uint8_t blinks)
{
    if (currentRelays[system_flags.antenna].z == 1) {
        repeat_animation(&auto_off, blinks);
    } else {
        repeat_animation(&auto_on, blinks);
    }
}

void blink_scale(uint8_t blinks)
{
    if (system_flags.Scale100W == 0) {
        repeat_animation(&high_scale, blinks);
    } else {
        repeat_animation(&low_scale, blinks);
    }
}

void blink_thresh(uint8_t blinks)
{
    uint8_t i;
    uint16_t out;      
    out = (swrThreshDisplay[swrThreshIndex] << 8);
    
    for (i = 0; i < blinks; i++)
    {
        FP_update(out);
        delay_ms(100);
        FP_update(0x0000);
        delay_ms(100);
    }
}

/* -------------------------------------------------------------------------- */
// function-related, single frame display functions
// THESE HAVE NO DELAYS

void show_auto(void)
{
    if (system_flags.autoMode == 0) {
        FP_update(0x8181);
    } else {
        FP_update(0x1818);
    }
}

void show_HiLoZ(void)
{
    if (currentRelays[system_flags.antenna].z == 1) {
        FP_update(0xc0c0);
    } else {
        FP_update(0x0303);
    }
}

void show_relays(void)
{
    uint16_t out;

    out = ((currentRelays[system_flags.antenna].inds & 0x7f) | ((currentRelays[system_flags.antenna].z & 0x01) << 7));
    out |= (uint16_t)currentRelays[system_flags.antenna].caps << 8;

    FP_update(out);
}

void show_scale(void)
{
    if (system_flags.Scale100W == 0) {
        FP_update(0x0008);
    } else {
        FP_update(0x0080);
    }
}

void show_thresh(void)
{
    FP_update(swrThreshDisplay[swrThreshIndex] << 8);
}

/* -------------------------------------------------------------------------- */

/*  fwdIndex

    AT-600ProII PWR bargraph has the following markings:

    0       <- all bars off still needs a value
    10
    25
    50
    100
    200
    300
    450
    600    
*/

uint16_t fwdIndexArray[] = { 0, 10, 25, 50, 100, 200, 300, 450, 600, };

uint8_t calculate_fwd_index(uint16_t forwardWatts)
{
    uint8_t i = 0;

    while(fwdIndexArray[i] < forwardWatts) i++;

    return i;
}

/*  swrIndex

    AT-600ProII SWR bargraph has the following markings:

    1.0     <- all bars off still needs a value
    1.1
    1.3
    1.5
    1.7
    2.0
    2.5
    3.0
    3.0+
*/

double swrIndexArray[] = { 1.0, 1.1, 1.3, 1.5, 1.7, 2.0, 2.5, 3.0, 3.5, };

uint8_t calculate_swr_index(double swrValue)
{
    uint8_t i = 0;

    while(swrIndexArray[i] < swrValue) i++;

    return i;
}

void show_power_and_SWR(uint8_t fwdIndex, uint8_t swrIndex)
{
    uint16_t out = 0;
    
    out = ledBarTable[fwdIndex];
    out |= ledBarTable[swrIndex];

    FP_update(out);
}

int shell_show_bargraphs(int argc, char** argv)
{
    if(argc == 3)
    {
        print("first arg: ");
        print(argv[1]);
        uint16_t forwardWatts = atoi(argv[1]);
        printf(", forwardWatts: %u\r\n", forwardWatts);

        print("second arg: ");
        print(argv[2]);
        double swrValue = atof(argv[2]);
        printf(", swrValue: %u\r\n", swrValue);

        show_power_and_SWR(calculate_fwd_index(forwardWatts), calculate_swr_index(swrValue));
    }
    return 0;
}