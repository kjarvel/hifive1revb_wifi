# hifive1revb_wifi
SiFive HiFive1 Rev B ESP32 - WiFi connection demo

A small demo application that enables the following on a SiFive HiFive1 Rev B RISC-V board:
* UART: 115200 bps
* SPI: 80 KHz
* CPU: 320 MHz
* WiFi in Station mode (client)

## Requirements
* [IAR Embedded Workbench for RISC-V](https://www.iar.com/riscv)
* or [SiFive Freedom Studio](https://www.sifive.com/boards)

## Build instructions
Create an empty Freedom E SDK Software project for:
* Target: `sifive-hifive1-revb`
* Example program: `empty`

Copy the source files from this repository into the `src` folder and build.  

## Usage 
Copy the generated `hex` file into the HiFive USB drive.  
(Or, simply use the pre-built hex file `hifive1_revb_wifi.zip`).

Connect to the UART used for the FE310-G002/HiFive1 console, using for example [Tera Term](https://ttssh2.osdn.jp/index.html.en).  
The output should look like this (press the Reset button on the board if it does not work):
```
Bench Clock Reset Complete

ATE0-->ATE0
OK
AT+BLEINIT=0-->OK
AT+CWMODE=0-->OK

---- HiFive1 Rev B WiFi Demo --------
* UART: 115200 bps
* SPI: 80 KHz
* CPU: 320 MHz
Greetings!
Enter SSID:
Enter Password:
```
After the WiFi connection is made, press ENTER to disable it.  
After that, you can enter any AT commands from [ESP32 AT Instruction Set and Examples](https://www.espressif.com/sites/default/files/documentation/esp32_at_instruction_set_and_examples_en.pdf).  
For example, this will return the IP address:

```
ATE0
AT+CWMODE=1
AT+CWJAP="ssid","pwd"
AT+CIFSR
```


