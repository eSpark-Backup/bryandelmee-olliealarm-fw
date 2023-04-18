# Introduction 
LCD Evaluation using rlottie lib. Running in ESP-IDF with LVGL

# Getting Started
1. Install ESP-IDF
    1. TODO add more detail
    
2. Clone LVGL components from e_LVGL_Components in ADO/Electronics
    - 'git clone https://bissell@dev.azure.com/bissell/Electronics/_git/e_LVGL_Components components --recursive'
    - e_LVGL_Components hosts the relevant LVGL libs, must be contained within the ./components subdir

3. LVGL Configuration for DLC_ESP32_Devkit
    1. run 'get_idf' to get idf.py in current loc
    2. run 'idf.py menuconfig'
    3. Component config ----> LVGL Configuration
        - Set max horizontal resolution to 320
        - Set max vertical resolution to 240
        - Set color depth to 16
        - Select swap the 2 bytes of RB565 color
        - Enable anti-aliasing
    4. Component Config ----> LVGL ESP Drivers ----> LVGL TFT Display Controller
        - Set predefined display configuration to NONE
        - Set display orientation to PORTRAIT INVERTED
        - Set predefined board pinouts to NONE
        - Set display controller model to ST7789
        - Set TFT SPI Bus to VSPI
        - Set TFT Data transfer mode to SIO MOSI/MISO
        - Set TFT SPI Duplex mode to HALF DUPLEX
        - Select invert colors in display
    5. LVGL TFT Display Controller ----> Display Pin Assignments
        - Set GPIO for MOSI to GPIO13
        - Do not set GPIO for MISO
        - Set GPIO for CLK to GPIO14
        - Select Use CS signal to control the display
        - Set GPIO for CS to GPIO15
        - Select Use DC signal to control the display
        - Set GPIO for DC to GPIO33
        - Select Use a GPIO for resetting the display
        - Set GPIO for Reset to GPIO25



# Build and Test
1. Run idf.py build
2. Run idf.py flash monitor
    1. Sometimes requires ctrl-t -> ctrl-r to reset display
    2. idf.py flash and idf.py monitor can be run separately




# Contribute
TODO: Explain how other users and developers can contribute to make your code better. 

If you want to learn more about creating good readme files then refer the following [guidelines](https://docs.microsoft.com/en-us/azure/devops/repos/git/create-a-readme?view=azure-devops). You can also seek inspiration from the below readme files:
- [ASP.NET Core](https://github.com/aspnet/Home)
- [Visual Studio Code](https://github.com/Microsoft/vscode)
- [Chakra Core](https://github.com/Microsoft/ChakraCore)