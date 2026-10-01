# Hand Sanitizer Timer Display

Standalone PlatformIO/Arduino firmware scaffold for an ESP32-2432S028R (often called a CYD) with two bottle-removal inputs, individual countdown durations, six-dot progress displays, touch settings, and saved settings.

## Open and build

Open this directory as a folder in VS Code with PlatformIO installed. Run the PlatformIO **Build** command, or run `pio run` in this directory. Connect the board over USB and use PlatformIO **Upload** after selecting the correct serial port; `platformio.ini` intentionally does not pin a COM port.

## Current behavior

- Each bottle has its own adjustable countdown, initially 10 and 12 minutes. These are example values and can be changed on the settings screen.
- A timer starts when its sensor input indicates the bottle has been removed. Returning the bottle freezes its elapsed time until it is removed again.
- Six dots represent the timer interval; dots disappear as time elapses. The display shows a zero-time red state after timeout.
- Tap the lower-right settings control; use minus/plus to adjust each timer by one minute and tap DONE to return.
- Settings persist in ESP32 Preferences.

## Hardware bring-up before wiring

The board name can refer to revisions with different wiring. The TFT_eSPI pin mapping in `platformio.ini`, touch chip-select in `src/display.cpp`, and sensor inputs in `src/sensors.cpp` are candidate CYD mappings, not confirmed for your particular board. Verify the PCB/revision and test the display and touch before connecting external sensors. Touch raw ranges also need calibration for the actual panel.

TSOP38238 is a demodulating 38 kHz receiver, not a plain phototransistor. A TSAL6200 beam typically needs a correctly current-limited, 38 kHz modulated drive with a burst pattern that the receiver can reliably detect; sustained-carrier/continuous-beam behavior can be rejected by the receiver. Validate the optical circuit and output polarity on the bench. Do not connect the TSAL6200 directly to an ESP32 GPIO; choose its current-limiting resistor and driver circuit from the supply voltage and LED current requirement. The current firmware expects a digital input that is HIGH when its beam is broken and LOW when present, with a pull-up.

ESP32 GPIOs are 3.3 V only. Confirm sensor output voltage compatibility before connecting it. The two candidate sensor pins are GPIO34 and GPIO35, which are input-only on the classic ESP32.

## Source layout

- `src/main.cpp`: initialization and cooperative application loop
- `src/display.*`: TFT rendering and touchscreen input
- `src/sensors.*`: debounced bottle presence inputs
- `src/timer.*`: independent countdown and six-dot progress calculation
- `src/settings.*`: nonvolatile per-bottle duration settings

This scaffold has not been tested on the physical board. The mappings and optical sensor circuit must be checked against the purchased board and actual wiring.