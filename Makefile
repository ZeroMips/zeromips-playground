CC			= cc65
AS			= ca65
LD			= ld65

HOSTCC		= gcc
HOSTCFLAGS	= -O -Wall
HOSTLDFLAGS	=

# global includes
ASFLAGS		+= -I inc
# put all symbols into .sym files
ASFLAGS		+= -g
# all files are allowed to use 65SC02 features
ASFLAGS		+= --cpu 65SC02

BUILD_DIR = build

CORE_DEPS =

CORE_SOURCES = \
	init.s \
	vectors.s \
	xosera.s

XOBOING_SOURCES = \
	xoboing/xoboing.s

XOBOING_RLE_SOURCES = \
	$(BUILD_DIR)/bg_real.bin \
	$(BUILD_DIR)/tiles.bin \
	$(BUILD_DIR)/palettes.bin

all: $(BUILD_DIR)/playground.bin

clean:
	rm -rf $(BUILD_DIR)

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $$(dirname $@)
	$(AS) $(ASFLAGS) $< -o $@ -l $(BUILD_DIR)/$*.lst

CORE_OBJS  = $(addprefix $(BUILD_DIR)/, $(CORE_SOURCES:.s=.o))
XOBOING_OBJS  = $(addprefix $(BUILD_DIR)/, $(XOBOING_SOURCES:.s=.o))
XOBOING_RLE  = $(XOBOING_RLE_SOURCES:.bin=.rle-toolkit)

$(BUILD_DIR)/playground.bin: $(CORE_OBJS) $(CORE_DEPS) $(XOBOING_RLE) $(XOBOING_OBJS) core.cfg
	@mkdir -p $$(dirname $@)
	$(LD) -C core.cfg $(CORE_OBJS) $(XOBOING_OBJS) -o $@ -m $(BUILD_DIR)/playground.map -Ln $(BUILD_DIR)/playground.sym

$(BUILD_DIR)/xoboing_generate: xoboing/generate.c
	$(HOSTCC) xoboing/generate.c -o $(BUILD_DIR)/xoboing_generate -lm

$(BUILD_DIR)/rlepack: xoboing/rlepack.c
	$(HOSTCC) xoboing/rlepack.c -o $(BUILD_DIR)/rlepack

$(XOBOING_RLE_SOURCES): $(BUILD_DIR)/xoboing_generate $(BUILD_DIR)/rlepack
	@cd $(BUILD_DIR) && ./xoboing_generate

%.rle-toolkit: %.bin
	$(BUILD_DIR)/rlepack $< $@
