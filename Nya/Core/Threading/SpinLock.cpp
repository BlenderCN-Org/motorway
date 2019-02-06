#include <Shared.h> 
#include "SpinLock.h"

SpinLock::SpinLock()
    : syncPrimitive( false )
{

}

SpinLock::SpinLock( SpinLock& spinLock )
{
    syncPrimitive = spinLock.syncPrimitive.load();
}

SpinLock& SpinLock::operator = ( SpinLock& spinLock )
{
    syncPrimitive = spinLock.syncPrimitive.load();
    return *this;
}

SpinLock::~SpinLock()
{
    syncPrimitive.store( false, std::memory_order::memory_order_release );
}

void SpinLock::lock()
{
    thread_local bool isUnlocked = false;
    while ( !syncPrimitive.compare_exchange_weak( isUnlocked, true, std::memory_order::memory_order_acquire ) ) {
        isUnlocked = false;
    }
}

void SpinLock::unlock()
{
    syncPrimitive.store( false, std::memory_order::memory_order_release );
}
