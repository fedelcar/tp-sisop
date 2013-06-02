################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../nivel-gui/tad_items.o 

C_SRCS += \
../nivel-gui/nivel.c \
../nivel-gui/tad_items.c 

OBJS += \
./nivel-gui/nivel.o \
./nivel-gui/tad_items.o 

C_DEPS += \
./nivel-gui/nivel.d \
./nivel-gui/tad_items.d 


# Each subdirectory must supply rules for building sources it contributes
nivel-gui/%.o: ../nivel-gui/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


