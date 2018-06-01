#include "stdafx.h"
#include "PutMaterial.h"
#include "DebugTools.h"

namespace Workflow
{
	namespace Process
	{
		PutMaterial::PutMaterial(shared_ptr<INozzle> nozzlePtr) :
			NozzlePtr(nozzlePtr),
			LockZAxis(false)
		{

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
					double d = NozzlePtr->OffsetFactor();
					d = d*Offset;
					d = d + Position;
					this->NozzlePtr->ZToPosition(d, this->ZAxisSpeed);
				}
			});

			//等待气缸下感应器信号
			Actions.push_back([this]{
				if (NozzlePtr->get_down_output_port() != OutputPort::Unspecified)
				{
					DWORD millisecond = ::GetTickCount();
					while (true)
					{
						if (this->NozzlePtr->ReadInputPort(NozzlePtr->get_down_input_port()) == SensorStatus::Signal)
						{
							break;
						}
						Sleep(10);
						if (::GetTickCount() - millisecond > (unsigned int) this->NozzlePtr->ParamSensorTimeout())
						{
							shared_ptr<ProcessException> exception = make_shared<ProcessException>(ProcessExceptionCode::NotDetectNozzleDownSignal);
							exception_ = exception;
							throw *exception;
						}
					}
				}
			});

			//等待Z轴是否已下降
			Actions.push_back([this]{
				if (LockZAxis)
				{
					double d = NozzlePtr->OffsetFactor();
					d = d*Offset;
					d = d + Position;
					this->NozzlePtr->WaitZToPosition(d);
				}
			});

			//放料前等待
			Actions.push_back([this]{
				Sleep(this->BeforeWaitTime);
			});

			//关闭吸气
			Actions.push_back([this]{
				this->NozzlePtr->WriteOutputPort(NozzlePtr->get_suck_output_port(), ActionStatus::NotExecute);
			});

			//启动吹气
			Actions.push_back([this]{
				this->NozzlePtr->WriteOutputPort(NozzlePtr->get_blow_output_port(), ActionStatus::Execute);
			});

			//检测破真空
			Actions.push_back([this]{
				if (DetectFlatSensor)
				{
					int millisecond = ::GetTickCount();
					while (true)
					{
						if (this->NozzlePtr->ReadInputPort(NozzlePtr->get_suck_input_port()) == SensorStatus::NonSignal)
						{
							break;
						}
						Sleep(10);
						if (::GetTickCount() - millisecond > (unsigned int)this->NozzlePtr->ParamSuckSensorTimeout())
						{
							exception_ = make_shared<ProcessException>(ProcessExceptionCode::NotDetectNozzleFlatSignal);
							break;
						}
					}
				}
			});

			//放料后等待
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
						Sleep(10);
						if (::GetTickCount() - millisecond > (unsigned int)this->NozzlePtr->ParamSensorTimeout())
						{
							shared_ptr<ProcessException> exception = make_shared<ProcessException>(ProcessExceptionCode::NotDetectNozzleUpSignal);
							exception_ = exception;
							throw *exception;
						}
					}
				}
			});

			//等待Z轴到位
			//Z轴上升
			Actions.push_back([this]{
				if (LockZAxis)
				{
					this->NozzlePtr->WaitZToPosition(this->SafeHeight);
				}
			});

			//关闭吹气
			Actions.push_back([this]{
				this->NozzlePtr->WriteOutputPort(NozzlePtr->get_blow_output_port(), ActionStatus::NotExecute);
			});


			
			Actions.push_back([this]{
				if (ProcessBase::show_mode_){ return; }

				if (Delay > 0)
				{
					Sleep(Delay);
				}

				if (DetectFlatSensor)
				{
					if (exception_)
					{
						throw *dynamic_cast<ProcessException*>( exception_.get());
					}
				}
				else
				{
					int millisecond = ::GetTickCount();
					while (true)
					{
						if (this->NozzlePtr->ReadInputPort(NozzlePtr->get_suck_input_port()) == SensorStatus::NonSignal)
						{
							break;
						}
						if (::GetTickCount() - millisecond >1500)
						{
							shared_ptr<ProcessException> exception = make_shared<ProcessException>(ProcessExceptionCode::NotDetectNozzleFlatSignal);
							exception_ = exception;
							throw *exception;
						}
						Sleep(SLEEPTIME);
					}
				}
				WriteLog("放料工序已完成");
			});

		}


		PutMaterial::~PutMaterial()
		{
		}
	}
}