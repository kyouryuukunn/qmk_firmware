SRC += matrix.c spis.c
# MCU name
MCU_FAMILY = PICO
MCU_SERIES = RP2040
MCU = cortex-m0plus

PICO_FLASH_SPI_CLKDIV = 8

CUSTOM_MATRIX = lite
VIA_ENABLE = yes
POINTING_DEVICE_ENABLE = yes
DYNAMIC_MACRO_ENABLE = yes

include users/sekigon/host_os_eeconfig/rules.mk
include users/sekigon/use_layer_as_combo_config/rules.mk
