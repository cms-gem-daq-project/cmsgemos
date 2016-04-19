// copied from tcds/utils/include/tcds/utils/LockGuard.h
// would like to just include their library, but it's not in our xdaq release
#ifndef gem_utils_LockGuard_h
#define gem_utils_LockGuard_h

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

  } // namespace utils
} // namespace gem

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

#endif // _gem_utils_LockGuard_h_
