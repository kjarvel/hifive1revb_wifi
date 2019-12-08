#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "uart.h"
#include "cpu.h"
#include "spi.h"
#include "led.h"

#define DELAY           20000000
#define BAUDRATE_115200 115200
#define SPICLOCK_80KHZ  80000
#define STR_LEN         1024
#define BUF_LEN         4096
#ifdef __ICCRISCV__
#define fflush(a)
#endif

static char *tty_gets(char *str_p, uint32_t size);
static void get_ssid_pwd(char *ssid, char *pwd, uint32_t size);

static const uint32_t interactive = 1; // Set to 0 to use hardcoded SSID and pwd
static char wifi_ssid[STR_LEN] = "AndroidAPDE9B";
static char wifi_pwd[STR_LEN] = "isll3425";
static char at_cmd[STR_LEN*2];
static char recv_str[BUF_LEN];

int main(void)
{
    LED_off(LED_ALL);
    LED_on(LED_RED);

    cpu_clock_init();
    uart_init(BAUDRATE_115200);

    printf("---- HiFive1 Rev B WiFi Demo --------\r\n");
    printf("* UART: 115200 bps\r\n");
    printf("* SPI: 80 KHz\r\n");
    printf("* CPU: 320 MHz\r\n");

    delay(DELAY);

    if (interactive) {
        get_ssid_pwd(wifi_ssid, wifi_pwd, STR_LEN);
    }

    printf("* SSID = %s\r\n", wifi_ssid);
    printf("* Password = %s\r\n\r\n", wifi_pwd);

    spi_init(SPICLOCK_80KHZ);

    LED_off(LED_RED);
    LED_on(LED_BLUE);

    printf("[+] ESP32 reset\r\n");
    spi_send("AT+RST\r\n");
    delay(DELAY);

    // Set WiFi Station Mode
    spi_send("AT+CWMODE=1\r\n");

    // Connect to the AP
    snprintf(at_cmd, sizeof(at_cmd), "AT+CWJAP=\"%s\",\"%s\"\r\n",
             wifi_ssid, wifi_pwd);

    spi_send(at_cmd);

    LED_off(LED_BLUE);
    LED_on(LED_GREEN);

    if (!interactive) {
        while (1) {}
    }

    printf("Press ENTER to disconnect....");
    fflush(stdout);
    while (NULL == tty_gets(wifi_ssid, sizeof(wifi_ssid))) {}
    printf("\r\n");
    spi_send("AT+CWQAP\r\n");
    spi_send("AT+CWMODE=0\r\n");

    printf("* Optional: Enter AT commands (see \"ESP32 AT Instruction Set and Examples\")\r\n");
    while(1) {
        if (TRANS_ON == spi_transparent()) {
            printf("* ----> ");
        } else {
            printf("* Enter AT command: ");
        }
        fflush(stdout);
        while (NULL == tty_gets(wifi_ssid, sizeof(wifi_ssid))) {}
        printf("\r\n");
        snprintf(at_cmd, sizeof(at_cmd), "%s\r\n", wifi_ssid);
        spi_send(at_cmd);
        if (TRANS_OFF == spi_transparent()) {
            spi_recv(recv_str, sizeof(recv_str));
        }
    }
}

//----------------------------------------------------------------------
// A gets function that stops for \n AND \r (good with PUTTY)
//----------------------------------------------------------------------
static char *tty_gets(char *str_p, uint32_t size)
{
    uint32_t i;
    int32_t c;

    for (i = 0; i < size; i++) {
        c = uart_getchar();
        if (c == '\n' || c == '\r') {
            str_p[i] = '\0';
            return str_p;
        } else if (c > 0xFF) {
            return NULL;
        }
        str_p[i] = c;
    }

    if (size > 0) {
        str_p[size-1] = '\0';
    }
    return str_p;
}


//----------------------------------------------------------------------
// Hack to read ssid and pwd from terminal
//----------------------------------------------------------------------
static void get_ssid_pwd(char *ssid, char *pwd, uint32_t size)
{
    printf("Greetings!\r\n");
    printf("Enter SSID: ");
    fflush(stdout);
    while (NULL == tty_gets(ssid, size)) {}
    printf("\r\n");

    printf("Enter Password: ");
    fflush(stdout);
    while (NULL == tty_gets(pwd, size)) {}
    printf("\r\n");
}
