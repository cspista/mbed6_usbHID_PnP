/* mbed6_USBHID_pnp
 *
 * This is a simple Generic HID demo, which is more or less
 * compatible with the USB HID PnP demo example from the
 * Microchip Applications Library, so that the PC application 
 * from that package can be used with our firmware as well
 * (after two small modification).
 *
 * The differences:
 * - We use 12 bit ADC resolution (instead of 10 bit)
 * - We use Vid=0x0483, Pid=0x5750 which are the official
 *   STM32 Custom Human Interface Device identifiers from
 *   ST Microelectronics.
 *
 * Tested with mbed 6.16.0 and online Keil Studio 1.5.50
 */

#include "mbed.h"
#include "usb/USBHID.h"

//We declare a USBHID device. Here the input and output reports are 64 bytes long.
USBHID hid(true,64,64,0x0483,0x5750); // Vid/Pid: STM32 Custom Human Interface Device

HID_REPORT send_report;          //This report will contain data to be sent
HID_REPORT recv_report;          //This report will contain data received

DigitalOut LED_1(LED1);          //Buitin LED at PA5
DigitalIn SW1(BUTTON1,PullUp);   //Builtin button at PC13
AnalogIn adc(A0);                //Analog input at A0 (PA0)

int main(void) {
    uint16_t val = 0;   
    send_report.length = 64;
    LED_1 = 0;
    for (int i = 0; i < send_report.length; i++)                // Fill the report
        send_report.data[i] = 0x00;
    while (1) {
        if(hid.read_nb(&recv_report)) {                          // try to read a msg
            switch(recv_report.data[0]) {
                case 0x80: //Toggle LED state
                    LED_1 = !LED_1;
                    break;
                case 0x81: //Get push button state
                    send_report.data[0] = recv_report.data[0];
                    send_report.data[1] = SW1;
                    send_report.data[2] = 0;
                    hid.send(&send_report);                    // Send the report
                    break;
                case 0x37: //Read POT command.
                    send_report.data[0] = recv_report.data[0];
                    val = adc.read_u16()>>4;                   // Convert to 12-bits
                    send_report.data[1] = val & 0xFF;          // Measured value LSB
                    send_report.data[2] = val >> 8;            // Measured value MSB
                    hid.send(&send_report);                    // Send the report
                    break;
                default:  {
                    send_report.data[0] = 0xFF;                // Invalid command
                    hid.send(&send_report);                    // Send the report
                }
            }
        }
    }
}