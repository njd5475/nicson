################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../test/all_tests.cpp \
../test/test-objects.cpp \
../test/test-parser.cpp 

OBJS += \
./test/all_tests.o \
./test/test-objects.o \
./test/test-parser.o 

CPP_DEPS += \
./test/all_tests.d \
./test/test-objects.d \
./test/test-parser.d 


# Each subdirectory must supply rules for building sources it contributes
test/%.o: ../test/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/home/nick/workspace/nicson/src" -O0 -g3 -pg -ftest-coverage -fprofile-arcs -Wall -c -fmessage-length=0 -fpermissive -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


