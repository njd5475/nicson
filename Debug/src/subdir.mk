################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/fnv.c \
../src/json.c \
../src/nicson.c \
../src/parse.c 

C_DEPS += \
./src/fnv.d \
./src/json.d \
./src/nicson.d \
./src/parse.d 

OBJS += \
./src/fnv.o \
./src/json.o \
./src/nicson.o \
./src/parse.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -std=c11 -O0 --no-pie -g3 -p -pg -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/fnv.d ./src/fnv.o ./src/json.d ./src/json.o ./src/nicson.d ./src/nicson.o ./src/parse.d ./src/parse.o

.PHONY: clean-src

