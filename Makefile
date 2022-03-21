CC           = cc65
AS           = ca65
LD           = ld65

# put all symbols into .sym files
ASFLAGS     += -g
# all files are allowed to use 65SC02 features
ASFLAGS     += --cpu 65SC02

BUILD_DIR = build

CORE_DEPS =

CORE_SOURCES = \
	init.s \
	vectors.s \
	xosera.s

all: $(BUILD_DIR)/playground.bin

clean:
	rm -rf $(BUILD_DIR)

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $$(dirname $@)
	$(AS) $(ASFLAGS) $< -o $@ -l $(BUILD_DIR)/$*.lst

CORE_OBJS  = $(addprefix $(BUILD_DIR)/, $(CORE_SOURCES:.s=.o))

$(BUILD_DIR)/playground.bin: $(CORE_OBJS) $(CORE_DEPS) core.cfg
	@mkdir -p $$(dirname $@)
	$(LD) -C core.cfg $(CORE_OBJS) -o $@ -m $(BUILD_DIR)/playground.map -Ln $(BUILD_DIR)/playground.sym
