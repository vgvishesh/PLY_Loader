/**************************************
* Copyright (c) <2014> <Vishesh Gupta> *
**************************************

The MIT / X Window System License
=================================
Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include "plyloader.h"

using namespace std;
using namespace glm;

PLYModel::PLYModel() {
}

struct data
{
	float _x, _y, _z;
    float _nx, _ny, _nz;
    unsigned char _r, _g, _b, _a;
	data(){
		 _x= _y=_z=_nx=_ny=_nz=_r=_g=_b=_a=0;
	}
};

// To read files not having the normal data.
struct data_1
{
	float _x, _y, _z;
    unsigned char _r, _g, _b, _a;
	data_1(){
		 _x= _y=_z=_r=_g=_b=_a=0;
	}
};

// To read files not having the color data
struct data_2
{
	float _x, _y, _z;
    float _nx, _ny, _nz;
    data_2(){
		 _x= _y=_z=_nx=_ny=_nz=0;
	}
};

// To read files not having the color data and normal data
struct data_3
{
	float _x, _y, _z;
    data_3(){
		 _x= _y=_z=0;
	}
};

struct faceData
{
	int a,b,c;
	faceData()
	{
		a=b=c=0;
	}
};

struct colorData
{
	unsigned char r,g,b,a;
	colorData(){
		r=g=b=a=0;
	}
};

PLYModel::PLYModel(const char* filename, bool isNormal, bool isColor) {
    
	long nvertices;
	int position;
	struct faceData F;
	isMesh=0;

    string line;
    string s1, s2;

    ifstream inputPly;
	inputPly.open(filename,ios::binary);

	if (!inputPly.is_open()) {
        cerr << "Couldn't open " << filename << '\n';
        exit(1);
    }

	ifColor=0;
	ifNormal=0;

    getline(inputPly, line);
	while(line.compare("vertex")!=0)
	{
		getline(inputPly, line,' ');
		cout<<line<<"\n";
	}
	inputPly>>vertexCount; // 3rd line
	cout<<vertexCount;
	
	//getline(inputPly,line);
	//bool visit=0;
	while(line.compare("face")!=0)
	{
		if(line.find("red")==0) ifColor=1;
		if(line.find("nx")==0) ifNormal=1;
		getline(inputPly, line,' ');
		cout<<line<<endl;		
	}

	inputPly>>faceCount;
	cout<<faceCount;
	if(faceCount>0)
		isMesh=1;

	while(line.compare("end_header")!=0)
	{
		getline(inputPly, line);
		cout<<line<<endl;
	}
	

	// This runs if the file has both COLOR and NORMAL information
	if(isNormal && isColor)
	{
		struct data Values;

		if(!ifColor)
		{
			cout<<"\nFile doesn't contain the Color Information, check the constructor call";
			exit(0);
		}
		if(!ifNormal)
		{
			cout<<"\nFile doesn't contain the Normal Information, check the constructor call";
			exit(0);
		}

		inputPly.read((char *)&Values,sizeof(Values));

		//cout<<"\n"<<Values._x <<"\t"<< Values._y <<"\t"<< Values._z <<"\t"<< Values._nx <<"\t"<< Values._ny <<"\t"<< Values._nz <<"\t"<< Values._r <<"\t"<< Values._g <<"\t"<< Values._b <<"\t"<< Values._a;
		min = max = vec3(Values._x, Values._y, Values._z);

		positions.push_back(vec3(Values._x, Values._y, Values._z));
		normals.push_back(vec3(Values._nx, Values._ny, Values._nz));
		colors.push_back(vec3(Values._r, Values._g, Values._b) / 255.0f);

		for (long int i = 1; i < vertexCount; i++) 
		{
			inputPly.read((char *)&Values,sizeof(Values));
			//cout<<"\n"<<Values._x <<"\t"<< Values._y <<"\t"<< Values._z <<"\t"<< Values._nx <<"\t"<< Values._ny <<"\t"<< Values._nz <<"\t"<< (int)Values._r <<"\t"<< (int)Values._g <<"\t"<< (int)Values._b <<"\t"<< (int)Values._a;
        
			if (Values._x < min.x) min.x = Values._x;
			if (Values._y < min.y) min.y = Values._y;
			if (Values._z < min.z) min.z = Values._z;

			if (Values._x > max.x) max.x = Values._x;
			if (Values._y > max.y) max.y = Values._y;
			if (Values._z > max.z) max.z = Values._z;

			positions.push_back(vec3(Values._x, Values._y, Values._z));	// -1 is been multiplied to set the correct orientation of this model....
			normals.push_back(vec3(Values._nx, Values._ny, Values._nz));
			colors.push_back(vec3((int)Values._r, (int)Values._g, (int)Values._b) / 255.0f);
		}
		center = (min + max) / 2.0f;

		//Bounding volume measurements
		bvWidth = max.x - min.x;
		bvHeight = max.y - min.y;
		bvDepth = max.z - min.z;
		bvAspectRatio = bvWidth / bvHeight;
	}

	// Runs when there is COLOR information  but no NORMAL information
	else if (isColor && !isNormal)
	{
		struct data_1 Value;
		if(!ifColor)
		{
			cout<<"\nFile doesn't contain the Color Information, check the constructor call";
			exit(0);
		}
		
		inputPly.read((char *)&Value,sizeof(Value));
		//cout<<"\n"<<Values._x <<"\t"<< Values._y <<"\t"<< Values._z <<"\t"<< Values._nx <<"\t"<< Values._ny <<"\t"<< Values._nz <<"\t"<< Values._r <<"\t"<< Values._g <<"\t"<< Values._b <<"\t"<< Values._a;
		min = max = vec3(Value._x, Value._y, Value._z);

		positions.push_back(vec3(Value._x, Value._y, Value._z));
		colors.push_back(vec3(Value._r, Value._g, Value._b) / 255.0f);

		for (long int i = 1; i < vertexCount; i++) 
		{
			inputPly.read((char *)&Value,sizeof(Value));
			//cout<<"\n"<<Values._x <<"\t"<< Values._y <<"\t"<< Values._z <<"\t"<< Values._nx <<"\t"<< Values._ny <<"\t"<< Values._nz <<"\t"<< (int)Values._r <<"\t"<< (int)Values._g <<"\t"<< (int)Values._b <<"\t"<< (int)Values._a;
        
			if (Value._x < min.x) min.x = Value._x;
			if (Value._y < min.y) min.y = Value._y;
			if (Value._z < min.z) min.z = Value._z;

			if (Value._x > max.x) max.x = Value._x;
			if (Value._y > max.y) max.y = Value._y;
			if (Value._z > max.z) max.z = Value._z;

			positions.push_back(vec3(Value._x, Value._y, Value._z));	// -1 is been multiplied to set the correct orientation of this model....
			colors.push_back(vec3((int)Value._r, (int)Value._g, (int)Value._b) / 255.0f);
		}
		center = (min + max) / 2.0f;

		//Bounding volume measurements
		bvWidth = max.x - min.x;
		bvHeight = max.y - min.y;
		bvDepth = max.z - min.z;
		bvAspectRatio = bvWidth / bvHeight;

	}

	// Runs if there is NORMAL but no color Information
	else if(!isColor && isNormal)
	{
		struct data_2 Value;
		if(!ifNormal)
		{
			cout<<"\nFile doesn't contain the Normal Information, check the constructor call";
			exit(0);
		}
		inputPly.read((char *)&Value,sizeof(Value));
		//cout<<"\n"<<Values._x <<"\t"<< Values._y <<"\t"<< Values._z <<"\t"<< Values._nx <<"\t"<< Values._ny <<"\t"<< Values._nz <<"\t"<< Values._r <<"\t"<< Values._g <<"\t"<< Values._b <<"\t"<< Values._a;
		min = max = vec3(Value._x, Value._y, Value._z);

		positions.push_back(vec3(Value._x, Value._y, Value._z));
		normals.push_back(vec3(Value._nx, Value._ny, Value._nz));

		for (long int i = 1; i < vertexCount; i++) 
		{
			inputPly.read((char *)&Value,sizeof(Value));
			//cout<<"\n"<<Values._x <<"\t"<< Values._y <<"\t"<< Values._z <<"\t"<< Values._nx <<"\t"<< Values._ny <<"\t"<< Values._nz <<"\t"<< (int)Values._r <<"\t"<< (int)Values._g <<"\t"<< (int)Values._b <<"\t"<< (int)Values._a;
        
			if (Value._x < min.x) min.x = Value._x;
			if (Value._y < min.y) min.y = Value._y;
			if (Value._z < min.z) min.z = Value._z;

			if (Value._x > max.x) max.x = Value._x;
			if (Value._y > max.y) max.y = Value._y;
			if (Value._z > max.z) max.z = Value._z;

			positions.push_back(vec3(Value._x, Value._y, Value._z));	// -1 is been multiplied to set the correct orientation of this model....
			normals.push_back(vec3(Value._nx, Value._ny, Value._nz));
		}
		center = (min + max) / 2.0f;

		//Bounding volume measurements
		bvWidth = max.x - min.x;
		bvHeight = max.y - min.y;
		bvDepth = max.z - min.z;
		bvAspectRatio = bvWidth / bvHeight;

	}

	// Runs if there is no NORMAL and no color Information, Only the vertexd information
	else if(!isColor && !isNormal)
	{
		struct data_3 Value;
		
		inputPly.read((char *)&Value,sizeof(Value));
		//cout<<"\n"<<Values._x <<"\t"<< Values._y <<"\t"<< Values._z <<"\t"<< Values._nx <<"\t"<< Values._ny <<"\t"<< Values._nz <<"\t"<< Values._r <<"\t"<< Values._g <<"\t"<< Values._b <<"\t"<< Values._a;
		min = max = vec3(Value._x, Value._y, Value._z);

		positions.push_back(vec3(Value._x, Value._y, Value._z));
		
		for (long int i = 1; i < vertexCount; i++) 
		{
			inputPly.read((char *)&Value,sizeof(Value));
			//cout<<"\n"<<Values._x <<"\t"<< Values._y <<"\t"<< Values._z <<"\t"<< Values._nx <<"\t"<< Values._ny <<"\t"<< Values._nz <<"\t"<< (int)Values._r <<"\t"<< (int)Values._g <<"\t"<< (int)Values._b <<"\t"<< (int)Values._a;
        
			if (Value._x < min.x) min.x = Value._x;
			if (Value._y < min.y) min.y = Value._y;
			if (Value._z < min.z) min.z = Value._z;

			if (Value._x > max.x) max.x = Value._x;
			if (Value._y > max.y) max.y = Value._y;
			if (Value._z > max.z) max.z = Value._z;

			positions.push_back(vec3(Value._x, Value._y, Value._z));	// -1 is been multiplied to set the correct orientation of this model....
		}
		center = (min + max) / 2.0f;

		//Bounding volume measurements
		bvWidth = max.x - min.x;
		bvHeight = max.y - min.y;
		bvDepth = max.z - min.z;
		bvAspectRatio = bvWidth / bvHeight;

	}


	//face Reading Code 
	if(isMesh)
	{
		unsigned char numEdges;
		for(int i=0;i<faceCount;i++)
		{
			inputPly.read((char *)&numEdges,sizeof(numEdges));
			inputPly.read((char *)&F,sizeof(F));
			faces.push_back(ivec3(F.a,F.b,F.c));
		}
	}
    inputPly.close();
}

void PLYModel :: PLYWrite(const char* filename, bool isNormal, bool isColor)
{
	if(!isColor && isNormal)
	{
		FILE *outputPly;
		outputPly=fopen(filename,"wb");
		
		fprintf(outputPly,"ply\n");
		fprintf(outputPly,"format binary_little_endian 1.0\n");
		fprintf(outputPly,"comment This contains a Splatted Point Cloud\n");
		fprintf(outputPly,"element vertex %d\n",vertexCount);
		fprintf(outputPly,"property float x\n");
		fprintf(outputPly,"property float y\n");
		fprintf(outputPly,"property float z\n");
		fprintf(outputPly,"property float nx\n");
		fprintf(outputPly,"property float ny\n");
		fprintf(outputPly,"property float nz\n");
		fprintf(outputPly,"element face %d\n",faceCount);
		fprintf(outputPly,"property list uchar int vertex_indices\n");
		fprintf(outputPly,"end_header\n");

		//write vertices and normals
		for(long int i = 0; i < vertexCount ; i++) 
		{
			fwrite(&positions[i],sizeof(glm::vec3),1,outputPly);
			fwrite(&normals[i],sizeof(glm::vec3),1,outputPly);
		}
		// write faces
		unsigned char sides=3;
		for(int i=0;i<faceCount;i++)
		{
			fwrite(&sides,sizeof(unsigned char),1,outputPly);
			fwrite(&faces[i],sizeof(glm::ivec3),1,outputPly);
		}
		fclose(outputPly);
	}

	else if(isColor && isNormal)
	{
		FILE *outputPly;
		struct colorData C;
		outputPly=fopen(filename,"wb");
		
		fprintf(outputPly,"ply\n");
		fprintf(outputPly,"format binary_little_endian 1.0\n");
		fprintf(outputPly,"comment This contains a Splatted Point Cloud\n");
		fprintf(outputPly,"element vertex %d\n",vertexCount);
		fprintf(outputPly,"property float x\n");
		fprintf(outputPly,"property float y\n");
		fprintf(outputPly,"property float z\n");
		fprintf(outputPly,"property float nx\n");
		fprintf(outputPly,"property float ny\n");
		fprintf(outputPly,"property float nz\n");
		fprintf(outputPly,"property uchar red\n");
		fprintf(outputPly,"property uchar green\n");
		fprintf(outputPly,"property uchar blue\n");
		fprintf(outputPly,"property uchar alpha\n");
		fprintf(outputPly,"element face %d\n",faceCount);
		fprintf(outputPly,"property list uchar int vertex_indices\n");
		fprintf(outputPly,"end_header\n");

		//write vertices and normals
		for(long int i = 0; i < vertexCount ; i++) 
		{
			C.a=255;
			C.r=colors[i].r*255;
			C.g=colors[i].g*255;
			C.b=colors[i].b*255;
			fwrite(&positions[i],sizeof(glm::vec3),1,outputPly);
			fwrite(&normals[i],sizeof(glm::vec3),1,outputPly);
			fwrite(&C,sizeof(struct colorData),1,outputPly);
		}
		// write faces
		unsigned char sides=3;
		for(int i=0;i<faceCount;i++)
		{
			fwrite(&sides,sizeof(unsigned char),1,outputPly);
			fwrite(&faces[i],sizeof(glm::ivec3),1,outputPly);
		}
		fclose(outputPly);
	}
}


void PLYModel:: FreeMemory()
{
	positions.~vector();
	normals.~vector();
	colors.~vector();
	if(isMesh)
	{
		faces.~vector();
	}

}
