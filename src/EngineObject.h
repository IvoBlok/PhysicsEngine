#ifndef ENGINEOBJECT_H
#define ENGINEOBJECT_H

struct ObjectInfo_t {
	glm::mat4 geometryMatrix = glm::mat4(1.0);
	glm::vec4 color = glm::vec4(0.0); // only first three values are used
	glm::vec4 indices = glm::vec4(0.0); 
};

class EngineObject {
public:

	unsigned int objectInfoIndex;

	EngineObject(unsigned int objectInfoIndex_ = 0) : objectInfoIndex(objectInfoIndex_) {}

	~EngineObject() { }
};

#endif