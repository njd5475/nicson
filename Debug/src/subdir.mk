################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/fnv.c \
../src/json.c \
../src/nicson.c \
../src/parse.c 

OBJS += \
./src/fnv.o \
./src/json.o \
./src/nicson.o \
./src/parse.o 

C_DEPS += \
./src/fnv.d \
./src/json.d \
./src/nicson.d \
./src/parse.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -p -pg -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


