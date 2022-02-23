#ifndef ENGINEOBJECT_H
#define ENGINEOBJECT_H
// data structs / enums
// --------
enum objectTypes {
	CUBE,
	ARROW
};

struct ObjectInfo_t {
	glm::mat4 geometryMatrix = glm::mat4(1.0);
	glm::vec4 color = glm::vec4(0.0); // only first three values are used
};

struct dynamicIntArrayData {
	unsigned int* data;
	int size;
	int capacity;
};
struct dynamicVec3ArrayData {
	glm::vec3* data;
	int size;
	int capacity;
};
struct dynamicObjectInfoArrayData {
	ObjectInfo_t* data;
	int size;
	int capacity;
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