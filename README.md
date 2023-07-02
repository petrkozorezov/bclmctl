# bclmctl

A Linux userspace tool to read and write the `BCLM` and `BFCL` Apple SMC keys, which limits the maximum battery charge level on Apple laptops. This program was inspired by [AlDente](https://github.com/davidwernhart/AlDente) for macOS and is based on [smctool](https://github.com/gch1p/smctool).

### Usage

##### Main options

```
-h, --help:               print help
-p, --percent <number>:   value to write to BCLM and BFCL
```


Reading BCLM:
```
$ bclmctl
BCLM = 80
BFCL = 75
```
Writing BCLM:
```
$ bclmctl -p 60
BCLM = 60
BFCL = 55
```

### License

GPLv2
