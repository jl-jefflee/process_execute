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

			//关闭吹气
			Actions.push_back([this]{
				this->NozzlePtr->WriteOutputPort(NozzlePtr->get_blow_output_port(), ActionStatus::NotExecute);
			});

			//关闭吸气
			Actions.push_back([this]{
				this->NozzlePtr->WriteOutputPort(NozzlePtr->get_suck_output_port(), ActionStatus::NotExecute);
			});

			//气缸下降
			Actions.push_back([this]{
				if (NozzlePtr->get_down_output_port() != OutputPort::Unspecified)
				{
					this->NozzlePtr->WriteOutputPort(NozzlePtr->get_down_output_port(), ActionStatus::Execute);
				}
			});

			//Z轴下降
			Actions.push_back([this]{
				if (!LockZAxis)
				{
					this->NozzlePtr->ZToPosition(this->Position, this->ZAxisSpeed);
				}
			});

			//等待气缸下感应器信号
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

			//等待Z轴是否已经下降到指定位置
			Actions.push_back([this]{
				if (LockZAxis)
				{
					this->NozzlePtr->WaitZToPosition(this->Position);
				}
			});

			//取料前等待
			Actions.push_back([this]{
				Sleep(this->BeforeWaitTime);
			});

			//吸气
			Actions.push_back([this]{
				this->NozzlePtr->WriteOutputPort(NozzlePtr->get_suck_output_port(), ActionStatus::Execute);
			});

			//等待吸嘴真空感应器信号
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
							//关闭吸气
							this->NozzlePtr->WriteOutputPort(NozzlePtr->get_suck_output_port(), ActionStatus::NotExecute);
							shared_ptr<ProcessException> ex = make_shared<ProcessException>(ProcessExceptionCode::NotDetectNozzleSuckSignal);
							exception_ = ex;
							break;
						}
					}
				}
			});

			//取料后等待
			Actions.push_back([this]{
				Sleep(this->AfterWaitTime);
			});

			//气缸上抬
			Actions.push_back([this]{
				if (NozzlePtr->get_down_output_port() != OutputPort::Unspecified)
				{
					this->NozzlePtr->WriteOutputPort(NozzlePtr->get_down_output_port(), ActionStatus::NotExecute);
				}
			});

			//Z轴上升
			Actions.push_back([this]{
				if (!LockZAxis)
				{
					this->NozzlePtr->ZToPosition(this->SafeHeight, this->ZAxisSpeed);
				}
			});

			//等待气缸上感应器信号
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

			//判断Z轴是否到达指定位置
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
				
				//如果在工序过程中检测了吸嘴吸感应，直接判断是否产生了异常
				//否则检测吸嘴吸感应并在没有信号时抛出异常
				if (DetectSuckSensor)
				{
					if (exception_)
					{
						//这里一定是ProcessException
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
						if (::GetTickCount() - millisecond >500)//检测吸嘴超时时间
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