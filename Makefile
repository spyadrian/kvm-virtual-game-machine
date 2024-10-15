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

run_pv: guest_pv vmm
	./vmm/vmm guest/guest.bin

run_phys: guest_phys vmm
	./vmm/vmm guest/guest.bin


guest_pv:
	$(MAKE) -C guest PV=1 OUT=guest.bin

guest_phys:
	$(MAKE) -C guest PV=0 OUT=guest.bin

vmm:
	$(MAKE) -C vmm OUT=vmm


clean: clean_vmm clean_guest

clean_vmm:
	$(MAKE) -C vmm clean OUT=vmm

clean_guest:
	$(MAKE) -C guest clean


.PHONY: clean clean_vmm clean_guest vmm
