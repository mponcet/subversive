# Anima rootkit #

## INSTALL ##

### Build and load the kernel module ###

```
cd kernel
make ARCH=x86
insmod anima.ko
```


### Control rootkit ###

```
cd tools
make
./anima_ctl -h
```

## UNINSTALL ##

```
rmmod anima
```


## REFERENCES ##

- IA32 Software Developers Manual Vol. 3B, Chapter 18
- Mistifying the debugger, Phrack 65, halfdead
- Abuso dell Hard Hardware nell Attaco al Kernel di Linux, AntiFork
  Research, Pierre Falda
