# My simple Linux driver

The driver creates a device for calculating squares of integers.

## Usage

```sh
$ echo 123 | sudo tee /dev/squarer
123
$ sudo cat /dev/squarer
15129
```

## Build and install

```sh
$ make && sudo insmod squarer.ko
```

## Uninstalling

You can reboot your computer or call
```sh
$ sudo rmmod squarer
```