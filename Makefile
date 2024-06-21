$(shell mkdir -p rootfsimg/build)

APPS = busybox
APPS_DIR = $(addprefix apps/, $(APPS))

.DEFAULT_GOAL = all

.PHONY: init all $(APPS_DIR) clean

init:
	git submodule update --init --depth 1

all: $(APPS_DIR)

$(APPS_DIR): %:
	$(MAKE) -s -C $@ install

clean:
	$(foreach app, $(APPS_DIR), $(MAKE) -s -C $(app) clean ;)
	rm -f rootfsimg/build/*