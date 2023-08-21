# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/eSpark/esp/esp-idf/components/bootloader/subproject"
  "C:/Github/bryandelmee-olliealarm-firmware_lcd/Firmware-Development/AudioCodec/ES8388/build/bootloader"
  "C:/Github/bryandelmee-olliealarm-firmware_lcd/Firmware-Development/AudioCodec/ES8388/build/bootloader-prefix"
  "C:/Github/bryandelmee-olliealarm-firmware_lcd/Firmware-Development/AudioCodec/ES8388/build/bootloader-prefix/tmp"
  "C:/Github/bryandelmee-olliealarm-firmware_lcd/Firmware-Development/AudioCodec/ES8388/build/bootloader-prefix/src/bootloader-stamp"
  "C:/Github/bryandelmee-olliealarm-firmware_lcd/Firmware-Development/AudioCodec/ES8388/build/bootloader-prefix/src"
  "C:/Github/bryandelmee-olliealarm-firmware_lcd/Firmware-Development/AudioCodec/ES8388/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Github/bryandelmee-olliealarm-firmware_lcd/Firmware-Development/AudioCodec/ES8388/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Github/bryandelmee-olliealarm-firmware_lcd/Firmware-Development/AudioCodec/ES8388/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
