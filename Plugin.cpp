#include "stdafx.h"
#include "Plugin.h"
#include "IExamInterface.h"

using namespace Elite;

//Called only once, during initialization
void Plugin::Initialize(IBaseInterface* pInterface, PluginInfo& info)
{
	//This interface gives you access to certain actions the AI_Framework can perform for you
	m_pInterface = static_cast<IExamInterface*>(pInterface);

	//Retrieving the interface
	m_AgentInfo = { m_pInterface->Agent_GetInfo() };
	//Bit information about the plugin
	//Please fill this in!!
	info.BotName = "Lincoln";
	info.Student_FirstName = "Rufat";
	info.Student_LastName = "Aliyev";
	info.Student_Class = "2DAE01";

	/////////init blackboard and BT
	m_pBlackboard = CreateBlackboard();
	BehaviorTree* pBt = new BehaviorTree(m_pBlackboard,
		new BehaviorSelector(
			{
				new BehaviorSelector(
				{
					new BehaviorSequence(
					{
						new BehaviorConditional(IfHealthNotMaxAndHasMedkit), // if can use medkit
						new BehaviorAction(UseMedkit) // use medkit
					}),
					new BehaviorSequence(
					{
						new BehaviorConditional(IfHasTooManyMedkits), // if has too many medkits
						new BehaviorAction(UseMedkit) // use medkit
					})
				}),
				new BehaviorSelector(
				{
					new BehaviorSequence(
					{
						new BehaviorConditional(IfEnergyNotMaxAndHasFood), // if can use food
						new BehaviorAction(UseFood) // use food
					}),
					new BehaviorSequence(
					{
						new BehaviorConditional(IfHasTooManyFood), // if has too many food
						new BehaviorAction(UseFood) // use food
					})
				}),
				new BehaviorSequence(
				{
						new BehaviorConditional(IsPurgeZoneInFOV), // if spots purgezone
						new BehaviorAction(GoOutsideOfPurgeZone) // run away
				}),
				new BehaviorSequence(
				{
					new BehaviorConditional(IsEnemyInFOV), // if enemy spotted
					new BehaviorSelector(
					{
						new BehaviorSequence(
							{
								new BehaviorConditional(CantAttack), // and can't attack
								new BehaviorAction(ChangeToFlee) // flee
							}),
						new BehaviorSequence(
							{
								new BehaviorAction(FaceTheEnemy), // face the enemy
								new BehaviorAction(AttackTheEnemyIfFacingIt) // attack the enemy
							})
					}),
				}),
				new BehaviorSequence(
				{
					new BehaviorConditional(IsPickUpInFOV), // if pick up spotted
					new BehaviorSelector(
					{
						new BehaviorSequence(
							{
								new BehaviorConditional(IsItemInGrabRange), // if it's in grab range
								new BehaviorAction(PickUpItem) // grab it
							}),
						new BehaviorSequence(
							{
								new BehaviorAction(GoToItem) // go to it
							})
					}),
				}),
				new BehaviorSequence(
				{
					new BehaviorConditional(IsBitten), // if is bitten
					new BehaviorSelector(
					{
							new BehaviorSequence(
								{
									new BehaviorConditional(CantAttack), // and can't attack
									new BehaviorAction(ChangeToFlee) // run away
								}),
							new BehaviorAction(TurnAround), // turn around (to kill enemies behind you)
					}),
				}),
				new BehaviorConditional(IsHouseInFOV), // if house in fov
				new BehaviorSequence(
				{
					new BehaviorConditional(HasTargetHouse), // if already has a target house
					new BehaviorSelector(
					{
						new BehaviorSequence(
							{
								new BehaviorConditional(IsInHouseCenter), // if you are in the center of the house
								new BehaviorAction(ExitHouse) // exit the house
							}),
						new BehaviorSequence(
							{
								new BehaviorAction(EnterHouse) // enter the house
							})
					}),
				}),
				new BehaviorAction(ChangeToWander) // wander if none of the above mentioned conditionals returned true
			})
	);
	m_pDecisionMaking = pBt;
}

//Called only once
void Plugin::DllInit()
{
	//Called when the plugin is loaded
}

//Called only once
void Plugin::DllShutdown()
{
	//Called wheb the plugin gets unloaded
	SAFE_DELETE(m_pDecisionMaking);
}

//Called only once, during initialization
void Plugin::InitGameDebugParams(GameDebugParams& params)
{
	params.AutoFollowCam = true; //Automatically follow the AI? (Default = true)
	params.RenderUI = true; //Render the IMGUI Panel? (Default = true)
	params.SpawnEnemies = true; //Do you want to spawn enemies? (Default = true)
	params.EnemyCount = 20; //How many enemies? (Default = 20)
	params.GodMode = false; //GodMode > You can't die, can be usefull to inspect certain behaviours (Default = false)
	params.AutoGrabClosestItem = true; //A call to Item_Grab(...) returns the closest item that can be grabbed. (EntityInfo argument is ignored)
	//params.LevelFile = "LevelThree.gppl";
	params.SpawnPurgeZonesOnMiddleClick = true;
}

//Update
//This function calculates the new SteeringOutput, called once per frame
SteeringPlugin_Output Plugin::UpdateSteering(float dt)
{
	//update decisionmaking
	if (m_pDecisionMaking)
		m_pDecisionMaking->Update(dt);
	//updating vars
	m_AgentInfo = m_pInterface->Agent_GetInfo();
	m_EntitiesInFov = GetEntitiesInFOV();
	m_HousesInFov = GetHousesInFOV();
	m_DeltaTime = dt;
	//updating those vars in blackboard
	m_pBlackboard->ChangeData("AgentInfo", m_AgentInfo);
	m_pBlackboard->ChangeData("EntitiesInFov", m_EntitiesInFov);
	m_pBlackboard->ChangeData("HousesInFov", m_HousesInFov);
	m_pBlackboard->ChangeData("DeltaTime", m_DeltaTime);
	m_pBlackboard->ChangeData("Interface", m_pInterface);
	//
	m_ClearHouseVecCounter += dt;
	if (m_ClearHouseVecCounter >= m_MaxTimeForClearHouse)
	{
		m_ClearHouseVecCounter -= m_ClearHouseVecCounter;
		m_VisitedHouseCenters.clear();
		m_pBlackboard->ChangeData("VisitedHousesCenterVec", m_VisitedHouseCenters);
	}
	return m_Steering;
}
//This function should only be used for rendering debug elements
void Plugin::Render(float dt) const
{
	//This Render function should only contain calls to Interface->Draw_... functions
	//m_pInterface->Draw_SolidCircle(m_Target, .7f, { 0,0 }, { 1, 0, 0 });
}

vector<HouseInfo> Plugin::GetHousesInFOV() const
{
	vector<HouseInfo> vHousesInFOV = {};

	HouseInfo hi = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetHouseByIndex(i, hi))
		{
			vHousesInFOV.push_back(hi);
			continue;
		}

		break;
	}

	return vHousesInFOV;
}

vector<EntityInfo> Plugin::GetEntitiesInFOV() const
{
	vector<EntityInfo> vEntitiesInFOV = {};

	EntityInfo ei = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetEntityByIndex(i, ei))
		{
			vEntitiesInFOV.push_back(ei);
			continue;
		}

		break;
	}
	return vEntitiesInFOV;
}

Elite::Blackboard* Plugin::CreateBlackboard()
{
	Elite::Blackboard* pBlackboard = new Elite::Blackboard();
	pBlackboard->AddData("AgentInfo", m_AgentInfo);
	pBlackboard->AddData("Steering", &m_Steering);
	pBlackboard->AddData("SavedPrevPos", m_PrevPos);
	//
	pBlackboard->AddData("EntitiesInFov", m_EntitiesInFov);
	pBlackboard->AddData("HousesInFov", m_HousesInFov);
	pBlackboard->AddData("EnemyInFov", m_EnemyInFov);
	pBlackboard->AddData("ItemInFov", m_ItemInFov);
	//
	pBlackboard->AddData("Interface", m_pInterface);
	//
	pBlackboard->AddData("PistolIndex", m_PistolIndex);
	pBlackboard->AddData("GoOutsideOfHouse", m_GoOutsideOfHouse);
	pBlackboard->AddData("DeltaTime", m_DeltaTime);
	pBlackboard->AddData("TargetHouse", m_TargetHouse);
	pBlackboard->AddData("HasTargetHouse", m_HasTargetHouse);
	pBlackboard->AddData("TurnedAround", m_TurnedAround);
	pBlackboard->AddData("VisitedHousesCenterVec", m_VisitedHouseCenters);
	pBlackboard->AddData("FilledInventoryAmount", m_FilledInventorySlotAmount);

	return pBlackboard;
}