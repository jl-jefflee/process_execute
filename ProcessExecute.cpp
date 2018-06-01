#include "stdafx.h"
#include "ProcessExecute.h"
#include "ProduceException.h"
#include "VerifyException.h"
#include "HardwareException.h"
#include "ExceptionHandler.h"
#include "DebugTools.h"
#include "IProduceExceptionHandle.h"

using namespace UI;
namespace Workflow
{
	namespace Produce
	{
		namespace
		{
			bool ignore_get_material_failed = false;
		}

		ProcessExecute::ProcessExecute(const vector<shared_ptr<ProcessBase>> processes):
			processes_(processes)
		{
			exception_handler_ = make_shared<ExceptionHandler>();
		}
		ProcessExecute::ProcessExecute(shared_ptr<ProcessBase> process):
			ProcessExecute(vector<shared_ptr<ProcessBase>>({ process }))
		{}


		ProcessExecute::~ProcessExecute()
		{
		}


		void ProcessExecute::IgnoreGetMaterialFailed(bool ignore)
		{
			ignore_get_material_failed = ignore;
		}
		void ProcessExecute::Run(const vector<shared_ptr<ProcessBase>> processes, bool handleException)
		{
			ProcessExecute execute(processes);
			try
			{
				for (auto p : processes)
				{
					p->ResetStatus();
					p->Start();
				}
			}
			catch (...){}

			for (auto p : processes)
			{
				execute.verifyProcess(p,handleException);
			}
		}
		void ProcessExecute::Run(const vector<shared_ptr<ProcessBase>> processes)
		{
			ProcessExecute::Run(processes, true);
		}
		void ProcessExecute::Run(const shared_ptr<ProcessBase> process)
		{
			ProcessExecute::Run(vector<shared_ptr<ProcessBase>>{process});
		}
		
		void ProcessExecute::Run()
		{
			exception_handle_result_ = ExceptionHandleResult::Unknown;
			execute_index_ = 0;
			try
			{
				for (auto p : processes_)
				{
					p->ResetStatus();
					p->Start();
					if (OnProcessFinished)
					{
						OnProcessFinished(execute_index_);
					}
					execute_index_++;
				}
			}
			catch (...){}

			for (auto p : processes_)
			{
				verifyProcess(p,true);
			}
			finished_ = true;
		}
		void ProcessExecute::RunAsync()
		{
			cs_.Lock();
			finished_ = false;
			exception_handle_result_ = ExceptionHandleResult::Unknown;
			init_ = false;
			for (auto flag : thread_finished_flags_)
			{
				if (!flag)
				{
					cs_.Unlock();
					throw exception("�����߳�����ִ��");
				}
			}
			thread_finished_flags_.clear();
			for (auto process : processes_)
			{
				process->ResetStatus();
				thread_finished_flags_.push_back(false);
			}
			if (!AfxBeginThread(AsyncThread, this))
			{
				cs_.Unlock();
				throw exception("����ִ�й�����첽�߳�ʧ��");
			}
			cs_.Unlock();
		}
		void ProcessExecute::RunParallelAsync()
		{
			cs_.Lock();
			init_ = false;
			exception_handle_result_ = ExceptionHandleResult::Unknown;
			for (auto flag : thread_finished_flags_)
			{
				if (!flag)
				{
					cs_.Unlock();
					throw exception("�����߳�����ִ��");
				}
			}
			thread_finished_flags_.clear();
			thread_params_.clear();
			int index = 0;
			for (auto process : processes_)
			{
				process->ResetStatus();
				thread_finished_flags_.push_back(false);
				thread_params_.push_back(make_shared<ThreadParam>(*process, thread_finished_flags_, index));
				if (!AfxBeginThread(ParallelThread, thread_params_.back().get()))
				{
					cs_.Unlock();
					throw exception("����ִ�й�����첽�߳�ʧ��");
				}
				index++;
			}
			cs_.Unlock();
		}
		bool ProcessExecute::Wait()
		{
			if (init_){ return true; }

			DWORD tick = ::GetTickCount();
			bool finished = true;
			do
			{
				finished = true;
				for (auto flag : thread_finished_flags_)
				{
					finished = finished&& flag;
				}

				Sleep(SLEEPTIMESHORT);
				if (::GetTickCount() - tick > 3 * 60 * 1000)//3���ӳ�ʱ
				{
					//DebugTools::WriteLine("AsyncTask Wait timeout!!!");
					//����ֱ���׳�ִ�г�ʱ���쳣
					throw exception("����ִ�г�ʱ");
				}
			} while (!finished);
			
			bool result = true;
			for (auto process : processes_)
			{
				if (process->get_status() != ProcessStatus::Success)
				{
					result = false;
					break;
				}
			}
			return result;
		}



		bool ProcessExecute::WaitVerify(bool handleException)
		{
			if (!Wait())
			{
				for (auto p : processes_)
				{
					verifyProcess(p,handleException);
				}
				return false;
			}
			return true;
		}
		bool ProcessExecute::Finished()
		{
			return finished_;
		}
		bool ProcessExecute::WaitVerify()
		{
			return ProcessExecute::WaitVerify(true);
		}
		
		const ExceptionHandleResult ProcessExecute::get_exception_handle_result() const
		{
			return exception_handle_result_;
		}
		const int ProcessExecute::get_process_count() const
		{
			return processes_.size();
		}




		void ProcessExecute::verifyProcess(shared_ptr<ProcessBase> process, bool handleException)
		{
			if (process->get_status() == ProcessStatus::Success){ return; }
			int step = 0;
			if (process->get_status() == ProcessStatus::Init)
			{
				step = 1;
			}


			while (true)
			{
				try
				{
					if (step == 0)
					{
						if (!process->get_exception())
						{
							throw exception("����ִ��ʧ�ܣ�����û��ȡ���쳣��Ϣ");
						}
						if (typeid(*process->get_exception()) == typeid(ProduceException))
						{
							throw *dynamic_cast<ProduceException*>(process->get_exception().get());
						}
						else if (typeid(*process->get_exception()) == typeid(ProcessException))
						{
							throw *dynamic_cast<ProcessException*>(process->get_exception().get());
						}
						else if (typeid(*process->get_exception()) == typeid(VerifyException))
						{
							throw *dynamic_cast<VerifyException*>(process->get_exception().get());
						}
						else if (typeid(*process->get_exception()) == typeid(HardwareException))
						{
							throw *dynamic_cast<HardwareException*>(process->get_exception().get());
						}
						else
						{
							throw *process->get_exception();
						}
					}
					else if (step == 1)
					{
						process->Start();
					}
					else if (step == 2)
					{
						process->Continue();
					}
					else
					{
						process->Jump();
					}
					break;
				}
				catch (ProcessException &pe)
				{
					if (!handleException){ throw; }
					if (pe.code_ == ProcessExceptionCode::NotDetectNozzleSuckSignal||
						pe.code_==ProcessExceptionCode::NotDetectFeederSuckSignal)
					{
						if (ignore_get_material_failed)
						{
							break;
						}
						else
						{
							if (process->get_station() == ProcessStation::Corrector)
							{
								exception_handle_result_ = exception_handler_->HandleCorrectorGetMaterialFail(pe, string(typeid(*process).name()));
							}
							else if (process->get_station() == ProcessStation::Socket)
							{
								exception_handle_result_ = exception_handler_->HandleSocketGetMaterialFail(pe, string(typeid(*process).name()));
							}
							else if (process->get_station() == ProcessStation::Feeder)
							{
								exception_handle_result_ = exception_handler_->HandleFeederGetMaterialFail(pe, string(typeid(*process).name()));
							}
							else if (process->get_station() == ProcessStation::Tray)
							{
								exception_handle_result_ = exception_handler_->HandleTrayGetMaterialFail(pe, string(typeid(*process).name()));
							}
							else if (process->get_station() == ProcessStation::ShakeFeeder)
							{
								if (OnGetMaterialFail&&OnGetMaterialFail())
								{
									exception_handle_result_ = ExceptionHandleResult::Retry;
								}
								else
								{
									exception_handle_result_ = exception_handler_->HandleShakeFeederGetMaterialFail(pe, string(typeid(*process).name()));
									if (exception_handle_result_ == ExceptionHandleResult::Retry)
									{
										if (OnRetry)
										{
											OnRetry();
										}
									}
									if ((int)exception_handle_result_ == (int)ShakeFeederGetMaterialFailHandleResult::Retry)
									{
										exception_handle_result_ = ExceptionHandleResult::Retry;
									}
									else if ((int)exception_handle_result_ == (int)ShakeFeederGetMaterialFailHandleResult::Finish)
									{
										exception_handle_result_ = ExceptionHandleResult::Finish;
									}
									else
									{
										exception_handle_result_ = ExceptionHandleResult::Terminal;
									}
								}
							}
							else
							{
								//δ�����ȡ��ʧ��
								throw ProduceException(ProduceExceptionCode::GetMaterialFail);
							}


							if (exception_handle_result_ == ExceptionHandleResult::Retry)
							{
								step = 1;
							}
							else if (exception_handle_result_ == ExceptionHandleResult::Ignore)
							{
								step = 3;
							}
							else if (exception_handle_result_ == ExceptionHandleResult::Next)
							{
								break;
							}
							else if (exception_handle_result_ == ExceptionHandleResult::Finish)
							{
								throw ProduceException(ProduceExceptionCode::UserFinish);
							}
							else
							{
								throw ProduceException(ProduceExceptionCode::UserTeminal);
							}
						}
					}
					else if (pe.code_ == ProcessExceptionCode::NotDetectAutoTrayReadyTraySensor)
					{
						//�Զ����ϻ�׼������
						exception_handle_result_ = exception_handler_->
							HandleAutoTrayReadyEmpty(pe, string(typeid(*process).name()));
						if (exception_handle_result_ == ExceptionHandleResult::Loaded)
						{
							step = 2;
						}
						else if (exception_handle_result_ == ExceptionHandleResult::Finish)
						{
							throw ProduceException(ProduceExceptionCode::UserFinish);
						}
						else//Terminal
						{
							throw ProduceException(ProduceExceptionCode::UserTeminal);
						}
					}
					else if (pe.code_ == ProcessExceptionCode::DetectAutoTrayCompleteTraySensor)
					{
						//�Զ����ϻ��������
						exception_handle_result_ = exception_handler_->
							HandleAutoTrayCompleteFull(pe, string(typeid(*process).name()));
						if (exception_handle_result_ == ExceptionHandleResult::Clean)
						{
							step = 2;
						}
						else
						{
							throw ProduceException(ProduceExceptionCode::UserTeminal);
						}
					}
					else if (pe.code_ == ProcessExceptionCode::DetectEmergencySignal)
					{
						exception_handle_result_ = exception_handler_->
							HandleEmergencySignal(pe, string(typeid(*process).name()));
						if (exception_handle_result_ == ExceptionHandleResult::Retry)
						{
							step = 1;
						}
						else
						{
							throw ProduceException(ProduceExceptionCode::UserTeminal);
						}
					}
					else
					{
						exception_handle_result_ = exception_handler_->
							HandleSensorSignalError(pe, string(typeid(*process).name()));
						if (exception_handle_result_ == ExceptionHandleResult::Ignore)
						{
							step = 3;
						}
						else if (exception_handle_result_ == ExceptionHandleResult::Wait)
						{
							step = 2;
						}
						else if (exception_handle_result_ == ExceptionHandleResult::Retry)
						{
							step = 1;
						}
						else//Terminal
						{
							throw ProduceException(ProduceExceptionCode::UserTeminal);
						}
					}
				}
				catch (HardwareException &he)
				{
					if (!handleException){ throw; }
					if (he.code_ == HardwareExceptionCode::AxisNotArrivePosition)
					{
						//��δ����ָ��λ��
						exception_handle_result_ = exception_handler_->
							HandleAxisNotArrive(he, string(typeid(*process).name()));
						if (exception_handle_result_ == ExceptionHandleResult::Retry)
						{
							WriteLog("��δ����ָ��λ��->����");
							step = 2;
						}
						else
						{
							WriteLog("��δ����ָ��λ��->��ֹ");
							throw ProduceException(ProduceExceptionCode::UserTeminal);
						}
					}
					else
					{
						throw;
					}
				}



			}
			if (OnProcessFinished)
			{
				OnProcessFinished(execute_index_);
				execute_index_++;
			}
		}


		UINT AFX_CDECL ProcessExecute::AsyncThread(LPVOID myself)
		{
			ProcessExecute *execute = (ProcessExecute*)myself;
			execute->execute_index_ = 0;
			try
			{
				for (auto p : execute->processes_)
				{
					p->Start();
#if _DEBUG
					if (execute->processes_.size() >= 3)
					{
						DebugTools::WriteLine("ִ���˹���:");
						DebugTools::WriteLine(DebugTools::DoubleToString(execute->execute_index_));
					}
					
#endif
					if (execute->OnProcessFinished)
					{
						execute->OnProcessFinished(execute->execute_index_);
					}
					execute->execute_index_++;
				}
			}
			catch (...)
			{
				//�������쳣���쳣�ᱣ�浽���ԵĹ�����
			}
			for (auto it = execute->thread_finished_flags_.begin(); it != execute->thread_finished_flags_.end(); it++)
			{
				*it = true;
			}
			execute->finished_ = true;
			return 0;
		}
		UINT AFX_CDECL ProcessExecute::ParallelThread(LPVOID param)
		{
			ThreadParam *item = (ThreadParam*)param;
			try
			{
				item->process_.Start();
			}
			catch (...)
			{
			}
			item->flags_[item->index_] = true;
			return 0;
		}
	}
}