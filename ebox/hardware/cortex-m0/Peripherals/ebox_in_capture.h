/**
  ******************************************************************************
  * @file    incapture.h
  * @author  shentq
  * @version V2.1
  * @date    2017/07/23
  * @brief
  ******************************************************************************
  * @attention
  *
  * No part of this software may be used for any commercial activities by any form
  * or means, without the prior written consent of shentq. This specification is
  * preliminary and is subject to change at any time without notice. shentq assumes
  * no responsibility for any errors contained herein.
  * <h2><center>&copy; Copyright 2015 shentq. All Rights Reserved.</center></h2>
  ******************************************************************************
  */
/**
 * Modification History:
 * -shentq                  -version 1.2(2016/08/17)
 *      *优化内部结构，降低频繁中断带来的额外计算开销
 *      *单独增加了波形基本测量工具，简化用户的波形测量编程
 */

/* Define to prevent recursive inclusion -------------------------------------*/


#ifndef __IN_CAPTURE_H
#define __IN_CAPTURE_H
#include "ebox_core.h"
#include "mcu.h"
#include "FunctionPointer.h"
#include "ebox_timer.h"
#include "ebox_gpio.h"

#define TIM_ICPOLARITY_FALLING LL_TIM_IC_POLARITY_FALLING
#define TIM_ICPOLARITY_RISING  LL_TIM_IC_POLARITY_RISING
/*
1.支持TIM2，3，4的ch1,2,3,4.共计12个通道
2.支持测量周期、频率、高级用法支持测量占空比
3.默认测量下降沿，可通过函数设置测量边沿模式
4.定时器计数器最大值为0xffff,为了突破这个限制，
    在本例程中，使用了update溢出中断调用tx_overflow_times可以将计数器
    拓展至2^32。大大提高测量范围，可以实现最高频率（1分频）测量周期低于120s的信号。
    如果使用2分频，可测量周期低于240s的信号。以此类推。
5.关于分频系数和脉冲宽度测量的计算关系，要遵循一个原则：在不溢出的情况下尽量使用低分频系数（高TIM时钟）去检测对象
6.关于最窄脉宽的测量
   由于STM32本身的速度限制，无法测量非常窄的脉宽。如果使用默认分频系数（1）则最窄脉宽为4us。
   最窄脉宽受PWM的占空比和频率两个条件限制。


7.如果使用某个定时器通道用于输入捕获，则该定时器所有通道都必须是输入捕获模式，不能再设置为其他工作模式
重点：
    在采用低分频系数的时候，可以保证测量精度，但是会增大定时器溢出频率，进而增大cpu开销，
    在采用高分频系数的时候，测量精度较低，但是会降低定时器溢出频率，进而降低cpu开销，
    stm32在72M主频下，最高可测160Khz的信号。如果再大，将无法测量。


*/

typedef enum
{
    SIMPLE = 0,
    COMPLEX = 1
} ICMode_t;


class InCapture: T_base
{
public:
    InCapture(TIM_TypeDef *TIMx, mcuGpio *pin): T_base(TIMx)
    {
        uint8_t _index;
        uint32_t t = (uint32_t)TIMx;
        _index = getIndex(pin->id, TIM_MAP, t);
        pin->mode(TIM_MAP[_index]._pinMode, TIM_MAP[_index]._pinAf);
        _timx = TIMx;
        //		_overflow_times = &t1_overflow_times;
        _last_value 		= 0;

        _chNum = (TIM_MAP[_index]._periph) - (uint32_t)_timx;
        switch (_chNum)
        {
        case TIMxCH1:
            _channel = LL_TIM_CHANNEL_CH1;
            _getCapture = &LL_TIM_IC_GetCaptureCH1;
            _CCEnableIT = &LL_TIM_EnableIT_CC1;
            break;
        case TIMxCH2:
            _channel = LL_TIM_CHANNEL_CH2;
            _getCapture = &LL_TIM_IC_GetCaptureCH2;
            _CCEnableIT = &LL_TIM_EnableIT_CC2;
            break;
        case TIMxCH3:
            _channel = LL_TIM_CHANNEL_CH3;
            _getCapture = &LL_TIM_IC_GetCaptureCH3;
            _CCEnableIT = &LL_TIM_EnableIT_CC3;
            break;
        case TIMxCH4:
            _channel = LL_TIM_CHANNEL_CH4;
            _getCapture = &LL_TIM_IC_GetCaptureCH4;
            _CCEnableIT = &LL_TIM_EnableIT_CC4;
            break;
        }
    }

    void SetPorlicy(uint8_t porlicy);
    void        begin(uint16_t prescaler = 1, ICMode_t mode = SIMPLE); //使用默认参数，分频系数为1；最大量程为120s

    void        set_count(uint16_t count);
    void        set_polarity_falling()
    {
        SetPorlicy(TIM_ICPOLARITY_FALLING);
    };
    void        set_polarity_rising()
    {
        SetPorlicy(TIM_ICPOLARITY_RISING);
    };
    //需要用户在中断中处理更精细的任务，处理状态机等事务，比如红外解码，超声波测距
    uint32_t    get_capture();//不建议使用、后期使用下面新的函数代替
    float       get_zone_time_us();//不建议使用、后期使用下面新的函数代替

    //波形的基本的测量工具
    void        complex_event();//适用于要求测量占空比的情况，但是最短脉宽不能低于4us****
    void        simple_event();//适用于所有情况，执行效率高，最高可测180K,但是不能测量占空比
    float       get_wave_frq();///<波形的频率
    float       get_wave_peroid();///<波形的周期
    float       get_wave_high_duty();///<波形的高电平占空比
    float       get_wave_low_duty();///<波形的低电平占空比
    float       get_wave_high_time();///<波形的高电平时间
    float       get_wave_low_time();///<波形的低电平时间
    bool        available();///<波形的测量完成

    uint32_t    get_timer_clock();
    uint32_t    get_timer_source_clock();
    uint32_t    get_detect_max_frq();
    uint32_t    get_detect_min_frq();
    uint8_t     get_detect_min_pulse_us();

    uint32_t GetDetectMaxFrq(void);
    uint32_t GetDetectMinFrq(void);

    void attach(void (*fptr)(void))
    {
        _pirq.attach(fptr);
    }
    template<typename T>
    void attach(T *tptr, void (T::*mptr)(void))
    {
        _pirq.attach(tptr, mptr);
    }

private:
    uint32_t 		_channel;	// 通道
    uint8_t         _chNum;     // 通道号
    uint16_t 		_duty;		// 占空比
    uint8_t	 		_accuracy; 	// 精度
    __IO uint8_t	_porlicy;	// 极性

    uint16_t   		*_overflow_times;
    __IO uint32_t	_last_value;	//最后值
    __IO uint32_t   _capture;		//捕获值
    __IO bool	   	_available;		//是否有效
    __IO uint32_t   _high_capture;	//高电平捕获
    __IO uint32_t   _low_capture;	//低电平捕获

    uint32_t   _timeClock;			//time时钟

    //	pGetFun  _ICgetCompare;			//捕获函数
    //	pCCIT	 _CCEnableIT;			//使能捕获中断

    uint32_t    (*_getCapture)(TIM_TypeDef *TIMx); //捕获函数
    void        (*_CCEnableIT)(TIM_TypeDef *);      //使能捕获中断

    void _setMode(void);

    FunctionPointer _pirq;
    static void _irq_handler(uint32_t id);
};

#endif
