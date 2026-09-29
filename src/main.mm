#import <Foundation/Foundation.h>
void renderer_setup();
__attribute__((constructor))
static void _init(){
    @autoreleasepool{
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(1.5*NSEC_PER_SEC)),
            dispatch_get_main_queue(),^{renderer_setup();});
    }
}
