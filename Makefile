CROSS_COMPILE ?= powerpc-eabi-

#CC = wine ~/GitClones/PetariDeps/Compilers/Wii/1.3/mwcceppc.exe
CC = /opt/devkitpro/devkitPPC/bin/powerpc-eabi-gcc
#AS = wine ~/GitClones/PetariDeps/Compilers/Wii/1.3/mwasmeppc.exe
AS = /opt/devkitpro/devkitPPC/bin/powerpc-eabi-as
LD = /opt/devkitpro/devkitPPC/bin/powerpc-eabi-ld
KAMEK ?= ~/dotnet/dotnet ~/GitClones/Kamek/Kamek/bin/Debug/net6.0/Kamek.dll
PETARI = ~/GitClones/Petari

CFLAGS ?= -Os -Wall -I $(PETARI)/libs/RVL_SDK/include -I $(PETARI)/libs/MSL_C/include

ADDRESS ?= 0x80002A00

O_FILES = net.o khooks.o substitute.o packets.o

REGION ?= us
SYMBOL_MAP ?= symbols-$(REGION).txt
DEFINES += -D$(REGION)

ifneq ($(BUBBLE),)
	DEFINES += -DBUBBLE
endif

ifneq ($(SPLITSCREEN),)
	DEFINES += -DSPLITSCREEN
	O_FILES += splitscreen.o
endif

all: multiplayerpatch.xml multiplayerpatch.ini

clean:
	rm -f $(O_FILES) multiplayerpatch.o multiplayerpatch.xml multiplayerpatch.ini


%.o: %.s
	$(AS) -o $@ $<

%.o: %.S
	$(CC) $(DEFINES) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) $(DEFINES) -c -o $@ $<

# Kamek can't link symbols across multiple object files, so
# combine all our object files into one and pass that to Kamek.
multiplayerpatch.o: $(O_FILES)
	$(LD) --relocatable -o $@ $(O_FILES)

multiplayerpatch.xml: multiplayerpatch.o
	$(KAMEK) -static=$(ADDRESS) -externals=$(SYMBOL_MAP) -output-riiv=$@ $<

multiplayerpatch.ini: multiplayerpatch.o
	$(KAMEK) -static=$(ADDRESS) -externals=$(SYMBOL_MAP) -output-dolphin=$@ $<
