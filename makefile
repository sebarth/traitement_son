CC = gcc
CFLAGS = 
LDFLAGS = -lm -lfftw3f -lportaudio -lSDL2 -lSDL2_ttf -lSDL2_gfx -lfreetype
TARGET = audio_processing
SRCS = audio_capture.c fft.c graphs.c rendering.c main.c
OBJS = $(SRCS:.c=.o)

# Cross-compilation settings
CC_WIN = x86_64-w64-mingw32-gcc
CFLAGS_WIN = -I/usr/local/x86_64-w64-mingw32/include \
	-I/usr/x86_64-w64-mingw32/include
LDFLAGS_WIN = -static -L/usr/local/x86_64-w64-mingw32/lib \
			-L/usr/x86_64-w64-mingw32/lib \
			-lm -lfftw3f -lpthread -lportaudio -lSDL2_gfx -lSDL2 -lSDL2_ttf -lmingw32 -lmsvcrt \
			-lwinmm -limm32 -lole32 -loleaut32 -luuid -lsetupapi -lversion -lgdi32 -lrpcrt4
TARGET_WIN = audio_processing.exe

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

win: CC=$(CC_WIN)
win: CFLAGS=$(CFLAGS_WIN)
win: LDFLAGS=$(LDFLAGS_WIN)
win: $(TARGET_WIN)

$(TARGET_WIN): $(OBJS)
	$(CC_WIN) $(CFLAGS_WIN) -o $(TARGET_WIN) $(OBJS) $(LDFLAGS_WIN)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) $(TARGET_WIN)
