################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../parser/parser/metadata_program.c \
../parser/parser/parser.c 

OBJS += \
./parser/parser/metadata_program.o \
./parser/parser/parser.o 

C_DEPS += \
./parser/parser/metadata_program.d \
./parser/parser/parser.d 


# Each subdirectory must supply rules for building sources it contributes
parser/parser/%.o: ../parser/parser/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


