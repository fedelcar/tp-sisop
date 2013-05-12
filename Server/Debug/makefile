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
all: Server

dependents:
	-cd /home/federico/git/tp-20131c-tp-so-1c2013/commons/Debug && $(MAKE) clean all
	-cd /home/federico/git/tp-20131c-tp-so-1c2013/uncommons/Debug && $(MAKE) clean all

# Tool invocations
Server: $(OBJS) $(USER_OBJS) /home/federico/git/tp-20131c-tp-so-1c2013/commons/Debug/libcommons.so /home/federico/git/tp-20131c-tp-so-1c2013/uncommons/Debug/libuncommons.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/federico/git/tp-20131c-tp-so-1c2013/uncommons/Debug" -L"/home/federico/git/tp-20131c-tp-so-1c2013/commons/Debug" -o "Server" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) Server
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/federico/git/tp-20131c-tp-so-1c2013/commons/Debug/libcommons.so:
/home/federico/git/tp-20131c-tp-so-1c2013/uncommons/Debug/libuncommons.so:

-include ../makefile.targets
