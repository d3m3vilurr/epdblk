Reimplement [epdblk][epdblk]

[epdblk]: https://bitbucket.org/d3m3vilurr/crema-shine-root-image/src/master/root/system/bin/epdblk

## Improvement than original blob
- fixed segfault if not pass first argument
- initial support Rockchip EBC

## How to build
```bash
NDK_PROJECT_PATH=. ndk-build NDK_APPLICATION_MK=./Application.mk
```

## How to use
copy epdblk to `/system/bin/` and give execute perm
then call

```bash
epdblk 10
```

if you need forceful full refresh, call it

```bash
epdblk 10 1
```
