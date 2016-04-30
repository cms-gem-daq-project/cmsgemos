// copied from tcds/utils/src/common/Lock.cc

#include <gem/utils/Lock.h>

gem::utils::Lock::Lock(toolbox::BSem::State state, bool recursive) :
  m_semaphore(state, recursive)
{
  // default constructor
}

gem::utils::Lock::~Lock()
{
  unlock();
}

void gem::utils::Lock::lock()
{
  m_semaphore.take();
}

void gem::utils::Lock::unlock()
{
  m_semaphore.give();
}
