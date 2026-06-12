# -----------------------------------------------------------------------------
# @author    STM32Cube bundle system
# -----------------------------------------------------------------------------
# @attention
#
# Copyright (c) 2025 STMicroelectronics.
# All rights reserved.
#
# This software is licensed under terms that can be found in the LICENSE file
# in the root directory of this software component.
# If no LICENSE file comes with this software, it is provided AS-IS.
#
# -----------------------------------------------------------------------------

# This file describes GNU Tools for STM32 Toolchain.
#
#   - Applies to toolchain: GNU Tools for STM32 Toolchain 13.3.1 and greater

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Some target device related default GCC settings
if(CMSIS_Dcore STREQUAL "Cortex-M0")
  set(CPU_FLAGS -mcpu=cortex-m0)
elseif(CMSIS_Dcore STREQUAL "Cortex-M0+")
  set(CPU_FLAGS -mcpu=cortex-m0plus)
elseif(CMSIS_Dcore STREQUAL "Cortex-M1")
  set(CPU_FLAGS -mcpu=cortex-m1)
elseif(CMSIS_Dcore STREQUAL "Cortex-M3")
  set(CPU_FLAGS -mcpu=cortex-m3)
elseif(CMSIS_Dcore STREQUAL "Cortex-M4")
  if(CMSIS_Dfpu STREQUAL "SP_FPU")
    set(CPU_FLAGS -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
  else()
    set(CPU_FLAGS -mcpu=cortex-m4)
  endif()
elseif(CMSIS_Dcore STREQUAL "Cortex-M7")
  if(CMSIS_Dfpu STREQUAL "DP_FPU")
    set(CPU_FLAGS -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard)
  elseif(CMSIS_Dfpu STREQUAL "SP_FPU")
    set(CPU_FLAGS -mcpu=cortex-m7 -mfpu=fpv5-sp-d16 -mfloat-abi=hard)
  else()
    set(CPU_FLAGS -mcpu=cortex-m7)
  endif()
elseif(CMSIS_Dcore STREQUAL "Cortex-M23")
  set(CPU_FLAGS -mcpu=cortex-m23)
elseif(CMSIS_Dcore STREQUAL "Cortex-M33")
  if(CMSIS_Dfpu STREQUAL "SP_FPU")
    if(CMSIS_Ddsp STREQUAL "DSP")
      set(CPU_FLAGS -mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard)
    else()
      set(CPU_FLAGS -mcpu=cortex-m33+nodsp -mfpu=fpv5-sp-d16 -mfloat-abi=hard)
    endif()
  else()
    if(CMSIS_Ddsp STREQUAL "DSP")
      set(CPU_FLAGS -mcpu=cortex-m33)
    else()
      set(CPU_FLAGS -mcpu=cortex-m33+nodsp)
    endif()
  endif()
elseif(CMSIS_Dcore STREQUAL "Cortex-M35P")
  if(CMSIS_Dfpu STREQUAL "SP_FPU")
    if(CMSIS_Ddsp STREQUAL "DSP")
      set(CPU_FLAGS -mcpu=cortex-m35p -mfpu=fpv5-sp-d16 -mfloat-abi=hard)
    else()
      set(CPU_FLAGS -mcpu=cortex-m35p+nodsp -mfpu=fpv5-sp-d16 -mfloat-abi=hard)
    endif()
  else()
    if(CMSIS_Ddsp STREQUAL "DSP")
      set(CPU_FLAGS -mcpu=cortex-m35p)
    else()
      set(CPU_FLAGS -mcpu=cortex-m35p+nodsp)
    endif()
  endif()
elseif(CMSIS_Dcore STREQUAL "Cortex-M55")
  if(CMSIS_Dfpu STREQUAL "NO_FPU")
    if(CMSIS_Dmve STREQUAL "NO_MVE")
      set(CPU_FLAGS -mcpu=cortex-m55+nofp+nomve)
    else()
      set(CPU_FLAGS -mcpu=cortex-m55+nofp)
    endif()
  else()
    if(CMSIS_Dmve STREQUAL "NO_MVE")
      set(CPU_FLAGS -mcpu=cortex-m55+nomve -mfloat-abi=hard)
    elseif(CMSIS_Dmve STREQUAL "MVE")
      set(CPU_FLAGS -mcpu=cortex-m55+nomve.fp -mfloat-abi=hard)
    else()
      set(CPU_FLAGS -mcpu=cortex-m55 -mfloat-abi=hard)
    endif()
  endif()
elseif(CMSIS_Dcore STREQUAL "Cortex-M85")
  if(CMSIS_Dfpu STREQUAL "NO_FPU")
    if(CMSIS_Dmve STREQUAL "NO_MVE")
      set(CPU_FLAGS -mcpu=cortex-m85+nofp+nomve)
    else()
      set(CPU_FLAGS -mcpu=cortex-m85+nofp)
    endif()
  else()
    if(CMSIS_Dmve STREQUAL "NO_MVE")
      set(CPU_FLAGS -mcpu=cortex-m85+nomve -mfloat-abi=hard)
    elseif(CMSIS_Dmve STREQUAL "MVE")
      set(CPU_FLAGS -mcpu=cortex-m85+nomve.fp -mfloat-abi=hard)
    else()
      set(CPU_FLAGS -mcpu=cortex-m85 -mfloat-abi=hard)
    endif()
  endif()
elseif(CMSIS_Dcore STREQUAL "Cortex-A5")
  if(CMSIS_Dfpu STREQUAL "DP_FPU")
    set(CPU_FLAGS -mcpu=cortex-a5+nosimd -mfpu=auto -mfloat-abi=hard)
  else()
    set(CPU_FLAGS -mcpu=cortex-a5+nosimd+nofp)
  endif()
elseif(CMSIS_Dcore STREQUAL "Cortex-A7")
  if(CMSIS_Dfpu STREQUAL "DP_FPU")
    set(CPU_FLAGS -mcpu=cortex-a7+nosimd -mfpu=auto -mfloat-abi=hard)
  else()
    set(CPU_FLAGS -mcpu=Cortex-a7+nosimd+nofp)
  endif()
elseif(CMSIS_Dcore STREQUAL "Cortex-A9")
  if(CMSIS_Dfpu STREQUAL "DP_FPU")
    set(CPU_FLAGS -mcpu=cortex-a9+nosimd -mfpu=auto -mfloat-abi=hard)
  else()
    set(CPU_FLAGS -mcpu=cortex-a9+nosimd+nofp)
  endif()
endif()

if(CMSIS_Dendian STREQUAL "Little-endian")
  set(CPU_FLAGS ${CPU_FLAGS} -mlittle-endian)
elseif(CMSIS_Dendian STREQUAL "Big-endian")
  set(CPU_FLAGS ${CPU_FLAGS} -mbig-endian)
endif()

set(CPU_FLAGS ${CPU_FLAGS} -mthumb)

# Some C Pre-Processor related default GCC settings
if(CMSIS_Dsecure STREQUAL "Secure" OR CMSIS_Dsecure STREQUAL "Secure-only")
  set(SECURE_FLAGS "-mcmse")
endif()

# Locate the compiler in PATH or in the STM32Cube bundle installation.
set(_STM32_TOOLCHAIN_HINTS)
if(DEFINED ENV{CUBE_BUNDLE_PATH})
  list(APPEND _STM32_TOOLCHAIN_HINTS
    "$ENV{CUBE_BUNDLE_PATH}/gnu-tools-for-stm32/14.3.1+st.2/bin"
  )
endif()

if(DEFINED ENV{LOCALAPPDATA})
  file(GLOB _STM32_BUNDLE_TOOLCHAINS LIST_DIRECTORIES true
    "$ENV{LOCALAPPDATA}/stm32cube/bundles/gnu-tools-for-stm32/*/bin"
  )
  list(SORT _STM32_BUNDLE_TOOLCHAINS COMPARE NATURAL ORDER DESCENDING)
  list(APPEND _STM32_TOOLCHAIN_HINTS ${_STM32_BUNDLE_TOOLCHAINS})
endif()

find_program(STM32_GCC arm-none-eabi-gcc HINTS ${_STM32_TOOLCHAIN_HINTS})
find_program(STM32_GXX arm-none-eabi-g++ HINTS ${_STM32_TOOLCHAIN_HINTS})
find_program(STM32_OBJCOPY arm-none-eabi-objcopy HINTS ${_STM32_TOOLCHAIN_HINTS})
find_program(STM32_SIZE arm-none-eabi-size HINTS ${_STM32_TOOLCHAIN_HINTS})

if(NOT STM32_GCC)
  message(FATAL_ERROR
    "GNU Tools for STM32 was not found. Install the gnu-tools-for-stm32 "
    "bundle in STM32Cube for Visual Studio Code."
  )
endif()

# Some default GCC settings
set(FLAGS                           "-fdata-sections -ffunction-sections -Wl,--gc-sections -Wno-comment")
set(CPP_FLAGS                       "${FLAGS} -fno-rtti -fno-exceptions -fno-threadsafe-statics")

set(CMAKE_C_FLAGS                   ${FLAGS})
set(CMAKE_CXX_FLAGS                 ${CPP_FLAGS})

set(CMAKE_C_COMPILER                "${STM32_GCC}")
set(CMAKE_ASM_COMPILER              ${CMAKE_C_COMPILER})
set(CMAKE_CXX_COMPILER              "${STM32_GXX}")
set(CMAKE_OBJCOPY                   "${STM32_OBJCOPY}")
set(CMAKE_SIZE                      "${STM32_SIZE}")

set(CMAKE_EXECUTABLE_SUFFIX_ASM     ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_C       ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_CXX     ".elf")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
