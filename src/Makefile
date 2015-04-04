# Builds battery-applet

# Define top directory of install. Can be /usr, /usr/local or /opt.
# The code is written to automatically figure out where it is.
PREFIX_DIR=/usr/local

# Without this, gtk+-3.0 can't be found.
# It apparently needs adjusted for different machine architectures.
PKG_CONFIG_PATH=/usr/lib/powerpc-linux-gnu/pkgconfig:/usr/share/pkgconfig

APP=mac-battery-applet
OBJS=main.o mac-powerpc-pmu.o files.o
ICON_THEMES_DIR=$(PREFIX_DIR)/share/$(APP)/icon-themes
CFLAGS=-g -O2 -Wall `pkg-config --cflags gtk+-3.0`
LDFLAGS=`pkg-config --libs gtk+-3.0`

# Eclipse calls this target.
all: $(APP)

$(APP): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(APP)
    
# The compile command is defined by default. We just need to provide dependencies.
main.o: main.c mac-powerpc-pmu.h strings.h files.h

mac-powerpc-pmu.o: mac-powerpc-pmu.c mac-powerpc-pmu.h

files.o: files.c files.h

clean:
	rm -rf $(OBJS) $(APP)

install:
	sudo install --backup=numbered --compare $(APP) $(PREFIX_DIR)/bin
	sudo install -d $(ICON_THEMES_DIR)/default
	sudo install ../icon-themes/default/* $(ICON_THEMES_DIR)/default
