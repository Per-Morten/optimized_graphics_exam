//===========================================================
// File: GameEvent.h	
// StudentName: Per-Morten Straume                          
//                                                          
// Exam 2015: IMT-2531 Graphics Programming Exam.                                
//===========================================================
#pragma once

//An emun for all the possible actions in the game

enum class ActionEnum :int
{
    NOACTION = 0,
    LEFT, // movment
    RIGHT,
    FORWARD,
    BACK, 
    DOWN,
    UP,
    RAISE, // raising terrain
    LOWER,
    RESET,
    LATER, // changing time
    EARLIER,
    CREATE, // creating and removing blocks
    DESTROY,
    MOUSEMOTION, // Moving the mouse
    ENABLEMOVEMENT,
    NEXTTEXTURE, // Flips through the textures in the textureAtlas
    TOGGLESNOW,
    TOGGLERAIN,
    TOGGLEWARPMODE
};

/* This struct deals with an agent creating events.  This has an agent number and the action*/
struct GameEvent
{
    int agent;
    ActionEnum action;
};

