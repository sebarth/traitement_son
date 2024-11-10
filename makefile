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
LDFLAGS_WIN = -L/usr/local/x86_64-w64-mingw32/lib \
			-L/usr/x86_64-w64-mingw32/lib
LIBS = -lm -lfftw3f -lpthread -lportaudio -lSDL2 -lSDL2_ttf -lSDL2_gfx
TARGET_WIN = audio_processing.exe

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

win: $(OBJS)
	$(CC_WIN) $(CFLAGS_WIN) -o $(TARGET_WIN) $(OBJS) $(LDFLAGS_WIN) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) $(TARGET_WIN)