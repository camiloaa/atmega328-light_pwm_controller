include project.mk

# add seach paths
vpath %.c $(SOURCEDIRS)
vpath %.s $(SOURCEDIRS)
vpath %.h $(INCLUDEDIRS)
vpath %.lib $(LIBPATH)
vpath %.o $(LIBPATH)

PWD=.

########################################################################################################################################
# define the objs
OBJS  = $(C_SOURCES:%.c=$(OBJDIR)/%.o)
OBJS += $(CPP_SOURCES:%.c=$(OBJDIR)/%.o)
OBJS += $(ASM_SOURCES:%.s=$(OBJDIR)/%.o)

########################################################################################################################################
# include dependencies
-include $(OBJS:.o=.d) $(OBJS:.o=.d)

########################################################################################################################################
# compiler configuration
CC_ROOT = /usr
CC_PREFIX = avr-
CC_SUFIX =
#compiler path
CC = $(CC_ROOT)/bin/$(CC_PREFIX)gcc$(CC_SUFFIX)

# compiler flags
CFLAGS += -I./ -c -mmcu=atmega328p
# Debug flags 
CFLAGS += -g
# Warnings: enable all warnings, warn if inline fails
CFLAGS += -Wall -Winline
# optimization: use regs as debug symbols, optimization for size
CFLAGS += -O2
# compile each data element to its own data section
#CFLAGS += -fdata-sections
#assembler directives
#CFLAGS += -Wa
# make dependencies
# CFLAGS += -MD
# add flags for include dirs
CFLAGS += $(addprefix -I,$(INCLUDEDIRS))

# compile directive
$(OBJDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $(PWD)/$<

# clean directives
.PHONY: cleanobjs
cleanobjs:
	-$(RM) $(OBJS) $(OBJS:.o=.d)
	
#cleanbackups:
#	-$(RM) 
clean: cleanobjs

########################################################################################################################################

# preprocess directive
$(OBJDIR)/%.e: %.c
	$(CC) -E $(CFLAGS) $(CPPFLAGS) -o $@ $(PWD)/$<

########################################################################################################################################
#C++ compiler

# compile directive
CFLAGSTOREMOVE = -std=%
CXXFLAGS = $(filter-out $(CFLAGSTOREMOVE), $(CFLAGS)) -std=gnu++0x

$(OBJDIR)/%.o: %.cpp
	$(CC) -c $(CXXFLAGS) $(CPPFLAGS) -o $@ $(PWD)/$<

########################################################################################################################################

# Assembler
AR = $(CC_ROOT)/bin/${CC_PREFIX}ax${CC_SUFFIX}
# Include search path
ASFLAGS += $(addprefix -I,$(INCLUDEDIRS))
ASFLAGS += -mmcu=atmega328p 
# Warnings: enable all warnings
ASFLAGS += --warn --fatal-warnings
# make dependencies
# ASFLAGS += -MD
# assemble directive
$(OBJDIR)/%.o : %.s
	$(AS) $(ASFLAGS) -o $@ $(PWD)/$<

########################################################################################################################################
# Linker. Using gcc as linker
# linker path
LD = $(CC_ROOT)/bin/${CC_PREFIX}gcc${CC_SUFFIX}
#LDFLAGS += -T$(LDSCRIPT)
# make linker remove unused sections
#LDFLAGS += --gc-sections
#LDFLAGS += -nostartfiles
LDFLAGS += -mmcu=atmega328p


# Make project target
$(BIN) : $(OBJS) $(APP) $(LDSCRIPT) 
	$(LD) $(OBJS)  $(APP) $(LIBPATH:%=-L%) $(LDFLAGS) -o $@ -Xlinker -Map -Xlinker $@.map

$(MON) : $(OBJS) obj/defdevInitList.o $(LDSCRIPT)
	$(LD) $(OBJS) obj/defdevInitList.o $(LIBPATH:%=-L%) $(LDFLAGS) -o $@ -Xlinker -Map -Xlinker $@.map

# clean directives
.PHONY: cleanlinker
cleanlinker:
	-$(RM) $(OBJDIR)/*.o
clean: cleanlinker

distclean: cleanlinker
	-$(RM) $(BIN) $(BIN).map $(HEXREG)

########################################################################################################################################

# Archiver
AR = $(CC_ROOT)/bin/${CC_PREFIX}ar${CC_SUFFIX}

# archiving directive
$(APP) : $(AOBJS) 
	$(AR) cr $@ $(AOBJS)

########################################################################################################################################

# Generate hex
OBJCP = $(CC_ROOT)/bin/$(CC_PREFIX)objcopy$(CC_SUFFIX)

$(HEXREC) : $(BIN)
	$(OBJCP) -O srec $(BIN) $@ -j .app.init -j .app.text -j .app.data -j .app.bss
