################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/orquestador.c \
../src/planificador.c 

OBJS += \
./src/orquestador.o \
./src/planificador.o 

C_DEPS += \
./src/orquestador.d \
./src/planificador.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/federico/git/tp-20131c-tp-so-1c2013/so-commons-library" -I"/home/federico/git/tp-20131c-tp-so-1c2013/uncommons" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


