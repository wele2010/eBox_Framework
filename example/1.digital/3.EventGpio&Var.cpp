/**
  ******************************************************************************
  * @file   : *.cpp
  * @author : shentq
  * @version: V1.2
  * @date   : 2016/08/14

  * @brief   ebox application example .
	*					 2018-8-2	ͨ������bsp������Ӳ���˿ڣ��������̿����ڲ�ͬƽ̨������
  * Copyright 2016 shentq. All Rights Reserved.
  ******************************************************************************
 */


#include "ebox.h"
#include "EventGpio.h"
#include "EventVar.h"
#include "ringbuf.h"
#include "bsp_ebox.h"

/**
	*	1	��������Ҫ����appsĿ¼�µ�EventGpio , eventVar,EventManagerģ��
	*	2	��������ʾ��IO�¼���Ӧ���ֱ�Ϊ�͵�ƽ���ߵ�ƽ���½��أ������أ��������ͷţ�����
	*	3	�߶�ƽ���͵�ƽ��������������������5ms����ͨ���޸�EvenGpio.h�е�IO_EDGE_FILTER_COUNTS�޸�
	* 4	����ʱ����õ����¼����ҳ��������󲻴����ͷ��¼�������Ӱ�������غ��½����¼�
	* 5 ���û�г��������½���=������������=�ͷ�
	*
	*/

/* ���������������̷������� */
#define EXAMPLE_NAME	"EventGPIO&EventVar example"
#define EXAMPLE_DATE	"2019-05-17"


// �½��ؼ��
void neg()
{
    UART.println("��⵽�½����ź�");
}
// �����ؼ��
void pos()
{
    UART.println("��⵽�������ź�");
}
// �ߵ�ƽ�ص�����
void high()
{
    UART.println("��⵽�ߵ�ƽ");
}

// �ߵ�ƽ�ص�����
void low()
{
    UART.println("��⵽�͵�ƽ");
}
// �����ص�����
void click()
{
    UART.println("��⵽��������");
}
// �ͷŻص�����
void release()
{
    UART.println("��⵽�����ͷ�");
}
// �����ص�����
void long_press()
{
    UART.println("��⵽����������ʱ����Ӧ����");
}
void click1()
{
    UART.println("��⵽��������");
}
void release1()
{
    UART.println("��⵽�����ͷ�");
}
void long_press1()
{
    UART.println("��⵽����������ʱ����Ӧ����");
}
void up()
{
    UART.println("var ����");
}
void down()
{
    UART.println("var ����");
}void change()
{
    UART.println("var �仯");
}
/** ����EventGpio���󣬲������¼��ص������ߵ�ƽ���͵�ƽ�������أ�
  *�½��أ��������ͷţ�����.����Ҫ��Ӧ���¼�����Ҫ����
	*/
// ʹ�ó����¼����Զ����õ����¼����ҳ��������󲻴����ͷ��¼�������Ӱ�������غ��½����¼�
// ʹ�ó����¼������Ժ��ͷ��¼���ϣ���û�д��������¼���ʱ���ͷ��¼��ᱻִ��
EventGpio btn(&PA8, 1);

uint8_t volume = 0;
EventVar<uint8_t> var(&volume);

EventManager event_manager;
void setup()
{
    ebox_init();
    UART.begin(115200);
    print_log(EXAMPLE_NAME, EXAMPLE_DATE);
    //event_io_1.begin(1);
    
    
    btn.event_click = click;
    btn.event_release = release;
    btn.event_long_press = long_press;
    btn.long_press_type = EventGpio::Continue;
//    btn.event_high = high;
//    btn.event_low = low;
//    btn.event_neg_edge = neg;
//    btn.event_pos_edge = pos;
    
    var.event_changed = change;
    var.event_nag_edge = down;
    var.event_pos_edge = up;
    btn.begin();
    
    event_manager.add(&btn);
    event_manager.add(&var);
}
uint32_t last;
uint8_t flag = 0;
int main(void)
{
    setup();

    while(1)
    {
        event_manager.loop();
        delay_ms(1);
        if(millis() - last > 1000)
        {
            last = millis();
            if(flag==0)
                volume++;
            else
                volume--;
            if(volume >= 3)
                flag = 1;
            else if (volume == 0)
                flag = 0;
        }
        
    }
}