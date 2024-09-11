# MiniCO2

The MiniCO2 is a miniature USB-powered CO2 sensor that can be used 
anywhere you need a simple indicator of CO2. Its built-in LEDs light up green, orange, or red, depending on the 
amount of CO2 in the air. The default levels can be adjusted to suit your needs.
<div style="text-align: center;">
<img src="docs/src/images/v1_top_pcb.jpg" width="300">
</div>

The image above is of a prototype. The final version will have a USB-C connector and wireless connectivity. 

## I want one!
The MiniCO2 is under development and cannot be purchased yet. But you can register your interest by adding your email
to the bottom of this form: https://docs.google.com/forms/d/e/1FAIpQLSczLEsP1Wsz9qHPjULFHvHtzbTa30NaE-r1zrS9D4k2iNyR5Q/viewform?usp=sf_link

Once finished, the MiniCO2 will be available for purchase on Tindie.

## Repo status
The repository is an active work in progress. Currently, only the KiCAD design files are provided. I intend to add:
 - CAD files for a 3D-printable enclosure
 - Firmware
 - Control application
 - Python example snippets on how to receive data


## Compatibility
The MiniCO2 will work with any standard USB type C port that can supply power. That means it can be plugged in nearly 
anywhere: Laptops, displays, chargers, battery banks, etc. It will also have wireless connectivity thanks to an 
ESP32-C6 module. This provides Wi-Fi, Bluetooth, Bluetooth Low Power, Thread, and Zigbee.

## Licenses
The MiniCO2 firmware is licensed under the GNU General Public License v3. The MiniCO2 hardware is licensed under 
CERN-OHL-S v2. This includes the PCB design files and enclosure design files. As an individual you are free to make your 
own MiniCO2, or to modify it, as long as these licenses are respected.
