#ifndef _MODULE_DRIVER_HPP_
#define _MODULE_DRIVER_HPP_

#include "module/i_module.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <map>
#include <string>

namespace face2wind
{
	class ModuleDriver
	{
		typedef boost::shared_ptr<IModule> IModulePtr;
		typedef std::map<std::string, IModulePtr> ModuleMap;
		typedef std::map<std::string, IModulePtr>::iterator ModuleMapIterator;

	public:
		~ModuleDriver();

		static ModuleDriver *GetInstance();

		void Run();
		void Stop();
		void Exist();
		void SignalExit();

		void DoStopThread();
		void OnProgramExitThread();

		bool RegisterModule(const std::string name, IModule* module);
		bool UnregisterModule(const std::string name);

	private:
		ModuleDriver();
		ModuleDriver(const ModuleDriver&);
		ModuleDriver& operator=(const ModuleDriver&);

	protected:
		boost::asio::io_service m_io_service;
		boost::asio::signal_set m_signals;
		bool keep_running;
		bool program_exit;
		ModuleMap m_module_map;
		boost::mutex m_write_file_lock;
	};
}

#endif