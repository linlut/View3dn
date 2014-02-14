<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<HTML><HEAD>
<META http-equiv=Content-Type content="text/html; charset=windows-1252"></HEAD>
<BODY><XMP>#include <iostream.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

//#define _DEBUG
typedef struct PNT
{
   float x;
   float y;
   float z;
}PNT;

typedef struct RGBA
{
    GLubyte r,g,b,a;
}RGBA;


const int TABSIZE=64;
const int TABMASK=TABSIZE-1;
const float PI=3.1415926;

int perm[TABSIZE];
double noise_tab[TABSIZE+1];
inline int Perm(int i)
{
    i=i&TABMASK;
    return perm[i];
};

#define INDEX(ix,iy,iz) Perm(ix+Perm(iy+Perm(iz)))

 
GLubyte* TexImg; 

#define WOOD    0
#define MARBLE  1

int TexType=WOOD;
static int flag=0;  


void InitTables(void);

void display(void);

void init(void);
void IniTex(int xsize, int ysize, int zsize);
RGBA marble(PNT pnt);
RGBA marble_color(float x);
void CreatTexture(int xsize, int ysize, int zsize);            
float turbulence(PNT pnt, float pixel_size);
float calc_noise( PNT pnt);
RGBA wood(PNT pnt);
RGBA wood_color(float x);
//void idleFunc();
void reshapeFunc(int w,int h);
void keyFunc(unsigned char key, int x,int y);
inline float SNoise(PNT p)
{
    return 1-2*calc_noise(p);
};



#ifdef _DEBUG
 void printperm();
 void printImg(int xsize, int ysize, int zsize);
#endif
 
int main(int argc,char** argv)
{
   if (argc==2)
     {
        if ( strcmp(argv[1],"m")==0) TexType=MARBLE;
        else if(strcmp(argv[1],"w")==0) TexType=WOOD; 
     }   
   glutInit(&argc, argv);
   glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA|GLUT_DEPTH);
   glutInitWindowSize (500, 500); 
   glutInitWindowPosition (500, 500);
   glutCreateWindow ("Solid Texture");
   init ();
   glutDisplayFunc(display);
   glutReshapeFunc(reshapeFunc);
   glutKeyboardFunc(keyFunc);
  // glutIdleFunc(idleFunc); 
   glutMainLoop();
   return 0;   
}


/* initialize permutation table and noise table */
void InitTables()
{
   int i,j,k;
   float temp[TABSIZE];
   float tt;
   
   for(i=0;i<TABSIZE;i++)
      {
         temp[i]=(float)drand48();
         noise_tab[i] =(float)drand48();
         perm[i]=i;
      }
   noise_tab[TABSIZE]=noise_tab[0];
      
   /* sort a random array to initialize the permutation table */
   for(i=1;i<TABSIZE;i++)
      for(j=TABSIZE-1;j>=i;j--)
         {
            if(temp[j-1]>temp[j])
               {
                  tt=temp[j];
                  temp[j]=temp[j-1];
                  temp[j-1]=tt; 
                  k=perm[j];
                  perm[j]=perm[j-1];
                  perm[j-1]=k;
               }
         };
}

void init () 
{

   GLfloat light_position[]={1.0,1.0,1.0,0.0};
   GLfloat mat_shin[]={80.0};
   GLfloat mat_ambient[]={0.4,0.4,0.4,1.0};
   GLfloat mat_diffuse[]={0.6,0.6,0.6,1.0};
   GLfloat light_ambient[]={0.7,0.7,0.6,1.0};  
   
   
   InitTables();
#ifdef _DEBUG
   printperm();
  // printImg();
#endif   
   
/* select clearing color        */
   glClearColor (0.0, 0.0, 0.0, 1.0);
   glShadeModel(GL_SMOOTH);


   glLightfv(GL_LIGHT0,GL_POSITION, light_position);
   glLightfv(GL_LIGHT0,GL_AMBIENT, light_ambient);
   glMaterialfv(GL_FRONT,GL_SHININESS, mat_shin);
   glMaterialfv(GL_FRONT,GL_AMBIENT, mat_ambient);
   glMaterialfv(GL_FRONT,GL_DIFFUSE, mat_diffuse);
   
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   
   IniTex(TABSIZE,TABSIZE,TABSIZE);


#ifdef _DEBUG
   //printperm();
   printImg(TABSIZE,TABSIZE,TABSIZE);
#endif   
}

void reshapeFunc(int w,int h)
{
 /* initialize viewing values  */
   glViewport(0,0,w,h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(-12.0, 12.0, -12.0, 12.0, -12.0, 12.0);   
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   //gluLookAt(0.0,0,5.0, 0.0,0.0,0.0, 0.0,1.0,0.0);
  flag=0;
}

void display()
{
   
     
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    
    glRotatef(3,0.0,1.0,0.0);
    if(flag==0)
       {
          glRotatef(45,1.0,0.0,1.0);
          flag=1;
       };   
    
/* Make a cube here*/
    
         glBegin(GL_QUAD_STRIP);
         
          glNormal3f(0.0, 0.0, 1.0);
         //vertex 1
            glTexCoord3f(0.0,0.0,0.0);
            glVertex3f(-4.0, -4.0,4.0);
         
         //vertex 2   
            glTexCoord3f(1.0,0.0,0.0);
            glVertex3f(4.0,-4.0,4.0);
            
         //vertex 3
            glTexCoord3f(0.0,1.0,0.0);
            glVertex3f(-4.0,4.0,4.0);
        
        //vertex 4  
            glTexCoord3f(1.0,1.0,0.0);
            glVertex3f(4.0,4.0,4.0);
         
       glNormal3f(0.0, 1.0, 0.0);
        //vertex 5  
            glTexCoord3f(0.0,1.0,1.0);
            glVertex3f(-4.0,4.0,-4.0);
            
        //vertex 6  
            glTexCoord3f(1.0,1.0,1.0);
            glVertex3f(4.0,4.0,-4.0);
     
            
        glNormal3f(0.0, 0.0, -1.0);
        //vertex 7  
            glTexCoord3f(0.0,0.0,1.0);
            glVertex3f(-4.0,-4.0,-4.0);
            
            
         //vertex 8  
            glTexCoord3f(1.0,0.0,1.0);
            glVertex3f(4.0,-4.0,-4.0);
           
        glNormal3f(0.0, -1.0, 0.0);
        //vertex 1
            glTexCoord3f(0.0,0.0,0.0);
            glVertex3f(-4.0, -4.0,4.0);
         
         //vertex 2   
            glTexCoord3f(1.0,0.0,0.0);
            glVertex3f(4.0,-4.0,4.0);
            
         glEnd();
   
   glBegin(GL_QUADS);
   
         glNormal3f(-1.0, 0.0, 0.0);
        //vertex 1
            glTexCoord3f(0.0,0.0,0.0);
            glVertex3f(-4.0, -4.0,4.0);
   
         //vertex 3
            glTexCoord3f(0.0,1.0,0.0);
            glVertex3f(-4.0,4.0,4.0);
   
         //vertex 5  
            glTexCoord3f(0.0,1.0,1.0);
            glVertex3f(-4.0,4.0,-4.0);
  
        //vertex 7  
            glTexCoord3f(0.0,0.0,1.0);
            glVertex3f(-4.0,-4.0,-4.0);
         
        glNormal3f(1.0,0.0, 0.0);
         //vertex 2   
            glTexCoord3f(1.0,0.0,0.0);
            glVertex3f(4.0,-4.0,4.0);
        
        //vertex 4  
            glTexCoord3f(1.0,1.0,0.0);
            glVertex3f(4.0,4.0,4.0);
         
        //vertex 6  
            glTexCoord3f(1.0,1.0,1.0);
            glVertex3f(4.0,4.0,-4.0);
     
            
         //vertex 8  
            glTexCoord3f(1.0,0.0,1.0);
            glVertex3f(4.0,-4.0,-4.0);
   
   glEnd();
   
   glutSwapBuffers();
   glutPostRedisplay();
   
}


/* Initialize Texture */
void IniTex(int xsize,int ysize,int zsize)
{
   CreatTexture(xsize,ysize,zsize);            
   glEnable(GL_TEXTURE_3D_EXT);
   glTexParameterf(GL_TEXTURE_3D_EXT,GL_TEXTURE_WRAP_S,GL_REPEAT);
   glTexParameterf(GL_TEXTURE_3D_EXT,GL_TEXTURE_WRAP_T,GL_REPEAT);
   glTexParameterf(GL_TEXTURE_3D_EXT,GL_TEXTURE_WRAP_R_EXT,GL_REPEAT);
   glTexParameterf(GL_TEXTURE_3D_EXT,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
   glTexParameterf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
   glTexImage3DEXT(GL_TEXTURE_3D_EXT,0,GL_RGBA,xsize,ysize,zsize,0,GL_RGBA,GL_UNSIGNED_BYTE,TexImg);
   
}

RGBA marble(PNT pnt)
{
  float y;
  y = pnt.y+ 3.0*turbulence(pnt, 1.0/TABSIZE);
  y = sin(y*PI); 
  return (marble_color(y));
}


RGBA marble_color(float x)
{
  RGBA  clr;
  x = sqrt(x+1.0)*.7071; 
  clr.g =(GLubyte)(255*( .30 + .8*x));
  x=sqrt(x);
  clr.r = (GLubyte)(255*(.30 + .6*x));
  clr.b =(GLubyte)(255*( .60 + .4*x)); 
  clr.a=1;
  return (clr);
}

float turbulence(PNT pnt, float pixel_size)
{
  float t, scale;

  t=0;
  for(scale=1.0; scale >pixel_size; scale/=2.0)
    {
      pnt.x = pnt.x/scale; pnt.y = pnt.y/scale; pnt.z = pnt.z/scale;
      t+=fabs(SNoise(pnt)* scale) ; 
    }
  return(t);
}


/* Sampling texture */
void CreatTexture(int xsize, int ysize, int zsize)
{
   RGBA temp;
   PNT pnt;
   int i,j,k;
   GLubyte* p;
   
   TexImg=new GLubyte[xsize*ysize*zsize*4];
   if(!TexImg)
      {
         cerr<<"Out of memory! Program aborted."<<endl;
         exit(1);
       }     
   for(k=0;k<zsize;k++)
      for(i=0;i<ysize;i++)
         for(j=0;j<xsize;j++)
           {
               pnt.y=(float)i/ysize; 
               pnt.x=(float)j/xsize;
               pnt.z=(float)k/zsize;
               p=&TexImg[(k*xsize*ysize+i*xsize+j)*4];
               if(TexType==MARBLE)
                  temp=marble(pnt);   
               else   
                  temp=wood(pnt);
               p[0]=temp.r;
               p[1]=temp.g;
               p[2]=temp.b;
               p[3]=temp.a;
        };

   
}


/* trilinear interpolation to calculate noise at given point */
float calc_noise( PNT pnt)
{
  float  t1;
  float  p_l,p_l2,    /* value lerped down left side of face1 & face 2 */
         p_r,p_r2,    /* value lerped down left side of face1 & face 2 */
         p_face1,     /* value lerped across face 1 (x-y plane ceil of z) */
         p_face2,     /* value lerped across face 2 (x-y plane floor of z) */
         p_final;     /* value lerped through cube (in z)                  */

  //float   tnoise;
  register int      x, y, z,px,py,pz;
  
  px = (int)pnt.x;
  py = (int)pnt.y;
  pz = (int)pnt.z;
  x = px &(TABSIZE-1); /* make sure the values are in the table           */
  y = py &(TABSIZE-1); /* Effectively, replicates the table thought space */
  z = pz &(TABSIZE-1);
  
  t1 = pnt.y - py;
  p_l  = noise_tab[INDEX(x,y,z+1)]+t1*(noise_tab[INDEX(x,y+1,z+1)]-noise_tab[INDEX(x,y,z+1)]);  
  p_r  =noise_tab[INDEX(x+1,y,z+1)]+t1*(noise_tab[INDEX(x+1,y+1,z+1)]-noise_tab[INDEX(x+1,y,z+1)]);
  p_l2 = noise_tab[INDEX(x,y,z)]+ t1*( noise_tab[INDEX(x,y+1,z)] - noise_tab[INDEX(x,y,z)]);     
  p_r2 = noise_tab[INDEX(x+1,y,z)]+ t1*(noise_tab[INDEX(x+1,y+1,z)] - noise_tab[INDEX(x+1,y,z)]);
  
  t1 = pnt.x - px; 
  p_face1 = p_l + t1 * (p_r - p_l);
  p_face2 = p_l2 + t1 * (p_r2 -p_l2);
  
  t1 = pnt.z - pz;
  p_final =  p_face2 + t1*(p_face1 -p_face2);
  
  return(p_final);
}




/*void idleFunc()
{
    
    spin=(spin+1)%360;
    glutPostRedisplay();
    return;
}

*/

void keyFunc(unsigned char key, int x,int y)
{
   switch(key)
   {
      case 0x1b:
         delete[] TexImg;
         exit(0);
         break;
         
   }
}


RGBA wood(PNT pnt)
{
  float x, y;

  x=pnt.x*0.6; y=pnt.y-0.5;
  y=sqrt(x*x+y*y)+0.08*turbulence(pnt, 1.0/TABSIZE);
  y=y*y;
  y = sin(y*PI*20);
  //y=sqrt(y);
  return (wood_color(y));
}


RGBA wood_color(float x)
{
  RGBA  clr;
  
  clr.r=165+(x+1)*45;
  clr.g=100+(x+1)*50;
  clr.b=10;
  clr.a=1;
  return (clr);
}



#ifdef _DEBUG
void printperm()
{
   int i;
   for (i=0;i<4;i++)
      cout<<"   "<<perm[i];
   cout<<endl;   
}

void printImg(int xsize,int ysize,int zsize)
{
   int i,j,k;
   for (i=0;i<4;i++)
      for (j=0;j<8;j++)
        {
           for (k=0;k<4;k++)
              cout<<"   "<<(int)TexImg[(i*xsize*ysize+j*xsize)*4+k];
           cout<<endl;   
        }   
}

#endif

</XMP></BODY></HTML>
