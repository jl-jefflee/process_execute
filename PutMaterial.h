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
		//���Ϲ���
		class PutMaterial:public ProcessBase
		{
		public:
			PutMaterial(shared_ptr<INozzle> nozzlePtr);
			~PutMaterial();

			shared_ptr<INozzle> NozzlePtr;

			//Z��İ�ȫ�߶�
			double SafeHeight;
			//���ϵ�Z��λ��
			double Position;
			//����ǰ�ĵȴ�ʱ��
			int BeforeWaitTime;
			//���Ϻ�ĵȴ�ʱ��
			int AfterWaitTime;
			//�Ƿ���suck��Ӧ��
			bool DetectFlatSensor;
			//Z����ٶ�
			AxisSpeed ZAxisSpeed;
			//�Ƿ�����z��
			bool LockZAxis = false;
			double Offset{ 0 };
			int Delay{ 0 };
		};

	}
}