PROGRAM: mac-battery-applet

AUTHOR:  Bill Chatfield <bill_chatfield@yahoo.com>

LICENSE: GPL 3.0

![Screenshot](/docs/images/screenshot.jpg?raw=true "Screenshot of Gnome 3 with the Mac Battery Applet")


DESCRIPTION

This is a Linux PowerPC app for the Gnome 3 desktop which displays the
battery status in an icon on the top panel. It was developed and tested on
Debian Wheezy 7.8. It should work on any PowerPC-based Mac running Linux and 
Gnome 3. It probably won't work on BSD because it reads the battery info
from /proc/pmu.

Because the PowerPC Macs have a different type of power unit (PMU), the normal
battery applets for Intel hardware do not work. There was an emulator but it 
is non-functional now.


REQUIREMENTS

* PowerPC Macintosh with a PMU power unit - G3, G4, G5
* Linux
* Gnome 3


INSTALLATION

The binary is in the src directory. It is, of course, a Linux PowerPC 
executable. Run "make install" to install the binary and the icon files into 
/usr/local.

To manually install, copy src/mac-battery-applet to /usr/local/bin and
copy icon-themes/default/* to 
/usr/local/share/mac-battery-applet/icon-themes/default.


GNOME AUTO-START SETUP

To configure it to start when Gnome is started, add an entry for
/usr/local/bin/mac-battery-applet to "Startup Applications"
(Applications -> System Tools -> Preferences -> Startup Applications).


DEBIAN PACKAGE

I wanted to provide a .deb package. But after reading all the documentation,
rules, commands, processes and procedures associated with doing this, I no
longer have the desire to do this. My goal instead is to make this easy to
compile and install with a simple Makefile. Maybe someone else will suffer
the process of creating a Debian package.
