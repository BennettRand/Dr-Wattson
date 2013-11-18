CONFIG = Release

.PHONY: all directory clean size

DEVICE = atmega256rfr2
TARGET = meshTest

STACK_PATH = ../LwMesh_1_1_1/
APP_PATH = .

ATPROGRAM = "C:\Program Files (x86)\Atmel\Atmel Studio 6.1\atbackend\atprogram.exe"
PROG_INTERFACE = edbg
PROG_PROTOCOL = jtag

CC = avr-gcc
OBJCOPY = avr-objcopy
SIZE = avr-size

CFLAGS += -W -Wall --std=gnu99 -Os
CFLAGS += -fdata-sections -ffunction-sections -fpack-struct -fshort-enums
CFLAGS += -funsigned-char -funsigned-bitfields
CFLAGS += -mmcu=atmega256rfr2
CFLAGS += -MD -MP -MT $(CONFIG)/$(*F).o -MF $(CONFIG)/$(@F).d

ifeq ($(CONFIG), Debug)
  CFLAGS += -g
endif

LDFLAGS += -Wl,--gc-sections
LDFLAGS += -mmcu=atmega256rfr2


INCLUDES += \
  -I$(STACK_PATH)/hal/atmega256rfr2/inc \
  -I$(STACK_PATH)/phy/atmega256rfr2/inc \
  -I$(STACK_PATH)/nwk/inc \
  -I$(STACK_PATH)/sys/inc \
  -I$(APP_PATH) 

SRCS += \
  $(STACK_PATH)/hal/atmega256rfr2/src/hal.c \
  $(STACK_PATH)/hal/atmega256rfr2/src/halTimer.c \
  $(STACK_PATH)/phy/atmega256rfr2/src/phy.c \
  $(STACK_PATH)/nwk/src/nwk.c \
  $(STACK_PATH)/nwk/src/nwkDataReq.c \
  $(STACK_PATH)/nwk/src/nwkSecurity.c \
  $(STACK_PATH)/nwk/src/nwkFrame.c \
  $(STACK_PATH)/nwk/src/nwkGroup.c \
  $(STACK_PATH)/nwk/src/nwkRoute.c \
  $(STACK_PATH)/nwk/src/nwkRouteDiscovery.c \
  $(STACK_PATH)/nwk/src/nwkRx.c \
  $(STACK_PATH)/nwk/src/nwkTx.c \
  $(STACK_PATH)/sys/src/sys.c \
  $(STACK_PATH)/sys/src/sysTimer.c \
  $(STACK_PATH)/sys/src/sysEncrypt.c \
  $(APP_PATH)/main.c 

DEFINES += \
  -DPHY_ATMEGA256RFR2 \
  -DHAL_ATMEGA256RFR2 \
  -DPLATFORM_XPLAINED_PRO \
  -DF_CPU=8000000 

CFLAGS += $(INCLUDES) $(DEFINES)

OBJS = $(addprefix $(CONFIG)/, $(notdir %/$(subst .c,.o, $(SRCS))))

all: directory $(CONFIG)/$(TARGET).elf $(CONFIG)/$(TARGET).hex $(CONFIG)/$(TARGET).bin size

$(CONFIG)/$(TARGET).elf: $(OBJS)
	@echo LD $@
	@$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

$(CONFIG)/$(TARGET).hex: $(CONFIG)/$(TARGET).elf
	@echo OBJCOPY $@
	@$(OBJCOPY) -O ihex -R .eeprom $^ $@

$(CONFIG)/$(TARGET).bin: $(CONFIG)/$(TARGET).elf
	@echo OBJCOPY $@
	@$(OBJCOPY) -O binary -R .eeprom $^ $@

%.o:
	@echo CC $@
	@$(CC) $(CFLAGS) $(filter %$(subst .o,.c,$(notdir $@)), $(SRCS)) -c -o $@

directory:
	@gmkdir -p $(CONFIG)

size: $(CONFIG)/$(TARGET).elf
	@echo size:
	@$(SIZE) -t $^

clean:
	@echo clean
	@-rm -rf $(CONFIG)
	
program: $(CONFIG)/$(TARGET).hex
	@echo programming
	$(ATPROGRAM) -t $(PROG_INTERFACE) -i $(PROG_PROTOCOL) -d $(DEVICE) program --flash --format hex -f $(CURDIR)/$(CONFIG)/$(TARGET).hex

-include $(wildcard $(CONFIG)/*.d)
