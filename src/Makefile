# Builds battery-applet

PREFIX_DIR=/usr/local

# Without this, gtk+-3.0 can't be found.
# It apparently needs adjusted for different machine architectures.
PKG_CONFIG_PATH=/usr/lib/powerpc-linux-gnu/pkgconfig:/usr/share/pkgconfig

APP=mac-battery-applet
OBJS=main.o mac-powerpc-pmu.o
ICON_THEMES_DIR=$(PREFIX_DIR)/share/$(APP)/icon-themes
CFLAGS=-g -O2 -DICON_THEMES_DIR=$(ICON_THEMES_DIR) `pkg-config --cflags gtk+-3.0`
LDFLAGS=`pkg-config --libs gtk+-3.0`

# Eclipse calls this target.
all: $(APP)

$(APP): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(APP)
    
# The compile command is defined by default. We just need to provide dependencies.
main.o: main.c mac-powerpc-pmu.h

mac-powerpc-pmu.o: mac-powerpc-pmu.c mac-powerpc-pmu.h

clean:
	rm -rf $(OBJS) $(APP)

install:
	sudo install --backup=numbered --compare $(APP) $(PREFIX_DIR)/bin
	sudo install -d $(ICON_THEMES_DIR)/default
	sudo install ../icon-themes/default/* $(ICON_THEMES_DIR)/default
