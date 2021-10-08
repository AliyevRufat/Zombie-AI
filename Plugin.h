#pragma once
#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"
#include "EBlackboard.h"
#include "IDecisionMaking.h"
#include "Behaviors.h"

class IBaseInterface;
class IExamInterface;

class Plugin :public IExamPlugin
{
public:
	Plugin() {};
	virtual ~Plugin() {};

	void Initialize(IBaseInterface* pInterface, PluginInfo& info) override;
	void DllInit() override;
	void DllShutdown() override;

	void InitGameDebugParams(GameDebugParams& params) override;
	void Update(float dt) override {};

	SteeringPlugin_Output UpdateSteering(float dt) override;
	void Render(float dt) const override;

private:
	//Interface, used to request data from/perform actions with the AI Framework
	IExamInterface* m_pInterface = nullptr;
	vector<HouseInfo> GetHousesInFOV() const;
	vector<EntityInfo> GetEntitiesInFOV() const;
	//added methods
	Elite::Blackboard* CreateBlackboard();
	Elite::Vector2 m_PrevPos = { 0,0 };
	//added datamembers
	AgentInfo m_AgentInfo;
	SteeringPlugin_Output m_Steering;
	//
	Elite::IDecisionMaking* m_pDecisionMaking = nullptr;
	Elite::Blackboard* m_pBlackboard = nullptr;
	//
	vector<EntityInfo> m_EntitiesInFov = {};
	vector<HouseInfo> m_HousesInFov = {};
	//
	EntityInfo m_EnemyInFov = {};
	EntityInfo m_ItemInFov = {};
	//
	UINT m_PistolIndex = {};
	float m_DeltaTime = {};
	bool m_GoOutsideOfHouse = {};
	HouseInfo m_TargetHouse = {};
	bool m_HasTargetHouse = { false };
	bool m_TurnedAround = { false };
	vector<Elite::Vector2> m_VisitedHouseCenters = {};
	int m_FilledInventorySlotAmount;
	//
	float m_ClearHouseVecCounter;
	const int m_MaxTimeForClearHouse = { 60 };
};

//ENTRY
//This is the first function that is called by the host program
//The plugin returned by this function is also the plugin used by the host program
extern "C"
{
	__declspec (dllexport) IPluginBase* Register()
	{
		return new Plugin();
	}
}