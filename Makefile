DISK=disk.raw
DISK_SIZE=128G

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
	./vmm/vmm guest/guest.bin ./vmm/$(DISK)

run_phys: guest_phys vmm
	./vmm/vmm guest/guest.bin ./vmm/$(DISK)


guest_pv:
	$(MAKE) -C guest PV=1 OUT=guest.bin

guest_phys:
	$(MAKE) -C guest PV=0 OUT=guest.bin

vmm: create_disk
	$(MAKE) -C vmm OUT=vmm


create_disk:
	$(MAKE) -C vmm create_disk DISK=$(DISK) DISK_SIZE=$(DISK_SIZE)


clean: clean_vmm clean_guest

clean_vmm:
	$(MAKE) -C vmm clean OUT=vmm DISK=$(DISK)

clean_guest:
	$(MAKE) -C guest clean OUT=guest.bin


.PHONY: clean clean_vmm clean_guest vmm create_disk
