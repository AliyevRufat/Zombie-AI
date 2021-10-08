//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"

//SEEK
//****
SteeringPlugin_Output Seek::CalculateSteering(float deltaT, const AgentInfo& agent)
{
	SteeringPlugin_Output steering = {};

	steering.LinearVelocity = m_Target - agent.Position; //Desired Velocity
	steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	steering.LinearVelocity *= agent.MaxLinearSpeed; //Rescale to Max Speed

	return steering;
}

//WANDER (base> SEEK)
//******
SteeringPlugin_Output Wander::CalculateSteering(float deltaT, const AgentInfo& agent)
{
	SteeringPlugin_Output steering = {};
	//get circles pos
	Elite::Vector2 direction = agent.LinearVelocity;
	direction.Normalize();
	Elite::Vector2 posCircle = agent.Position + direction * m_Offset;

	m_WanderAngle += Elite::randomFloat(-m_AngleChange, m_AngleChange);

	m_Target.x = posCircle.x + cos(m_WanderAngle) * m_Radius;
	m_Target.y = posCircle.y + sin(m_WanderAngle) * m_Radius;

	//rand movement
	steering.LinearVelocity = m_Target - agent.Position;
	steering.LinearVelocity.Normalize(); //Normalize desired Velocity
	steering.LinearVelocity *= agent.MaxLinearSpeed; //Rescale to max speed
	return steering;
}
//FLEE
//****
SteeringPlugin_Output Flee::CalculateSteering(float deltaT, const AgentInfo& agent)
{
	SteeringPlugin_Output steering = {};

	steering.LinearVelocity = m_Target - agent.Position; //Desired Velocity
	steering.LinearVelocity.Normalize(); //Normalize desired Velocity
	steering.LinearVelocity *= agent.MaxLinearSpeed; //Rescale to max speed

	return steering;
}