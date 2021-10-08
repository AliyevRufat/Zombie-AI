/*=============================================================================*/
// Copyright 2017-2018 Elite Engine
// Authors: Matthieu Delaere, Thomas Goussaert
/*=============================================================================*/
// SteeringBehaviors.h: SteeringBehaviors interface and different implementations
/*=============================================================================*/
#ifndef ELITE_STEERINGBEHAVIORS
#define ELITE_STEERINGBEHAVIORS

//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#pragma region **ISTEERINGBEHAVIOR** (BASE)
#include "Exam_HelperStructs.h"

class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	virtual SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) = 0;

	//Seek Functions
	void SetTarget(const Elite::Vector2& target) { m_Target = target; }

	template<class T, typename std::enable_if<std::is_base_of<ISteeringBehavior, T>::value>::type* = nullptr>
	T* As()
	{
		return static_cast<T*>(this);
	}

protected:
	Elite::Vector2 m_Target;
};
#pragma endregion

///////////////////////////////////////
//SEEK
//****
class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() = default;

	//Seek Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, const AgentInfo& agent);
};

//////////////////////////
//WANDER
//******
class Wander : public Seek
{
public:
	Wander() = default;
	virtual ~Wander() = default;

	//Wander Behavior
	SteeringPlugin_Output CalculateSteering(float deltaT, const AgentInfo& agent);

protected:

	float m_Offset = 6.f; //agent direction
	float m_Radius = 4.f; //wander radius
	float m_AngleChange = Elite::ToRadians(45); //max wanderangle change per frame
	float m_WanderAngle = 0.f; //internal
};
//////////////////////////
//FLEE
//******
class Flee : public Seek
{
public:
	Flee() = default;
	virtual ~Flee() = default;

	//Wander Behavior
	SteeringPlugin_Output CalculateSteering(float deltaT, const AgentInfo& agent);
};
#endif
