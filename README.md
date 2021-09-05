# Subversive rootkit #

## INSTALL ##

### Build and load the kernel module ###

```
cd kernel
make ARCH=x86
insmod subversive.ko
```


### Control rootkit ###

```
cd tools
make
./subversive_ctl -h
```

## UNINSTALL ##

```
rmmod subversive
```


## REFERENCES ##

- IA32 Software Developers Manual Vol. 3B, Chapter 18
- Mistifying the debugger, Phrack 65, halfdead
- Abuso dell Hard Hardware nell Attaco al Kernel di Linux, AntiFork
  Research, Pierre Falda
