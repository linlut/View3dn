//shapefunc.cpp
#include "fortranarray.h"
#include "shapefunc.h"


#define call
const FREAL FHALF=0.5;
const FREAL QUARTER=0.25;
const FREAL ONETHREE=(1.0/3);


class Bvect8Init
{
private:
    void _specialInit(M2 & BVECT)
    {
		BVECT(1,1)=-1;
		BVECT(1,4)=-1;
		BVECT(1,5)=-1;
		BVECT(1,8)=-1;
		BVECT(2,1)=-1;
		BVECT(2,2)=-1;
		BVECT(2,5)=-1;
		BVECT(2,6)=-1;
		BVECT(3,1)=-1;
		BVECT(3,2)=-1;
		BVECT(3,3)=-1;
		BVECT(3,4)=-1;
    }

public:
    Bvect8Init(M2 * pp)
    {
        if (pp==NULL) return;
        M2 & BVECT = *pp;
	    BVECT.setValue(1);
        _specialInit(BVECT);
    }
};


class Bvect20Init: public Bvect8Init
{
private:
    void _specialInit(M2 & BVECT)
    {
		BVECT(1,9)=0; BVECT(2,9)=-1; BVECT(3,9)=-1;
		BVECT(1,10)=1; BVECT(2,10)=0; BVECT(3,10)=-1;
		BVECT(1,11)=0; BVECT(2,11)=1; BVECT(3,11)=-1; 
        BVECT(1,12)=-1; BVECT(2,12)=0; BVECT(3,12)=-1;
		BVECT(1,13)=-1; BVECT(2,13)=-1; BVECT(3,13)=0;
		BVECT(1,14)=1; BVECT(2,14)=-1; BVECT(3,14)=0;
		BVECT(1,15)=1; BVECT(2,15)=1; BVECT(3,15)=0;
		BVECT(1,16)=-1; BVECT(2,16)=1; BVECT(3,16)=0;
		BVECT(1,17)=0; BVECT(2,17)=-1; BVECT(3,17)=1;
		BVECT(1,18)=1; BVECT(2,18)=0; BVECT(3,18)=1; 
        BVECT(1,19)=0; BVECT(2,19)=1;BVECT(3,19)=1;
		BVECT(1,20)=-1;	BVECT(2,20)=0; BVECT(3,20)=1;
    }

public:
    Bvect20Init(M2 * pp): Bvect8Init(pp)
    {
        if (pp==NULL) return;
        M2 & BVECT = *pp;
        _specialInit(BVECT);
    }
};


class Bvect26Init: public Bvect20Init
{
private:
    void _specialInit(M2 & BVECT)
    {
        int i=21;
		BVECT(1,i)=0; BVECT(2,i)=0; BVECT(3,i)=-1, i++;
		BVECT(1,i)=0; BVECT(2,i)=0; BVECT(3,i)=+1, i++;

		BVECT(1,i)=0; BVECT(2,i)=-1; BVECT(3,i)=0, i++;
		BVECT(1,i)=0; BVECT(2,i)=+1; BVECT(3,i)=0, i++;

		BVECT(1,i)=-1; BVECT(2,i)=0; BVECT(3,i)=0, i++;
		BVECT(1,i)=+1; BVECT(2,i)=0; BVECT(3,i)=0, i++;
    }

public:
    Bvect26Init(M2 * pp): Bvect20Init(pp)
    {
        if (pp==NULL) return;
        M2 & BVECT = *pp;
        _specialInit(BVECT);
    }
};



static M2 BVECT27(3,27);
static Bvect26Init zziniter27(&BVECT27);


void SHAPEFTN (const int NNODE, M1& SHAPE, const FREAL& EXISP, const FREAL& ETASP, const FREAL& ZTASP) 
{
	int I;                                                                                                      
    const FREAL S=EXISP;                                          
    const FREAL T=ETASP;                                                           
    const FREAL Z=ZTASP;    
	
    //==== SHAPE FUNCTIONS FOR 8 NODED ELEMENT ====
    if (NNODE==8){
        M2 & BVECT = BVECT27;
        for (I=1; I<=8; I++)
		    SHAPE(I)=(1+S*BVECT(1,I))*(1+T*BVECT(2,I))*(1+Z*BVECT(3,I))*0.1250;
        //for (I=1; I<=8; I++){
		    //DERIV(1,I)=BVECT(1,I)*(1+T*BVECT(2,I))*(1+Z*BVECT(3,I))/8;
		    //DERIV(2,I)=(1+S*BVECT(1,I))*BVECT(2,I)*(1+Z*BVECT(3,I))/8;
		    //DERIV(3,I)=(1+S*BVECT(1,I))*(1+T*BVECT(2,I))*BVECT(3,I)/8;
	    //}
	    return;
    }
    if (NNODE==26) goto label2;
    if (NNODE==27) goto label3;   
	
label2:
	assert(0);
	return;
	
    /*                                                                   
//==== SHAPE FUNCTIONS FOR 8 NODED ELEMENT                                
                                                                       
      SHAPE(1)=(-1.0+ST+SS+TT-SST-STT)/4.0                              
      SHAPE(2)=(1.0-T-SS+SST)*FHALF.0                                       
      SHAPE(3)=(-1.0-ST+SS+TT-SST+STT)/4.0                              
      SHAPE(4)=(1.0+S-TT-STT)*FHALF.0                                       
      SHAPE(5)=(-1.0+ST+SS+TT+SST+STT)/4.0                              
      SHAPE(6)=(1.0+T-SS-SST)*FHALF.0                                       
      SHAPE(7)=(-1.0-ST+SS+TT+SST-STT)/4.0                              
      SHAPE(8)=(1.0-S-TT+STT)*FHALF.0                                       
                                                                       
//==== AND DERIVATIVES                                                    
                                                                       
      DERIV(1,1)=(T+S2-ST2-TT)/4.0                                      
      DERIV(1,2)=-S+ST                                                  
      DERIV(1,3)=(-T+S2-ST2+TT)/4.0                                     
      DERIV(1,4)=(1.0-TT)*FHALF.0                                           
      DERIV(1,5)=(T+S2+ST2+TT)/4.0                                      
      DERIV(1,6)=-S-ST                                                  
      DERIV(1,7)=(-T+S2+ST2-TT)/4.0                                     
      DERIV(1,8)=(-1.0+TT)*FHALF.0                                          
      DERIV(2,1)=(S+T2-SS-ST2)/4.0                                      
      DERIV(2,2)=(-1.0+SS)*FHALF.0                                          
      DERIV(2,3)=(-S+T2-SS+ST2)/4.0                                     
      DERIV(2,4)=-T-ST                                                  
      DERIV(2,5)=(S+T2+SS+ST2)/4.0                                      
      DERIV(2,6)=(1.0-SS)*FHALF.0                                           
      DERIV(2,7)=(-S+T2+SS-ST2)/4.0                                     
      DERIV(2,8)=-T+ST                                                  
      return;                                                            
*/

label3:
	assert(0);
	  /*
	  continue;                                                          

      STQ=ST*ST                                                         
                                                                       
//C *** SHAPE FUNCTIONS FOR 9 NODED ELEMENT                               
                                                                      
      SHAPE(1)=(ST-STT-SST+STQ)*0.25                                    
      SHAPE(2)=(-T+TT+SST-STQ)*0.5                                      
      SHAPE(3)=(-ST+STT-SST+STQ)*0.25                                   
      SHAPE(4)=(S-STT+SS-STQ)*0.5                                       
      SHAPE(5)=(ST+STT+SST+STQ)*0.25                                    
      SHAPE(6)=(T+TT-SST-STQ)*0.5                                       
      SHAPE(7)=(-ST-STT+SST+STQ)*0.25                                   
      SHAPE(8)=(-S+STT+SS-STQ)*0.5                                      
      SHAPE(9)=1.0-TT-SS+STQ                                            
      Q2=STT*2.0                                                        
                                                                       
// *** AND DERIVATIVES                                                   
                                                                       
      DERIV(1,1)=(T-TT-ST2+Q2)*0.25                                     
      DERIV(1,2)=ST-STT                                                 
      DERIV(1,3)=(-T+TT-ST2+Q2)*0.25                                    
      DERIV(1,4)=(1.0-TT+2.0*S-Q2)*0.5                                  
      DERIV(1,5)=(T+TT+ST2+Q2)*0.25                                     
      DERIV(1,6)=-ST-STT                                                
      DERIV(1,7)=(-T-TT+ST2+Q2)*0.25                                    
      DERIV(1,8)=(-1.0+TT+2.0*S-Q2)*0.5                                 
      DERIV(1,9)=-2.0*S+Q2                                              
      Q2=SST*2.0                                                        
      DERIV(2,1)=(S-ST2-SS+Q2)*0.25                                     
      DERIV(2,2)=(-1.0+2.0*T+SS-Q2)*0.5                                 
      DERIV(2,3)=(-S+ST2-SS+Q2)*0.25                                    
      DERIV(2,4)=-ST-SST                                                
      DERIV(2,5)=(S+ST2+SS+Q2)*0.25                                     
      DERIV(2,6)=(1.0+2.0*T-SS-Q2)*0.5                                  
      DERIV(2,7)=(-S-ST2+SS+Q2)*0.25                                    
      DERIV(2,8)=ST-SST                                                 
      DERIV(2,9)=-2.0*T+Q2       
	  */

	return;
}



//C********************************************************************   
//C                                                                       
//C***  CALCULATES SHAPE FUNCTIONS AND THEIR DERIVATIVES FOR 8,20,21 nodes  
//C                                                                       
//C******************************************************************** 
#define EVAL_SHAPE_FUNC8(I)\
	SHAPE[I]=(1+S*BVECT(1,I))*(1+T*BVECT(2,I))*(1+Z*BVECT(3,I))*EIGHTH;

void SHAPEGRD8_NODEDRI(FREAL SHAPE[9], const FREAL& EXISP, const FREAL& ETASP, const FREAL& ZTASP) 
{
    const FREAL S=EXISP;
    const FREAL T=ETASP;                                                           
    const FREAL Z=ZTASP;
    const FREAL EIGHTH=0.125;
	M2& BVECT=BVECT27;
    EVAL_SHAPE_FUNC8(1);
    EVAL_SHAPE_FUNC8(2);
    EVAL_SHAPE_FUNC8(3);
    EVAL_SHAPE_FUNC8(4);
    EVAL_SHAPE_FUNC8(5);
    EVAL_SHAPE_FUNC8(6);
    EVAL_SHAPE_FUNC8(7);
    EVAL_SHAPE_FUNC8(8);
}


#define _SHAPEFUNC20(BVECT,SHAPE)  \
    for (I=1; I<=8; I++){          \
        const FREAL B1=BVECT(1,I); \
        const FREAL B2=BVECT(2,I); \
        const FREAL B3=BVECT(3,I); \
        const FREAL SB1=S*B1;      \
        const FREAL TB2=T*B2;      \
        const FREAL ZB3=Z*B3;      \
		SHAPE[I]=(1+SB1)*(1+TB2)*(1+ZB3)*(SB1+TB2+ZB3-2)*EIGHTH; \
    } \
    for (I=9; I<=20; I++){         \
        const FREAL B1=BVECT(1,I); \
        const FREAL B2=BVECT(2,I); \
        const FREAL B3=BVECT(3,I); \
		SHAPE[I]=(1+S*B1)*(1+T*B2)*(1+Z*B3)*(1+(B1*B1-1)*SS+(B2*B2-1)*TT+(B3*B3-1)*ZZ)*QUARTER; \
    }

static void 
SHAPEGRD20_NODEDRI(FREAL SHAPE[21], const FREAL& EXISP, const FREAL& ETASP, const FREAL& ZTASP) 
{
	int I;
    const FREAL S=EXISP;
    const FREAL T=ETASP;                                                           
    const FREAL Z=ZTASP;
    const FREAL EIGHTH=0.125;
    const FREAL SS=S*S, TT=T*T, ZZ=Z*Z;

	M2& BVECT=BVECT27;
    _SHAPEFUNC20(BVECT, SHAPE);
}                                                    


static void 
SHAPEGRD21_NODEDRI(FREAL SHAPE[22], const FREAL& EXISP, const FREAL& ETASP, const FREAL& ZTASP) 
{
	int I;
    const FREAL S=EXISP;
    const FREAL T=ETASP;                                                           
    const FREAL Z=ZTASP;
    const FREAL EIGHTH=0.125;
    const FREAL SS=S*S, TT=T*T, ZZ=Z*Z;

	M2& BVECT=BVECT27;
    _SHAPEFUNC20(BVECT, SHAPE);

    const FREAL SS1=1-SS;
    const FREAL TT1=1-TT;
    const FREAL ZZ1=1-ZZ;
    const FREAL SS1TT1=SS1*TT1;
    const FREAL TT1ZZ1=TT1*ZZ1;
    const FREAL ZZ1SS1=ZZ1*SS1;

    //5-6-7-8
    SHAPE[21]=SS1TT1*(1+Z)*0.5;
    const FREAL s214= +SHAPE[21]*0.25;
    const FREAL s212= -SHAPE[21]*0.5;

    SHAPE[5]+=s214;
    SHAPE[6]+=s214;
    SHAPE[7]+=s214;
    SHAPE[8]+=s214;

    SHAPE[17]+=s212;
    SHAPE[18]+=s212;
    SHAPE[19]+=s212;
    SHAPE[20]+=s212; 
 
}                                                    


static inline void 
SHAPEGRD_NODEDRI(const int NNODE, FREAL SHAPE[], const FREAL& EXISP, const FREAL& ETASP, const FREAL& ZTASP) 
{
    if (NNODE==20)
        SHAPEGRD20_NODEDRI(SHAPE, EXISP, ETASP, ZTASP); 
    else if (NNODE==8)
        SHAPEGRD8_NODEDRI(SHAPE, EXISP, ETASP, ZTASP);
    else if (NNODE==21)
        SHAPEGRD21_NODEDRI(SHAPE, EXISP, ETASP, ZTASP);  
    else{
        assert(0);
    }
}                                                    


void SHAPEGRD(const int NNODE, M1 & SHAPE, M2& DERIV, const FREAL& EXISP, const FREAL& ETASP, const FREAL& ZTASP) 
{
	M2 BVECT(3,27);
	BVECT.setValue(1);
	int I;
    const FREAL S=EXISP;
    const FREAL T=ETASP;                                                           
    const FREAL Z=ZTASP;

#define bvect BVECT 
	switch(NNODE){
	case 8:  //C*** SHAPE FUNCTIONS FOR 8 NODED ELEMENT  
		BVECT(1,1)=-1;
		BVECT(1,4)=-1;
		BVECT(1,5)=-1;
		BVECT(1,8)=-1;
		BVECT(2,1)=-1;
		BVECT(2,2)=-1;
		BVECT(2,5)=-1;
		BVECT(2,6)=-1;
		BVECT(3,1)=-1;
		BVECT(3,2)=-1;
		BVECT(3,3)=-1;
		BVECT(3,4)=-1;
		for (I=1; I<=8; I++)
			SHAPE(I)=(1+S*BVECT(1,I))*(1+T*BVECT(2,I))*(1+Z*BVECT(3,I))/8;
		for (I=1; I<=8; I++){
			DERIV(1,I)=BVECT(1,I)*(1+T*BVECT(2,I))*(1+Z*BVECT(3,I))/8;
			DERIV(2,I)=(1+S*BVECT(1,I))*BVECT(2,I)*(1+Z*BVECT(3,I))/8;
			DERIV(3,I)=(1+S*BVECT(1,I))*(1+T*BVECT(2,I))*BVECT(3,I)/8;
		}
		break;
	case 20:  //c***  shape function for 20 nodal element
		BVECT(1,1)=-1;
		BVECT(1,4)=-1;
		BVECT(1,5)=-1;
		BVECT(1,8)=-1;
		BVECT(2,1)=-1;
		BVECT(2,2)=-1;
		BVECT(2,5)=-1;
		BVECT(2,6)=-1;
		BVECT(3,1)=-1;
		BVECT(3,2)=-1;
		BVECT(3,3)=-1;
		BVECT(3,4)=-1;
		bvect(1,9)=0;
		bvect(2,9)=-1;
		bvect(3,9)=-1;
		bvect(1,10)=1;
		bvect(2,10)=0;
		bvect(3,10)=-1;
		bvect(1,11)=0;
		bvect(2,11)=1;
		bvect(3,11)=-1;
		bvect(1,12)=-1;
		bvect(2,12)=0;
		bvect(3,12)=-1;
		bvect(1,13)=-1;
		bvect(2,13)=-1;
		bvect(3,13)=0;
		bvect(1,14)=1;
		bvect(2,14)=-1;
		bvect(3,14)=0;
		bvect(1,15)=1;
		bvect(2,15)=1;
		bvect(3,15)=0;
		bvect(1,16)=-1;
		bvect(2,16)=1;
		bvect(3,16)=0;
		bvect(1,17)=0;
		bvect(2,17)=-1;
		bvect(3,17)=1;
		bvect(1,18)=1;
		bvect(2,18)=0;
		bvect(3,18)=1;
		bvect(1,19)=0;
		bvect(2,19)=1;
		bvect(3,19)=1;
		bvect(1,20)=-1;
		bvect(2,20)=0;
		bvect(3,20)=1;

		for (I=1; I<=8; I++)
			SHAPE(I)=(1+S*BVECT(1,I))*(1+T*BVECT(2,I))*(1+Z*BVECT(3,I))*(bvect(1,I)*S+bvect(2,I)*T+bvect(3,I)*Z-2)/8;
		for (I=9; I<=20; I++)
			SHAPE(I)=(1+S*BVECT(1,I))*(1+T*BVECT(2,I))*(1+Z*BVECT(3,I))*
		         (1+(bvect(1,I)*bvect(1,I)-1)*S*S+(bvect(2,I)*bvect(2,I)-1)*T*T+(bvect(3,I)*bvect(3,I)-1)*Z*Z)/4;
		for (I=1; I<=8; I++){
			DERIV(1,I)=BVECT(1,I)*(1+T*BVECT(2,I))*(1+Z*BVECT(3,I))*
		       (bvect(1,I)*S+bvect(2,I)*T+bvect(3,I)*Z-2)/8 +
		       (1+S*BVECT(1,I))*(1+T*BVECT(2,I))*(1+Z*BVECT(3,I))*
		       bvect(1,I)/8;
			DERIV(2,I)=(1+S*BVECT(1,I))*BVECT(2,I)*(1+Z*BVECT(3,I))*
		       (bvect(1,I)*S+bvect(2,I)*T+bvect(3,I)*Z-2)/8+
		       (1+S*BVECT(1,I))*(1+T*BVECT(2,I))*(1+Z*BVECT(3,I))*
		       bvect(2,I)/8;
			DERIV(3,I)=(1+S*BVECT(1,I))*(1+T*BVECT(2,I))*BVECT(3,I)*
		       (bvect(1,I)*S+bvect(2,I)*T+bvect(3,I)*Z-2)/8+
		       (1+S*BVECT(1,I))*(1+T*BVECT(2,I))*(1+Z*BVECT(3,I))*
		       bvect(3,I)/8;
		}
		for (I=9; I<=20; I++){
			DERIV(1,I)=BVECT(1,I)*(1+T*BVECT(2,I))*(1+Z*BVECT(3,I))*
				(1+(BVECT(1,I)*BVECT(1,I)-1)*S*S+(BVECT(2,I)*BVECT(2,I)
				-1)*T*T+(BVECT(3,I)*BVECT(3,I)-1)*Z*Z)/4.
				+(1+S*BVECT(1,I))*(1+T*BVECT(2,I))*(1+Z*BVECT(3,I))*
				2*(BVECT(1,I)*BVECT(1,I)-1)*S/4;
			DERIV(2,I)=(1+S*BVECT(1,I))*BVECT(2,I)*(1+Z*BVECT(3,I))*
				(1+(BVECT(1,I)*BVECT(1,I)-1)*S*S+(BVECT(2,I)*BVECT(2,I)
				-1)*T*T+(BVECT(3,I)*BVECT(3,I)-1)*Z*Z)/4.
				+(1+S*BVECT(1,I))*(1+T*BVECT(2,I))*(1+Z*BVECT(3,I))*
				2*(BVECT(2,I)*BVECT(2,I)-1)*T/4;
			DERIV(3,I)=(1+S*BVECT(1,I))*(1+T*BVECT(2,I))*BVECT(3,I)*
				(1+(BVECT(1,I)*BVECT(1,I)-1)*S*S+(BVECT(2,I)*BVECT(2,I)
				-1)*T*T+(BVECT(3,I)*BVECT(3,I)-1)*Z*Z)/4
				+(1+S*BVECT(1,I))*(1+T*BVECT(2,I))*(1+Z*BVECT(3,I))*
				2.*(BVECT(3,I)*BVECT(3,I)-1)*Z/4;
		}
		break;

	case 26: 
		assert(0);
	/*
//C*** SHAPE FUNCTIONS FOR 8 NODED ELEMENT                                
      SHAPE(1)=(-1.0+ST+SS+TT-SST-STT)/4.0;                              
      SHAPE(2)=(1.0-T-SS+SST)*FHALF.0;                                       
      SHAPE(3)=(-1.0-ST+SS+TT-SST+STT)/4.0;                              
      SHAPE(4)=(1.0+S-TT-STT)*FHALF.0;                                       
      SHAPE(5)=(-1.0+ST+SS+TT+SST+STT)/4.0;                              
      SHAPE(6)=(1.0+T-SS-SST)*FHALF.0;                                       
      SHAPE(7)=(-1.0-ST+SS+TT+SST-STT)/4.0;                              
      SHAPE(8)=(1.0-S-TT+STT)*FHALF.0;                                                                                                             
//C*** AND DERIVATIVES                                                    
      DERIV(1,1)=(T+S2-ST2-TT)/4.0;                                      
      DERIV(1,2)=-S+ST;                                                  
      DERIV(1,3)=(-T+S2-ST2+TT)/4.0;                                     
      DERIV(1,4)=(1.0-TT)*FHALF.0;                                           
      DERIV(1,5)=(T+S2+ST2+TT)/4.0;                                      
      DERIV(1,6)=-S-ST;                                                  
      DERIV(1,7)=(-T+S2+ST2-TT)/4.0;                                     
      DERIV(1,8)=(-1.0+TT)*FHALF.0;                                          
      DERIV(2,1)=(S+T2-SS-ST2)/4.0;                                     
      DERIV(2,2)=(-1.0+SS)*FHALF.0;                                          
      DERIV(2,3)=(-S+T2-SS+ST2)/4.0;                                     
      DERIV(2,4)=-T-ST;                                                  
      DERIV(2,5)=(S+T2+SS+ST2)/4.0;                                      
      DERIV(2,6)=(1.0-SS)*FHALF.0;                                           
      DERIV(2,7)=(-S+T2+SS-ST2)/4.0;                                     
      DERIV(2,8)=-T+ST; 
	  */
	return;

	case 27:
		assert(0);
	/*
      STQ=ST*ST;                                                         
//C *** SHAPE FUNCTIONS FOR 9 NODED ELEMENT                               
      SHAPE(1)=(ST-STT-SST+STQ)*0.25;                                   
      SHAPE(2)=(-T+TT+SST-STQ)*0.5;                                      
      SHAPE(3)=(-ST+STT-SST+STQ)*0.25;                                   
      SHAPE(4)=(S-STT+SS-STQ)*0.5;                                       
      SHAPE(5)=(ST+STT+SST+STQ)*0.25;                                    
      SHAPE(6)=(T+TT-SST-STQ)*0.5;                                       
      SHAPE(7)=(-ST-STT+SST+STQ)*0.25;                                   
      SHAPE(8)=(-S+STT+SS-STQ)*0.5;                                      
      SHAPE(9)=1.0-TT-SS+STQ;                                            
      Q2=STT*2.0;                                                        
//C *** AND DERIVATIVES                                                   
      DERIV(1,1)=(T-TT-ST2+Q2)*0.25;                                     
      DERIV(1,2)=ST-STT;                                                 
      DERIV(1,3)=(-T+TT-ST2+Q2)*0.25;                                    
      DERIV(1,4)=(1.0-TT+2.0*S-Q2)*0.5;                                  
      DERIV(1,5)=(T+TT+ST2+Q2)*0.25;                                     
      DERIV(1,6)=-ST-STT;                                                
      DERIV(1,7)=(-T-TT+ST2+Q2)*0.25;                                    
      DERIV(1,8)=(-1.0+TT+2.0*S-Q2)*0.5;                                 
      DERIV(1,9)=-2.0*S+Q2;                                              
      Q2=SST*2.0;                                                        
      DERIV(2,1)=(S-ST2-SS+Q2)*0.25;                                     
      DERIV(2,2)=(-1.0+2.0*T+SS-Q2)*0.5;                                 
      DERIV(2,3)=(-S+ST2-SS+Q2)*0.25;                                    
      DERIV(2,4)=-ST-SST;                                                
      DERIV(2,5)=(S+ST2+SS+Q2)*0.25;                                     
      DERIV(2,6)=(1.0+2.0*T-SS-Q2)*0.5;                                  
      DERIV(2,7)=(-S-ST2+SS+Q2)*0.25;                                    
      DERIV(2,8)=ST-SST;                                                 
      DERIV(2,9)=-2.0*T+Q2;
	  */
	default:
		assert(0);
		break;
	}
#undef bvect 

	return;
}                                                    


static inline void 
maps(FREAL& xx, FREAL& yy, FREAL& zz, const int izone, IA1& itype)
{
//  dimension itype(1),dirc(3,3,1),xyz0(3,1),sca(3,1)
//      goto (1,1,1,2,1,3,1,1,1,
//     .      1,1,1,1,1,1,1,1,1,1,1,
//     .      1,1,1,1,2,1,1,1,1,2) itype(izone)
	static int ztypeid[]={
		1,1,1,2,1,3,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,     
		1,1,1,1,2,1,1,1,1,2};
	const int tt = itype(izone)-1;
	FREAL dx, dy, dz, dxsindz;
	switch(ztypeid[tt]){
	case 1:
//c	not need to map
		break;
//c	h.cylinder map 
	case 2:
		dx=xx; dy=yy;
		xx=dx*cos(dy);
		yy=dx*sin(dy);
		break;
//c	h.sphere map 
	case 3:
		//dx=xx; dy=yy; dz=zz;
		//xx=dx*cos(dy)*sin(dz);
		//yy=dx*sin(dy)*sin(dz);
		//zz=dx*cos(dz);
		dx=xx; dy=yy; dz=zz;
        dxsindz=dx*sin(dz);
		xx=cos(dy)*dxsindz;
		yy=sin(dy)*dxsindz;
		zz=dx*cos(dz);
		break;
	default:
		assert(0);
		break;
	}
}


static inline void 
maps(Vector3d & v, const int izone, IA1& itype, M3&, M1&, M2&)
{
	static int ztypeid[]={
		1,1,1,2,1,3,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,     
		1,1,1,1,2,1,1,1,1,2};
	const int tt = itype(izone)-1;
	FREAL dx, dy, dz, dxsindz;
	switch(ztypeid[tt]){
	case 1:
//c	not need to map
		break;
//c	h.cylinder map 
	case 2:
		dx=v.x;	dy=v.y; dz=v.z;
		v.x=dx*cos(dy);
		v.y=dx*sin(dy);
		break;
//c	h.sphere map 
	case 3:
		dx=v.x;	dy=v.y; dz=v.z;
        dxsindz=dx*sin(dz);
		v.x=cos(dy)*dxsindz;
		v.y=sin(dy)*dxsindz;
		//v.x=dx*cos(dy)*sin(dz);
		//v.y=dx*sin(dy)*sin(dz);
		v.z=dx*cos(dz);
		break;
	default:
		assert(0);
		break;
	}
}


void mshKNodeZone(const int NNODE, MV3 &vertex,
		IA1 &ixstr, IA1& ixend, IA1& iystr, IA1&iyend, IA1& izstr, IA1& izend,
		M3 &xyz, const int izn, IA1& itype)
{
    ASSERT0(NNODE<=27);
    const int xleng=ixend(izn)-ixstr(izn);
    const int yleng=iyend(izn)-iystr(izn);
    const int zleng=izend(izn)-izstr(izn);
    FREAL shape[28], xleng12=1, yleng12=1, zleng12=1;
    if (xleng>0) xleng12=2.0/xleng;
    if (yleng>0) yleng12=2.0/yleng;
    if (zleng>0) zleng12=2.0/zleng;
    int iCONST=1, jCONST=1, kCONST=1;
	if (xleng != 0) iCONST = -1;
	if (yleng != 0) jCONST = -1;
	if (zleng != 0) kCONST = -1;

    int l;
	Vector3d vv[28];
    for (l=1; l<=NNODE; l++){
        Vector3d * p = &vv[l];
        p->x = xyz(1,l,izn); p->y = xyz(2,l,izn); p->z = xyz(3,l,izn);
    }

	for (int k=0; k<=zleng; k++){
		const int k1=k+1;
		const FREAL ztasp=kCONST+k*zleng12;
		for (int j=0; j<=yleng; j++){
			const int j1=j+1;			
			const FREAL etasp=jCONST+j*yleng12;			
			for (int i=0; i<=xleng; i++){
				const int i1=i+1;
				const FREAL exisp=iCONST+i*xleng12;
				//call the shape function
				call SHAPEGRD_NODEDRI(NNODE, shape, exisp, etasp, ztasp);   
				Vector3d sum= vv[1]*shape[1];
                for (l=2; l<=NNODE; l++){
                    const FREAL shapel=shape[l];
                    if (shapel!=0){
                        const Vector3d *p=&vv[l];
                        sum.x += shapel*p->x;
                        sum.y += shapel*p->y;
                        sum.z += shapel*p->z;
                    }
                }
                //if (NNODE==8)
                //    maps(sum.x, sum.y, sum.z, izn, itype);
				vertex(i1,j1,k1)=sum;
			}
		}
	}
}



void mesh4zn(MV3& vertex, IA1& nzsti,
		IA1 &ixstr, IA1& ixend, IA1& iystr, IA1& iyend, IA1& izstr, IA1& izend,
		M3& xyz, const int izn, IA1& itype)
{
	M2 xyzo(3,6), xyz1(3,28), xyz2(3,19);
	const int lengx=ixend(izn)-ixstr(izn);
    const int lengy=iyend(izn)-iystr(izn);
    const int lengz=izend(izn)-izstr(izn);
      ixstr(izn)=ixstr(izn);
      ixend(izn)=ixstr(izn)+lengx/2;
      iystr(izn)=iystr(izn);
      iyend(izn)=iystr(izn)+lengy/2;
      izstr(izn)=izstr(izn);
      izend(izn)=izstr(izn)+lengz/2;
	const int izn1=izn+1;
      ixstr(izn1)=ixstr(izn)+lengx/2;
      ixend(izn1)=ixstr(izn)+lengx;
      iystr(izn1)=iystr(izn);
      iyend(izn1)=iystr(izn)+lengy/2;
      izstr(izn1)=izstr(izn);
      izend(izn1)=izstr(izn)+lengz/2;
	const int izn2=izn+2;
      ixstr(izn2)=ixstr(izn)+lengx/2;
      ixend(izn2)=ixstr(izn)+lengx;
      iystr(izn2)=iystr(izn)+lengy/2;
      iyend(izn2)=iystr(izn)+lengy;
      izstr(izn2)=izstr(izn);
      izend(izn2)=izstr(izn)+lengz/2;
	const int izn3=izn+3;
      ixstr(izn3)=ixstr(izn);
      ixend(izn3)=ixstr(izn)+lengx/2;
      iystr(izn3)=iystr(izn);
      iyend(izn3)=iystr(izn)+lengy/2;
      izstr(izn3)=izstr(izn)+lengz/2;
      izend(izn3)=izstr(izn)+lengz;

	int k, l;
    for (l=1; l<=4; l++)
		for (k=1; k<=3; k++)
			xyzo(k,l)=xyz(k,l,izn);
	for (k=1; k<=3; k++){
		xyz2(k,1)=0;
		for (l=1; l<=4; l++)
			xyz2(k,1)=xyz2(k,1)+xyz(k,l,izn);
		xyz2(k,1)=xyz2(k,1)*QUARTER;
	}
    for (k=1; k<=3; k++){
		xyz1(k,1)=(xyzo(k,1)+xyzo(k,2)+xyzo(k,3))*ONETHREE;
		xyz1(k,2)=(xyzo(k,1)+xyzo(k,2))*FHALF;
		xyz1(k,3)=(xyzo(k,2)+xyzo(k,3))*FHALF;
		xyz1(k,4)=(xyzo(k,1)+xyzo(k,3))*FHALF;
		xyz2(k,2)=(xyzo(k,1)+xyzo(k,2)+xyzo(k,4))*ONETHREE;
		xyz2(k,3)=(xyzo(k,2)+xyzo(k,3)+xyzo(k,4))*ONETHREE;
		xyz2(k,4)=(xyzo(k,1)+xyzo(k,3)+xyzo(k,4))*ONETHREE;
		xyz2(k,5)=(xyzo(k,2)+xyzo(k,4))*FHALF;
		xyz2(k,6)=(xyzo(k,3)+xyzo(k,4))*FHALF;
		xyz2(k,7)=(xyzo(k,1)+xyzo(k,4))*FHALF;
	}
    for (k=1; k<=3; k++){
		xyz(k,1,izn)=xyzo(k,1);
		xyz(k,2,izn)=xyz1(k,2);
		xyz(k,3,izn)=xyz1(k,1);
		xyz(k,4,izn)=xyz1(k,4);
		xyz(k,5,izn)=xyz2(k,7);
		xyz(k,6,izn)=xyz2(k,2);
		xyz(k,7,izn)=xyz2(k,1);
		xyz(k,8,izn)=xyz2(k,4);
	}
    for (k=1; k<=3; k++){
      xyz(k,1,izn1)=xyz1(k,2);
      xyz(k,2,izn1)=xyzo(k,2);
      xyz(k,3,izn1)=xyz1(k,3);
      xyz(k,4,izn1)=xyz1(k,1);;
      xyz(k,5,izn1)=xyz2(k,2);
      xyz(k,6,izn1)=xyz2(k,5);
      xyz(k,7,izn1)=xyz2(k,3);
      xyz(k,8,izn1)=xyz2(k,1);
	}
    for(k=1; k<=3; k++){
      xyz(k,1,izn2)=xyz1(k,1);
      xyz(k,2,izn2)=xyz1(k,3);
      xyz(k,3,izn2)=xyzo(k,3);
      xyz(k,4,izn2)=xyz1(k,4);
      xyz(k,5,izn2)=xyz2(k,1);
      xyz(k,6,izn2)=xyz2(k,3);
      xyz(k,7,izn2)=xyz2(k,6);
      xyz(k,8,izn2)=xyz2(k,4);
	}
	for (k=1; k<=3; k++){
      xyz(k,1,izn3)=xyz2(k,7);
      xyz(k,2,izn3)=xyz2(k,2);
      xyz(k,3,izn3)=xyz2(k,1);
      xyz(k,4,izn3)=xyz2(k,4);
      xyz(k,5,izn3)=xyzo(k,4);
      xyz(k,6,izn3)=xyz2(k,5);
      xyz(k,7,izn3)=xyz2(k,3);
      xyz(k,8,izn3)=xyz2(k,6);
	}
	for (int iz=izn; iz<=izn3; iz++){
		MV3 mat=vertex(nzsti(iz));
		call mesh8zn(mat,ixstr,ixend,iystr,iyend,izstr,izend,xyz,iz,itype);
	}
}