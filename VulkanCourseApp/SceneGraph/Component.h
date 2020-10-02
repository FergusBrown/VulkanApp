#pragma once

#include <string>
#include <typeindex>
#include <vector>

// Base class to be used in scene graph nodes
class Component
{
public:
	Component() = default;
	Component(const std::string& name);
	virtual ~Component() = default;

	const std::string& name() const;

	virtual std::type_index type() = 0;
private:
	std::string mName;

};

