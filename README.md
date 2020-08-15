# bclmctl

A Linux userspace tool to read and write the `BCLM` Apple SMC key, which limits the maximum battery charge level on Apple laptops. This program was inspired by [AlDente](https://github.com/davidwernhart/AlDente) for macOS and is based on [smctool](https://github.com/gch1p/smctool).

### Usage

##### Main options

```
-h, --help:               print help
-p, --percent <number>:   value to write to BCLM
```


Reading BCLM:
```
$ bclmctl
BCLM = 80
```
Writing BCLM:
```
$ bclmctl -p 80
BCLM = 80
```

### License

GPLv2
