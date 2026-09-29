#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#include "prediction.hpp"
#include "memory.hpp"
static CGRect g_table=CGRectZero;
static GameState g_state;
static CGPoint g2s(Vec2 p){
    float nx=(float)((p.x-TABLE_L)/(TABLE_R-TABLE_L));
    float ny=(float)((TABLE_B-p.y)/(TABLE_B-TABLE_T));
    return CGPointMake(g_table.origin.x+nx*g_table.size.width,
                       g_table.origin.y+ny*g_table.size.height);
}
@interface _OV:UIView @end
@implementation _OV
-(instancetype)initWithFrame:(CGRect)f{
    self=[super initWithFrame:f];
    if(self){self.backgroundColor=[UIColor clearColor];self.userInteractionEnabled=NO;}
    return self;
}
-(void)drawRect:(CGRect)rect{
    if(!g_state.valid)return;
    CGContextRef ctx=UIGraphicsGetCurrentContext();if(!ctx)return;
    CGContextSetLineCap(ctx,kCGLineCapRound);
    for(auto&ball:g_state.balls){
        if(!ball.on_table||ball.path.size()<2)continue;
        bool cue=(ball.index==0);
        if(cue)CGContextSetStrokeColorWithColor(ctx,[UIColor colorWithRed:1 green:1 blue:1 alpha:0.9f].CGColor);
        else    CGContextSetStrokeColorWithColor(ctx,[UIColor colorWithRed:1 green:0.85f blue:0 alpha:0.6f].CGColor);
        CGContextSetLineWidth(ctx,cue?2.5f:1.8f);
        CGFloat dash[]={8,6};CGContextSetLineDash(ctx,0,dash,2);
        CGPoint first=g2s(ball.path[0]);
        CGContextMoveToPoint(ctx,first.x,first.y);
        for(size_t i=1;i<ball.path.size();i++){CGPoint pt=g2s(ball.path[i]);CGContextAddLineToPoint(ctx,pt.x,pt.y);}
        CGContextStrokePath(ctx);
        CGContextSetLineDash(ctx,0,nullptr,0);
        CGPoint end=g2s(ball.path.back());
        float dr=cue?6:4;
        CGRect dot=CGRectMake(end.x-dr,end.y-dr,dr*2,dr*2);
        if(cue)CGContextSetFillColorWithColor(ctx,[UIColor colorWithRed:1 green:1 blue:1 alpha:0.95f].CGColor);
        else    CGContextSetFillColorWithColor(ctx,[UIColor colorWithRed:1 green:0.85f blue:0 alpha:0.8f].CGColor);
        CGContextFillEllipseInRect(ctx,dot);
        if(!ball.alive){
            CGContextSetStrokeColorWithColor(ctx,[UIColor colorWithRed:0 green:1 blue:0.4f alpha:0.9f].CGColor);
            CGContextSetLineWidth(ctx,2.5f);
            CGRect pr=CGRectMake(end.x-9,end.y-9,18,18);
            CGContextStrokeEllipseInRect(ctx,pr);
        }
    }
    CGContextSetLineDash(ctx,0,nullptr,0);
    CGContextSetLineWidth(ctx,1.2f);
    CGContextSetStrokeColorWithColor(ctx,[UIColor colorWithRed:1 green:1 blue:1 alpha:0.2f].CGColor);
    for(auto&pk:POCKETS){
        CGPoint c=g2s(pk);double pr=(POCKET_R/(TABLE_R-TABLE_L))*g_table.size.width;
        CGRect r=CGRectMake(c.x-pr,c.y-pr,pr*2,pr*2);CGContextStrokeEllipseInRect(ctx,r);
    }
}
@end
static _OV* g_view=nil;
static void update_state(){
    static uintptr_t base=0;
    if(!base)base=find_game_base();if(!base)return;
    uintptr_t sgm=read_u64(base+OFF::SHARED_GAME_MGR);if(!sgm)return;
    uintptr_t vc=read_u64(sgm+OFF::VIS_CUE);
    uintptr_t vg=vc?read_u64(vc+OFF::VIS_GUIDE):0;
    if(!vg){g_state.valid=false;return;}
    GameState gs;
    gs.aim_angle=read_f64(vg+OFF::AIM_ANGLE);
    gs.power=read_f64(vc+OFF::POWER);
    gs.valid=true;
    uintptr_t table=read_u64(sgm+OFF::GM_TABLE);if(!table)return;
    uintptr_t bc=read_u64(table+OFF::TABLE_BALLS);if(!bc)return;
    int count=read_i32(bc+OFF::BALLS_COUNT);
    uintptr_t bl=read_u64(bc+OFF::BALLS_ENTRY);
    if(!bl||count<=0||count>MAX_BALLS)return;
    for(int i=0;i<count;i++){
        uintptr_t bp=read_u64(bl+i*8);if(!bp)continue;
        int st=read_i32(bp+OFF::BALL_STATE);if(st!=1&&st!=2)continue;
        BallState b;b.index=i;b.cls=read_i32(bp+OFF::BALL_CLASS);b.on_table=true;
        b.pos=read_vec2(bp+OFF::BALL_POS);b.vel=read_vec2(bp+OFF::BALL_VEL);
        b.spin=read_vec3(bp+OFF::BALL_SPIN);gs.balls.push_back(b);
    }
    std::sort(gs.balls.begin(),gs.balls.end(),[](const BallState&a,const BallState&b){return a.index<b.index;});
    predict(gs);
    g_state=gs;
}
@interface _Ticker:NSObject @end
@implementation _Ticker
-(void)tick:(CADisplayLink*)link{
    update_state();
    dispatch_async(dispatch_get_main_queue(),^{[g_view setNeedsDisplay];});
}
@end
void renderer_setup(){
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(2.0*NSEC_PER_SEC)),
    dispatch_get_main_queue(),^{
        // iOS 26 compatible screen size
        CGRect scr=CGRectZero;
        NSArray<UIScene*>*scenes=[UIApplication sharedApplication].connectedScenes.allObjects;
        for(UIScene*scene in scenes){
            if([scene isKindOfClass:[UIWindowScene class]]){
                UIWindowScene*ws=(UIWindowScene*)scene;
                scr=ws.coordinateSpace.bounds;
                break;
            }
        }
        if(CGRectIsEmpty(scr))scr=CGRectMake(0,0,390,844);// fallback
        float tx=scr.size.width*0.04f,ty=scr.size.height*0.12f;
        float tw=scr.size.width*0.92f,th=scr.size.height*0.76f;
        g_table=CGRectMake(tx,ty,tw,th);
        // iOS 26 window setup
        UIWindowScene*ws=nil;
        for(UIScene*scene in [UIApplication sharedApplication].connectedScenes.allObjects){
            if([scene isKindOfClass:[UIWindowScene class]]){ws=(UIWindowScene*)scene;break;}
        }
        UIWindow*win=ws?[[UIWindow alloc]initWithWindowScene:ws]:[[UIWindow alloc]initWithFrame:scr];
        win.windowLevel=UIWindowLevelStatusBar+100;
        win.backgroundColor=[UIColor clearColor];
        win.userInteractionEnabled=NO;
        UIViewController*vc=[[UIViewController alloc]init];
        vc.view.backgroundColor=[UIColor clearColor];
        win.rootViewController=vc;
        g_view=[[_OV alloc]initWithFrame:scr];
        [vc.view addSubview:g_view];
        [win makeKeyAndVisible];
        _Ticker*ticker=[[_Ticker alloc]init];
        CADisplayLink*link=[CADisplayLink displayLinkWithTarget:ticker selector:@selector(tick:)];
        [link addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSRunLoopCommonModes];
    });
}
