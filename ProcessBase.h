#pragma once
#include <vector>
#include "HardwareException.h"
#include <memory>
#include "ProcessException.h"
#include <exception>

using namespace std;

namespace Workflow
{
	namespace Process
	{
		//工序的状态
		enum class ProcessStatus
		{
			Init = 0,
			Success,
			Fail,
		
		};


		//工序执行的工位
		enum class ProcessStation
		{
			Unknown=0,
			Socket,
			Feeder,
			Collector,
			Tray,
			Corrector,
			ShakeFeeder,
			NgBox,
		};


		//工序的基类
		class ProcessBase
		{
		public:
			ProcessBase();
			virtual ~ProcessBase();

			//启动工序的执行
			void Start();
			//工序从中断处继续执行
			void Continue();
			//跳过工序中执行失败的动作继续执行
			void Jump();
			


			
			const std::string get_status_description() const;
			virtual const shared_ptr<exception> get_exception() const;
			virtual const ProcessStatus get_status() const;
			virtual const ProcessStation get_station() const;
			virtual void set_station(const ProcessStation station);
			virtual void ResetStatus();


			//暂停所有工序
			static void Pause();
			//恢复所有工序
			static void Resume();
			




			static bool show_mode_;
			//以下两个成员应改成私有成员
			vector<function<void()>> Actions;
			virtual void ResetException();
		private:
			//要执行的下一个动作的索引
			unsigned int action_index_{ 0 };

			void executeAction(vector<function<void()>>::const_iterator begin,
				vector<function<void()>>::const_iterator end);
		protected:
			shared_ptr<exception> exception_{ nullptr };
			ProcessStatus status_{ ProcessStatus::Init };
			ProcessStation station_{ ProcessStation::Unknown };
			
		};
	}
}