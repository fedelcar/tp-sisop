################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../uncommons/SocketsBasic.c \
../uncommons/SocketsCliente.c \
../uncommons/SocketsServer.c 

OBJS += \
./uncommons/SocketsBasic.o \
./uncommons/SocketsCliente.o \
./uncommons/SocketsServer.o 

C_DEPS += \
./uncommons/SocketsBasic.d \
./uncommons/SocketsCliente.d \
./uncommons/SocketsServer.d 


# Each subdirectory must supply rules for building sources it contributes
uncommons/%.o: ../uncommons/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/lucas/workspace/commons" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


