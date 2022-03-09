#ifndef ENGINEOBJECT_H
#define ENGINEOBJECT_H

#include "settings.h"

// data structs / enums
// --------
enum objectTypes {
	CUBE,
	ARROW,
	MODEL
};

struct ObjectInfo_t {
	glm::mat4 geometryMatrix = glm::mat4(1.0);
	glm::vec4 color = glm::vec4(0.0); // only first three values are used
};

class dynamicIntArrayData {
public:
	unsigned int* data = new unsigned int[INITIAL_INDEX_BUFFER_CAPACITY];
	int size = 0;
	int capacity = INITIAL_INDEX_BUFFER_CAPACITY;

	void addData(unsigned int* newData, int size_) {

	}
	void addData(std::vector<unsigned int> newData) {
		if (size + newData.size() > capacity) {
			unsigned int* placeholder = data;
			while (size + newData.size() > capacity) {
				capacity *= 2;
			}

			data = new unsigned int[capacity];
			for (int i = 0; i < size; i++)
			{
				data[i] = placeholder[i];
			}
		}
		for (int i = size; i < size + newData.size(); i++)
		{
			data[i] = newData[i - size];
		}

		size += newData.size();
	}
};

class dynamicFloatArrayData {
public:
	float* data = new float[INITIAL_INDEX_BUFFER_CAPACITY];
	int size = 0;
	int capacity = INITIAL_INDEX_BUFFER_CAPACITY;

	void addData(float* newData, int size_) {

	}
	void addData(std::vector<float> newData) {
		if (size + newData.size() > capacity) {
			float* placeholder = data;
			while (size + newData.size() * 3 > capacity) {
				capacity *= 2;
			}

			data = new float[capacity];
			for (int i = 0; i < size; i++)
			{
				data[i] = placeholder[i];
			}
			delete[] placeholder;
		}
		for (int i = size; i < size + newData.size(); i++)
		{
			data[i] = newData[i - size];
		}

		size += newData.size();
	}
	void addData(std::vector<glm::vec3> newData) {
		if (size + newData.size() * 3 > capacity) {
			float* placeholder = data;
			while (size + newData.size() * 3 > capacity) {
				capacity *= 2;
			}

			data = new float[capacity];
			for (int i = 0; i < size; i++)
			{
				data[i] = placeholder[i];
			}
			delete[] placeholder;
		}
		for (int i = size; i < size + newData.size()*3; i++)
		{
			data[i] = newData[(int)std::floor((i - (int)size) / 3)][(i - (int)size + 3) % 3];
		}

		size += newData.size() * 3;
	}
};

class dynamicVec3ArrayData {
public:
	glm::vec3* data = new glm::vec3[INITIAL_VERTEX_BUFFER_CAPACITY];
	int size = 0;
	int capacity = INITIAL_VERTEX_BUFFER_CAPACITY;

	void addData(glm::vec3* newData, int size_) {

	}
	void addData(std::vector<glm::vec3> newData) {
		if (size + newData.size() > capacity) {
			glm::vec3* placeholder = data;
			while (size + newData.size() * 3 > capacity) {
				capacity *= 2;
			}

			data = new glm::vec3[capacity];
			for (int i = 0; i < size; i++)
			{
				data[i] = placeholder[i];
			}
			delete[] placeholder;
		}
		for (int i = size; i < size + newData.size(); i++)
		{
			data[i] = newData[i - size];
		}

		size += newData.size();
	}
};

class dynamicObjectInfoArrayData {
public:
	ObjectInfo_t* data = new ObjectInfo_t[INITIAL_OBJECT_CAPACITY];
	int size = 0;
	int capacity = INITIAL_OBJECT_CAPACITY;

	void addData(ObjectInfo_t* newData, int size_) {

	}

	void addData(ObjectInfo_t newData) {
		if (size + 1 > capacity) {
			ObjectInfo_t* placeholder = data;

			capacity *= 2;
			data = new ObjectInfo_t[capacity];
			for (int i = 0; i < size; i++)
			{
				data[i].geometryMatrix = placeholder[i].geometryMatrix;
				data[i].color = placeholder[i].color;
			}
			delete[] placeholder;
		}
		
		data[size].geometryMatrix = newData.geometryMatrix;
		data[size].color = newData.color;

		size++;
	}
};

class EngineObject {
public:

	bool instancedObject = false;
	unsigned int objectInfoIndex = -1;
	unsigned int verticesIndex = -1;
	unsigned int indicesIndex = -1;

	EngineObject() {}
	~EngineObject() {}
};

#endif