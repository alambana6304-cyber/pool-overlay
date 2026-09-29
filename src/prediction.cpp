#include "prediction.hpp"
#include <cmath>
#include <algorithm>
static constexpr double C_FRIC_A=0.00145772594752187;
static constexpr double C_FRIC_B=196.0;
static constexpr double C_FRIC_C=10.878;
static constexpr double C_SPIN_F=2.5;
static constexpr double C_GRAV_S=9.8;
static constexpr double C_CUSH_N=0.804;
static constexpr double C_CUSH_T=0.54;
static bool moving(const BallState&b){return!(b.vel.null()&&b.spin.null());}
static void apply_friction(BallState&b){
    double vx=b.vel.x,vy=b.vel.y,sx=b.spin.x,sy=b.spin.y,sz=b.spin.z;
    const double R=BALL_R;
    if(!moving(b))return;
    double v15=R*sx-vy,v16=-vx-sy*R,v17=std::sqrt(v16*v16+v15*v15),v18=v17*C_FRIC_A,v19=DT;
    if(v18>EPS){double v20=(v18>=v19)?DT:v17*C_FRIC_A,v21=C_FRIC_B*v20/v17,v22=v16*v21,v23=v15*v21;vx+=v22;vy+=v23;sx-=v23*C_SPIN_F/R;sy+=v22*C_SPIN_F/R;}
    if(v18<v19){double spd=std::sqrt(vy*vy+vx*vx);if(spd>0){double v28=std::max(0.0,1.0-(v19-v18)*C_FRIC_C/spd);vx*=v28;vy*=v28;sx=vy/R;sy=-vx/R;}}
    double v29=C_GRAV_S*v19;
    sz=(sz<=0)?std::min(0.0,sz+v29):std::max(0.0,sz-v29);
    b.vel={vx,vy};b.spin={sx,sy,sz};
}
static void cushion_bounce(BallState&b,double angle){
    double vx=b.vel.x,vy=b.vel.y,sx=b.spin.x,sy=b.spin.y,sz=b.spin.z;
    const double R=BALL_R;double ca=std::cos(angle),sa=std::sin(angle);
    double v18=ca*vx-sa*vy,v19=v18-R*sz,v20=sa*vx+ca*vy,v21=v18-R*sz;
    if(v19<0)v21=-v19;
    double v22=v21/C_SPIN_F,v23=(v20<0)?-v20:v20,v24=0.2*2.0*v23,v25=(v19<=0)?-1.0:1.0;
    if(v22<v24)v24=v22;
    double v26=v25*v24,sz2=sz+C_SPIN_F*v26/R,v29=v18-v26/C_SPIN_F,v30=-(v20*C_CUSH_N);
    b.vel={sa*v30+ca*v29,ca*v30-v29*sa};
    double v32=sa*sx+ca*sy,v33=ca*sx-sa*sy-v20*C_CUSH_T/R;
    b.spin={sa*v32+ca*v33,ca*v32-v33*sa,sz2};
}
static double ball_ball_time(const BallState&a,const BallState&b,double t_max){
    double dx=b.pos.x-a.pos.x,dy=b.pos.y-a.pos.y,dvx=b.vel.x-a.vel.x,dvy=b.vel.y-a.vel.y;
    double dot=2.0*(dx*dvx+dy*dvy);if(dot>=0)return-1;
    double dv2=dvx*dvx+dvy*dvy;if(dv2==0)return-1;
    double dd=dx*dx+dy*dy,diam2=BALL_R2*BALL_R2,disc=dot*dot-4.0*dv2*(dd-diam2);
    if(disc<0)return-1;
    double t=(-dot-std::sqrt(disc))/(2.0*dv2);
    if(t<0||t-EPS>t_max)return-1;return t;
}
static double wall_time(const BallState&b,double t_max,double&ang_out){
    const double L=TABLE_L+BALL_R,R=TABLE_R-BALL_R,T=TABLE_T+BALL_R,B=TABLE_B-BALL_R;
    double vx=b.vel.x,vy=b.vel.y,x=b.pos.x,y=b.pos.y,best=t_max+1.0,ang=0;
    auto try_t=[&](double t,double a){if(t>EPS&&t<best){best=t;ang=a;}};
    if(vx<0&&x>L)try_t((L-x)/vx,0.0);
    if(vx>0&&x<R)try_t((R-x)/vx,M_PI);
    if(vy<0&&y>T)try_t((T-y)/vy,M_PI_2);
    if(vy>0&&y<B)try_t((B-y)/vy,-M_PI_2);
    if(best>t_max)return-1;ang_out=ang;return best;
}
static bool in_pocket(const Vec2&p){
    for(auto&pk:POCKETS)if((p.x-pk.x)*(p.x-pk.x)+(p.y-pk.y)*(p.y-pk.y)<POCKET_R*POCKET_R)return true;
    return false;
}
static void elastic(BallState&a,BallState&b){
    double dx=a.pos.x-b.pos.x,dy=a.pos.y-b.pos.y,d=std::sqrt(dx*dx+dy*dy);
    if(d<EPS)return;double nx=dx/d,ny=dy/d;
    double v15=-(nx*a.vel.x)-a.vel.y*ny,v16=nx*b.vel.x+ny*b.vel.y;
    a.vel.x+=nx*v16-(-(nx*v15)-a.vel.x);a.vel.y+=ny*v16-(-(ny*v15)-a.vel.y);
    b.vel.x-=-(nx*v15)-(nx*v16-b.vel.x);b.vel.y-=-(ny*v15)-(ny*v16-b.vel.y);
}
void predict(GameState&gs){
    if(!gs.valid||gs.balls.empty())return;
    auto*cue=&gs.balls[0];
    if(cue->index==0){const double S=295.0*gs.power;cue->vel={S*std::cos(gs.aim_angle),S*std::sin(gs.aim_angle)};}
    for(auto&b:gs.balls)b.path.push_back(b.pos);
    for(int tick=0;tick<MAX_TICKS;tick++){
        std::vector<BallState*>alive;
        for(auto&b:gs.balls)if(b.alive)alive.push_back(&b);
        bool any=false;for(auto*b:alive)if(moving(*b)){any=true;break;}
        if(!any)break;
        double dt=DT;int ct=0,ci=-1,cj=-1;double ca=0,col_t=dt+1;
        for(int i=0;i<(int)alive.size();i++){
            if(!moving(*alive[i]))continue;
            for(int j=i+1;j<(int)alive.size();j++){double t=ball_ball_time(*alive[i],*alive[j],dt);if(t>=0&&t<col_t){col_t=t;ct=1;ci=i;cj=j;}}
            double wt,wa;wt=wall_time(*alive[i],dt,wa);if(wt>=0&&wt<col_t){col_t=wt;ct=2;ci=i;ca=wa;}
        }
        double step=(ct>0)?col_t:dt;
        for(auto*b:alive)if(moving(*b)){b->pos.x+=b->vel.x*step;b->pos.y+=b->vel.y*step;}
        if(ct==1){elastic(*alive[ci],*alive[cj]);alive[ci]->path.push_back(alive[ci]->pos);alive[cj]->path.push_back(alive[cj]->pos);}
        else if(ct==2){cushion_bounce(*alive[ci],ca);alive[ci]->path.push_back(alive[ci]->pos);}
        if(ct>0){double rem=dt-step;if(rem>EPS)for(auto*b:alive)if(moving(*b)){b->pos.x+=b->vel.x*rem;b->pos.y+=b->vel.y*rem;}}
        for(auto*b:alive)apply_friction(*b);
        for(auto*b:alive)if(in_pocket(b->pos))b->alive=false;
        if(tick%8==0)for(auto*b:alive)b->path.push_back(b->pos);
    }
    for(auto&b:gs.balls)b.path.push_back(b.pos);
}
