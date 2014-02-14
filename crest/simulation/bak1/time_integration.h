//FILE: time_integration.h
#ifndef _INC_TIMEINTEGRATION_ZN200808_H_
#define _INC_TIMEINTEGRATION_ZN200808_H_


#include <assert.h>
#include <vectorall.h>

class CSimuEntity;

extern void VelocityVerletIntegration(CSimuEntity &obj, const int timeid, const double& dt);
extern void VerletIntegration(CSimuEntity &obj, const int timeid, const double& dt);
extern void ImplicitRectangularIntegration(CSimuEntity &obj, const int timeid, const double& dt);
extern void VelocityBasedIntegration(CSimuEntity &obj, const int timeid, const double& dt);
extern void ImplicitEulerIntegration(CSimuEntity &obj, const int timeid, const double& dt);


#endif
