# AutoAway for HexChat

This [HexChat](http://hexchat.org) plugin will automatically mark you away
when your computer is idle. It works on systems with X11, such as GNU/Linux.

## Installation

* Clone this repository: `git clone https://github.com/andreyv/hexchat-autoaway.git`,
  or download the [ZIP file](https://github.com/andreyv/hexchat-autoaway/archive/master.zip)
* Move to the `hexchat-autoaway` directory and type `make`
* Copy the resulting `autoaway.so` file to `~/.config/hexchat/addons`
  (create this directory if it doesn't exist)

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
