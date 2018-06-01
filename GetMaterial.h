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

		//��ʾ�����������Ϲ���
		class GetMaterial:public ProcessBase
		{
		public:
			GetMaterial(shared_ptr<INozzle> nozzlePtr);
			~GetMaterial();


			
			shared_ptr<INozzle> NozzlePtr;

			//Z��İ�ȫ�߶�
			double SafeHeight;
			//ȡ�ϵ�Z��λ��
			double Position;
			//ȡ��ǰ�ĵȴ�ʱ��
			int BeforeWaitTime;
			//ȡ�Ϻ�ĵȴ�ʱ��
			int AfterWaitTime;
			//�Ƿ���suck��Ӧ��
			bool DetectSuckSensor;
			//Z����ٶ�
			AxisSpeed ZAxisSpeed;
			//�Ƿ�����z��
			bool LockZAxis = false;
			//�ӳ�
			int Delay{ 0 };
			
		};

	}
}