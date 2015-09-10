#include "network/network.hpp"
#include <boost/thread.hpp>
#include "module_driver.hpp"

namespace face2wind
{
ModuleDriver::ModuleDriver() : m_io_service(), m_signals(m_io_service), keep_running(false), program_exit(false)
{
  m_signals.add(SIGINT);
  m_signals.add(SIGILL);
  m_signals.add(SIGFPE);
  m_signals.add(SIGSEGV);
  m_signals.add(SIGTERM);
  m_signals.add(SIGABRT);
#if defined(SIGBREAK)
  m_signals.add(SIGBREAK);
#endif // defined(SIGQUIT)
#if defined(SIGQUIT)
  m_signals.add(SIGQUIT);
#endif // defined(SIGQUIT)

  m_signals.async_wait(boost::bind(&ModuleDriver::SignalExit, this));

  boost::thread thread_(boost::bind(&boost::asio::io_service::run, &m_io_service));
  thread_.detach();
}

ModuleDriver::~ModuleDriver()
{
  //Stop();
}

ModuleDriver *ModuleDriver::GetInstance()
{
  static ModuleDriver instance;
  return &instance;
}

void ModuleDriver::Run()
{
  m_write_file_lock.lock();
  for (ModuleMapIterator it = m_module_map.begin(); it != m_module_map.end(); ++ it)
  {
    if (IModule::ST_Invalid == it->second->m_module_state)
    {
      m_module_map.erase(it);
    }
    if (IModule::ST_Created == it->second->m_module_state || IModule::ST_Released == it->second->m_module_state)
    {
      it->second->Init();
      it->second->m_module_state = IModule::ST_Inited;
    }
    if (IModule::ST_Inited == it->second->m_module_state || IModule::ST_Stoped == it->second->m_module_state )
    {
      it->second->Start();
      it->second->m_module_state = IModule::ST_Started;
    }
  }
  m_write_file_lock.unlock();

  keep_running = true;
  while(!program_exit)
  {
    if (keep_running)
    {
      m_write_file_lock.lock();
      for (ModuleMapIterator it = m_module_map.begin(); it != m_module_map.end(); ++ it)
      {
        it->second->m_module_state = IModule::ST_Running;
        it->second->Update();
      }
      m_write_file_lock.unlock();
    }
    boost::this_thread::sleep(boost::posix_time::milliseconds(300));
  }
}

void ModuleDriver::Stop()
{
  boost::thread t(boost::bind(&ModuleDriver::DoStopThread, this));
  t.detach();
}

void ModuleDriver::Exist()
{
  boost::thread t(boost::bind(&ModuleDriver::OnProgramExitThread, this));
  t.detach();
}

void ModuleDriver::SignalExit()
{
	this->OnProgramExitThread();
}

void ModuleDriver::DoStopThread()
{
  keep_running = false;

  m_write_file_lock.lock();
  for (ModuleMapIterator it = m_module_map.begin(); it != m_module_map.end(); ++it)
  {
    it->second->Stop();
    it->second->m_module_state = IModule::ST_Stoped;
  }
  m_write_file_lock.unlock();
}

void ModuleDriver::OnProgramExitThread()
{
  if (program_exit)
    return;

  m_io_service.post(boost::bind(&boost::asio::io_service::stop, &m_io_service));

  this->DoStopThread();

  m_write_file_lock.lock();
  for (ModuleMapIterator it = m_module_map.begin(); it != m_module_map.end(); ++ it)
  {
    it->second->Release();
    it->second->m_module_state = IModule::ST_Released;
  }
  m_module_map.clear();
  m_write_file_lock.unlock();

  program_exit = true;
}

bool ModuleDriver::RegisterModule(const std::string name, IModule* module)
{
  if (NULL == module)
  {
    return false;
  }

  if (0 < m_module_map.count(name))
  {
    return false;
  }
	
  m_write_file_lock.lock();
  m_module_map[name] = IModulePtr(module);
  m_write_file_lock.unlock();

  return true;
}

bool ModuleDriver::UnregisterModule(const std::string name)
{
  if (0 >= m_module_map.count(name))
  {
    return false;
  }

  m_write_file_lock.lock();
  m_module_map.erase(name);
  m_write_file_lock.unlock();

  return true;
}
}
