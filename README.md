# My simple Linux driver

The driver creates a device for calculating squares of integers.

## Usage

```sh
$ echo 123 > /dev/squarer
$ cat /dev/squarer
15129
```

## Build and install

```sh
$ make && sudo make install
```

## Uninstalling

You can reboot your device or call
```sh
$ sudo make uninstall
```