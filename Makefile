# BSD make compatibility shim — full build rules live in GNUmakefile.
# FreeBSD and other BSD make hosts: pkg install gmake (or use this shim).
#
# GNU make reads GNUmakefile first when present; BSD make uses this file and
# delegates to gmake.

GMAKE ?= gmake

.PHONY: all
all:
	@command -v $(GMAKE) >/dev/null 2>&1 || { \
		echo "ERROR: GNU Make ($(GMAKE)) is required." >&2; \
		echo "  FreeBSD: pkg install gmake && gmake all" >&2; \
		exit 1; \
	}
	@$(GMAKE) -f GNUmakefile all

%:
	@command -v $(GMAKE) >/dev/null 2>&1 || { \
		echo "ERROR: GNU Make ($(GMAKE)) is required." >&2; \
		echo "  FreeBSD: pkg install gmake && gmake $(MAKECMDGOALS)" >&2; \
		exit 1; \
	}
	@$(GMAKE) -f GNUmakefile $@
