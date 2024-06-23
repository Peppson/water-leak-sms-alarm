
# water-leak-sms-alarm <img src="./_images/water_icon.svg" style=" width: 40px; margin-bottom: -4px;">  
A small battery-powered device that detects water leakage and sends an SMS alert.  
&nbsp;

## Features
- Completely `off` until water *(or anything conductive)* closes the circuit between two of its legs,  
thereby latching on the "soft latching power circuit".  
Which can later be turned off again by the microcontroller.

- Latches `on` by either water detection or by pressing the test button.

- Sends an SMS alert if water is detected.

- Sends SMS diagnostics if the test button is pressed.

- Built with an ESP32 and PlatformIO, in C++.

&nbsp; 
## Schematic and PCB
KiCad files can be found here: 
[📂 _kicad/](./_kicad/)  
More images in: 
[📂 _images/](./_images/)
<details>
    <summary>
        Parts used
    </summary>   

- ESP32 D1 Mini Node MCU 
- SIM800L module with external antenna  
- 3.3V LDO: MCP1700-3302E (250mA)
- LED: WS2812B  
- P-channel mos: NTR4101PT1G 
- N-channel mos: PMV16XNR  
- Cap 470uF: Electrolytic  
- Cap 1uF: Ceramic  
- Cap 100nF: Ceramic
- Switch: Momentary DPST  
- Conductive legs in copper/brass  
</details>

<details>
    <summary>
        PCB
    </summary>   

> Uses mostly through-hole components for easier soldering.  
> ESP32 is mounted on the bottom side of the PCB.

![PCB](./_images/pcb_schematic.PNG)
&nbsp;
</details>


### Schematic
![Schematic](./_images/schematic_v2.png)

&nbsp;
### Hardware

![Hardware](./_images/hardware.jpg)

&nbsp;
## SMS Example

<img src="./_images/sms_example.png" alt="SMS Example" style="width: 50%;">
