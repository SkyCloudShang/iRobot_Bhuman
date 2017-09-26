// Same base finctionality as on Linux, hence the includes.
#include "Platform/Linux/ThreadBase.cpp"

void Thread::nameThread(const std::string& name)
{
  VERIFY(!pthread_setname_np(name.c_str()));
}