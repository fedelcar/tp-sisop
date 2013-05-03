################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/fileStructures.c \
../src/main.c \
../src/orquestador.c \
../src/planificador.c 

OBJS += \
./src/fileStructures.o \
./src/main.o \
./src/orquestador.o \
./src/planificador.o 

C_DEPS += \
./src/fileStructures.d \
./src/main.d \
./src/orquestador.d \
./src/planificador.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/lucas/workspace/commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


