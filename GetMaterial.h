#pragma once
#include "ProcessBase.h"
#include "INozzle.h"
#include <memory>

using namespace std;
using namespace Workflow::HardwareInterface;
namespace Workflow
{
	namespace Process
	{

		//表示吸嘴吸起物料工序
		class GetMaterial:public ProcessBase
		{
		public:
			GetMaterial(shared_ptr<INozzle> nozzlePtr);
			~GetMaterial();


			
			shared_ptr<INozzle> NozzlePtr;

			//Z轴的安全高度
			double SafeHeight;
			//取料的Z轴位置
			double Position;
			//取料前的等待时间
			int BeforeWaitTime;
			//取料后的等待时间
			int AfterWaitTime;
			//是否检测suck感应器
			bool DetectSuckSensor;
			//Z轴的速度
			AxisSpeed ZAxisSpeed;
			//是否锁定z轴
			bool LockZAxis = false;
			//延迟
			int Delay{ 0 };
			
		};

	}
}