/////////////////////////////////////////////////////////////////////////////
// MedVR Simulation
// S.Y.H.
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// This is the External Variables declaration File.
// Include this file, if the following context are to be refered.
/////////////////////////////////////////////////////////////////////////////
#ifndef __EXTERNDECLARE_H__
#define __EXTERNDECLARE_H__

#include "Defines.h"

class CMI;
class CShare;
class CPerformMonitor;
class CCollision;
class CGraphic;
class CHaptic;
class CDeformation;


//extern int e_iUpdateRate; // 18+15 for the visual updating freq. of 30 Hz.
//// Gain of force feedback amplitude.
////extern float e_fForceDB;//1E-3;//0.06;
//// Collision detection threashold and response spring constant
//extern float e_fCt;
//extern float e_fKresp;
//
//// Collision detection flag...
//extern int e_iGrab;	
//extern int e_iHold;
//
//// synchronization flags...
//extern int e_iHaptictime;
//extern int e_iGraphictime;
//
//// Model type, render mode...
//extern int e_iModels;
//extern int e_iForceType;
//extern int e_iRenderMode;
//
//// Phantom tip status...
//extern float e_fCursorX, e_fCursorY, e_fCursorZ;
//extern vertex e_orientation;
//extern vertex e_velocity;
//extern vertex e_deForce; // The force feedback.
//extern vertex e_pos;
//
//// Camera transform...
//extern float e_fTx, e_fTy, e_fTz, e_fCx, e_fCy, e_fCz, e_fRoll, e_fPitch, e_fHeading, e_fTransRate;
//extern float e_fFlySpeed;

// Modules' general interface
extern CMI e_mi;
// Performance monitor, system log.
extern CPerformMonitor e_performMoniter;
// Shared data for synchronization among threads and processes.
extern CShare e_share;//(SIZE_X,SIZE_Y);

// Instrument transform.
extern float e_toolMatrix[16];

//// Obsolete...///////////////////////////////////////////////////////////////
//extern MASS* e_mass;
//extern SPRING* e_spring;
//extern float e_fKs; //Spring Constant 0.3
//extern float e_fKd;//Damping Constant 0.2
//extern float e_fDrag;//viscous drag
//
//// Dynamic extent.
//extern int e_iExtent;//SIZE_X;
//extern int e_iLGrab;
//
//// Obsolete end	/////////////////////////////////////////////////////////////

#endif	// End of this .h file