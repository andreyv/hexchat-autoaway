# AutoAway for HexChat

This [HexChat](http://hexchat.org) plugin will automatically mark you away
when your computer is idle. It works on systems with X11, such as GNU/Linux.

## Prerequisites

* HexChat
* X11 and Xss (X Screen Saver extension) libraries
* CMake and pkg-config (for building)

## Building and installation

    cd hexchat-autoaway
    cmake -DCMAKE_BUILD_TYPE=Release .
    make
    sudo make install # Better create a package if you can

You can also simply copy `libautoaway.so` to `~/.config/hexchat/addons`
instead of the last step.

## Configuration

The plugin should work out of the box. To change the default settings,
close HexChat and edit the file `~/.config/hexchat/addon_autoaway.conf`.

Settings list:
* `away_cmd`: The command to execute when going away
  (default: `ALLSERV AWAY Idle`)
* `back_cmd`: The command to execute when returning back
  (default: `ALLSERV BACK`)
* `polling_timeout`: The X status polling period, in seconds (default: 10)
* `idle_time`: The idle time after which the user is considered
  being away, in seconds (default: 600)

To change the away reason, replace the `Idle` part in `away_cmd`. To execute
multiple commands on away/back, create a text file with the commands and set
`away_cmd`/`back_cmd` to `LOAD -e <file>`.

## Other links

* http://haus.nakedape.cc/svn/public/trunk/small-projects/desktop/screensaverAutoAway.py
* https://sites.google.com/site/thvortex/xchat
