# Makefile for JKA

CC = gcc
#-masm=intel goes after -m32
CFLAGS = --verbose -DNDEBUG -fPIC -shared -m32 -O3 -static -shared -rdynamic -nostdlib -lgcc -Wall -fno-strict-aliasing -I. -I../shared/ -I../libraries/
LDFLAGS = --verbose -m32 -shared -L../libraries/openssl -lcrypto -lssl

TARGET = galaxiessvi386.so
# Game files
OBJECTS = gls_bspdecrypter.o gls_challenge.o gls_crashreport.o gls_engine.o \
gls_enginefuncs.o gls_hrtimer.o gls_main.o gls_stresslevel.o

# Base128 files
OBJECTS += ../libraries/encoding/base128.o

# Base64 files
OBJECTS += ../libraries/encoding/base64.o

# BitStream files
OBJECTS += ../libraries/encoding/bitstream.o

# cJSON files
OBJECTS += ../libraries/json/cJSON.o

# LibUDis86 files
OBJECTS += ../libraries/libudis86/decode.o ../libraries/libudis86/input.o \
../libraries/libudis86/itab.o ../libraries/libudis86/syn-att.o \
../libraries/libudis86/syn-intel.o ../libraries/libudis86/syn.o \
../libraries/libudis86/udis86.o

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
#	rm -f $(TARGET) $(OBJECTS)
	rm -f $(OBJECTS)


