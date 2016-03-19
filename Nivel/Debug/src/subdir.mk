################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/interbloqueo.c \
../src/movimiento.c \
../src/nivelBase.c 

OBJS += \
./src/interbloqueo.o \
./src/movimiento.o \
./src/nivelBase.o 

C_DEPS += \
./src/interbloqueo.d \
./src/movimiento.d \
./src/nivelBase.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/federico/git/tp-20131c-tp-so-1c2013/so-commons-library" -I"/home/federico/git/tp-20131c-tp-so-1c2013/uncommons" -I"/home/federico/git/tp-20131c-tp-so-1c2013/Libreria-nivel" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


