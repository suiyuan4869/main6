#include "stm32f10x.h"

uint16_t pwm_duty = 0;
int8_t dir = 1;   // +1渐亮，-1渐暗

void TIM2_PWM_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
    TIM_OCInitTypeDef TIM_OCInitStruct;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    // PA0 → TIM2_CH1 复用推挽输出
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // PWM频率：1kHz，ARR=999，范围0~999
    TIM_TimeBaseStruct.TIM_Prescaler = 35;
    TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStruct.TIM_Period = 999;
    TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStruct);

    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_Pulse = pwm_duty;
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM2, &TIM_OCInitStruct);
    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);

    TIM_ARRPreloadConfig(TIM2, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
}

// 放大延时，这个循环大概几百微秒，数值越大越慢
void Soft_Delay(uint32_t t)
{
    uint32_t i;
    for(i = 0; i < t; i++)
    {
        __NOP();   // 加空操作，防止编译器把空循环直接优化删掉！很关键
    }
}

int main(void)
{
    TIM2_PWM_Init();
    while(1)
    {
        pwm_duty += dir;

        if(pwm_duty >= 999) dir = -1;
        if(pwm_duty <= 0)   dir = 1;

        TIM_SetCompare1(TIM2, pwm_duty);

        // ✅这里是调速点！
        // 50000：完整呼吸≈1.5s； 80000：≈2s
        Soft_Delay(10000);
    }
}
