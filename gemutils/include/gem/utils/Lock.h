#ifndef GEM_UTILS_LOCK_H
#define GEM_UTILS_LOCK_H
// copied from tcds/utils/include/tcds/utils/Lock.h
// would like to just include their library, but it's not in our xdaq release

#include <toolbox/BSem.h>

namespace gem {
  namespace utils {

    class Lock
    {
    public:
      Lock(toolbox::BSem::State state=toolbox::BSem::EMPTY, bool recursive=true);
      ~Lock();

      void lock();
      void unlock();

    private:
      toolbox::BSem m_semaphore;

      // Prevent copying.
      Lock(Lock const&);
      Lock& operator=(Lock const&);
    };

  }  // namespace utils
}  // namespace gem

#endif  // GEM_UTILS_LOCK_H
