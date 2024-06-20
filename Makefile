$(shell mkdir -p rootfsimg/build)

APPS = busybox
APPS_DIR = $(addprefix apps/, $(APPS))

.DEFAULT_GOAL = all

.PHONY: all $(APPS_DIR) clean

all: $(APPS_DIR)

$(APPS_DIR): %:
	$(MAKE) -s -C $@ install

clean:
	$(foreach app, $(APPS_DIR), $(MAKE) -s -C $(app) clean ;)
	rm -f rootfsimg/build/*