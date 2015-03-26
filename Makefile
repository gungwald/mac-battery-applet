# Builds battery-applet

# Without this, gtk+-3.0 can't be found.
# It apparently needs adjusted for different machine architectures.
PKG_CONFIG_PATH=/usr/lib/powerpc-linux-gnu/pkgconfig:/usr/share/pkgconfig

APP=battery-applet
OBJS=main.o mac-powerpc-pmu.o
CFLAGS=-g -O2 `pkg-config --cflags gtk+-3.0`
LDFLAGS=`pkg-config --libs gtk+-3.0`

# Eclipse calls this target.
all: $(APP)

$(APP): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(APP)
    
main.o: main.c mac-powerpc-pmu.h

mac-powerpc-pmu.o: mac-powerpc-pmu.c mac-powerpc-pmu.h

clean:
	rm -rf $(OBJS) $(APP)
