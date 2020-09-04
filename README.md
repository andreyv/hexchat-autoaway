# AutoAway for HexChat

This [HexChat](http://hexchat.github.io/) plugin will automatically mark you away
when your computer is idle. It works on systems that use the GTK+ X11 backend,
such as GNU/Linux.

## Dependencies

* HexChat
* X11 and Xss (X Screen Saver extension) libraries
* GTK+ 2
* CMake and pkg-config (for building)

## Building and installation

    cd hexchat-autoaway
    cmake -DCMAKE_BUILD_TYPE=Release .
    make
    cp libautoaway.so ~/.config/hexchat/addons

## Configuration

The plugin should work out of the box. To change the default settings,
close HexChat and edit the file `~/.config/hexchat/addon_autoaway.conf`.

Settings list:
* `away_msg`: The away message to set (default: `Idle`)
* `idle_time`: The idle time after which the user is considered
  being away, in seconds (default: 600)
* `polling_timeout`: The X status polling period, in seconds (default: 10)
* `away_extra`: Extra per-server command to execute when going away
  (default: empty)
* `back_extra`: Extra per-server command to execute when returning back
  (default: empty)
* `away_nick_suffix`: The string to be appended to nick name. e.g. `|away`
  (default: empty)
* `away_nick_net`: The comma-seperated list of network that `away_nick_suffix` applied when away. e.g. `net1,net2`
  (default: empty)

To execute multiple extra commands on away/back, create a text file with
the commands and set `away_extra`/`back_extra` to `LOAD -e <file>`.

## Related software

* https://github.com/yrro/xchat-idle
* https://github.com/TingPing/plugins/blob/master/HexChat/duplicates/screensaver.py
* https://sites.google.com/site/thvortex/xchat
