#include "Shoot.h"
#include "Chassis.h"
#include "cmsis_os.h"
#include "MotorCtrl.h"
#include "MyLib.h"
#include "dbus.h"
#include "Gimbal.h"
#include "judge.h"
#include "vision_can.h"
#include "vision.h"

#define SHOOT_SPEED_15 4574
#define SHOOT_SPEED_18 5137
#define SHOOT_SPEED_22 6400
#define SHOOT_SPEED_25 7150
#define SHOOT_SPEED_30 8000

Shoot_Type Shoot_Data;

void Ammunition_Motor_Ctrol(void);
void Friction_Motor_Ctrol(void);
void Revolver_Heat_Limit(void);

void StartShootTask(void const * argument)
{
	portTickType currentTime;
	while(1)
	{
		currentTime = xTaskGetTickCount(); //当前系统时间
		Ammunition_Motor_Ctrol();
		Friction_Motor_Ctrol();
		vTaskDelayUntil(&currentTime, 1); //绝对延时
	}
}



/********************拨弹盘控制程序*********************/
float ammu_speed = SHOOT_HEAT_ENOUGH;
float Shoot_Speed;
float Last_Shoot_Speed;
uint8_t Have_Shooted;	//单发模式下已经发射了一颗弹丸
uint16_t danfa_delay_time;
uint16_t have_ready_time = 1;
uint16_t p_angle = 34200;
void Ammunition_Motor_Ctrol(void)
{
	if(Gimbal_Data.Shoot_Data.use_vision == 1)	//如果是视觉模式
	{
		Revolver_Heat_Limit();
		
		if(Gimbal_Data.Shoot_Data.vision_mode == 1)		//自瞄
		{
			if(VisionValue.center_flag == '1')	//如果可以射击
			{
				if(VisionValue.vision_Left_Gimbal_distance.value < 8)
				{
					Motor_2006[0].speed_rpm_target = ammu_speed * 2.f;
				}
				else
				{
					Motor_2006[0].speed_rpm_target = ammu_speed;
				}
			}
			else
			{
				Motor_2006[0].speed_rpm_target = 0;
			}
		}
	}
	else if((RC_Ctl.rc.s1 == 3 && RC_Ctl.rc.s2 == 1) || (RC_Ctl.rc.s1 == 3 && RC_Ctl.rc.s2 == 2))
	{

			if(RC_Ctl.rc.wheel >= 50 && RC_Ctl.rc.wheel <= 660)
			{
				Motor_2006[0].speed_rpm_target = ammu_speed;
			}
			else if(RC_Ctl.rc.wheel <= -50 && RC_Ctl.rc.wheel >= -660)
			{
				Motor_2006[0].speed_rpm_target = -ammu_speed;
			}
			else
			{
				Motor_2006[0].speed_rpm_target = 0;
			}
	}
	else
	{
				Motor_2006[0].speed_rpm_target = 0;
	}
	Last_Shoot_Speed = Shoot_Speed;
}


/***********摩擦轮控制程序*************/
float Now_speed = 0;		//当前弹速
float Last_Speed = 0;		//上一次弹速
float Speed_Allow = 0;	//最高允许弹速	
float Speed_Allow_Last = 0;	//最高允许弹速

float init_speed = 26.5;
uint8_t is_shoot = 0;


void Friction_Motor_Ctrol(void)
{
			if((RC_Ctl.rc.s1 == 3 && RC_Ctl.rc.s2 == 1) || (Gimbal_Data.Shoot_Data.use_vision == 1))
		{
			Speed_Allow_Last = Speed_Allow;
			Speed_Allow = init_speed;
			Saturation(&Speed_Allow, 30, 0);	
			Last_Speed = Now_speed;
			if(Speed_Allow_Last != Speed_Allow)
			{
				Gimbal_Data.Shoot_Data.Friction_Motor_Speed_Set = (8000 / 30) * init_speed;
				Gimbal_Data.Shoot_Data.set_bullet_speed = Speed_Allow * 0.9f;
				Now_speed = JUDGE_usGetSpeedHeat17();
			  if(is_shoot == 1)
					{
						if(Now_speed > 28.f) init_speed = init_speed - (Now_speed - 28) * 1.2f;
						else if(Now_speed < 26.5f) init_speed = init_speed + (27 - Now_speed) * 0.5f;
						is_shoot = 0;
					}
			}
					Motor_3508[0].speed_rpm_target = Gimbal_Data.Shoot_Data.Friction_Motor_Speed_Set;
					Motor_3508[1].speed_rpm_target = -Gimbal_Data.Shoot_Data.Friction_Motor_Speed_Set;
	      
		    }
		  else
		    {
			    Motor_3508[0].speed_rpm_target = 0;
		      Motor_3508[1].speed_rpm_target = 0;
		    }
}

/**
  * @brief  枪管热量限制
  * @param  void
  * @retval 热量是否超限
  * @attention  超限要重置一下拨盘,根据剩余可发弹量来做闭环
  *             如果做双枪管则此函数不适用
  */
			uint16_t  usHeatReal_1		= 0;
      uint16_t  usHeatLimit_Rest_1 = 0;
			uint16_t  usHeatLimit;
void Revolver_Heat_Limit(void)
{
//	if(JUDGE_Game_Status()!=4)
//		return;
/*数据处理*/
	/* 读取热量 */
	usHeatReal_1 = VisionValue.left_real_heat.value;
	/* 读取热量上限*/
	usHeatLimit = VisionValue.left_bullet_speed.value;//裁判系统检测到打一颗，枪口热量加10，与其初速度无关+

  usHeatLimit_Rest_1 = usHeatLimit - usHeatReal_1;
/*数据处理*/
		if (usHeatLimit_Rest_1 >= usHeatLimit / 2)//剩余热量大于一半
	{
    ammu_speed =  SHOOT_HEAT_ENOUGH ;
	}
	else if(usHeatLimit_Rest_1 >= usHeatLimit / 4)	//剩余能量大于四分之一
	{
    ammu_speed =  (150.0f-usHeatLimit_Rest_1)/100.0f*SHOOT_HEAT_ENOUGH ;
	}
	else
	{
    ammu_speed =  0 ;
	}
}
