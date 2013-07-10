// assign2.cpp : Defines the entry point for the console application.
//

/*
	CSCI 480 Computer Graphics
	Assignment 2: Simulating a Roller Coaster
*/

#include "stdafx.h"
#include "matrix.h"
#include "vector.h"
#include <pic.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <cmath>
#include <GL/glu.h>
#include <GL/glut.h>

/* represents one control point along the spline */
struct point 
{
	double x;
	double y;
	double z;
};

/* spline struct which contains how many control points, and an array of control points */
struct spline 
{
	int numControlPoints;
	struct point *points;
};

/* the spline array */
struct spline *g_Splines;

/* total number of splines */
int g_iNumOfSplines;

//Basis Matrix
matrix* m_CRBasis;

//Point Array for Spline
point* spline_Points;

//Left Inner Top Track Point Array
point* left_Inner_Top_Track_Points;

//Left Outer Top Track Point Array
point* left_Outer_Top_Track_Points;

//Left Inner Bottom Track Point Array
point* left_Inner_Bottom_Track_Points;

//Left Outer Bottom Track Point Array
point* left_Outer_Bottom_Track_Points;

//Right Inner Top Track Point Array
point* right_Inner_Top_Track_Points;

//Right Outer Top Track Point Array
point* right_Outer_Top_Track_Points;

//Right Inner Bottom Track Point Array
point* right_Inner_Bottom_Track_Points;

//Right Outer Bottom Track Point Array
point* right_Outer_Bottom_Track_Points;

//Vector Array for Tangents of each point on spline
vector* spline_Tangents;

//Vector Array for Normals of each point on spline
vector* spline_Normals;

//Vector Array for Binormals of each point on spline
vector* spline_Binormals;

//U increemnt using 0.001
int uIncrement = 1000;

//Total Segments of all Splines
int spline_Segments;

//Total points
int splinePointsSize;

int currentFrame = 0;

float acceleration = 4.0f;

GLenum error;

Pic* skybox_1;
Pic* skybox_2;
Pic* skybox_3;
Pic* skybox_4;
Pic* skybox_5;
Pic* skybox_6;
Pic* woodBoard;

GLuint sb1;
GLuint sb2;
GLuint sb3;
GLuint sb4;
GLuint sb5;
GLuint sb6;
GLuint wood;

int loadSplines(char *argv) {
	char *cName = (char *)malloc(128 * sizeof(char));
	FILE *fileList;
	FILE *fileSpline;
	int iType, i = 0, j, iLength;

	/* load the track file */
	fileList = fopen(argv, "r");
	if (fileList == NULL) {
		printf ("can't open file\n");
		exit(1);
	}
  
	/* stores the number of splines in a global variable */
	fscanf(fileList, "%d", &g_iNumOfSplines);

	g_Splines = (struct spline *)malloc(g_iNumOfSplines * sizeof(struct spline));


	/* reads through the spline files */
	for (j = 0; j < g_iNumOfSplines; j++) {
		i = 0;
		fscanf(fileList, "%s", cName);
		fileSpline = fopen(cName, "r");

		if (fileSpline == NULL) {
			printf ("can't open file\n");
			exit(1);
		}

		/* gets length for spline file */
		fscanf(fileSpline, "%d %d", &iLength, &iType);

		/* allocate memory for all the points */
		g_Splines[j].points = (struct point *)malloc(iLength * sizeof(struct point));
		g_Splines[j].numControlPoints = iLength;

		//Subtract 3 because segments are not determined for beginning and end points
		spline_Segments += (iLength - 3);

		/* saves the data to the struct */
		while (fscanf(fileSpline, "%lf %lf %lf", 
			&g_Splines[j].points[i].x, 
			&g_Splines[j].points[i].y, 
			&g_Splines[j].points[i].z) != EOF) {
			i++;
		}
	}

	free(cName);

	return 0;
}

void setBasis()
{
	//Assuming s = 0.5 

	m_CRBasis = new matrix();
	//Row 0
	m_CRBasis->setCell(0,0,-1.0);
	m_CRBasis->setCell(0,1,3.0);
	m_CRBasis->setCell(0,2,-3.0);
	m_CRBasis->setCell(0,3,1.0);

	//Row 1
	m_CRBasis->setCell(1,0,2.0);
	m_CRBasis->setCell(1,1,-5.0);
	m_CRBasis->setCell(1,2,4.0);
	m_CRBasis->setCell(1,3,-1.0);

	//Row 2
	m_CRBasis->setCell(2,0,-1.0);
	m_CRBasis->setCell(2,1,0.0);
	m_CRBasis->setCell(2,2,1.0);
	m_CRBasis->setCell(2,3,0.0);

	//Row 3
	m_CRBasis->setCell(3,0,0.0);
	m_CRBasis->setCell(3,1,2.0);
	m_CRBasis->setCell(3,2,0.0);
	m_CRBasis->setCell(3,3,0.0);

	double zerozero = m_CRBasis->getCell(0,0);
}

void createPoints()
{
	//Allocates space for all the points on the spline coaster
	spline_Points = (struct point*)malloc((spline_Segments * (uIncrement+1)) * sizeof(struct point));

	left_Inner_Top_Track_Points = (struct point*)malloc((spline_Segments * (uIncrement+1)) * sizeof(struct point));
	left_Outer_Top_Track_Points = (struct point*)malloc((spline_Segments * (uIncrement+1)) * sizeof(struct point));
	left_Inner_Bottom_Track_Points = (struct point*)malloc((spline_Segments * (uIncrement+1)) * sizeof(struct point));
	left_Outer_Bottom_Track_Points = (struct point*)malloc((spline_Segments * (uIncrement+1)) * sizeof(struct point));

	right_Inner_Top_Track_Points = (struct point*)malloc((spline_Segments * (uIncrement+1)) * sizeof(struct point));
	right_Outer_Top_Track_Points = (struct point*)malloc((spline_Segments * (uIncrement+1)) * sizeof(struct point));
	right_Inner_Bottom_Track_Points = (struct point*)malloc((spline_Segments * (uIncrement+1)) * sizeof(struct point));
	right_Outer_Bottom_Track_Points = (struct point*)malloc((spline_Segments * (uIncrement+1)) * sizeof(struct point));

	spline_Tangents = (vector*)malloc((spline_Segments * (uIncrement+1)) * sizeof(vector));
	spline_Normals = (vector*)malloc((spline_Segments * (uIncrement+1)) * sizeof(vector));
	spline_Binormals = (vector*)malloc((spline_Segments * (uIncrement+1)) * sizeof(vector));

	splinePointsSize = 0;

	for(int i = 0; i < g_iNumOfSplines; i++)
	{
		for(int j = 0; j < g_Splines[i].numControlPoints - 3; j++)
		{
			//double uValue = 0.001;
			//while(uValue <= 1.000)
			for(int k = 0; k <= uIncrement; k++)
			{
				//1x4 Matrix [u^3 u^2 u^1 1]
				matrix* uMatrix = new matrix(1,4);

				//Set U matrix for calculating point
				double uValue = 0.001 * k;

				uMatrix->setCell(0,0,pow(uValue,3));
				uMatrix->setCell(0,1,pow(uValue,2));
				uMatrix->setCell(0,2,pow(uValue,1));
				uMatrix->setCell(0,3,pow(uValue,0));

				//Set U' Matrix for derivative of spline (for Tangent)
				
				//1x4 Matrix [3u^2 2u^1 1 0]
				matrix* duMatrix = new matrix(1,4);

				duMatrix->setCell(0,0,(3.0 * pow(uValue,2)));
				duMatrix->setCell(0,1,(2.0 * pow(uValue,1)));
				duMatrix->setCell(0,2,(1.0 * pow(uValue,0)));
				duMatrix->setCell(0,3,0.0);


				//Matrix to hold control point values
				matrix* cpMatrix = new matrix(4,3);
				
				//Set control point matrix
				cpMatrix->setCell(0,0,g_Splines[i].points[j].x);
				cpMatrix->setCell(0,1,g_Splines[i].points[j].y);
				cpMatrix->setCell(0,2,g_Splines[i].points[j].z);

				cpMatrix->setCell(1,0,g_Splines[i].points[j+1].x);
				cpMatrix->setCell(1,1,g_Splines[i].points[j+1].y);
				cpMatrix->setCell(1,2,g_Splines[i].points[j+1].z);

				cpMatrix->setCell(2,0,g_Splines[i].points[j+2].x);
				cpMatrix->setCell(2,1,g_Splines[i].points[j+2].y);
				cpMatrix->setCell(2,2,g_Splines[i].points[j+2].z);

				cpMatrix->setCell(3,0,g_Splines[i].points[j+3].x);
				cpMatrix->setCell(3,1,g_Splines[i].points[j+3].y);
				cpMatrix->setCell(3,2,g_Splines[i].points[j+3].z);

				//Basis * CPMatrix
				matrix* basis_CPMatrix = m_CRBasis->multiply(cpMatrix);

				matrix* posMatrix = uMatrix->multiply(basis_CPMatrix);

				matrix* tanMatrix = duMatrix->multiply(basis_CPMatrix);

				//Set position in spline points
				spline_Points[splinePointsSize].x = (0.5 * posMatrix->getCell(0,0));
				spline_Points[splinePointsSize].y = (0.5 * posMatrix->getCell(0,1));
				spline_Points[splinePointsSize].z = (0.5 * posMatrix->getCell(0,2));

				//Set Tangent vector for given spline point
				vector* tempTan = new vector(0.5 * tanMatrix->getCell(0,0),
															0.5 * tanMatrix->getCell(0,1),
															0.5 * tanMatrix->getCell(0,2));

				tempTan->normalize();

				spline_Tangents[splinePointsSize].setVector(tempTan->getX(),tempTan->getY(),tempTan->getZ());
				spline_Tangents[splinePointsSize].normalize();

				if(splinePointsSize == 0)
				{
					vector* arbitraryStart = new vector(0,1,0);

					//Set Normal for that point
					vector* newNormal = spline_Tangents[splinePointsSize].cross(arbitraryStart);
					newNormal->normalize();
					spline_Normals[splinePointsSize].setVector(newNormal->getX(),newNormal->getY(),newNormal->getZ());

					//Set Binormal for point
					vector* newBinormal = spline_Tangents[splinePointsSize].cross(newNormal);
					newBinormal->normalize();
					spline_Binormals[splinePointsSize].setVector(newBinormal->getX(),newBinormal->getY(),newBinormal->getZ());

					newNormal = spline_Tangents[splinePointsSize].cross(newBinormal);
					newNormal->normalize();
					spline_Normals[splinePointsSize].setVector(newNormal->getX(),newNormal->getY(),newNormal->getZ());
				}
				else //splinePointsSize > 0
				{
					//Set Normal for that point
					vector* newNormal = spline_Binormals[splinePointsSize-1].cross(&spline_Tangents[splinePointsSize]);
					newNormal->normalize();
					spline_Normals[splinePointsSize].setVector(newNormal->getX(),newNormal->getY(),newNormal->getZ());

					//Set Binormal for point
					vector* newBinormal = spline_Tangents[splinePointsSize].cross(newNormal);
					newBinormal->normalize();
					spline_Binormals[splinePointsSize].setVector(newBinormal->getX(),newBinormal->getY(),newBinormal->getZ());

				}

				left_Inner_Top_Track_Points[splinePointsSize].x = spline_Points[splinePointsSize].x + (spline_Normals[splinePointsSize].getX()/10) + (-spline_Binormals[splinePointsSize].getX()/10);
				left_Inner_Top_Track_Points[splinePointsSize].y = spline_Points[splinePointsSize].y + (spline_Normals[splinePointsSize].getY()/10) + (-spline_Binormals[splinePointsSize].getY()/10); 
				left_Inner_Top_Track_Points[splinePointsSize].z = spline_Points[splinePointsSize].z + (spline_Normals[splinePointsSize].getZ()/10) + (-spline_Binormals[splinePointsSize].getZ()/10);

				left_Outer_Top_Track_Points[splinePointsSize].x = spline_Points[splinePointsSize].x + (spline_Normals[splinePointsSize].getX()/9) + (-spline_Binormals[splinePointsSize].getX()/10);
				left_Outer_Top_Track_Points[splinePointsSize].y = spline_Points[splinePointsSize].y + (spline_Normals[splinePointsSize].getY()/9) + (-spline_Binormals[splinePointsSize].getY()/10); 
				left_Outer_Top_Track_Points[splinePointsSize].z = spline_Points[splinePointsSize].z + (spline_Normals[splinePointsSize].getZ()/9) + (-spline_Binormals[splinePointsSize].getZ()/10);

				left_Inner_Bottom_Track_Points[splinePointsSize].x = spline_Points[splinePointsSize].x + (spline_Normals[splinePointsSize].getX()/10) + (-spline_Binormals[splinePointsSize].getX()/9);
				left_Inner_Bottom_Track_Points[splinePointsSize].y = spline_Points[splinePointsSize].y + (spline_Normals[splinePointsSize].getY()/10) + (-spline_Binormals[splinePointsSize].getY()/9); 
				left_Inner_Bottom_Track_Points[splinePointsSize].z = spline_Points[splinePointsSize].z + (spline_Normals[splinePointsSize].getZ()/10) + (-spline_Binormals[splinePointsSize].getZ()/9);

				left_Outer_Bottom_Track_Points[splinePointsSize].x = spline_Points[splinePointsSize].x + (spline_Normals[splinePointsSize].getX()/9) + (-spline_Binormals[splinePointsSize].getX()/9);
				left_Outer_Bottom_Track_Points[splinePointsSize].y = spline_Points[splinePointsSize].y + (spline_Normals[splinePointsSize].getY()/9) + (-spline_Binormals[splinePointsSize].getY()/9); 
				left_Outer_Bottom_Track_Points[splinePointsSize].z = spline_Points[splinePointsSize].z + (spline_Normals[splinePointsSize].getZ()/9) + (-spline_Binormals[splinePointsSize].getZ()/9);

				right_Inner_Top_Track_Points[splinePointsSize].x = spline_Points[splinePointsSize].x + (-spline_Normals[splinePointsSize].getX()/10) + (-spline_Binormals[splinePointsSize].getX()/10);
				right_Inner_Top_Track_Points[splinePointsSize].y = spline_Points[splinePointsSize].y + (-spline_Normals[splinePointsSize].getY()/10) + (-spline_Binormals[splinePointsSize].getY()/10); 
				right_Inner_Top_Track_Points[splinePointsSize].z = spline_Points[splinePointsSize].z + (-spline_Normals[splinePointsSize].getZ()/10) + (-spline_Binormals[splinePointsSize].getZ()/10);

				right_Outer_Top_Track_Points[splinePointsSize].x = spline_Points[splinePointsSize].x + (-spline_Normals[splinePointsSize].getX()/9) + (-spline_Binormals[splinePointsSize].getX()/10);
				right_Outer_Top_Track_Points[splinePointsSize].y = spline_Points[splinePointsSize].y + (-spline_Normals[splinePointsSize].getY()/9) + (-spline_Binormals[splinePointsSize].getY()/10); 
				right_Outer_Top_Track_Points[splinePointsSize].z = spline_Points[splinePointsSize].z + (-spline_Normals[splinePointsSize].getZ()/9) + (-spline_Binormals[splinePointsSize].getZ()/10);

				right_Inner_Bottom_Track_Points[splinePointsSize].x = spline_Points[splinePointsSize].x + (-spline_Normals[splinePointsSize].getX()/10) + (-spline_Binormals[splinePointsSize].getX()/9);
				right_Inner_Bottom_Track_Points[splinePointsSize].y = spline_Points[splinePointsSize].y + (-spline_Normals[splinePointsSize].getY()/10) + (-spline_Binormals[splinePointsSize].getY()/9); 
				right_Inner_Bottom_Track_Points[splinePointsSize].z = spline_Points[splinePointsSize].z + (-spline_Normals[splinePointsSize].getZ()/10) + (-spline_Binormals[splinePointsSize].getZ()/9);

				right_Outer_Bottom_Track_Points[splinePointsSize].x = spline_Points[splinePointsSize].x + (-spline_Normals[splinePointsSize].getX()/9) + (-spline_Binormals[splinePointsSize].getX()/9);
				right_Outer_Bottom_Track_Points[splinePointsSize].y = spline_Points[splinePointsSize].y + (-spline_Normals[splinePointsSize].getY()/9) + (-spline_Binormals[splinePointsSize].getY()/9); 
				right_Outer_Bottom_Track_Points[splinePointsSize].z = spline_Points[splinePointsSize].z + (-spline_Normals[splinePointsSize].getZ()/9) + (-spline_Binormals[splinePointsSize].getZ()/9);

				splinePointsSize++;
			}
		}
	}
}

void drawSkybox(float width, float height, float length)
{
	//Texture found here: http://www.zfight.com/misc/images/textures/envmaps/stormydays_large.jpg

	//Dunno what this does...
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);

	glEnable(GL_TEXTURE_2D);

	//glLoadIdentity();
	glRotatef(270,1,0,0);
	//glColor3f(0.0,0.0,1.0);
	//Draw Square 1
	glBindTexture(GL_TEXTURE_2D, sb1);
	glBegin(GL_QUADS);	
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-(width/2),-(height/2),(length/2));
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-(width/2),(height/2),(length/2));
		glTexCoord2f(0.0f, 1.0f); glVertex3f((width/2),(height/2),(length/2)); 
		glTexCoord2f(0.0f, 0.0f); glVertex3f((width/2),-(height/2),(length/2));
	glEnd();

	//Draw Square 2
	glBindTexture(GL_TEXTURE_2D, sb2);
	glBegin(GL_QUADS);	
		glTexCoord2f(1.0f, 0.0f); glVertex3f((width/2),-(height/2),-(length/2));
		glTexCoord2f(1.0f, 1.0f); glVertex3f((width/2),(height/2),-(length/2)); 
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-(width/2),(height/2),-(length/2));
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-(width/2),-(height/2),-(length/2));
	glEnd();

	//Draw Square 3
	glBindTexture(GL_TEXTURE_2D, sb3);
	glBegin(GL_QUADS);	
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-(width/2),(height/2),-(length/2));	
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-(width/2),(height/2),(length/2)); 
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-(width/2),-(height/2),(length/2));
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-(width/2),-(height/2),-(length/2));		
	glEnd();

	//Draw Square 4
	glBindTexture(GL_TEXTURE_2D, sb4);
	glBegin(GL_QUADS);	
		glTexCoord2f(0.0f, 0.0f); glVertex3f((width/2),-(height/2),-(length/2));
		glTexCoord2f(1.0f, 0.0f); glVertex3f((width/2),-(height/2),(length/2));
		glTexCoord2f(1.0f, 1.0f); glVertex3f((width/2),(height/2),(length/2)); 
		glTexCoord2f(0.0f, 1.0f); glVertex3f((width/2),(height/2),-(length/2));
	glEnd();

	//Draw Square 5
	glBindTexture(GL_TEXTURE_2D, sb5);
	glBegin(GL_QUADS);	
		glTexCoord2f(0.0f, 0.0f); glVertex3f((width/2),(height/2),-(length/2));
		glTexCoord2f(1.0f, 0.0f); glVertex3f((width/2),(height/2),(length/2)); 
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-(width/2),(height/2),(length/2));
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-(width/2),(height/2),-(length/2));
	glEnd();

	//Draw Square 6
	glBindTexture(GL_TEXTURE_2D, sb6);
	glBegin(GL_QUADS);	
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-(width/2),-(height/2),-(length/2));
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-(width/2),-(height/2),(length/2));
		glTexCoord2f(1.0f, 1.0f); glVertex3f((width/2),-(height/2),(length/2)); 
		glTexCoord2f(0.0f, 1.0f); glVertex3f((width/2),-(height/2),-(length/2));
	glEnd();



	glDisable(GL_TEXTURE_2D);
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	glLoadIdentity();

	vector* posAsVector = new vector(spline_Points[currentFrame].x,spline_Points[currentFrame].y,spline_Points[currentFrame].z);
	vector* lookAt = posAsVector->add(&spline_Tangents[currentFrame]);

	gluLookAt(posAsVector->getX(),posAsVector->getY(),posAsVector->getZ(),
		lookAt->getX(),lookAt->getY(),lookAt->getZ(),
		spline_Binormals[currentFrame].getX(),spline_Binormals[currentFrame].getY(),spline_Binormals[currentFrame].getZ());

	glPushMatrix();
	drawSkybox(200.0f,200.0f,200.0f);
	glPopMatrix();

	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	double x,y,z;

	//Left Side

	glBegin(GL_TRIANGLE_STRIP);

	for(int i = 0; i < splinePointsSize-1; i++)
	{
		glColor3f(0.4, 0.4, 0.4);

		//May Need to fix for winding order

		x = left_Inner_Top_Track_Points[i].x;
		y = left_Inner_Top_Track_Points[i].y; 
		z = left_Inner_Top_Track_Points[i].z;
		glVertex3f(x,y,z); //Point 1

		x = left_Inner_Top_Track_Points[i+1].x;
		y = left_Inner_Top_Track_Points[i+1].y; 
		z = left_Inner_Top_Track_Points[i+1].z;
		glVertex3f(x,y,z); //Point 2

		x = left_Outer_Top_Track_Points[i].x;
		y = left_Outer_Top_Track_Points[i].y; 
		z = left_Outer_Top_Track_Points[i].z;
		glVertex3f(x,y,z); //Point 3
		
		x = left_Outer_Top_Track_Points[i+1].x;
		y = left_Outer_Top_Track_Points[i+1].y; 
		z = left_Outer_Top_Track_Points[i+1].z;
		glVertex3f(x,y,z); //Point 4
	}

	glEnd();

	glBegin(GL_TRIANGLE_STRIP);

	for(int i = 0; i < splinePointsSize-1; i++)
	{
		glColor3f(1.0, 1.0, 1.0);

		//May Need to fix for winding order

		x = left_Outer_Top_Track_Points[i].x;
		y = left_Outer_Top_Track_Points[i].y; 
		z = left_Outer_Top_Track_Points[i].z;
		glVertex3f(x,y,z); //Point 1

		x = left_Outer_Top_Track_Points[i+1].x;
		y = left_Outer_Top_Track_Points[i+1].y; 
		z = left_Outer_Top_Track_Points[i+1].z;
		glVertex3f(x,y,z); //Point 2

		x = left_Outer_Bottom_Track_Points[i].x;
		y = left_Outer_Bottom_Track_Points[i].y; 
		z = left_Outer_Bottom_Track_Points[i].z;
		glVertex3f(x,y,z); //Point 3
		
		x = left_Outer_Bottom_Track_Points[i+1].x;
		y = left_Outer_Bottom_Track_Points[i+1].y; 
		z = left_Outer_Bottom_Track_Points[i+1].z;
		glVertex3f(x,y,z); //Point 4
	}

	glEnd();

	glBegin(GL_TRIANGLE_STRIP);

	for(int i = 0; i < splinePointsSize-1; i++)
	{
		glColor3f(0.75, 0.75, 0.75);

		//May Need to fix for winding order

		x = left_Inner_Top_Track_Points[i].x;
		y = left_Inner_Top_Track_Points[i].y; 
		z = left_Inner_Top_Track_Points[i].z;
		glVertex3f(x,y,z); //Point 1
		
		x = left_Inner_Top_Track_Points[i+1].x;
		y = left_Inner_Top_Track_Points[i+1].y; 
		z = left_Inner_Top_Track_Points[i+1].z;
		glVertex3f(x,y,z); //Point 2

		x = left_Inner_Bottom_Track_Points[i].x;
		y = left_Inner_Bottom_Track_Points[i].y; 
		z = left_Inner_Bottom_Track_Points[i].z;
		glVertex3f(x,y,z); //Point 3
		
		x = left_Inner_Bottom_Track_Points[i+1].x;
		y = left_Inner_Bottom_Track_Points[i+1].y; 
		z = left_Inner_Bottom_Track_Points[i+1].z;
		glVertex3f(x,y,z); //Point 4
	}

	glEnd();

	//Right Side
	glBegin(GL_TRIANGLE_STRIP);

	for(int i = 0; i < splinePointsSize-1; i++)
	{
		glColor3f(0.4, 0.4, 0.4);

		//May Need to fix for winding order

		x = right_Inner_Top_Track_Points[i].x;
		y = right_Inner_Top_Track_Points[i].y; 
		z = right_Inner_Top_Track_Points[i].z;
		glVertex3f(x,y,z); //Point 1

		x = right_Inner_Top_Track_Points[i+1].x;
		y = right_Inner_Top_Track_Points[i+1].y; 
		z = right_Inner_Top_Track_Points[i+1].z;
		glVertex3f(x,y,z); //Point 2

		x = right_Outer_Top_Track_Points[i].x;
		y = right_Outer_Top_Track_Points[i].y; 
		z = right_Outer_Top_Track_Points[i].z;
		glVertex3f(x,y,z); //Point 3
		
		x = right_Outer_Top_Track_Points[i+1].x;
		y = right_Outer_Top_Track_Points[i+1].y; 
		z = right_Outer_Top_Track_Points[i+1].z;
		glVertex3f(x,y,z); //Point 4
	}

	glEnd();

	glBegin(GL_TRIANGLE_STRIP);

	for(int i = 0; i < splinePointsSize-1; i++)
	{
		glColor3f(1.0, 1.0, 1.0);

		//May Need to fix for winding order

		x = right_Outer_Top_Track_Points[i].x;
		y = right_Outer_Top_Track_Points[i].y; 
		z = right_Outer_Top_Track_Points[i].z;
		glVertex3f(x,y,z); //Point 1

		x = right_Outer_Top_Track_Points[i+1].x;
		y = right_Outer_Top_Track_Points[i+1].y; 
		z = right_Outer_Top_Track_Points[i+1].z;
		glVertex3f(x,y,z); //Point 2

		x = right_Outer_Bottom_Track_Points[i].x;
		y = right_Outer_Bottom_Track_Points[i].y; 
		z = right_Outer_Bottom_Track_Points[i].z;
		glVertex3f(x,y,z); //Point 3
		
		x = right_Outer_Bottom_Track_Points[i+1].x;
		y = right_Outer_Bottom_Track_Points[i+1].y; 
		z = right_Outer_Bottom_Track_Points[i+1].z;
		glVertex3f(x,y,z); //Point 4
	}

	glEnd();

	glBegin(GL_TRIANGLE_STRIP);

	for(int i = 0; i < splinePointsSize-1; i++)
	{
		glColor3f(0.75, 0.75, 0.75);

		//May Need to fix for winding order

		x = right_Inner_Top_Track_Points[i].x;
		y = right_Inner_Top_Track_Points[i].y; 
		z = right_Inner_Top_Track_Points[i].z;
		glVertex3f(x,y,z); //Point 1

		x = right_Inner_Top_Track_Points[i+1].x;
		y = right_Inner_Top_Track_Points[i+1].y; 
		z = right_Inner_Top_Track_Points[i+1].z;
		glVertex3f(x,y,z); //Point 2

		x = right_Inner_Bottom_Track_Points[i].x;
		y = right_Inner_Bottom_Track_Points[i].y; 
		z = right_Inner_Bottom_Track_Points[i].z;
		glVertex3f(x,y,z); //Point 3
		
		x = right_Inner_Bottom_Track_Points[i+1].x;
		y = right_Inner_Bottom_Track_Points[i+1].y; 
		z = right_Inner_Bottom_Track_Points[i+1].z;
		glVertex3f(x,y,z); //Point 4
	}

	glEnd();

	//Draw Rails
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, wood); 
	for(int i = 0; i < splinePointsSize; i++)
	{
		if((i%50 == 0) && (i+8 < splinePointsSize))
		{

			glBegin(GL_QUADS);
			
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(left_Inner_Top_Track_Points[i].x,left_Inner_Top_Track_Points[i].y,left_Inner_Top_Track_Points[i].z);
			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(right_Inner_Top_Track_Points[i].x,right_Inner_Top_Track_Points[i].y,right_Inner_Top_Track_Points[i].z);
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(right_Inner_Top_Track_Points[i+8].x,right_Inner_Top_Track_Points[i+8].y,right_Inner_Top_Track_Points[i+8].z);
			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(left_Inner_Top_Track_Points[i+8].x,left_Inner_Top_Track_Points[i+8].y,left_Inner_Top_Track_Points[i+8].z);

			glEnd();


			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(left_Inner_Top_Track_Points[i].x,left_Inner_Top_Track_Points[i].y,left_Inner_Top_Track_Points[i].z);
			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(right_Inner_Top_Track_Points[i].x,right_Inner_Top_Track_Points[i].y,right_Inner_Top_Track_Points[i].z);
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(right_Inner_Bottom_Track_Points[i].x,right_Inner_Bottom_Track_Points[i].y,right_Inner_Bottom_Track_Points[i].z);
			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(left_Inner_Bottom_Track_Points[i].x,left_Inner_Bottom_Track_Points[i].y,left_Inner_Bottom_Track_Points[i].z);
			
			glEnd();
		}
	}
	glDisable(GL_TEXTURE_2D);

	//Physics Calculations to read for accelerations
	vector* ground = new vector(-spline_Points[currentFrame].x,0.0f,0.0f);
	ground->normalize();
	float dotProduct = ground->dot(&spline_Tangents[currentFrame]);

	float angle = acos(dotProduct) * (180.0f/3.141592f);

	if(angle > 90)
	{
		acceleration -= 0.1f;
	}
	else
	{
		acceleration += 0.1f;
	}

	if(acceleration < 4.0f)
	{
		acceleration = 4.0f;
	}
	else if(acceleration > 20.0f)
	{
		acceleration = 18.0f;
	}	 
	currentFrame += ceil(acceleration);


	if(currentFrame >= splinePointsSize)
	{
		currentFrame = 0;
		acceleration = 4.0f;
	}

	glutSwapBuffers();

}

void doIdle()
{
	//For Double Buffering
	glutPostRedisplay();
}

void myReshape(int w, int h)
{
	GLfloat aspect = (GLfloat) w / (GLfloat) h;

    /* scale viewport with window */
    glViewport(0, 0, w, h);
    /* prevent distortion */

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0, aspect, 0.01, 500.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void initTextures()
{
	//Read in Skybox Textures
	std::string sbTexture1 = "Skybox_1.jpg";
	std::string sbTexture2 = "Skybox_2.jpg";
	std::string sbTexture3 = "Skybox_3.jpg";
	std::string sbTexture4 = "Skybox_4.jpg";
	std::string sbTexture5 = "Skybox_6.jpg";
	std::string sbTexture6 = "Skybox_5.jpg";
	std::string woodTexture = "Wood.jpg";

	//Use Pic library to Read JPEGs 
	skybox_1 = jpeg_read((char*)sbTexture1.c_str(), NULL);
	skybox_2 = jpeg_read((char*)sbTexture2.c_str(), NULL);
	skybox_3 = jpeg_read((char*)sbTexture3.c_str(), NULL);
	skybox_4 = jpeg_read((char*)sbTexture4.c_str(), NULL);
	skybox_5 = jpeg_read((char*)sbTexture5.c_str(), NULL);
	skybox_6 = jpeg_read((char*)sbTexture6.c_str(), NULL);
	woodBoard = jpeg_read((char*)woodTexture.c_str(), NULL);

	//Skybox 1
	glGenTextures(1, &sb1);
	glBindTexture(GL_TEXTURE_2D, sb1);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,512,512,0,GL_RGB,GL_UNSIGNED_BYTE,skybox_1->pix);


	//Skybox 2
	glGenTextures(1, &sb2);
	glBindTexture(GL_TEXTURE_2D, sb2);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,512,512,0,GL_RGB,GL_UNSIGNED_BYTE,skybox_2->pix);

	//Skybox 3
	glGenTextures(1, &sb3);
	glBindTexture(GL_TEXTURE_2D, sb3);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,512,512,0,GL_RGB,GL_UNSIGNED_BYTE,skybox_3->pix);

	//Skybox 4
	glGenTextures(1, &sb4);
	glBindTexture(GL_TEXTURE_2D, sb4);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,512,512,0,GL_RGB,GL_UNSIGNED_BYTE,skybox_4->pix);

	//Skybox 5
	glGenTextures(1, &sb5);
	glBindTexture(GL_TEXTURE_2D, sb5);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,512,512,0,GL_RGB,GL_UNSIGNED_BYTE,skybox_5->pix);

	//Skybox 6
	glGenTextures(1, &sb6);
	glBindTexture(GL_TEXTURE_2D, sb6);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,512,512,0,GL_RGB,GL_UNSIGNED_BYTE,skybox_6->pix);

	//Wood Texture
	glGenTextures(1, &wood);
	glBindTexture(GL_TEXTURE_2D, wood);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,512,512,0,GL_RGB,GL_UNSIGNED_BYTE,woodBoard->pix);
}

void myinit()
{
	glClearColor(1.0,1.0,1.0,0.0);
	glShadeModel(GL_SMOOTH);
	initTextures();
	spline_Segments = 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	// I've set the argv[1] to track.txt.
	// To change it, on the "Solution Explorer",
	// right click "assign1", choose "Properties",
	// go to "Configuration Properties", click "Debugging",
	// then type your track file name for the "Command Arguments"
	if (argc<2)
	{  
		printf ("usage: %s <trackfile>\n", argv[0]);
		exit(0);
	}

	loadSplines(argv[1]);

	setBasis();
	createPoints();

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
	glutInitWindowSize(640, 480);

	glutCreateWindow("JChu Spline Coaster");

	/* do initialization */
	myinit();

	glutReshapeFunc(myReshape);

	glutDisplayFunc(display);
	glutIdleFunc(doIdle);

	glEnable(GL_DEPTH_TEST);

	glutMainLoop();

	return 0;
}