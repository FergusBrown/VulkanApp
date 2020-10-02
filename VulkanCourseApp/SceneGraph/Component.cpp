#include "Component.h"

Component::Component(const std::string& name) :
	mName(name)
{
}

const std::string& Component::name() const
{
	return mName;
}
