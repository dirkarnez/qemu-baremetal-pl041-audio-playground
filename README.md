qemu-baremetal-pl041-audio-playground
=====================================
```bash
qemu-system-arm -M versatilepb -cpu cortex-a8 -kernel your_baremetal.elf -audiodev dsound,id=win_audio -device pl041,audiodev=win_audio -nographic
```

### Tutorials
- https://github.com/ghaflims/cortex-a9/blob/master/src/pl041.c
