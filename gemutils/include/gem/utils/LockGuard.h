// copied from tcds/utils/include/tcds/utils/LockGuard.h
// would like to just include their library, but it's not in our xdaq release
#ifndef GEM_UTILS_LOCKGUARD_H
#define GEM_UTILS_LOCKGUARD_H

namespace gem {
  namespace utils {

    template <class L>
      class LockGuard
      {
      public:
        LockGuard(L& lock);
        ~LockGuard();

      private:
        L& m_lock;

        // Prevent copying.
        LockGuard(LockGuard const&);
        LockGuard& operator=(LockGuard const&);
      };

  }  // namespace utils
}  // namespace gem

template <class L>
gem::utils::LockGuard<L>::LockGuard(L& lock) :
m_lock(lock)
{
  m_lock.lock();
}

template <class L>
gem::utils::LockGuard<L>::~LockGuard()
{
  m_lock.unlock();
}

#endif  // GEM_UTILS_LOCKGUARD_H
