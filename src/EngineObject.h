#ifndef ENGINEOBJECT_H
#define ENGINEOBJECT_H

#include "settings.h"

#include <GLM/gtc/quaternion.hpp>

// data structs / enums
// --------
enum objectTypes {
	CUBE,
	VECTOR,
	MODEL
};

struct Vector3D {
	glm::vec3 direction;
	glm::vec3 position;
	float length;
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

class ObjectRotation {
public:
	glm::vec3 axis = glm::vec3{ 0, 1, 0 };
	float angle = 0;

	void setDirection(glm::vec3 direction_) {
		axis = glm::normalize(glm::cross(direction_, glm::vec3{ 0, 1, 0 }));

		if (direction_.x == 0 && direction_.y != 0 && direction_.z == 0) {
			if (direction_.y < 0) {
				axis = glm::vec3{ 0, 0, 1 };
				angle = glm::radians(180.f);
			}
			else {
				axis = glm::vec3{ 0, 1, 0 };
				angle = 0;
			}
			return;
		}

		angle = glm::acos(glm::dot(glm::normalize(direction_), glm::vec3{ 0, 1, 0 }));
		glm::vec3 refVec = glm::normalize(glm::cross(axis, glm::vec3{ 0, 1, 0 }));

		if (glm::dot(refVec, direction_) < 0) {
			angle = -angle;
		}
	}

	void rotateAroundAxis(float angle_) { angle += angle_; }

	void setRotationAroundAxis(float angle_) { angle = angle_; }

	void setRotationAxis(glm::vec3 axis_) { axis = glm::normalize(axis_); }
};

class EngineObject {
public:
	glm::vec3 position = glm::vec3{ 0.f };
	glm::vec3 scale = glm::vec3{ 1.f };
	glm::vec3 color;

	ObjectRotation orientation;
	
	EngineObject() {}
	~EngineObject() {}

	// translate
	// -------
	void moveTo(glm::vec3 position_) {
		position = position_;
	}

	void moveBy(glm::vec3 translation_) {
		position += translation_;
	}

	// scale
	// -------
	void scaleBy(glm::vec3 scale_) {
		scale += scale_;
	}

	void scaleBy(float scale_) {
		scale += scale_;
	}

	void setScale(glm::vec3 scale_) {
		scale = scale_;
	}

	void setScale(float scale_) {
		scale = glm::vec3{ scale_ };
	}
	
	// rotation 
	// -------

	void pointTo(glm::vec3 point_) {
		glm::vec3 direction_ = point_ - position;
		orientation.setDirection(direction_);
	}

	// variables
	// -------
	bool instancedObject;
	unsigned int objectInfoIndex = -1;
	unsigned int verticesIndex = -1;
	unsigned int indicesIndex = -1;
	unsigned int engineObjectListIndex = -1;
};

#endif