/*
CoffeeBubblesSpheres.cpp
*/

#include <ri.h>
#include <RixInterfaces.h>
#include <RiTypesHelper.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <iomanip>
#include <vector>

//#include "PNoise3D.h"

#define DEFAULT_NUM 5000
#define DEFAULT_OUTERR 4.5
#define DEFAULT_INTERR 2.0
#define DEFAULT_MINRAD 0.02
#define DEFAULT_MAXRAD 0.1
#define DEFAULT_MAXFORM 0.5
#define DEFAULT_THRESHOLD 0.15

using namespace std;

// A RiProcedural must implement these functions.
extern "C"
{
	PRMANEXPORT RtPointer ConvertParameters ( RtString paramStr              );
	PRMANEXPORT RtVoid    Subdivide         ( RtPointer data, RtFloat detail );
	PRMANEXPORT RtVoid    Free              ( RtPointer data                 );
}

// A custom data structure for defining an arbitrary number of spheres
typedef struct 
{
	RtInt	num ;
	RtFloat interR ;
	RtFloat outerR ;
	RtFloat minrad ;
	RtFloat maxrad ;
	RtFloat maxform ;
	RtFloat threshold ;
} BubblesData;

RtFloat randBetween(RtFloat min, RtFloat max);

//Create an instance of PNoise3D
//PNoise3D *perlinPtr = new PNoise3D( );

// Create a group of data elements grouped together under one name
// as a kind of costumnazia data type
typedef struct 
{
	RtFloat xpos;
	RtFloat zpos;
	RtFloat spheresize;
}Circle;

// Create a vector to storage all the bubbles
// vector<data type> vector name
vector<Circle> circles;
void createSpheres(Circle thisCircle);

// ----------------------------------------------------
// A RiProcedural required function
// ----------------------------------------------------
//
RtPointer ConvertParameters(RtString paramStr)
{
	// Allocate a block of memory to store one instance of our custom data structure.
	BubblesData *dataPtr = (BubblesData*)calloc(1,sizeof(BubblesData));

	// The user has forgotten to specify the number and size of the spheres in the
	// RenderManProcedural's data text field - so we use our default values.	
	if(paramStr == NULL || strlen(paramStr) == 0) 
	{
		dataPtr->num = DEFAULT_NUM;
		dataPtr->interR = DEFAULT_INTERR;
		dataPtr->outerR = DEFAULT_OUTERR;
		dataPtr->minrad = DEFAULT_MINRAD;
		dataPtr->maxrad = DEFAULT_MAXRAD;
		dataPtr->maxform = DEFAULT_MAXFORM;
		dataPtr->threshold = DEFAULT_THRESHOLD;
		
	}
	
	else
	{
		sscanf(paramStr, "%i %f %f %f %f %f %f", &dataPtr->num,
									      		 &dataPtr->interR, 
										  		 &dataPtr->outerR, 
										  		 &dataPtr->minrad, 
										  		 &dataPtr->maxrad, 
										  		 &dataPtr->maxform, 
										 		 &dataPtr->threshold);
	}
	return (RtPointer)dataPtr;
}

// ----------------------------------------------------
// A RiProcedural required function
// ----------------------------------------------------
RtVoid Subdivide(RtPointer data, RtFloat detail) 
{
    RtInt 	num = ((BubblesData*)data)->num;
	RtFloat interR = ((BubblesData*)data)->interR;
	RtFloat outerR = ((BubblesData*)data)->outerR;
	RtFloat minrad = ((BubblesData*)data)->minrad;
	RtFloat maxrad = ((BubblesData*)data)->maxrad;
	RtFloat maxform = ((BubblesData*)data)->maxform;
	RtFloat threshold = ((BubblesData*)data)->threshold;
	
	RtFloat dist;
	
	srand(1);
	RtFloat value = 0.1;
	RiAttribute("displacementbound", "float sphere", &value, RI_NULL);
	
	//--------------------Creat bubbles----------------------------
	for (int n = -1; n < num; )
	{
		RtFloat x = randBetween(-outerR,outerR);
		RtFloat z = randBetween(-outerR,outerR);
		RtFloat dist = sqrt(x * x + z * z) ;
		
		//RtFloat jitter = perlinPtr -> eval(x,0,z,5);
		RtFloat jitter = 0;
		
		if ( dist <= outerR  && dist >= interR )
		{
			// Use this parameter to make the bubbles that near the center bigger
			RtFloat gapseed = fabs((outerR-dist)-(dist-(interR - jitter)));
			RtFloat gap = 1 - gapseed / fabs(outerR - interR + jitter);
				
			RtFloat size = randBetween(minrad,maxrad) * gap + minrad;
			bool check = true;
			n = n++;
			
			if ( n == 0 )
			{
				// Save bubble info
				Circle ThisCircle = Circle{x, z, size};
				circles.push_back(ThisCircle);
				createSpheres(ThisCircle);	
			}
			
			else
			{
				for ( auto oldCircles : circles )
				{
					// Determine if bubbles are coincident
					float cdist = sqrt(pow(oldCircles.xpos - x ,2) + pow(oldCircles.zpos - z, 2));
					float cdd = cdist - oldCircles.spheresize + size;
					
					if( cdist <= oldCircles.spheresize ) 
					{
						// Because in c++, for..else loop doesn't work.
						// I have to put a sign of break.
						check = false;
						break;
					}
					
					if( cdd <= 0 )
					{
						size = cdist - oldCircles.spheresize - randBetween(0, cdist - oldCircles.spheresize - minrad);
					}
					
					if( size <= minrad )
					{
						check = false;
						break;
					}
					
				}
				if( check != false)
				{
					Circle ThisCircle = Circle{x, z, size};
					circles.push_back(ThisCircle);
					createSpheres(ThisCircle);
				}
			}		
		}
	}
}
// ----------------------------------------------------
// A RiProcedural required function
// ----------------------------------------------------
RtVoid Free(RtPointer data) {
    free(data);
	}
	
// ----------------------------------------------------
// Our utility functions begin here 
// ----------------------------------------------------
RtFloat randBetween(RtFloat min, RtFloat max) {
    return ((RtFloat)rand()/RAND_MAX) * (max - min) + min;
    }

void createSpheres(Circle thisCircle)
{
		RiTransformBegin();
			RiTranslate(thisCircle.xpos,0,thisCircle.zpos);
    		RiSphere(thisCircle.spheresize, -thisCircle.spheresize, thisCircle.spheresize, 360, RI_NULL);
		RiTransformEnd();
}
