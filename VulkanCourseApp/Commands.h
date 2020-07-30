#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Pawn.h"
#include <memory>

//typedef std::unique_ptr<Command> CommandPtr;

class Command
{
public:
	virtual ~Command() {}
	virtual void execute(Pawn& pawn) = 0;
};

class MoveUp : public Command
{
public:
	MoveUp(float speedModifier) :
		mSpeedModifier(speedModifier)
	{}

	virtual void execute(Pawn& pawn)
	{
		pawn.moveUpBy(1.0 * mSpeedModifier);
	}

private:
	float mSpeedModifier;
};

class MoveDown : public Command
{
public:
	MoveDown(float speedModifier) :
		mSpeedModifier(speedModifier)
	{}

	virtual void execute(Pawn& pawn)
	{
		pawn.moveUpBy(-1.0 * mSpeedModifier);
	}

private:
	float mSpeedModifier;
};

class MoveForward : public Command
{
public:
	MoveForward(float speedModifier) :
		mSpeedModifier(speedModifier)
	{}

	virtual void execute(Pawn& pawn)
	{
		pawn.moveForwardBy(1.0 * mSpeedModifier);
	}

private:
	float mSpeedModifier;
};

class MoveBack : public Command
{
public:
	MoveBack(float speedModifier) :
		mSpeedModifier(speedModifier)
	{}

	virtual void execute(Pawn& pawn)
	{
		pawn.moveForwardBy(-1.0 * mSpeedModifier);
	}

private:
	float mSpeedModifier;
};

class MoveLeft : public Command
{
public:
	MoveLeft(float speedModifier) :
		mSpeedModifier(speedModifier)
	{}

	virtual void execute(Pawn& pawn)
	{
		pawn.moveRightBy(-1.0 * mSpeedModifier);
	}

private:
	float mSpeedModifier;
};

class MoveRight : public Command
{
public:
	MoveRight(float speedModifier) :
		mSpeedModifier(speedModifier)
	{}

	virtual void execute(Pawn& pawn)
	{
		pawn.moveRightBy(1.0 * mSpeedModifier);
	}

private:
	float mSpeedModifier;
};

class rotateView : public Command
{
public:
	rotateView(vec3 eulerAngles) :
		mEulerAngles(eulerAngles)
	{}

	virtual void execute(Pawn& pawn)
	{
		pawn.rotate(mEulerAngles);
	}

private:
	vec3 mEulerAngles;
};