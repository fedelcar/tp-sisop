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
all: Nivel

dependents:
	-cd /home/lucas/git/tp-20131c-tp-so-1c2013/commons/Debug && $(MAKE) clean all
	-cd /home/lucas/git/tp-20131c-tp-so-1c2013/uncommons/Debug && $(MAKE) clean all
	-cd /home/lucas/workspace/Libreria-nivel/Debug && $(MAKE) clean all

# Tool invocations
Nivel: $(OBJS) $(USER_OBJS) /home/lucas/git/tp-20131c-tp-so-1c2013/commons/Debug/libcommons.so /home/lucas/git/tp-20131c-tp-so-1c2013/uncommons/Debug/libuncommons.so /home/lucas/workspace/Libreria-nivel/Debug/libLibreria-nivel.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/lucas/workspace/Libreria-nivel/Debug" -L"/home/lucas/git/tp-20131c-tp-so-1c2013/uncommons/Debug" -L"/home/lucas/git/tp-20131c-tp-so-1c2013/commons/Debug" -o "Nivel" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) Nivel
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/lucas/git/tp-20131c-tp-so-1c2013/commons/Debug/libcommons.so:
/home/lucas/git/tp-20131c-tp-so-1c2013/uncommons/Debug/libuncommons.so:
/home/lucas/workspace/Libreria-nivel/Debug/libLibreria-nivel.so:

-include ../makefile.targets
