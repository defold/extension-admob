#if defined(DM_PLATFORM_IOS)

#include <Foundation/Foundation.h>
#include <dispatch/dispatch.h>
#include "firebase/admob/banner_view.h"


#include <mach/thread_info.h> 
#include <pthread.h> 


namespace AdMobHelper {

void DispatchDelete(::firebase::admob::BannerView* banner_view)
{
    dispatch_async(dispatch_get_main_queue(), ^{
        NSLog(@"Dispatching on main thread: %@", [NSThread currentThread]);

        {
        	mach_port_t tid = pthread_mach_thread_np(pthread_self());
        	printf("MAWE: %s: MAIN THREAD %u\n", __FUNCTION__, tid);
        }
    });
}


void PrintThread(const char* s)
{
	NSLog(@"THREAD ID: %@ %s", [NSThread currentThread], s);
}

} // namespace

#endif // ios