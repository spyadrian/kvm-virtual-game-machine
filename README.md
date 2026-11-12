# Virtual Game Machine

A **hypervisor (VMM) built on Linux KVM** that runs a small bare-metal x86 guest – hepia, Advanced Virtualization, 2024-2025.

## Overview

- The **VMM** (`vmm/`) creates a VM through the `/dev/kvm` API, loads the guest, handles VM exits (port I/O, MMIO) and emulates devices. Display and keyboard use **SDL2**.
- The **guest** (`guest/`) is a freestanding x86 kernel with its own IDT, IRQs and drivers.
- Every device exists in two versions, so the two approaches can be compared:
  - **physical (emulated)**: the guest talks to emulated hardware registers
  - **paravirtualized**: the guest calls the VMM directly through **hypercalls**
- Emulated / paravirtualized devices: timer, console, IDE disk, graphics, **sprites** (with a state machine) and keyboard.

## Build & run

Requirements: Linux with KVM, `gcc`, `nasm`, SDL2.

```bash
make run_pv      # paravirtualized guest
make run_phys    # guest using emulated hardware
```

## Tech

C · Linux KVM API · x86 · hypercalls · SDL2 · pthreads / semaphores
