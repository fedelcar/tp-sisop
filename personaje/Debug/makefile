################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
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
all: personaje

dependents:
	-cd /home/federico/git/tp-20131c-tp-so-1c2013/uncommons/Debug && $(MAKE) clean all
	-cd /home/federico/git/tp-20131c-tp-so-1c2013/so-commons-library/Debug && $(MAKE) clean all

# Tool invocations
personaje: $(OBJS) $(USER_OBJS) /home/federico/git/tp-20131c-tp-so-1c2013/uncommons/Debug/libuncommons.so /home/federico/git/tp-20131c-tp-so-1c2013/so-commons-library/Debug/libso-commons-library.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/federico/git/tp-20131c-tp-so-1c2013/so-commons-library/Debug" -L"/home/federico/git/tp-20131c-tp-so-1c2013/uncommons/Debug" -o "personaje" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) personaje
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/federico/git/tp-20131c-tp-so-1c2013/uncommons/Debug/libuncommons.so:
/home/federico/git/tp-20131c-tp-so-1c2013/so-commons-library/Debug/libso-commons-library.so:

-include ../makefile.targets
