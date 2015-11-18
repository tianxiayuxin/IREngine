################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../jni/IRBox.cpp \
../jni/Interface.cpp \
../jni/Render.cpp 

OBJS += \
./jni/IRBox.o \
./jni/Interface.o \
./jni/Render.o 

CPP_DEPS += \
./jni/IRBox.d \
./jni/Interface.d \
./jni/Render.d 


# Each subdirectory must supply rules for building sources it contributes
jni/%.o: ../jni/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cygwin C++ Compiler'
	g++ -I"E:\android\android-ndk-r10c\platforms\android-18\arch-arm\usr\include" -I"E:\android\android-ndk-r10c\toolchains\arm-linux-androideabi-4.8\prebuilt\windows-x86_64\lib\gcc\arm-linux-androideabi\4.8\include" -I"E:\android\android-ndk-r10c\sources\cxx-stl\stlport\stlport" -I"H:\AR\IREngine\GLRender\GLRender\jni" -I"H:\AR\IREngine\GLRender\GLRender\jni\Assimp\include" -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


