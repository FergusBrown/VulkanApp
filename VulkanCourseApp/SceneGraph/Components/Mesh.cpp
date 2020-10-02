#include "Mesh.h"


Mesh::Mesh(const std::string& name) :
	Component(name)
{
}

std::type_index Mesh::type()
{
	return typeid(Mesh);
}