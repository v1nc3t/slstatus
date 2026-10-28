# slstatus

My build of [slstatus](https://tools.suckless.org/slstatus/), the status monitor from [suckless](https://suckless.org/). It feeds the bar of my dwm build: [v1nc3t/dwm](https://github.com/v1nc3t/dwm).

This repository is a fork of the GitHub mirror [drkhsh/slstatus](https://github.com/drkhsh/slstatus). The official source is [git.suckless.org/slstatus](https://git.suckless.org/slstatus). The upstream `README` is kept unchanged.

## What this build adds

- `backlight`: screen brightness in percent from a sysfs backlight directory.
- `pwsink`, `pwsource`: speaker volume and mic state from PipeWire (`wpctl`), re-read only when `pw-mon` reports a change.
- `wifi_essid` reads the SSID with the wireless extensions ioctl, because nl80211 no longer returns it on kernel 7.1.
- The sleep between updates is not cut short by stray signals. SIGUSR1 still refreshes at once.
- `config.def.h` prints the fields dwm expects, separated by the byte `0x1f`, every 200 ms: mic, volume, brightness, CPU, RAM, network, download, upload, battery, date and time.

The field order must match `statusclicks` and `statusslot` in the dwm `config.def.h`.

## Install

```sh
git clone git@github.com:v1nc3t/slstatus.git ~/.config/slstatus
cd ~/.config/slstatus
make
```

dwm starts `~/.config/slstatus/slstatus` at login. Edit `config.def.h`, not `config.h`, then `rm -f config.h && make` and log out and back in.

Needs `wpctl` and `pw-mon` (PipeWire), and the X11 headers.

## Update from upstream

```sh
git remote add upstream https://git.suckless.org/slstatus   # once
git fetch upstream
git merge upstream/master
make
git push
```

## License

ISC. See `LICENSE`. Upstream copyright stays with the suckless authors.
