#include "stdafx.h"
#include "GetMaterial.h"


namespace Workflow
{
	namespace Process
	{
		GetMaterial::GetMaterial(shared_ptr<INozzle> nozzlePtr) :
			NozzlePtr(nozzlePtr),
			LockZAxis(false)
		{

			//�رմ���
			Actions.push_back([this]{
				this->NozzlePtr->WriteOutputPort(NozzlePtr->get_blow_output_port(), ActionStatus::NotExecute);
			});

			//�ر�����
			Actions.push_back([this]{
				this->NozzlePtr->WriteOutputPort(NozzlePtr->get_suck_output_port(), ActionStatus::NotExecute);
			});

			//�����½�
			Actions.push_back([this]{
				if (NozzlePtr->get_down_output_port() != OutputPort::Unspecified)
				{
					this->NozzlePtr->WriteOutputPort(NozzlePtr->get_down_output_port(), ActionStatus::Execute);
				}
			});

			//Z���½�
			Actions.push_back([this]{
				if (!LockZAxis)
				{
					this->NozzlePtr->ZToPosition(this->Position, this->ZAxisSpeed);
				}
			});

			//�ȴ������¸�Ӧ���ź�
			Actions.push_back([this]{
				if (NozzlePtr->get_down_output_port() != OutputPort::Unspecified)
				{
					int millisecond = ::GetTickCount();
					while (true)
					{
						if (this->NozzlePtr->ReadInputPort(NozzlePtr->get_down_input_port()) == SensorStatus::Signal)
						{
							break;
						}
						Sleep(SLEEPTIME);
						if (::GetTickCount() - millisecond > (unsigned int) this->NozzlePtr->ParamSensorTimeout())
						{
							shared_ptr<ProcessException> ex = make_shared<ProcessException>(ProcessExceptionCode::NotDetectNozzleDownSignal);
							exception_ = ex;
							throw *ex;
						}
					}
				}
			});

			//�ȴ�Z���Ƿ��Ѿ��½���ָ��λ��
			Actions.push_back([this]{
				if (LockZAxis)
				{
					this->NozzlePtr->WaitZToPosition(this->Position);
				}
			});

			//ȡ��ǰ�ȴ�
			Actions.push_back([this]{
				Sleep(this->BeforeWaitTime);
			});

			//����
			Actions.push_back([this]{
				this->NozzlePtr->WriteOutputPort(NozzlePtr->get_suck_output_port(), ActionStatus::Execute);
			});

			//�ȴ�������ո�Ӧ���ź�
			Actions.push_back([this]{
				if (DetectSuckSensor)
				{
					int millisecond = ::GetTickCount();
					while (true)
					{
						if (this->NozzlePtr->ReadInputPort(NozzlePtr->get_suck_input_port()) == SensorStatus::Signal)
						{
							break;
						}
						Sleep(SLEEPTIME);
						if (::GetTickCount() - millisecond >(unsigned int) this->NozzlePtr->ParamSuckSensorTimeout())
						{
							//�ر�����
							this->NozzlePtr->WriteOutputPort(NozzlePtr->get_suck_output_port(), ActionStatus::NotExecute);
							shared_ptr<ProcessException> ex = make_shared<ProcessException>(ProcessExceptionCode::NotDetectNozzleSuckSignal);
							exception_ = ex;
							break;
						}
					}
				}
			});

			//ȡ�Ϻ�ȴ�
			Actions.push_back([this]{
				Sleep(this->AfterWaitTime);
			});

			//������̧
			Actions.push_back([this]{
				if (NozzlePtr->get_down_output_port() != OutputPort::Unspecified)
				{
					this->NozzlePtr->WriteOutputPort(NozzlePtr->get_down_output_port(), ActionStatus::NotExecute);
				}
			});

			//Z������
			Actions.push_back([this]{
				if (!LockZAxis)
				{
					this->NozzlePtr->ZToPosition(this->SafeHeight, this->ZAxisSpeed);
				}
			});

			//�ȴ������ϸ�Ӧ���ź�
			Actions.push_back([this]{
				if (NozzlePtr->get_down_output_port() != OutputPort::Unspecified)
				{
					int millisecond = ::GetTickCount();
					while (true)
					{
						if (this->NozzlePtr->ReadInputPort(NozzlePtr->get_up_input_port()) == SensorStatus::Signal)
						{
							break;
						}
						Sleep(SLEEPTIME);
						if (::GetTickCount() - millisecond > (unsigned int)this->NozzlePtr->ParamSensorTimeout())
						{
							shared_ptr<ProcessException> ex = make_shared<ProcessException>(ProcessExceptionCode::NotDetectNozzleUpSignal);
							exception_ = ex;
							throw *ex;
						}
					}
				}
			});

			//�ж�Z���Ƿ񵽴�ָ��λ��
			Actions.push_back([this]{
				if (LockZAxis)
				{
					this->NozzlePtr->WaitZToPosition(this->SafeHeight);
				}
			});

			Actions.push_back([this]{
				if (ProcessBase::show_mode_){ return; }

				if (Delay > 0)
				{
					Sleep(Delay);
				}
				
				//����ڹ�������м������������Ӧ��ֱ���ж��Ƿ�������쳣
				//��������������Ӧ����û���ź�ʱ�׳��쳣
				if (DetectSuckSensor)
				{
					if (exception_)
					{
						//����һ����ProcessException
						throw *dynamic_cast<ProcessException*>(exception_.get());
					}
				}
				else
				{
					int millisecond = ::GetTickCount();
					while (true)
					{
						if (this->NozzlePtr->ReadInputPort(NozzlePtr->get_suck_input_port()) == SensorStatus::Signal)
						{
							break;
						}
						if (::GetTickCount() - millisecond >500)//������쳬ʱʱ��
						{
							shared_ptr<ProcessException> ex = make_shared<ProcessException>(ProcessExceptionCode::NotDetectNozzleSuckSignal);
							exception_ = ex;
							throw *ex;
						}
						Sleep(SLEEPTIME);
					}
				}

			});
			
		}


		GetMaterial::~GetMaterial()
		{
		}




		


	}
}