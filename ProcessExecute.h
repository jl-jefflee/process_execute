#pragma once
#include "ProcessBase.h"
#include <vector>
#include <memory>
#include "IExceptionHandler.h"

using namespace Workflow;
using namespace Process;
namespace Workflow
{
	namespace Produce
	{
		//工序的执行与异常处理
		class ProcessExecute
		{
		private:
			struct ThreadParam
			{
				ThreadParam(ProcessBase &process, vector<bool> &flags, int index) :
				process_(process), flags_(flags), index_(index)
				{

				}
				ProcessBase &process_;
				vector<bool> &flags_;
				int index_;
			};
		public:
			ProcessExecute(const vector<shared_ptr<ProcessBase>> processes);
			ProcessExecute(shared_ptr<ProcessBase> process);
			~ProcessExecute();

			static void Run(const vector<shared_ptr<ProcessBase>> processes, bool handleException);
			static void Run(const vector<shared_ptr<ProcessBase>> processes);
			static void Run(const shared_ptr<ProcessBase> process);
			static void IgnoreGetMaterialFailed(bool ignore);
			
			void Run();
			void RunAsync();
			void RunParallelAsync();
			bool Wait();
			bool WaitVerify();
			bool WaitVerify(bool handleException);
			bool Finished();
			const ExceptionHandleResult get_exception_handle_result() const;
			const int get_process_count() const;
			function<bool()> OnGetMaterialFail{ nullptr };
			function<void()> OnRetry{ nullptr };
			function<void(int)> OnProcessFinished{ nullptr };

		private:
			void verifyProcess(shared_ptr<ProcessBase> process,bool handleException);
			static UINT AFX_CDECL AsyncThread(LPVOID myself);
			static UINT AFX_CDECL ParallelThread(LPVOID param);

		private:
			const vector<shared_ptr<ProcessBase>> processes_;
			shared_ptr<IExceptionHandler> exception_handler_{ nullptr };
			vector<bool> thread_finished_flags_;
			vector<shared_ptr<ThreadParam>> thread_params_;
			CCriticalSection cs_;
			bool init_{ true };
			ExceptionHandleResult exception_handle_result_{ ExceptionHandleResult::Unknown };
			bool finished_{ false };
			int execute_index_{ 0 };
		};
	}
}

