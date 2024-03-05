# TENS4OSC
DIY Controllable TENS Stimulator for VRChat Avatars via OSC

<img src="https://github.com/UiharuKazari2008/TENS4OSC/assets/15165770/eae91475-7a30-4f0c-b10a-061e04670fee" height=400px/>

## CAUTION
I am not responsible for if you suffer serious side effects or harm, Your an adult if you are even thinking about using TENS/EMS for activities please read the warnines with the device and do not put the electrides near your heart or sensitive skin. Do not keep the device attached to a PC when active and its suggested that you use a battery to prevent the possibliity of being connected to your AC ground. If you must use a USB isolator.

## Use Case
- ERP
- .... I don't know

## Hardware
- ESP32
- 2 Relays
- AVCOO TENS Stimulator kit from Amazon

![IMG_0172](https://github.com/UiharuKazari2008/TENS4OSC/assets/15165770/dcdab399-0a0b-4a91-86e4-317cf9ab32da)
![IMG_0171](https://github.com/UiharuKazari2008/TENS4OSC/assets/15165770/56e6868c-d0cc-4681-bcf2-6c88fa22e7c4)

* Not Shown, Relays break output + channels. If possible use 2 relays for both positive and nagative

```c

#define DEFAULT_OFF // Device is off by default

// Remove unused buttons to remove the feature
#define POWER_BUTTON 19 // Power Button
#define UP_BUTTON 18    // Intensity Up (REQUIRED)
#define DOWN_BUTTON 5   // Intensity Down (REQUIRED)
#define TIME_BUTTON 14  // Time Setting
#define PUP_BUTTON 12   // Preset Up
#define PDOWN_BUTTON 27 // Preset Down

#define M_PRESS_TIME_FOR_C 8 // Number of presses to enter continuous timer

#define BUTTON_DEBOUNCE_PRESS 45 // Your devices button minimum to press buttonin ms for its debouncer
#define BUTTON_DEBOUNCE_DELAY 50 // Your devices button minimum wait time in ms for its debouncer

// Stop / Disconnect Relays
#define CHANNEL_A_RELAY 16
#define CHANNEL_B_RELAY 17

// Relays On Off State (some relays are active on low or high)
#define RELAY_ON_STATE HIGH
#define RELAY_OFF_STATE LOW

// Max Intensivty Level supported by your device
#define MAX_INTENSITY 30

// WiFi Connection
const char * ssid = "Radio Noise AX";
const char * password = "Radio Noise AX";
```

## Known Issues
* If you swing your from values to quickly there is tendicy for it to get out of sync (Low then expected thankfully)

## Implimentation
* Use VRCOSC to route the output from VRChat to your devices IP Address @ Port 9001
* Add the 4 Parameters
  * TENSLevel (INT)
  * TENSActive (BOOL)
  * TENSESTOP (BOOL)
  * KACommand (INT) (Not Required)

Animate the TENSLevel to set the intensity, Animate the TENSActive Bool to turn the output on or off (if your jumping to higher levels or need sudden output

Use TENSESTOP as a Menu Toggle to Toggle Master Output, Add KACommand Buttons to your meun to send commands like bellow
* 51: Preset Up
* 50: Preset Down
* 54: Resync Intensity
