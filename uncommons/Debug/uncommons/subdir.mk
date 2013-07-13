################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../uncommons/SocketsBasic.c \
../uncommons/SocketsCliente.c \
../uncommons/SocketsServer.c \
../uncommons/algo.c \
../uncommons/fileStructures.c \
../uncommons/inotify.c \
../uncommons/select.c 

OBJS += \
./uncommons/SocketsBasic.o \
./uncommons/SocketsCliente.o \
./uncommons/SocketsServer.o \
./uncommons/algo.o \
./uncommons/fileStructures.o \
./uncommons/inotify.o \
./uncommons/select.o 

C_DEPS += \
./uncommons/SocketsBasic.d \
./uncommons/SocketsCliente.d \
./uncommons/SocketsServer.d \
./uncommons/algo.d \
./uncommons/fileStructures.d \
./uncommons/inotify.d \
./uncommons/select.d 


# Each subdirectory must supply rules for building sources it contributes
uncommons/%.o: ../uncommons/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/git/tp-20131c-tp-so-1c2013/so-commons-library" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


