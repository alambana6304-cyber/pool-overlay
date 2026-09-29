#pragma once
#include <vector>
#include <array>
#include "math.hpp"
constexpr double BALL_R=3.800475;
constexpr double BALL_R2=BALL_R*2.0;
constexpr double DT=0.005;
constexpr double TABLE_L=-127.0;
constexpr double TABLE_R=127.0;
constexpr double TABLE_T=-63.5;
constexpr double TABLE_B=63.5;
constexpr int MAX_BALLS=16;
constexpr int MAX_TICKS=3000;
constexpr double EPS=1e-11;
constexpr std::array<Vec2,6> POCKETS={{
    {TABLE_L,TABLE_T},{0.0,TABLE_T},{TABLE_R,TABLE_T},
    {TABLE_L,TABLE_B},{0.0,TABLE_B},{TABLE_R,TABLE_B},
}};
constexpr double POCKET_R=7.0;
struct BallState{
    int index=0,cls=0;
    bool on_table=false,alive=true;
    Vec2 pos,vel;
    Vec3 spin;
    std::vector<Vec2> path;
};
struct GameState{
    double aim_angle=0.0,power=1.0;
    bool valid=false;
    std::vector<BallState> balls;
};
void predict(GameState&gs);
