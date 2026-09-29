#pragma once
#include <cmath>
struct Vec2 {
    double x=0, y=0;
    constexpr Vec2()=default;
    constexpr Vec2(double x,double y):x(x),y(y){}
    Vec2 operator+(const Vec2&o)const{return{x+o.x,y+o.y};}
    Vec2 operator-(const Vec2&o)const{return{x-o.x,y-o.y};}
    Vec2 operator*(double s)const{return{x*s,y*s};}
    Vec2&operator+=(const Vec2&o){x+=o.x;y+=o.y;return*this;}
    Vec2&operator*=(double s){x*=s;y*=s;return*this;}
    double dot(const Vec2&o)const{return x*o.x+y*o.y;}
    double len()const{return std::sqrt(x*x+y*y);}
    double len2()const{return x*x+y*y;}
    bool null()const{return x==0&&y==0;}
    void zero(){x=y=0;}
};
struct Vec3 {
    double x=0,y=0,z=0;
    constexpr Vec3()=default;
    constexpr Vec3(double x,double y,double z):x(x),y(y),z(z){}
    bool null()const{return x==0&&y==0&&z==0;}
    void zero(){x=y=z=0;}
};
