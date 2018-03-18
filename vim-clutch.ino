/**
 * Vim Clutch v2
 * 
 * Inspired by Aleksandr Levchuk's vim clutch: https://github.com/alevchuk/vim-clutch
 * 
 * For this build, I chose to use an Arduino Uno R3 (ATMega16U2) to behave as the HID.
 * Turns out the stock firmware on the Uno isn't able to present as a HID, but some
 * work was done to use a LUFA stack implementation in ~2012 by darran on hunt.net.nz,
 * which has now gone down, but I was able to find it on Wayback Machine:
 * http://web.archive.org/web/20120301033429/http://hunt.net.nz/users/darran/weblog/faf5e/Arduino_UNO_Keyboard_HID_part_2.html
 * I also found steps to install Windows driver for DFU mode (error "AtLibUsbDfu.dll not found"):
 * https://www.youtube.com/watch?v=KQ9BjKjGnIc Thanks a bunch, MDGrein!
 * 
 * Steps:
 * 1. Load this sketch to the Uno
 * 2. Short the flash reset pins on the Uno
 * 3. Load the Arduino-keyboard.hex (you may need to do more Waybacking to get the download)
 * 
 * After unplugging and replugging the Uno, you should have a working Vim clutch! But
 * if you want to make a change to it, you'll need to short the reset pins on the Uno
 * again and load the appropriate serial flash again:
 * https://github.com/arduino/Arduino/tree/master/hardware/arduino/avr/firmwares/atmegaxxu2/arduino-usbserial
 * 
 * The circuit goes:
 * 5V to left switch terminal
 * GND to 10Kohm to right switch terminal (external pulldown)
 * Pin 2 to right switch terminal
 * 
 * For the switch, I used a momentary pushbutton.  With the pulldown, it's active-high,
 * so I chose "idle" state to be command mode and "engaged" state to be insert mode (append).
 * 
 * I put it all in an enclosure, drilled a hole for the USB cable and pushbutton,
 * and then hinged on a metal plate covered with grip tape over the pushbutton to provide
 * a nice grippy pedal.
 */

#include <Bounce2.h>


uint8_t buf[8] = { 0 };  /* Keyboard report buffer */

#define KEY_LEFT_CTRL 0x01
#define KEY_LEFT_SHIFT  0x02
#define KEY_RIGHT_CTRL  0x10
#define KEY_RIGHT_SHIFT 0x20

#define EDIT_MODE_CHARACTER 0x04
#define COMMAND_MODE_CHARACTER 0x29

#define BUTTON_PIN 2

Bounce debouncer = Bounce();
#define BOUNCE_INTERVAL_MS 1

void sendKeypress(uint8_t key)
{
  // Send keypress
  memset(buf, 0, 8);
  buf[2] = key;
  Serial.write(buf, 8);

  // Send release
  memset(buf, 0, 8);
  Serial.write(buf, 8);
}

void setup() 
{
  // Set up serial for USB keyboard transmission
  Serial.begin(9600);
  delay(200);

  // Set up pushbutton pin
  pinMode(BUTTON_PIN, INPUT); // I attached an external pulldown

  // Set up the debouncer
  debouncer.attach(BUTTON_PIN);
  debouncer.interval(BOUNCE_INTERVAL_MS);
  
}

void loop() {
  // Update the debouncer instance
  debouncer.update();

  // If we caught a rising edge, I pressed the pedal, so enter edit mode:
  if (debouncer.rose()) {
    sendKeypress(EDIT_MODE_CHARACTER);
  }
  // If we caught a falling edge, enter command mode, because I let go of the pedal:
  else if (debouncer.fell()) {
    sendKeypress(COMMAND_MODE_CHARACTER);
  }
}
