#pragma once
#include "SKR_Rect.h"

//Quantities such as speed, force and gravity do not guarantee to be in a certain unit, play with numbers to get the result you want.

typedef struct SKR_StaticRect SKR_StaticRect;

typedef struct SKR_KinematicRect SKR_KinematicRect;

typedef struct SKR_DynamicRect SKR_DynamicRect;

typedef struct SKR_RectWorld SKR_RectWorld;

typedef enum { SKR_SIDESCROLLER, SKR_ISOMETRIC }SKR_GAMETYPE;


int SKR_IntersectRectLine(SKR_Rect* Rect, float* x1, float* y1, float* x2, float* y2);//returns 1 if the rect and the line are intersected, returns 0 otherwise

int SKR_IntersectRectRect(SKR_Rect* Rect1, SKR_Rect* Rect2);//returns 1 if the rects are intersected, returns 0 otherwise


SKR_RectWorld* SKR_CreateRectWorld(float Gravity, float AirFrictionCoefficient, SKR_GAMETYPE GameType);//creates a SKR world to hold static kinematic and dynamic rects' data//you must define your gametype here SKR_SIDESCROLLER or SKR_ISOMETRIC, when you set this SKR_ISOMETRIC gravity will be 0 regardless of your input

void SKR_SetGravity(SKR_RectWorld* World, float Gravity);

float SKR_GetGravity(SKR_RectWorld* World);

void SKR_SetAirFriction(SKR_RectWorld* World, float AirFrictionCoefficient);

float SKR_GetAirFriction(SKR_RectWorld* World);

void SKR_DestroyRectWorld(SKR_RectWorld* World);//this frees all the memory which is retained by the world and all the rects which are created in it


SKR_StaticRect* SKR_CreateStaticRect(SKR_RectWorld* World, SKR_Rect* Position);//static rects aren't affected physically by other rectangles and gravity, they dont have velocity, you can use them to make stationary platforms

void SKR_DestroyStaticRect(SKR_RectWorld* World, SKR_StaticRect* StaticRect);//frees the memory which is retained by the static rect and deletes it from its world

SKR_Rect* SKR_GetPositionStaticRect(SKR_StaticRect* StaticRect);

int SKR_GetStaticRectNumber(SKR_RectWorld* World);//returns how many static rects there are in the world


SKR_KinematicRect* SKR_CreateKinematicRect(SKR_RectWorld* World, SKR_Rect* Position);//kinematic rects aren't affected physically by other rectangles and gravity, they do have velocity, you can use them to make moving platforms

void SKR_DestroyKinematicRect(SKR_RectWorld* World, SKR_KinematicRect* KinematicRect);//frees the memory which is retained by the kinematic rect and deletes it from its world

SKR_Rect* SKR_GetPositionKinematicRect(SKR_KinematicRect* KinematicRect);

int SKR_GetKinematicRectNumber(SKR_RectWorld* World);//returns how many kinematic rects there are in the world

void SKR_SetXVelocityKinematicRect(SKR_KinematicRect* KinematicRect, float Xspeed);

void SKR_SetYVelocityKinematicRect(SKR_KinematicRect* KinematicRect, float Yspeed);

float SKR_GetXVelocityKinematicRect(SKR_KinematicRect* KinematicRect);

float SKR_GetYVelocityKinematicRect(SKR_KinematicRect* KinematicRect);

void SKR_AnimateKinematicRect(SKR_KinematicRect* KinematicRect, float X1, float Y1, float X2, float Y2, float Velocity);//you can use this function to make a moving platform easily, also you must put this function in your game loop like SKR_SimulateWorld function

void SKR_StopAnimatingKinematicRect(SKR_KinematicRect* KinematicRect);//you should use this function to stop the kinematic rect after it is animated by SKR_AnimateKinematicRect function, also if you want to apply another animation on same kinematic rect, you must use this function between SKR_AnimateKinematicRect functions 


SKR_DynamicRect* SKR_CreateDynamicRect(SKR_RectWorld* World, SKR_Rect* Position, float Mass, float FrictionCoefficient, float GravityMultiplier);//dynamic rects are affected physically by other rectangles and gravity, you can set their velocities or apply forces

float SKR_GetFriction(SKR_DynamicRect* DynamicRect);

void SKR_SetFriction(SKR_DynamicRect* DynamicRect, float FrictionCoefficient);

float SKR_GetGravityMultiplier(SKR_DynamicRect* DynamicRect);

void SKR_SetGravityMultiplier(SKR_DynamicRect* DynamicRect, float GravityMultiplier);

void SKR_DestroyDynamicRect(SKR_RectWorld* World, SKR_DynamicRect* DynamicRect);//frees the memory which is retained by the dynamic rect and deletes it from its world

SKR_Rect* SKR_GetPositionDynamicRect(SKR_DynamicRect* DynamicRect);

int SKR_GetDynamicRectNumber(SKR_RectWorld* World);//returns how many dynamic rects there are in the world

void SKR_SetXVelocityDynamicRect(SKR_DynamicRect* DynamicRect, float Xspeed);

void SKR_SetYVelocityDynamicRect(SKR_DynamicRect* DynamicRect, float Yspeed);

float SKR_GetXVelocityDynamicRect(SKR_DynamicRect* DynamicRect);

float SKR_GetYVelocityDynamicRect(SKR_DynamicRect* DynamicRect);

float SKR_GetMassDynamicRect(SKR_DynamicRect* DynamicRect);

void SKR_SetMassDynamicRect(SKR_DynamicRect* DynamicRect, float Mass);

void SKR_ApplyForceX(SKR_DynamicRect* DynamicRect, float Force);//applying force means you are adding force to the forces which are already applying

void SKR_ApplyForceY(SKR_DynamicRect* DynamicRect, float Force);

void SKR_SetForceX(SKR_DynamicRect* DynamicRect, float Force);//setting force means, you set the force this number regardless of previously applied forces

void SKR_SetForceY(SKR_DynamicRect* DynamicRect, float Force);

float SKR_GetForceX(SKR_DynamicRect* DynamicRect);

float SKR_GetForceY(SKR_DynamicRect* DynamicRect);

int SKR_IsOnground(SKR_DynamicRect* DynamicRect);//this function returns 1 if the dynamic rect is on something, returns 0 otherwise. this function can be helpful in situations like deciding if a character can jump in your side scroller game, will return 0 always in an isometric game


void SKR_SimulateWorld(SKR_RectWorld* World, float Milliseconds);//simulates the world for some time, you must put this in your main game loop. for example your one frame/cycle takes 15 milliseconds, so you should write it like SKR_SimulateWorld(world, 15);