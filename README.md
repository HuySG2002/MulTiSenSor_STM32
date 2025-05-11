# STM32F401RE 

The project focuses on how to use multiple sensors in a system. The main goal is to research and utilize various communication protocols commonly used in embedded 
systems such as SPI, I2C, UART, etc. The development of a multi-sensor system can be applied to a variety of different topics. The application is built with C using STM32CubeIDE and the STM32 HAL library for the STM32F401RE Nucleo board.


---

*Development Environment
**IDE:** STM32CubeIDE v1.15.0
**Board:** Nucleo STM32F401RE
**Language:** C
**SDK Used:** ‘SDK_1.0.3_NUCLEO-F401RE’

---

Project Setup Instructions

 Step 1: Install STM32CubeIDE

 Step 2: Create a New STM32 Project

1. Open STM32CubeIDE.
2. Go to: File > New > STM32 Project.
3. In the **Commercial Part Number** field, search for ‘STM32F401RE’.
4. Select the **Nucleo-64 Board** (NUCLEO-F401RE) and proceed.
5. In the **Target Project Type** window, select the ‘Empty’ template.

---

Step 3: Link External SDK to Project

1. Right-click your project → ‘Properties’.
2. Navigate to: ‘C/C++ General > Paths and Symbols > Source Location’.
3. Click ‘Link Folder’ → Click ‘Link to folder in the file system’.
4. Click ‘Browse’, select the SDK folder: ‘SDK_1.0.3_NUCLEO-F401RE’, and confirm.

---

 Step 4: Configure Build Settings
4.1. MCU Settings
- In ‘C/C++ Build > Settings > MCU Settings’:
  - Set **Floating Point Unit** to: ‘None’.

 4.2. MCU GCC Compiler
- Go to: ‘C/C++ Build > Settings > MCU GCC Compiler > Include Paths’.
- Click the ‘+’ icon to add paths from the SDK.
- Use ‘Workspace’ mode and add folders like:

   ![image](https://github.com/user-attachments/assets/1f6cd885-6255-40c0-8a25-ce0d0130ca50)


-	Choose all files in “shared” folder.
EX: shared-> drivers -> CMSIS -> Include -> OK. 
-	Do not include the ‘lib/’ folder in the include paths.

 ![image](https://github.com/user-attachments/assets/15a1221d-ab81-4a84-90b1-b688b774bacb)


4.3. MCU GCC Linker
- Go to: ‘C/C++ Build > Settings > MCU GCC Linker > Libraries > Library Search Path’.
- Add: SDK -> lib (Library seacrch path (-L) )
- Click on “+” icon in Libraries (-I) and write “Libraries” in the box.

![image](https://github.com/user-attachments/assets/65917548-fbfc-4291-8b00-fbdc84b7c801)

---
Step 5:  In main.c, you can use Src/ folder to generate the code.

- **Light_led.c**  
  Demonstrates how to control the brightness of an LED using PWM, based on the ambient light level measured by a light sensor.

- **MulTiSenSor.c**  
  Demonstrates how to read and display data from multiple sensors (temperature, light, and humidity). If any of the sensor values exceed the normal range, the system will trigger an alert by turning on a red LED.
