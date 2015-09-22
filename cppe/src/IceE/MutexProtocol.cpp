
#include <IceE/MutexProtocol.h>
#include <IceE/Features.h>

IceUtil::MutexProtocol
IceUtil::getDefaultMutexProtocol()
{
#ifdef _WIN32
   return PrioNone;
#else
   return ICEE_DEFAULT_MUTEX_PROTOCOL;
#endif
}
