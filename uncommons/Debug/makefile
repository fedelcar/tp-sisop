################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include uncommons/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: libuncommons.so

dependents:
	-cd /home/federico/git/tp-20131c-tp-so-1c2013/commons/Debug && $(MAKE) clean all

# Tool invocations
libuncommons.so: $(OBJS) $(USER_OBJS) /home/federico/git/tp-20131c-tp-so-1c2013/commons/Debug/libcommons.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/federico/git/tp-20131c-tp-so-1c2013/commons/Debug" -shared -o "libuncommons.so" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(LIBRARIES) libuncommons.so
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/federico/git/tp-20131c-tp-so-1c2013/commons/Debug/libcommons.so:

-include ../makefile.targets
