################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/DMDLib/DMD.c 

OBJS += \
./Core/Src/DMDLib/DMD.o 

C_DEPS += \
./Core/Src/DMDLib/DMD.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/DMDLib/%.o Core/Src/DMDLib/%.su Core/Src/DMDLib/%.cyclo: ../Core/Src/DMDLib/%.c Core/Src/DMDLib/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-DMDLib

clean-Core-2f-Src-2f-DMDLib:
	-$(RM) ./Core/Src/DMDLib/DMD.cyclo ./Core/Src/DMDLib/DMD.d ./Core/Src/DMDLib/DMD.o ./Core/Src/DMDLib/DMD.su

.PHONY: clean-Core-2f-Src-2f-DMDLib

