################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Nucleo.c \
../src/common_sockets.c \
../src/conexiones.c \
../src/hiloCPU.c \
../src/hiloConsola.c \
../src/inotify.c 

OBJS += \
./src/Nucleo.o \
./src/common_sockets.o \
./src/conexiones.o \
./src/hiloCPU.o \
./src/hiloConsola.o \
./src/inotify.o 

C_DEPS += \
./src/Nucleo.d \
./src/common_sockets.d \
./src/conexiones.d \
./src/hiloCPU.d \
./src/hiloConsola.d \
./src/inotify.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


