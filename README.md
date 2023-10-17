# Rubber DuckHunt

Rubber DuckHunt is a program that captures key presses and keeps track of certain statistics about the captured key presses, such as the time between presses and the keys that have been pressed. It also has logic for detecting abnormal keystroke patterns, such as an unusual keystroke speed, and can output this information to the console or the Windows event log..

## Hackfest 2023 presentation (Francais)
https://tome.app/ccnb-cyse-piratageethique/rubber-duckhunt-detection-de-lattaque-par-injection-de-frappe-clmuneydo03vyo77c98y0bhai


## Building

This program can be built on Windows with Visual Studio, using the following steps:
1. Open the project in Visual Studio.
2. Build the project in Release mode.
3. Run the program 

## Features
- Track key presses and statistics about the captured key presses, such as the time between presses and the keys that have been pressed.
- Detect unusual keystroke speed.
- Detect when a new keyboard device is plugged in
- Output information to the console and Windows event log.

## Usage

The program can be used to monitor for keystroke injection attacks.
It should detect every keystroke from any keyboard, the typing speed and if new devices as been plugged. 

If verbose is active (default), The output will show you the Detection Flags status.  
If something suspicious is detected, you can find the log in the Windows Application Events with ID 1337.

When in DEBUG mode,   ESC key to close the program

## Future Features
- Encrypt keystrokes in memory.
- Add program menu to configure parameters.
- Log events with malicious payloads.
- Intercept keystrokes when KeystrokeInjection is confirmed.
- Implement an authorization mechanism to add additional keyboards.

## Note

It's important to keep in mind that the use of keyloggers is generally illegal and unethical, unless it is done with the informed consent of the people being monitored. Additionally, it is a best practice to keep this kind of software on a secure network, and the use should be compliant with the laws and regulations of the countries or states where the software will be used.

