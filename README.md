# KVM Virtual Game Machine

A minimal **hypervisor (VMM) written from scratch on the Linux KVM API**, together with a small bare-metal **x86 guest OS** – every device exists both as an **emulated** device and as a **paravirtualized** one.

> hepia (HES-SO Geneva) · Advanced Virtualization · 2024-2025

## Overview

The virtualized platform is a small 32-bit "game machine": one IA-32 CPU, 512 KB of RAM and six devices. The project has two sides:

- **`vmm/` – the hypervisor**: creates the VM through `/dev/kvm`, loads the guest at physical address 0, runs the vCPU and handles every VM exit (port I/O, MMIO), then renders the screen with SDL2.
- **`guest/` – the guest OS**: a freestanding x86 kernel (GDT/IDT, IRQs) with a driver for each device, compiled either for **emulated hardware** (`PV=0`) or for **paravirtualized devices** (`PV=1`).

| Device | Emulated (full virtualization) | Paravirtualized (hypercall) |
|---|---|---|
| Console | – | ✅ |
| Timer | ✅ | ✅ |
| Display init | ✅ | ✅ |
| IDE disk | ✅ | ✅ |
| Sprite engine (8 sprites, RGBA, 60 Hz) | ✅ | ✅ |
| Keyboard (interrupt-driven) | – | ✅ |

## How it works

**Emulation.** The guest programs the devices through their hardware registers. Each register access traps into the VMM, which follows the device's protocol with a **table-driven state machine**: every step describes the expected access (write to check, value to store, value to inject on a read) and the last step runs a callback that performs the actual operation (read a disk sector, upload a sprite, …).

**Paravirtualization.** The guest writes the parameters into a **4 KB shared buffer mapped at 3 GB** in its address space, then writes the hypercall number to port `0xABBA`. A single VM exit is enough for the VMM to perform the whole operation – which is why paravirtualized devices are both simpler and much faster than emulated ones.

**Threads and interrupts.** The vCPU runs in its own thread, while the main thread refreshes the SDL2 window at 60 Hz and reads the keyboard; key presses are delivered to the guest as interrupts through the in-kernel IRQ chip (`KVM_IRQ_LINE`). A semaphore synchronises the display initialisation between the two threads.

## Build & run

Requirements: Linux with KVM (`/dev/kvm`), `gcc` with 32-bit support, `nasm`, `qemu-img` and SDL2.

```bash
# Debian / Ubuntu
sudo apt install build-essential gcc-multilib nasm qemu-utils libsdl2-dev libsdl2-image-dev
# Arch
sudo pacman -S base-devel lib32-gcc-libs nasm qemu-img sdl2 sdl2_image

make run_pv      # guest with paravirtualized drivers
make run_phys    # guest with drivers for emulated hardware
make             # list all targets
```

The VMM can also be run directly: `vmm/vmm GUEST_BINARY DISK_IMAGE`.

## Project structure

```
├── vmm/        hypervisor: KVM setup, VM-exit handlers, device state machines, SDL2 display
├── guest/      guest OS: boot, IDT/IRQ, one folder per device (*_phys.c / *_pv.c)
└── shared/     definitions shared by the VMM and the guest (hypercall parameters, IDE registers)
```

## Tech

C · Linux KVM API · x86 (IA-32) · NASM · port I/O / MMIO emulation · hypercalls · pthreads / semaphores · SDL2

## Credits & license

My code is released under the [MIT license](LICENSE), except for:

- the guest skeleton provided by the course (boot code, GDT/IDT, port I/O helpers, `stdio`, linker script, guest Makefile) and the Tux sprite, as well as the starting versions of the `shared/` headers and of the emulated IDE driver, which I then extended;
- `vmm/gfx.c` and `vmm/gfx.h`, from [gfxlib](https://github.com/thxbb12/gfxlib) by Florent Gluck, under the **GPL-3.0** license.
