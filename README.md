# TENS4OSC
DIY Controllable TENS Stimulator for VRChat Avatars via OSC

![IMG_0172](https://github.com/UiharuKazari2008/TENS4OSC/assets/15165770/dcdab399-0a0b-4a91-86e4-317cf9ab32da)

## Use Case
- ERP
- .... I don't know

## Hardware
- ESP32
- 2 Relays
- AVCOO TENS Stimulator kit from Amazon

![IMG_0171](https://github.com/UiharuKazari2008/TENS4OSC/assets/15165770/56e6868c-d0cc-4681-bcf2-6c88fa22e7c4)

* Not Shown, Relays break output + channels. If possible use 2 relays for both positive and nagative

```c
// Remove unused buttons to remove the feature
#define POWER_BUTTON 19 // Power Button
#define UP_BUTTON 18    // Intensity Up (REQUIRED)
#define DOWN_BUTTON 5   // Intensity Down (REQUIRED)
#define TIME_BUTTON 14  // Time Setting
#define PUP_BUTTON 12   // Preset Up
#define PDOWN_BUTTON 27 // Preset Down

#define M_PRESS_TIME_FOR_C 8 // Number of presses to enter continuous timer

#define BUTTON_DEBOUNCE_DELAY 45 // Your devices button minimum wait time in ms for its debouncer

// Stop / Disconnect Relays
#define CHANNEL_A_RELAY 16
#define CHANNEL_B_RELAY 17

// Relays On Off State (some relays are active on low or high)
#define RELAY_ON_STATE HIGH
#define RELAY_OFF_STATE LOW

// Max Intensivty Level supported by your device
#define MAX_INTENSITY 30
```

## Implimentation
* Use VRCOSC to route the output from VRChat to your devices IP Address @ Port 9001
* Add the 4 Parameters
  * TENS_Level (INT)
  * TENS_Active (BOOL)
  * TENS_ESTOP (BOOL)
  * ActiveCommand (INT) (Not Required)

Animate the TENS_Level to set the intensity, Animate the TENS_Active Bool to turn the output on or off (if your jumping to higher levels or need sudden output

Use TENS_ESTOP as a Menu Toggle to Toggle Master Output, Add ActiveCommand Buttons to your meun to send commands like bellow
* 51: Preset Up
* 50: Preset Down
* 54: Resync Intensity
