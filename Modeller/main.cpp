/*
Simple Modelling System
Nico Stepan 
Joel Straatman 

The additional features added in the program are three textures.
These three textures are accessible via '6', '7', and '8' keys.

For a full list of commands, please run the program with 'make'.
*/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <math.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>
#endif

using namespace std;

// textures --- adding three for the extra feature portion
GLubyte *grass_tex, *pond_tex, *stone_tex;
GLuint textures[4]; // change this number to add more textures
int width, height, maximum;

// 2 lights (one at other end of terrain)
float light0[] = {50, -1000, 100, 1};
float light1[] = {-50, -1000, -100, 1};

// add the materials stuff here
// materials courtesy of
// http://devernay.free.fr/cours/opengl/materials.html
float m_amb[] = {0.25, 0.25, 0.25, 1.0};
float m_dif[] = {0.4, 0.4, 0.4, 1.0};
float m_spec[] = {0.774597, 0.774597, 0.774597, 1.0};
float shiny = 0.6;

float m_amb1[] = {0.1745, 0.01175, 0.01175, 1.0};
float m_dif1[] = {0.61424, 0.04136, 0.04136, 1.0};
float m_spec1[] = {0.727811, 0.626959, 0.626959, 1.0};
float shiny1 = 20;

//emerald
float m_amb2[] = {0.0215, 0.1745, 0.0215};
float m_dif2[] = {0.07568, 0.61424, 0.07568};
float m_spec2[] = {0.633, 0.727811, 0.633};
float shiny2 = 0.6;

//obsidian
float m_amb3[] = {0.05375, 0.05, 0.06625};
float m_dif3[] = {0.18275, 0.17, 0.22525};
float m_spec3[] = {0.332741, 0.328634, 0.346435};
float shiny3 = 0.3;

//gold
float m_amb4[] = {0.24725, 0.1995, 0.0745};
float m_dif4[] = {0.75164, 0.60648, 0.22648};
float m_spec4[] = {0.628281, 0.555802, 0.366065};
float shiny4 = 0.4;

class SceneObject
{
  public:
	//for bounding box
	double min_x, max_x, min_y, max_y, min_z, max_z;
	//for transformation
	float tx, ty, tz, rx, ry, rz, sx, sy, sz;

	//for object:
	//0=cube, 1=sphere, 2=cone, 3=torus, 4=dodecahedron
	//n = insert sphere, //b = insert cube, //m = insert cone, //, = insert torus, //. = insert teapot
	int material, object, texture;
	bool wirebox;
	bool textureOn;

	//Constructor
	SceneObject(int inputMaterial, int inputObject, int inputTexture)
	{
		/* 
		the selected object will be able to move and thus will have a wireframe
		box around it (not necessarily the exact dimensions of the actual hit box, but it scales 
		like so). use ray picking to determine which box triggers these
		*/
		wirebox = true;
		textureOn = true;

		tx = ty = tz = rx = ry = rz = 0.0;
		sx = sy = sz = 1.0;
		material = inputMaterial;
		object = inputObject;
		texture = inputTexture;

		switch (object)
		{
		case 0:
		case 1:
			min_x = min_y = min_z = -0.5;
			max_x = max_y = max_z = 0.5;
			break;
		case 2:
			min_x = min_y = -0.1;
			min_z = 0;
			max_x = max_y = 0.1;
			max_z = 1;
			break;
		//glut icosahedron has radius 1.0
		case 3:
			min_x = min_y = min_z = -0.25;
			max_x = max_y = max_z = 0.25;
			break;
		//glut dodecahedron has radius sqrt(3)
		case 4:
			min_x = min_y = min_z = -0.25;
			max_x = max_y = max_z = 0.25;
			break;
		}

		switch (texture)
		{
		//
		case 0:
			texture = textures[0];
			break;
		case 1:
			texture = textures[1];
			break;
		case 2:
			texture = textures[2];
			break;
		}
	}
};

vector<SceneObject> SceneObjects;
int selectedObject;

//Globals
float camPos[] = {3.42f, 3.42f, 3.42f}; //where the camera is

void turnOffWirebox()
{
	for (int i = 0; i < SceneObjects.size(); i++)
	{
		SceneObjects[i].wirebox = false;
	}
}

//OpenGL functions
void keyboard(unsigned char key, int xIn, int yIn)
{
	int mod = glutGetModifiers();
	switch (key)
	{
	//////////////////
	//CREATE OBJECTS//
	//////////////////

	//b = insert cube
	case 'b':
		turnOffWirebox();
		SceneObjects.push_back(SceneObject(0, 0, 0));
		selectedObject = SceneObjects.size();
		break;

	//n = insert sphere
	case 'n':
		turnOffWirebox();
		SceneObjects.push_back(SceneObject(0, 1, 0));
		selectedObject = SceneObjects.size();
		SceneObjects[selectedObject - 1].rx += 60;
		break;

	//m = insert cone
	case 'm':
		turnOffWirebox();
		SceneObjects.push_back(SceneObject(0, 2, 0));
		selectedObject = SceneObjects.size();
		SceneObjects[selectedObject - 1].ry += 89;
		break;

	//, = insert torus
	case ',':
		turnOffWirebox();
		SceneObjects.push_back(SceneObject(0, 3, 0));
		selectedObject = SceneObjects.size();
		break;

	//. = insert teapot
	case '.':
		turnOffWirebox();
		SceneObjects.push_back(SceneObject(0, 4, 0));
		selectedObject = SceneObjects.size();
		break;

		//////////////////////
		//MANIPULATE OBJECTS//
		//////////////////////

	case 'Q':
		exit(0);
		break;
	//(alt + )q = translate x
	case 'q':
		if (mod == (GLUT_ACTIVE_ALT))
		{
			SceneObjects[selectedObject - 1].tx -= 0.1;
			SceneObjects[selectedObject - 1].min_x -= 0.1;
			SceneObjects[selectedObject - 1].max_x -= 0.1;
		}
		else
		{
			SceneObjects[selectedObject - 1].tx += 0.1;
			SceneObjects[selectedObject - 1].min_x += 0.1;
			SceneObjects[selectedObject - 1].max_x += 0.1;
		}
		break;

	//(alt + )w = translate y
	case 'w':
		if (mod == (GLUT_ACTIVE_ALT))
		{
			SceneObjects[selectedObject - 1].ty -= 0.1;
			SceneObjects[selectedObject - 1].min_y -= 0.1;
			SceneObjects[selectedObject - 1].max_y -= 0.1;
		}
		else
		{
			SceneObjects[selectedObject - 1].ty += 0.1;
			SceneObjects[selectedObject - 1].min_y += 0.1;
			SceneObjects[selectedObject - 1].max_y += 0.1;
		}
		break;

	//(alt + )e = translate z
	case 'e':
		if (mod == (GLUT_ACTIVE_ALT))
		{
			SceneObjects[selectedObject - 1].tz -= 0.1;
			SceneObjects[selectedObject - 1].min_z -= 0.1;
			SceneObjects[selectedObject - 1].max_z -= 0.1;
		}
		else
		{
			SceneObjects[selectedObject - 1].tz += 0.1;
			SceneObjects[selectedObject - 1].min_z += 0.1;
			SceneObjects[selectedObject - 1].max_z += 0.1;
		}
		break;

	//(alt + )r = rotate x
	case 'r':
		if (mod == (GLUT_ACTIVE_ALT))
		{
			SceneObjects[selectedObject - 1].rx -= 1;
		}
		else
		{
			SceneObjects[selectedObject - 1].rx += 1;
		}
		break;

	//(alt + )t = rotate y
	case 't':
		if (mod == (GLUT_ACTIVE_ALT))
		{
			SceneObjects[selectedObject - 1].ry -= 1;
		}
		else
		{
			SceneObjects[selectedObject - 1].ry += 1;
		}
		break;

	//(alt + )y = rotate z
	case 'y':
		if (mod == (GLUT_ACTIVE_ALT))
		{
			SceneObjects[selectedObject - 1].rz -= 1;
		}
		else
		{
			SceneObjects[selectedObject - 1].rz += 1;
		}
		break;

	//(alt + )u = scale x
	case 'u':
		if (mod == (GLUT_ACTIVE_ALT))
		{
			SceneObjects[selectedObject - 1].sx -= 0.1;
			SceneObjects[selectedObject - 1].min_x =
				SceneObjects[selectedObject - 1].min_x * SceneObjects[selectedObject - 1].sx / (SceneObjects[selectedObject - 1].sx + 0.1);
			SceneObjects[selectedObject - 1].max_x =
				SceneObjects[selectedObject - 1].max_x * SceneObjects[selectedObject - 1].sx / (SceneObjects[selectedObject - 1].sx + 0.1);
		}
		else
		{
			SceneObjects[selectedObject - 1].sx += 0.1;
			SceneObjects[selectedObject - 1].min_x =
				SceneObjects[selectedObject - 1].min_x * SceneObjects[selectedObject - 1].sx / (SceneObjects[selectedObject - 1].sx - 0.1);
			SceneObjects[selectedObject - 1].max_x =
				SceneObjects[selectedObject - 1].max_x * SceneObjects[selectedObject - 1].sx / (SceneObjects[selectedObject - 1].sx - 0.1);
		}
		break;

	//(alt + )i = scale y
	case 'i':
		if (mod == (GLUT_ACTIVE_ALT))
		{
			SceneObjects[selectedObject - 1].sy -= 0.1;
			SceneObjects[selectedObject - 1].min_y =
				SceneObjects[selectedObject - 1].min_y * SceneObjects[selectedObject - 1].sy / (SceneObjects[selectedObject - 1].sy + 0.1);
			SceneObjects[selectedObject - 1].max_y =
				SceneObjects[selectedObject - 1].max_y * SceneObjects[selectedObject - 1].sy / (SceneObjects[selectedObject - 1].sy + 0.1);
		}
		else
		{
			SceneObjects[selectedObject - 1].sy += 0.1;
			SceneObjects[selectedObject - 1].min_y =
				SceneObjects[selectedObject - 1].min_y * SceneObjects[selectedObject - 1].sy / (SceneObjects[selectedObject - 1].sy - 0.1);
			SceneObjects[selectedObject - 1].max_y =
				SceneObjects[selectedObject - 1].max_y * SceneObjects[selectedObject - 1].sy / (SceneObjects[selectedObject - 1].sy - 0.1);
		}
		break;

	//(alt + )o = scale z
	case 'o':
		if (mod == (GLUT_ACTIVE_ALT))
		{
			SceneObjects[selectedObject - 1].sz -= 0.1;
			SceneObjects[selectedObject - 1].min_z =
				SceneObjects[selectedObject - 1].min_z * SceneObjects[selectedObject - 1].sz / (SceneObjects[selectedObject - 1].sz + 0.1);
			SceneObjects[selectedObject - 1].max_z =
				SceneObjects[selectedObject - 1].max_z * SceneObjects[selectedObject - 1].sz / (SceneObjects[selectedObject - 1].sz + 0.1);
		}
		else
		{
			SceneObjects[selectedObject - 1].sz += 0.1;
			SceneObjects[selectedObject - 1].min_z =
				SceneObjects[selectedObject - 1].min_z * SceneObjects[selectedObject - 1].sz / (SceneObjects[selectedObject - 1].sz - 0.1);
			SceneObjects[selectedObject - 1].max_z =
				SceneObjects[selectedObject - 1].max_z * SceneObjects[selectedObject - 1].sz / (SceneObjects[selectedObject - 1].sz - 0.1);
		}
		break;

	//f = reset scene
	case 'f':
		SceneObjects.clear();
		break;

	// material stuff
	case '1':
		SceneObjects[selectedObject - 1].material = 0;
		//printf("changed material\n");
		break;

	case '2':
		SceneObjects[selectedObject - 1].material = 1;
		break;

	case '3':
		SceneObjects[selectedObject - 1].material = 2;
		break;

	case '4':
		SceneObjects[selectedObject - 1].material = 3;
		break;

	case '5':
		SceneObjects[selectedObject - 1].material = 4;
		break;

	// texture stuff
	// texture binding is handled in the display function
	case 'd':
		//toggle texture on an object (selected object)
		SceneObjects[selectedObject - 1].textureOn = false;
		break;
	case 'D':
		SceneObjects[selectedObject - 1].textureOn = true;
		//glutPostRedisplay();
		break;

	case '6':
		SceneObjects[selectedObject - 1].texture = textures[0];
		glutPostRedisplay();
		break;
	case '7':
		SceneObjects[selectedObject - 1].texture = textures[1];
		glutPostRedisplay();
		break;
	case '8':
		SceneObjects[selectedObject - 1].texture = textures[2];
		glutPostRedisplay();
		break;

		// increase/decrease lighting by changing the light's position
	case 'g':
		light0[1] += 50;
		light1[1] += 50;
		break;
	case 'G':
		//printf("changing light position\n");
		light0[1] -= 50;
		light1[1] -= 50;
		break;
	case 'h':
		light0[2] += 50;
		light1[2] += 50;
		break;
	case 'H':
		light0[2] -= 50;
		light1[2] -= 50;
		break;

	//l = load file from disc
	case 'l':
	{
		SceneObjects.clear();

		ifstream input("save_file.txt");

		//string split function from https://www.oreilly.com/library/view/c-cookbook/0596007612/ch04s07.html
		for (string line; getline(input, line);)
		{
			vector<string> split;
			string::size_type i = 0;
			string::size_type j = line.find(",");

			while (j != string::npos)
			{
				split.push_back(line.substr(i, j - i));
				i = ++j;
				j = line.find(",", j);

				if (j == string::npos)
					split.push_back(line.substr(i, line.length()));
			}

			//call constructor
			SceneObjects.push_back(SceneObject(stoi(split[15]), stoi(split[16]), '0'));
			SceneObjects[SceneObjects.size() - 1].tx = stof(split[0]);
			SceneObjects[SceneObjects.size() - 1].ty = stof(split[1]);
			SceneObjects[SceneObjects.size() - 1].tz = stof(split[2]);
			SceneObjects[SceneObjects.size() - 1].rx = stof(split[3]);
			SceneObjects[SceneObjects.size() - 1].ry = stof(split[4]);
			SceneObjects[SceneObjects.size() - 1].rz = stof(split[5]);
			SceneObjects[SceneObjects.size() - 1].sx = stof(split[6]);
			SceneObjects[SceneObjects.size() - 1].sy = stof(split[7]);
			SceneObjects[SceneObjects.size() - 1].sz = stof(split[8]);
			SceneObjects[SceneObjects.size() - 1].max_x = stof(split[9]);
			SceneObjects[SceneObjects.size() - 1].max_y = stof(split[10]);
			SceneObjects[SceneObjects.size() - 1].max_z = stof(split[11]);
			SceneObjects[SceneObjects.size() - 1].min_x = stof(split[12]);
			SceneObjects[SceneObjects.size() - 1].min_y = stof(split[13]);
			SceneObjects[SceneObjects.size() - 1].min_z = stof(split[14]);
		}
		turnOffWirebox();
		// selectedObject = SceneObjects.size() - 1;
		// SceneObjects[SceneObjects.size()-1].wirebox = true;
	}
	break;

	//s = save current scene to file
	case 's':
		fstream ofs;

		ofs.open("save_file.txt", ios::out | ios::trunc);

		for (int i = 0; i < SceneObjects.size(); i++)
		{
			ofs << to_string(SceneObjects[i].tx) + "," + to_string(SceneObjects[i].ty) + "," + to_string(SceneObjects[i].tz) + "," +
					   to_string(SceneObjects[i].rx) + "," + to_string(SceneObjects[i].ry) + "," + to_string(SceneObjects[i].rz) + "," +
					   to_string(SceneObjects[i].sx) + "," + to_string(SceneObjects[i].sy) + "," + to_string(SceneObjects[i].sz) + "," +
					   to_string(SceneObjects[i].max_x) + "," + to_string(SceneObjects[i].max_y) + "," + to_string(SceneObjects[i].max_z) + "," +
					   to_string(SceneObjects[i].min_x) + "," + to_string(SceneObjects[i].min_y) + "," + to_string(SceneObjects[i].min_z) + "," +
					   to_string(SceneObjects[i].material) + "," + to_string(SceneObjects[i].object) + "\n";
		}

		ofs.close();
		break;
	}
	// update light position if changed
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLightfv(GL_LIGHT0, GL_POSITION, light0);
	glLightfv(GL_LIGHT1, GL_POSITION, light1);
	glPopMatrix();
	glutPostRedisplay();
}

/* LoadPPM -- loads the specified ppm file, and returns the image data as a GLubyte 
 *  (unsigned byte) array. Also returns the width and height of the image, and the
 *  maximum colour value by way of arguments
 */
GLubyte *LoadPPM(char *file, int *width, int *height, int *max)
{
	GLubyte *img;
	FILE *fd;
	int n, m;
	int k, nm;
	char c;
	int i;
	char b[100];
	float s;
	int red, green, blue;

	fd = fopen(file, "r");
	fscanf(fd, "%[^\n] ", b);
	if (b[0] != 'P' || b[1] != '3')
	{
		exit(0);
	}
	fscanf(fd, "%c", &c);
	while (c == '#')
	{
		fscanf(fd, "%[^\n] ", b);
		fscanf(fd, "%c", &c);
	}
	ungetc(c, fd);
	fscanf(fd, "%d %d %d", &n, &m, &k);
	nm = n * m;
	img = (GLubyte *)malloc(3 * sizeof(GLuint) * nm);
	s = 255.0 / k;
	for (i = 0; i < nm; i++)
	{
		fscanf(fd, "%d %d %d", &red, &green, &blue);
		img[3 * nm - 3 * i - 3] = red * s;
		img[3 * nm - 3 * i - 2] = green * s;
		img[3 * nm - 3 * i - 1] = blue * s;
	}
	*width = n;
	*height = m;
	*max = k;
	return img;
}

void cubeTexture(int id, int tex)
{
	SceneObjects[id].texture = tex;
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, textures[tex]);
	glPopMatrix();
	glutPostRedisplay();
}

void drawCube(int i)
{
	if (SceneObjects[i].texture)
	{

		glBindTexture(GL_TEXTURE_2D, textures[SceneObjects[i].texture]);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glBegin(GL_QUADS);
		// front
		glNormal3f(-1, 1, 1);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(-1, -1, 1);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(1, -1, 1);
		glNormal3f(1, -1, 1);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(1, 1, 1);
		glNormal3f(1, 1, 1);
		glTexCoord2f(1.0, 0.0);
		glVertex3f(-1, 1, 1);
		glNormal3f(-1, 1, 1);

		//top
		glNormal3f(-1, 1, 1);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(-1, 1, 1);
		glNormal3f(-1, 1, 1);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(1, 1, 1);
		glNormal3f(1, 1, 1);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(1, 1, -1);
		glNormal3f(-1, 1, -1);
		glTexCoord2f(1.0, 0.0);
		glVertex3f(-1, 1, -1);

		//bottom
		glNormal3f(-1, 1, 1);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(-1, -1, 1);
		glNormal3f(-1, 1, 1);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(1, -1, 1);
		glNormal3f(1, 1, 1);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(1, -1, -1);
		glNormal3f(-1, 1, -1);
		glTexCoord2f(1.0, 0.0);
		glVertex3f(-1, -1, -1);

		//left side
		glNormal3f(-1, 1, 1);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(-1, 1, 1);
		glNormal3f(-1, 1, 1);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(-1, -1, 1);
		glNormal3f(1, 1, 1);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(-1, -1, -1);
		glNormal3f(-1, 1, -1);
		glTexCoord2f(1.0, 0.0);
		glVertex3f(-1, 1, -1);

		//right side
		glNormal3f(-1, 1, 1);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(1, 1, 1);
		glNormal3f(-1, 1, 1);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(1, -1, 1);
		glNormal3f(1, 1, 1);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(1, -1, -1);
		glNormal3f(-1, 1, -1);
		glTexCoord2f(1.0, 0.0);
		glVertex3f(1, 1, -1);

		//back side
		glNormal3f(-1, 1, 1);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(-1, 1, -1);
		glNormal3f(-1, 1, 1);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(-1, -1, -1);
		glNormal3f(1, 1, 1);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(1, -1, -1);
		glNormal3f(-1, 1, -1);
		glTexCoord2f(1.0, 0.0);
		glVertex3f(1, 1, -1);

		glEnd();
	}
}

void sphereTexture(int id, int tex)
{
	SceneObjects[id].texture = tex;
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, textures[tex]);
	glPopMatrix();
	glutPostRedisplay();
}

void teapotTexture(int id, int tex)
{
	SceneObjects[id].texture = tex;
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, textures[tex]);
	glPopMatrix();
	glutPostRedisplay();
}

void drawCone(int id)
{
	float radius = 1;
	float r = 1;
	float h = 1;

	if (SceneObjects[id].texture)
	{

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, textures[SceneObjects[id].texture]);
		float t, s;
		float i = 0.05;

		for (s = 0.0; s < 1.0; s += i)
		{
			for (t = 0.0; t <= 1.0; t += i)
			{
				float r = ((h - t) / h) * radius;
				glBegin(GL_TRIANGLE_FAN);
				glTexCoord2f(s, t);
				glVertex3f(r * cos(2 * 3.1459 * s), t, r * sin(2 * 3.1459 * s));
				glTexCoord2f(s + i, t);
				glVertex3f(r * cos(2 * 3.1459 * (s + i)), t, r * sin(2 * 3.1459 * (s + i)));
				glTexCoord2f(s + i, t + i);
				glVertex3f(r * cos(2 * 3.1459 * (s + i)), (t + i), r * sin(2 * 3.1459 * (s + i)));
				glTexCoord2f(s, t + i);
				glVertex3f(r * cos(2 * 3.1459 * s), (t + i), r * sin(2 * 3.1459 * s));
			}
			glEnd();
		}
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
		float t, s;
		float i = 0.05;

		for (s = 0.0; s < 1.0; s += i)
		{
			for (t = 0.0; t <= 1.0; t += i)
			{
				float r = ((h - t) / h) * radius;
				glBegin(GL_TRIANGLE_FAN);
				//glTexCoord2f(s, t);
				glVertex3f(r * cos(2 * 3.1459 * s), t, r * sin(2 * 3.1459 * s));
				//glTexCoord2f(s + i, t);
				glVertex3f(r * cos(2 * 3.1459 * (s + i)), t, r * sin(2 * 3.1459 * (s + i)));
				//glTexCoord2f(s + i, t + i);
				glVertex3f(r * cos(2 * 3.1459 * (s + i)), (t + i), r * sin(2 * 3.1459 * (s + i)));
				//glTexCoord2f(s, t + i);
				glVertex3f(r * cos(2 * 3.1459 * s), (t + i), r * sin(2 * 3.1459 * s));
			}
			glEnd();
		}
	}
}

void coneTexture(int id, int tex)
{
	SceneObjects[id].texture = tex;
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, textures[tex]);
	glPopMatrix();
	glutPostRedisplay();
}

void torusTexture(int id, int tex)
{

	SceneObjects[id].texture = tex;
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, textures[tex]);
	glPopMatrix();
	glutPostRedisplay();
}

// sets the texture for the given shape
void setTexture(int o, int t, int objectIndex)
{
	//0=cube, 1=sphere, 2=cone, 3=torus, 4=teapot
	switch (o)
	{
	case 0: // cube
		cubeTexture(objectIndex, t);
		break;

	case 1: // sphere
		sphereTexture(objectIndex, t);
		break;

	case 2: // cone
		coneTexture(objectIndex, t);
		break;

	case 3: // torus
		torusTexture(objectIndex, t);
		break;

	case 4: // teapot
		teapotTexture(objectIndex, t);
		break;

		// break;
	}
}

void init(void)
{
	glClearColor(66.0 / 256.0, 88.0 / 256.0, 96.0 / 256.0, 0);
	//glColor3f(1, 1, 1);

	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glLightfv(GL_LIGHT0, GL_POSITION, light0);
	glLightfv(GL_LIGHT1, GL_POSITION, light1);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, 1, 1, 100);

	// lighting stuff will go here

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	/*
	add additional lighting settings here
	...
	...

	*/

	/* Texture */
	// load textures --> 3 for purpose of the extra features
	// enable texturing
	glEnable(GL_TEXTURE_2D);
	// generate 3 texture IDs, store them in array "textures"
	glGenTextures(3, textures);
	// load the texture (grass)
	grass_tex = LoadPPM("grass.ppm", &width, &height, &maximum);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, grass_tex);
	// load the texture (pond)
	pond_tex = LoadPPM("pond.ppm", &width, &height, &maximum);
	glBindTexture(GL_TEXTURE_2D, textures[3]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pond_tex);
	// load the texture (stone)
	stone_tex = LoadPPM("stone.ppm", &width, &height, &maximum);
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, stone_tex);
}

void getMaterial(int i)
{
	if (i = 0)
	{
		//glPushMatrix();
		glEnable(GL_COLOR_MATERIAL);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_amb);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_dif);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_spec);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny);
		glColorMaterial(GL_AMBIENT, GL_DIFFUSE);
		///glPopMatrix();
	}
	else if (i = 1)
	{
		//glPushMatrix();
		glEnable(GL_COLOR_MATERIAL);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_amb1);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_dif1);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_spec1);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny1);
		glColorMaterial(GL_AMBIENT, GL_DIFFUSE);
		//glPopMatrix();
	}
	else if (i = 2)
	{
		//glPushMatrix();
		glEnable(GL_COLOR_MATERIAL);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_amb2);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_dif2);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_spec2);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny2);
		glColorMaterial(GL_AMBIENT, GL_DIFFUSE);
		//glPopMatrix();
	}

	else if (i = 3)
	{
		//glPushMatrix();
		glEnable(GL_COLOR_MATERIAL);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_amb3);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_dif3);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_spec3);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny3);
		glColorMaterial(GL_AMBIENT, GL_DIFFUSE);
		//glPopMatrix();
	}

	else if (i = 4)
	{
		//glPushMatrix();
		glEnable(GL_COLOR_MATERIAL);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_amb4);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_dif4);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_spec4);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny4);
		glColorMaterial(GL_AMBIENT, GL_DIFFUSE);
		//glPopMatrix();
	}
}

/* display function - GLUT display callback function
 * clears the screen, sets the camera position, draws the ground plane and movable box
 */
void display(void)
{
	glMatrixMode(GL_MODELVIEW);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glCullFace(GL_BACK);
	glLoadIdentity();

	// glPushMatrix();
	// glColor3f(1,1,1);
	// glTranslatef(0, -1.5, 0);
	// glScalef(50, 0.5, 50);
	// glutSolidCube(1);
	// glPopMatrix();

	gluLookAt(camPos[0], camPos[1], camPos[2], 0, 0, 0, 0, 1, 0);

	//load in each created object
	for (int i = 0; i < SceneObjects.size(); i++)
	{
		glPushMatrix();

		//load in transformations specific to each object
		glTranslatef(SceneObjects[i].tx, SceneObjects[i].ty, SceneObjects[i].tz);
		glRotatef(SceneObjects[i].rx, 1, 0, 0);
		glRotatef(SceneObjects[i].ry, 0, 1, 0);
		glRotatef(SceneObjects[i].rz, 0, 0, 1);
		glScalef(SceneObjects[i].sx, SceneObjects[i].sy, SceneObjects[i].sz);

		//set material
		//setMaterial(SceneObject[i].material);
		switch (SceneObjects[i].material)
		{
		case 0:
			glPushMatrix();
			glEnable(GL_COLOR_MATERIAL);
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_amb);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_dif);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_spec);
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny);
			glColorMaterial(GL_AMBIENT, GL_DIFFUSE);
			glPopMatrix();
			break;
		case 1:
			glPushMatrix();
			glEnable(GL_COLOR_MATERIAL);
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_amb1);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_dif1);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_spec1);
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny1);
			glColorMaterial(GL_AMBIENT, GL_DIFFUSE);
			glPopMatrix();
			break;
		case 2:
			glPushMatrix();
			glEnable(GL_COLOR_MATERIAL);
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_amb2);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_dif2);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_spec2);
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny2);
			glColorMaterial(GL_AMBIENT, GL_DIFFUSE);
			glPopMatrix();
			break;
		case 3:
			glPushMatrix();
			glEnable(GL_COLOR_MATERIAL);
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_amb3);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_dif3);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_spec3);
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny3);
			glColorMaterial(GL_AMBIENT, GL_DIFFUSE);
			glPopMatrix();
			break;
		case 4:
			glPushMatrix();
			glEnable(GL_COLOR_MATERIAL);
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_amb4);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_dif4);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_spec4);
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny4);
			glColorMaterial(GL_AMBIENT, GL_DIFFUSE);
			glPopMatrix();
			break;
		}

		//set texture
		if (SceneObjects.size() != 0)
		{
			setTexture(SceneObjects[i].object, SceneObjects[i].texture, i);
		}

		//create object
		switch (SceneObjects[i].object)
		{

		// cube, (with wirebox and texture) type b for cube
		case 0:
			if (SceneObjects[i].wirebox)
			{
				if (SceneObjects[i].textureOn)
				{
					getMaterial(SceneObjects[i].material);
					glutWireCube(2.5);
					// glBindTexture(GL_TEXTURE_2D, textures[SceneObjects[i].texture]);
					glEnable(GL_TEXTURE_2D);
					drawCube(i);
					//glutSolidCube(0.5);
				}
				else
				{
					getMaterial(SceneObjects[i].material);
					glutWireCube(2.5);
					glDisable(GL_TEXTURE_2D);
					drawCube(i);
					//glutSolidCube(0.5);
				}
			}
			else
			{

				if (SceneObjects[i].textureOn)
				{
					getMaterial(SceneObjects[i].material);
					//glutSolidCube(0.5);
					glEnable(GL_TEXTURE_2D);
					drawCube(i);
				}
				else
				{
					getMaterial(SceneObjects[i].material);
					glDisable(GL_TEXTURE_2D);
					drawCube(i);
					//glutSolidCube(1);
				}
			}
			break;

		// sphere (with wirebox and texture) type n for sphere
		// need to add material next (SceneObjects[i].material)
		case 1:
			if (SceneObjects[i].wirebox)
			{
				if (SceneObjects[i].textureOn)
				{

					glutWireCube(1);
					GLUquadricObj *sphere = NULL;
					sphere = gluNewQuadric();
					gluQuadricDrawStyle(sphere, GLU_FILL);
					gluQuadricTexture(sphere, true);
					gluQuadricNormals(sphere, GLU_SMOOTH);

					glBindTexture(GL_TEXTURE_2D, textures[SceneObjects[i].texture]);
					getMaterial(SceneObjects[i].material);
					gluSphere(sphere, 0.5, 20, 20);
					//glColor3f(0, 1, 0);

					//glutSolidSphere(0.5, 10, 10);
				}
				else
				{
					glutWireCube(1);
					GLUquadricObj *sphere = NULL;
					sphere = gluNewQuadric();
					gluQuadricDrawStyle(sphere, GLU_FILL);
					gluQuadricTexture(sphere, true);
					gluQuadricNormals(sphere, GLU_SMOOTH);

					glDisable(GL_TEXTURE_2D);
					SceneObjects[i].textureOn = false;
					getMaterial(SceneObjects[i].material);
					gluSphere(sphere, 0.5, 20, 20);
				}
			}
			else
			{
				if (SceneObjects[i].textureOn)
				{
					GLUquadricObj *sphere = NULL;
					sphere = gluNewQuadric();
					gluQuadricDrawStyle(sphere, GLU_FILL);
					gluQuadricTexture(sphere, true);
					gluQuadricNormals(sphere, GLU_SMOOTH);

					glBindTexture(GL_TEXTURE_2D, textures[SceneObjects[i].texture]);
					//glutSolidTeapot(1);
					getMaterial(SceneObjects[i].material);
					gluSphere(sphere, 0.5, 20, 20);
					//glColor3f(0, 1, 0);

					//glutSolidSphere(0.5, 10, 10);
				}
				else
				{
					GLUquadricObj *sphere = NULL;
					sphere = gluNewQuadric();
					gluQuadricDrawStyle(sphere, GLU_FILL);
					gluQuadricTexture(sphere, true);
					gluQuadricNormals(sphere, GLU_SMOOTH);
					glDisable(GL_TEXTURE_2D);
					getMaterial(SceneObjects[i].material);
					gluSphere(sphere, 0.5, 20, 20);
				}
			}
			break;

		case 2:
			if (SceneObjects[i].wirebox)
			{
				if (SceneObjects[i].textureOn)
				{
					getMaterial(SceneObjects[i].material);
					drawCone(i);
					glutWireCube(2);
				}
				else
				{
					getMaterial(SceneObjects[i].material);
					drawCone(i);
					glutWireCube(2);
				}

				//glutSolidCone(0.5, 1, 10, 10);
			}
			else
			{
				if (SceneObjects[i].textureOn)
				{
					getMaterial(SceneObjects[i].material);
					drawCone(i);
				}
				else
				{
					getMaterial(SceneObjects[i].material);
					drawCone(i);
					// glutSolidCone(0.5, 1, 10, 10);
				}
			}
			break;
		case 3:
			if (SceneObjects[i].wirebox)
			{
				if (SceneObjects[i].textureOn)
				{
					glutWireCube(2);
					glEnable(GL_TEXTURE_GEN_S); //enable texture coordinate generation
					glEnable(GL_TEXTURE_GEN_T);
					glBindTexture(GL_TEXTURE_2D, textures[SceneObjects[i].texture]);
					// glutSolidIcosahedron();
					getMaterial(SceneObjects[i].material);
					glutSolidTorus(0.1, 0.5, 50, 50);
					glDisable(GL_TEXTURE_GEN_S); //enable texture coordinate generation
					glDisable(GL_TEXTURE_GEN_T);
				}
				else
				{
					glutWireCube(2);
					// glutSolidIcosahedron();
					// glEnable(GL_TEXTURE_GEN_S); //enable texture coordinate generation
					// glEnable(GL_TEXTURE_GEN_T);
					//lBindTexture(GL_TEXTURE_2D, textures[SceneObjects[i].texture]);
					// glutSolidIcosahedron();
					glDisable(GL_TEXTURE_2D);
					getMaterial(SceneObjects[i].material);
					glutSolidTorus(0.1, 0.5, 50, 50);
					glDisable(GL_TEXTURE_GEN_S); //enable texture coordinate generation
					glDisable(GL_TEXTURE_GEN_T);
				}
			}
			else
			{
				if (SceneObjects[i].textureOn)
				{

					// glutSolidIcosahedron();
					glEnable(GL_TEXTURE_GEN_S); //enable texture coordinate generation
					glEnable(GL_TEXTURE_GEN_T);
					glBindTexture(GL_TEXTURE_2D, textures[SceneObjects[i].texture]);
					// glutSolidIcosahedron();
					getMaterial(SceneObjects[i].material);
					glutSolidTorus(0.1, 0.5, 50, 50);
					glDisable(GL_TEXTURE_GEN_S); //enable texture coordinate generation
					glDisable(GL_TEXTURE_GEN_T);
				}
				else{
					glutSolidTorus(0.1, 0.5, 50, 50);
				}
			}
			break;
		case 4:
			if (SceneObjects[i].wirebox)
			{
				if (SceneObjects[i].textureOn)
				{

					glutWireCube(2);
					glEnable(GL_TEXTURE_2D);
					glBindTexture(GL_TEXTURE_2D, textures[SceneObjects[i].texture]);
					getMaterial(SceneObjects[i].material);
					glutSolidTeapot(0.5);
				}
				else
				{
					glutWireCube(2);
					glDisable(GL_TEXTURE_2D);
					//glBindTexture(GL_TEXTURE_2D, textures[SceneObjects[i].texture]);
					getMaterial(SceneObjects[i].material);
					glutSolidTeapot(0.5);
				}
				//glutSolidDodecahedron();
			}
			else
			{
				getMaterial(SceneObjects[i].material);
				glutSolidTeapot(0.5);
			}
			break;
		}

		glPopMatrix();
	}

	glutSwapBuffers();
}

void special(int key, int x, int y)
{
	float radius;
	float theta;
	switch (key)
	{
	case GLUT_KEY_UP:
		radius = sqrt(pow(camPos[0], 2) + pow(camPos[1], 2));
		theta = atan2(camPos[1], camPos[0]);
		camPos[0] = radius * cos(theta + 3.1459 / 180.0);
		camPos[1] = radius * sin(theta + 3.1459 / 180.0);
		break;

	case GLUT_KEY_DOWN:
		radius = sqrt(pow(camPos[0], 2) + pow(camPos[1], 2));
		theta = atan2(camPos[1], camPos[0]);
		camPos[0] = radius * cos(theta - 3.1459 / 180.0);
		camPos[1] = radius * sin(theta - 3.1459 / 180.0);
		break;
	case GLUT_KEY_LEFT:
		radius = sqrt(pow(camPos[1], 2) + pow(camPos[2], 2));
		theta = atan2(camPos[1], camPos[2]);
		camPos[2] = radius * cos(theta - 3.1459 / 180.0);
		camPos[1] = radius * sin(theta - 3.1459 / 180.0);
		break;

	case GLUT_KEY_RIGHT:
		radius = sqrt(pow(camPos[1], 2) + pow(camPos[2], 2));
		theta = atan2(camPos[1], camPos[2]);
		camPos[2] = radius * cos(theta + 3.1459 / 180.0);
		camPos[1] = radius * sin(theta + 3.1459 / 180.0);
		break;
	}
}

void mouse(int btn, int state, int x, int y)
{
	if (state == GLUT_DOWN && SceneObjects.size() != 0)
	{
		// GLint viewport[4];
		// GLdouble modelview[16];
		// GLdouble projection[16];
		int viewport[4];
		double matModelView[16];
		double matProjection[16];
		// GLfloat winX, winY, winZ;
		double winX, winY, winZ;
		GLdouble posX, posY, posZ;
		GLdouble posX_1, posY_1, posZ_1;

		GLdouble Rd[3];

		glGetDoublev(GL_MODELVIEW_MATRIX, matModelView);
		glGetDoublev(GL_PROJECTION_MATRIX, matProjection);
		glGetIntegerv(GL_VIEWPORT, viewport);

		winX = (float)x;
		winY = (float)viewport[3] - (float)y;

		//glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );

		gluUnProject(winX, winY, 1.0, matModelView, matProjection, viewport, &posX_1, &posY_1, &posZ_1);

		//calcualte our ray from R0 and R1
		Rd[0] = posX_1 - posX;
		Rd[1] = posY_1 - posY;
		Rd[2] = posZ_1 - posZ;

		//iterate through all shapes' bounds to determine intersections with line from camera to clicked point
		int clicked_object = -1;
		double temp_object_t = (double)__INT_MAX__;
		for (int i = 0; i < SceneObjects.size(); i++)
		{
			double temp_t;
			double bound1;
			double bound2;

			////min_x plane////
			temp_t = -((double)camPos[1] + SceneObjects[i].min_x) / posX_1;
			bound1 = camPos[1] + temp_t * posY_1;
			bound2 = camPos[2] + temp_t * posZ_1;
			if ((SceneObjects[i].min_y < bound1) && (bound1 < SceneObjects[i].max_y) && //within y bounds
				(SceneObjects[i].min_z < bound2) && (bound2 < SceneObjects[i].max_z) && //within z bounds
				(temp_t < temp_object_t) && (temp_t > 0))
			{
				temp_object_t = temp_t;
				clicked_object = i;
				//printf("intersection in max_x plane of object %i\n", i);
			}

			////max_x plane////
			temp_t = -((double)camPos[0] + SceneObjects[i].max_x) / posX_1;
			bound1 = camPos[1] + temp_t * posY_1;
			bound2 = camPos[2] + temp_t * posZ_1;

			if ((SceneObjects[i].min_y < bound1) && (bound1 < SceneObjects[i].max_y) && //within y bounds
				(SceneObjects[i].min_z < bound2) && (bound2 < SceneObjects[i].max_z) && //within z bounds
				(temp_t < temp_object_t) && (temp_t > 0))
			{
				temp_object_t = temp_t;
				clicked_object = i;
				//printf("intersection in max_x plane of object %i\n", i);
			}

			////min_y plane////
			temp_t = -((double)camPos[1] + SceneObjects[i].min_y) / (double)posY_1;
			bound1 = camPos[0] + temp_t * posX_1;
			bound2 = camPos[2] + temp_t * posZ_1;
			if ((SceneObjects[i].min_x < bound1) && (bound1 < SceneObjects[i].max_x) && //within x bounds
				(SceneObjects[i].min_z < bound2) && bound2 < (SceneObjects[i].max_z) && //within z bounds
				(temp_t < temp_object_t) && (temp_t > 0))
			{
				temp_object_t = temp_t;
				clicked_object = i;
				//printf("intersection in min_y plane of object %i\n", i);
			}

			////max_y plane////
			temp_t = -((double)camPos[2] + SceneObjects[i].max_y) / (double)posY_1;
			bound1 = camPos[0] + temp_t * posX_1;
			bound2 = camPos[2] + temp_t * posZ_1;
			if ((SceneObjects[i].min_x < bound1) && (bound1 < SceneObjects[i].max_x) && //within x bounds
				(SceneObjects[i].min_z < bound2) && (bound2 < SceneObjects[i].max_z) && //within z bounds
				(temp_t < temp_object_t) && (temp_t > 0))
			{
				temp_object_t = temp_t;
				clicked_object = i;
				//printf("intersection in max_y plane of object %i\n", i);
			}

			////min_z plane////
			temp_t = -((double)camPos[2] + SceneObjects[i].min_z) / (double)posZ_1;
			bound1 = camPos[1] + temp_t * posY_1;
			bound2 = camPos[0] + temp_t * posX_1;
			if ((SceneObjects[i].min_y < bound1) && (bound1 < SceneObjects[i].max_y) && //within y bounds
				(SceneObjects[i].min_x < bound2) && (bound2 < SceneObjects[i].max_x) && //within x bounds
				(temp_t < temp_object_t) && (temp_t > 0))
			{
				temp_object_t = temp_t;
				clicked_object = i;
				//printf("intersection in min_z plane of object %i\n", i);
			}

			////max_z plane////
			temp_t = -((double)camPos[2] + SceneObjects[i].max_z) / (double)posZ_1;
			bound1 = camPos[1] + temp_t * posY_1;
			bound2 = camPos[0] + temp_t * posX_1;
			if ((SceneObjects[i].min_y < bound1) && (bound1 < SceneObjects[i].max_y) && //within y bounds
				(SceneObjects[i].min_x < bound2) && (bound2 < SceneObjects[i].max_x) && //within x bounds
				(temp_t < temp_object_t) && (temp_t > 0))
			{
				temp_object_t = temp_t;
				clicked_object = i;
				//printf("intersection in max_z plane of object %i\n", i);
			}

			turnOffWirebox();
			if (clicked_object == -1)
			{
				SceneObjects[i].wirebox = false;
				turnOffWirebox();
			}
		}
		//printf("object %i was clicked\n", clicked_object);

		if (btn == GLUT_LEFT_BUTTON)
		{
			selectedObject = clicked_object + 1;
			turnOffWirebox();
			SceneObjects[clicked_object].wirebox = true;
		}
		else if (btn == GLUT_RIGHT_BUTTON && clicked_object != -1)
		{
			SceneObjects.erase(SceneObjects.begin() + clicked_object);
			//selectedObject = -1;
		}
	}
	else if (btn == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		// deletes object, whatever said object is selected at the moment
		SceneObjects.erase(SceneObjects.begin() + selectedObject - 1);
		selectedObject -= 1;
	}
}

void motion(int x, int y)
{
	//printf("mouseMotion coords: %i,%i\n", x, y);
}

void passive(int x, int y)
{
	//printf("mousePassive coords: %i,%i\n", x, y);
}

void reshape(int w, int h)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluOrtho2D(0, w, 0, h);
	gluPerspective(45, (float)((w + 0.0f) / h), 1, 100);

	glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, w, h);
}

void FPS(int val)
{
	glutPostRedisplay();
	glutTimerFunc(17, FPS, 0); // 1sec = 1000, 60fps = 1000/60 = ~17
}

void callBackInit()
{
	glutDisplayFunc(display); //registers "display" as the display callback function
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(passive);
	glutReshapeFunc(reshape);
	glutTimerFunc(0, FPS, 0);
}

void commands()
{
	cout
		<< "------------------------------------------\n"
		<< "-----------      COMMANDS      -----------\n"
		<< "------------------------------------------\n"
		<< "------------------------------------------\n"
		<< "Shapes\n"
		<< "'b' --> insert cube 					  \n"
		<< "'n' --> insert sphere 					  \n"
		<< "'m' --> insert cone 					  \n"
		<< "',' --> insert torus 					  \n"
		<< "'.' --> insert teapot 					  \n"
		<< "------------------------------------------\n"
		<< "'q' --> translate x 					  \n"
		<< "'ALT + q' --> translate negative x 		  \n"
		<< "'w' --> translate y 					  \n"
		<< "'ALT + w' --> translate negative y 		  \n"
		<< "'e' --> translate z 					  \n"
		<< "'ALT + e' --> translate negative z 		  \n"
		<< "------------------------------------------\n"
		<< "'r' --> rotate x 						  \n"
		<< "'ALT + r' --> rotate negative x 		  \n"
		<< "'t' --> rotate y 						  \n"
		<< "'ALT + t' --> rotate negative y 		  \n"
		<< "'y' --> rotate z 						  \n"
		<< "'ALT + y' --> rotate negative z 		  \n"
		<< "------------------------------------------\n"
		<< "'u' --> scale x 						  \n"
		<< "'ALT + u' --> scale negative x 			  \n"
		<< "'i' --> scale y 						  \n"
		<< "'ALT + i' --> scale negative y 			  \n"
		<< "'o' --> scale z 						  \n"
		<< "'ALT + o' --> scale negative z 			  \n"
		<< "------------------------------------------\n"
		<< "Materials\n"
		<< "'1' --> apply chrome material 			  \n"
		<< "'2' --> apply ruby material 			  \n"
		<< "'3' --> apply emerald material 			  \n"
		<< "'4' --> apply obsidian material 		  \n"
		<< "'5' --> apply gold material 			  \n"
		<< "------------------------------------------\n"
		<< "Textures\n"
		<< "'6' --> apply grass texture				  \n"
		<< "'7' --> apply stone texture				  \n"
		<< "'8' --> apply water texture				  \n"
		<< "'d' --> disable object texture            \n"
		<< "'D' --> enable object texture             \n"
		<< "------------------------------------------\n"
		<< "'g' and 'G' change position of 1st light  \n"
		<< "'h' and 'H' change position of 2nd light  \n"
		<< "'l' --> load scene from save_file.txt	  \n"
		<< "'s' --> save scene to save_file.txt	      \n"
		<< "------------------------------------------\n"
		<< "------------------------------------------\n"
		<< "'f' --> reset scene 					  \n"
		<< "'Q' --> quit program 	 				  \n"
		<< "Left click to select object				  \n"
		<< "Right click to delete object			  \n"
		<< "------------------------------------------\n"
		<< "------------------------------------------\n";
}

/* main function - program entry point */
int main(int argc, char **argv)
{
	glutInit(&argc, argv); //starts up GLUT
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);

	glutInitWindowSize(800, 800);
	glutInitWindowPosition(50, 50);

	glutCreateWindow("Simple Modeller"); //creates the window
	commands();
	callBackInit();

	init();

	glutMainLoop(); //starts the event glutMainLoop
	return (0);		//return may not be necessary on all compilers
}
