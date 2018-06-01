#pragma once
#include "ProcessBase.h"
#include "INozzle.h"
#include <memory>
#include <vector>



using namespace std;
using namespace Workflow::HardwareInterface;
namespace Workflow
{
	namespace Process
	{
		//放料工序
		class PutMaterial:public ProcessBase
		{
		public:
			PutMaterial(shared_ptr<INozzle> nozzlePtr);
			~PutMaterial();

			shared_ptr<INozzle> NozzlePtr;

			//Z轴的安全高度
			double SafeHeight;
			//放料的Z轴位置
			double Position;
			//放料前的等待时间
			int BeforeWaitTime;
			//放料后的等待时间
			int AfterWaitTime;
			//是否检测suck感应器
			bool DetectFlatSensor;
			//Z轴的速度
			AxisSpeed ZAxisSpeed;
			//是否锁定z轴
			bool LockZAxis = false;
			double Offset{ 0 };
			int Delay{ 0 };
		};

	}
}