#pragma once
#include "SceneGraph/Component.h";

class Node;

class Mesh : public Component
{
public:
	Mesh(const std::string& name);
	~Mesh() = default;


	virtual std::type_index type() override;
private:
	Node& mNode;
};