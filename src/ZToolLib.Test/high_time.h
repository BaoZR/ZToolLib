#ifndef _ZTL_HIGH_TIME_H_
#define _ZTL_HIGH_TIME_H_

#include <stdint.h>

typedef struct high_time_st high_time_t;
struct high_time_st
{
    int64_t     prev_tv;
    int32_t     prev_time;
    int32_t     speed;
    int32_t     sleep_err;
};


/* ��ʼ�� */
int high_time_init(high_time_t* ht, int speed);

/* һ���ڵ�һ��ʹ��ʱ��ҵ��ʼǰ������ */
void high_time_update_first(high_time_t* ht, int32_t curr_time);

/* ȡ��һ�����飬Ȼ���Զ�sleep��Ӧ��ʱ��,
 * @param curr_time ��ǰ����ʱ��������룬�� 93012500
 */
int high_time_update_and_sleep(high_time_t* ht, int32_t curr_time);

#endif//_ZTL_HIGH_TIME_H_
