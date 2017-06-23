RTEMS_ROOT = $(PWD)/../rtems-4.12
RTEMS_BSP = br_uid

include $(RTEMS_ROOT)/make/custom/$(RTEMS_BSP).mk

LIB_PIECES =
LIB_PIECES += bed-trash-buffer
LIB_PIECES += bed-read
LIB_PIECES += bed-read-oob
LIB_PIECES += bed-is-block-valid
LIB_PIECES += bed-write-erase
LIB_PIECES += bed-partition-create
LIB_PIECES += bed-mutex
LIB_PIECES += bed-nand
LIB_PIECES += bed-nand-simulator
LIB_PIECES += bed-nand-device-info-8-bit-1-8-v
LIB_PIECES += bed-nand-device-info-8-bit-3-3-v
LIB_PIECES += bed-nand-device-info-16-bit-1-8-v
LIB_PIECES += bed-nand-device-info-16-bit-3-3-v
LIB_PIECES += bed-nand-device-info-all
LIB_PIECES += bed-nand-set-default-oob-layout
LIB_PIECES += bed-nor-simulator
LIB_PIECES += bed-null-partition
LIB_PIECES += bed-ones-per-byte
LIB_PIECES += bed-ecc-hamming-256-calc
LIB_PIECES += bed-ecc-hamming-256-corr
LIB_PIECES += bed-write-with-skip
LIB_PIECES += bed-read-with-skip
LIB_PIECES += bed-read-all
LIB_PIECES += bed-print-bad-blocks
LIB_PIECES += bed-vprintf-printer
LIB_PIECES += bed-test-if-erased
LIB_PIECES += bed-test-write-and-read
LIB_PIECES += bed-test-make-block-bad
LIB_PIECES += bed-elbc
LIB_PIECES += bed-yaffs

TEST_PIECES =
TEST_PIECES += test-init
TEST_PIECES += test-ecc
TEST_PIECES += test-bed
TEST_PIECES += test-nand
TEST_PIECES += test-nor-simulator

LIBS =

ifneq ($(findstring lpc32xx_mzx, $(RTEMS_MAKEFILE_PATH)),)
LIB_PIECES += bed-lpc32xx-mlc
LIB_PIECES += bed-lpc32xx-slc
TEST_PIECES += test-lpc32xx-mlc
TEST_PIECES += test-lpc32xx-slc
LIBS += -lmzx
endif

LIB = $(BUILDDIR)/libbed
LIB_OBJS = $(LIB_PIECES:%=$(BUILDDIR)/%.o)
LIB_DEPS = $(LIB_PIECES:%=$(BUILDDIR)/%.d)

LIB_RO = $(BUILDDIR)/libbed-ro
LIB_RO_OBJS = $(LIB_PIECES:%=$(BUILDDIR)/%.ro.o)
LIB_RO_DEPS = $(LIB_PIECES:%=$(BUILDDIR)/%.ro.d)

TEST = $(BUILDDIR)/app
TEST_OBJS = $(TEST_PIECES:%=$(BUILDDIR)/%.o)
TEST_DEPS = $(TEST_PIECES:%=$(BUILDDIR)/%.d)

all: $(BUILDDIR) $(LIB).a $(LIB_RO).a $(TEST).exe

debug: $(TEST).exe
	sparc-rtems4.11-gdb --command=~/bin/sparc.gdb $^

$(BUILDDIR):
	mkdir $(BUILDDIR)

$(LIB).a: $(LIB_OBJS)
	$(AR) rcu $@ $^
	$(RANLIB) $@

$(BUILDDIR)/%.ro.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -DNDEBUG -DBED_CONFIG_READ_ONLY -c $< -o $@

$(LIB_RO).a: $(LIB_RO_OBJS)
	$(AR) rcu $@ $^
	$(RANLIB) $@

$(TEST).exe: $(TEST_OBJS) $(LIB).a
	$(CXXLINK) -Wl,--start-group $^ -lgtest $(LIBS) -Wl,--end-group  -o $@

install:  $(BUILDDIR) $(LIB).a $(LIB_RO).a
	mkdir -p $(PROJECT_INCLUDE)/bed
	install -m 644 $(LIB).a $(LIB_RO).a $(PROJECT_LIB)
	install -m 644 *.h $(PROJECT_INCLUDE)/bed

.PHONY: doc

doc:
	doxygen

clean:
	rm -rf $(BUILDDIR)

-include $(LIB_DEPS) $(TEST_DEPS)
