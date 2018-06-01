#include "stdafx.h"
#include "ProcessBase.h"
#include "HardwareException.h"
#include "ProduceException.h"
#include "VerifyException.h"
#include "ProcessException.h"
#include "SafetyMonitor.h"

using namespace Hardware;

namespace Workflow
{
	namespace Process
	{
		namespace{
			//暂停标志
			bool pause_flag_ = false;
			
		}
		bool ProcessBase::show_mode_ = false;

		ProcessBase::ProcessBase() 
		{
		}


		ProcessBase::~ProcessBase()
		{
		}



		void ProcessBase::Start()
		{
			ResetException();
			action_index_ = 0;
			executeAction(Actions.cbegin(), Actions.cend());
			status_ = ProcessStatus::Success;
		}

		void ProcessBase::Continue()
		{
			ResetException();
			executeAction(Actions.begin() + action_index_, Actions.end());
			status_ = ProcessStatus::Success;
		}

		void ProcessBase::Jump()
		{
			action_index_++;
			ResetException();
			executeAction(Actions.begin() + action_index_, Actions.end());
			status_ = ProcessStatus::Success;
		}

		void ProcessBase::executeAction(vector<function<void()>>::const_iterator begin,
			vector<function<void()>>::const_iterator end)
		{
			try
			{
				for (; begin < end; begin++)
				{
					if (SafetyMonitor::GetDefault()->get_emergency_signal() == SensorStatus::Signal)
					{
						throw ProcessException(ProcessExceptionCode::DetectEmergencySignal);
					}
					while (pause_flag_)
					{
						Sleep(50);
					}
					(*begin)();
					action_index_++;
				}
			}
			catch (ProduceException &pe)
			{
				exception_ = make_shared<ProduceException>(pe);
				status_ = ProcessStatus::Fail;
				throw;
			}
			catch (ProcessException &pe)
			{
				exception_ = make_shared<ProcessException>(pe);
				status_ = ProcessStatus::Fail;
				throw;
			}
			catch (HardwareException &he)
			{
				exception_ = make_shared<HardwareException>(he);
				status_ = ProcessStatus::Fail;
				throw;
			}
			catch (VerifyException &ve)
			{
				exception_ = make_shared<VerifyException>(ve);
				status_ = ProcessStatus::Fail;
				throw;
			}
			catch (exception &ex)
			{
				exception_ = make_shared<exception>(ex);
				status_ = ProcessStatus::Fail;
				throw;
			}
		}



		void ProcessBase::Pause()
		{
			pause_flag_ = true;
		}
		void ProcessBase::Resume()
		{
			pause_flag_ = false;
		}
		

		const std::string ProcessBase::get_status_description() const
		{
			switch (status_)
			{
			case ProcessStatus::Init:
				return "初始化";
			case ProcessStatus::Success:
				return "成功";
			default:
				return "未知的工序状态";
			}
		}

		
		void ProcessBase::ResetException()
		{
			exception_.reset();
		}

		const shared_ptr<exception> ProcessBase::get_exception()const
		{
			return exception_;
		}
		const ProcessStatus ProcessBase::get_status() const
		{
			return status_;
		}
		const ProcessStation ProcessBase::get_station() const
		{
			return station_;
		}
		void ProcessBase::set_station(const ProcessStation station)
		{
			station_ = station;
		}
		void ProcessBase::ResetStatus()
		{
			status_ = ProcessStatus::Init;
		}

	}
}