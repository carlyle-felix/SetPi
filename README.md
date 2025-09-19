# SetPi
### A command-line manager for Raspberry Pi's config.txt


## Features

- Set config keys
- Display values for specified keys
- Display full config
- Create, edit, save and set config profiles
- Bash-completion for commonly used keys

## Installation
Ensure that gcc and git are installed on your system
```
$ gcc --version
$ git --version
```

Clone, compile and install SetPi
```
$ git clone https://github.com/carlyle-felix/setpi.git setpi
$ cd setpi
$ make
# make install
```

## Usage
| `setpi`&nbsp;Options |             |
| ---     | ---         |
| `config`  | Interact with values in current running config |
| `profile` | Interact with profiles of complete configs, if used without actions `setpi profile` returns the current profile a profile was set. |
---
<br>

| `config`&nbsp;actions| `setpi config <action>` |
| --- | --- |
| `--set` \| `-S` | Set a value of any key in config, prompt to create key if it's not already in the config. | 
| `--get` \| `-g` | Return a list of values for requested keys. |
---
<br>

| `profile`&nbsp;actions | `setpi profile <action>` |
| --- | --- |
| `--new` \| `-n` | Create a new profile using the currently applied config a base for the new profile's config
| `--set` \| `-S` | Set a config from a previously created config |
| `--list` \| `-l` | Returns a list of saved profiles
| `--save` \| `-s` | Save the currently running config as a profile
| `--del` \| `-d` | Delete a profile
> Profiles are stored in /etc/setpi/profiles
---
### Examples
- Setting the boot `kernel` in config.txt
```
# setpi config --set kernel kernel8.img
``` 
---
- Getting value of the `dtoverlay` from config.txt
```
# setpi config --get dtoverlay

dtoverlay=vc4-kms-v3d
```
---
- Creating a new profile named **overclock**
```
# setpi profile --new overclock arm_freq 2500 gpu_freq 850
```
---
- Setting the newly created profile named **overclock**
```
# setpi profile --set overclock
```

## Notes

- A reboot is required for changes to be applied.
- The bash-completion list does not list all available keys, see https://www.raspberrypi.com/documentation/computers/config_txt.html for more.