// copied from tcds/utils/src/common/LockGuard.cc
#include "gem/utils/LockGuard.h"

template <class L>
gem::utils::LockGuard::LockGuard(L& lock) :
  m_lock(lock)
{
  m_lock.lock();
}

template <class L>
gem::utils::LockGuard::~LockGuard()
{
  m_lock.unlock();
}
