#ifndef LED_H__
#define LED_H__


/*
 * Green LED : GPIO 19
 * Blue LED : GPIO 21
 * Red LED : GPIO 22
 */
typedef enum led_e
{
    LED_GREEN = (1U<<19),
    LED_BLUE = (1U<<21),
    LED_RED = (1U<<22),
    LED_ALL = ((1U<<19)|(1U<<21)|(1U<<22))
} led_t;

void LED_on(led_t led);
void LED_off(led_t led);
void LED_toggle(led_t led);

#endif
