################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/movimiento.c \
../src/nivel.c 

OBJS += \
./src/movimiento.o \
./src/nivel.o 

C_DEPS += \
./src/movimiento.d \
./src/nivel.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/lucas/workspace/Libreria-nivel" -I"/home/lucas/git/tp-20131c-tp-so-1c2013/uncommons" -I"/home/lucas/git/tp-20131c-tp-so-1c2013/commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


