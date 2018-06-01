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
		//�����״̬
		enum class ProcessStatus
		{
			Init = 0,
			Success,
			Fail,
		
		};


		//����ִ�еĹ�λ
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


		//����Ļ���
		class ProcessBase
		{
		public:
			ProcessBase();
			virtual ~ProcessBase();

			//���������ִ��
			void Start();
			//������жϴ�����ִ��
			void Continue();
			//����������ִ��ʧ�ܵĶ�������ִ��
			void Jump();
			


			
			const std::string get_status_description() const;
			virtual const shared_ptr<exception> get_exception() const;
			virtual const ProcessStatus get_status() const;
			virtual const ProcessStation get_station() const;
			virtual void set_station(const ProcessStation station);
			virtual void ResetStatus();


			//��ͣ���й���
			static void Pause();
			//�ָ����й���
			static void Resume();
			




			static bool show_mode_;
			//����������ԱӦ�ĳ�˽�г�Ա
			vector<function<void()>> Actions;
			virtual void ResetException();
		private:
			//Ҫִ�е���һ������������
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