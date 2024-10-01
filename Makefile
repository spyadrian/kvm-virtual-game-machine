DISK=disk.raw
DISK_SIZE=256K

help:
	@echo "Available targets:"
	@echo "    run_pv       run paravirtualized guest"
	@echo "    run_phys     run physical (native) guest"
	@echo "    vmm          build VMM"
	@echo "    guest_pv     build paravirtualized guest"
	@echo "    guest_phys   build physical (native) guest"
	@echo "    clean        delete VMM and guest"
	@echo "    clean_vmm    delete VMM"
	@echo "    clean_guest  delete guest"

guest:
	$(MAKE) -C guest PV=1 OUT=guest.bin

.PHONY: guest