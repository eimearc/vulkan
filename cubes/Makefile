ODIR=obj

all:
	$(MAKE) vulkan
	$(MAKE) gl

vulkan:
	cd vulkan && $(MAKE) -j 4

gl:
	cd gl && $(MAKE)

clean:
	cd vulkan && $(MAKE) clean
	cd gl && $(MAKE) clean
	rm -f $(ODIR)/*.o

.PHONY: vulkan gl