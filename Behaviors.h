/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the BT version of the Agario Game
/*=============================================================================*/
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "EBehaviorTree.h"
#include "SteeringBehaviors.h"
#include "IExamInterface.h"
//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------
//globals
float gTimeBeforeTurningBack = 0.0f;
//------------------------------------------------------------CONDITIONS--------------------------------------------------------------------------------

bool IsBitten(Elite::Blackboard* pBlackboard)
{
	AgentInfo agentInfo = {};

	auto dataAvailable = pBlackboard->GetData("AgentInfo", agentInfo);

	if (!dataAvailable)
	{
		return false;
	}

	if (agentInfo.WasBitten)
	{
		return true; // is bitten
	}

	return false; // is not bitten
}

bool IsEnemyInFOV(Elite::Blackboard* pBlackboard)
{
	EntityInfo enemyInfo = {};
	std::vector<EntityInfo> entities = {};

	auto dataAvailable = pBlackboard->GetData("EntitiesInFov", entities);

	if (!dataAvailable)
	{
		return false;
	}

	for (size_t i = 0; i < entities.size(); i++)
	{
		if (entities[i].Type == eEntityType::ENEMY)
		{
			enemyInfo = entities[i];
			pBlackboard->ChangeData("EnemyInFov", enemyInfo);
			return true; // there is an enemy in FOV
		}
	}
	return false; // there is no enemy in FOV
}

bool IsPurgeZoneInFOV(Elite::Blackboard* pBlackboard)
{
	std::vector<EntityInfo> entities = {};

	auto dataAvailable = pBlackboard->GetData("EntitiesInFov", entities);

	if (!dataAvailable)
	{
		return false;
	}
	for (size_t i = 0; i < entities.size(); i++)
	{
		if (entities[i].Type == eEntityType::PURGEZONE)
		{
			pBlackboard->ChangeData("HasTargetHouse", false);
			return true; // there is a purgezone in fov
		}
	}
	return false; // there is no purgezone in fov
}

bool IsPickUpInFOV(Elite::Blackboard* pBlackboard)
{
	//vars
	EntityInfo pickUpEntity = {};
	std::vector<EntityInfo> entities = {};
	IExamInterface* pInterface = nullptr;
	ItemInfo tempItem = {};
	bool canPickUp = { false };
	//get vars from blackboard
	auto dataAvailable = pBlackboard->GetData("EntitiesInFov", entities) && pBlackboard->GetData("Interface", pInterface);

	if (!pInterface || !dataAvailable)
	{
		return false;
	}

	//if entites is not empty
	if (entities.size() == 0)
	{
		return false;
	}
	//
	for (size_t i = 0; i < entities.size(); i++)
	{
		if (entities[i].Type == eEntityType::ITEM)
		{
			for (size_t j = 0; j < pInterface->Inventory_GetCapacity(); j++)
			{
				if (!(pInterface->Inventory_GetItem(j, tempItem)))
				{
					pickUpEntity = entities[i];
					pBlackboard->ChangeData("ItemInFov", pickUpEntity); // save the first item pick up in the FOV
					return true; // there is a spot in inventory and there is an item in FOV
				}
			}
		}
	}

	return false; // there is no empty slot in inventory or there is no item in FOV
}

bool IsHouseInFOV(Elite::Blackboard* pBlackboard)
{
	std::vector<HouseInfo> housesInfo = {};
	std::vector<Elite::Vector2> visitedHouseCenters = {};
	bool hasTargetHouse = {};

	auto dataAvailable = pBlackboard->GetData("HousesInFov", housesInfo) && pBlackboard->GetData("HasTargetHouse", hasTargetHouse) && pBlackboard->GetData("VisitedHousesCenterVec", visitedHouseCenters);

	if (hasTargetHouse || !dataAvailable || housesInfo.size() == 0)
	{
		return false;
	}

	auto it = std::find(visitedHouseCenters.begin(), visitedHouseCenters.end(), housesInfo[0].Center); //see if the house that you see is already visited or not

	if (it != visitedHouseCenters.end())
	{
		//found in the visited list
		return false;
	}
	//not found in the visited list
	pBlackboard->ChangeData("TargetHouse", housesInfo[0]);
	pBlackboard->ChangeData("HasTargetHouse", true);
	return true; // there is a house in FOV
}

bool HasTargetHouse(Elite::Blackboard* pBlackboard)
{
	bool hasTargetHouse = {};

	auto dataAvailable = pBlackboard->GetData("HasTargetHouse", hasTargetHouse);

	if (!dataAvailable)
	{
		return false;
	}

	if (hasTargetHouse)
	{
		return true;
	}
	return false;
}

bool IsInHouseCenter(Elite::Blackboard* pBlackboard)
{
	//vars
	bool goOutsideOfHouse = {};
	const float distance = 3.0f;
	HouseInfo targetHouse = {};
	std::vector<HouseInfo> housesInFov = {};
	AgentInfo agentInfo = {};
	//get vars from blackboard
	auto dataAvailable = pBlackboard->GetData("HousesInFov", housesInFov) && pBlackboard->GetData("AgentInfo", agentInfo)
		&& pBlackboard->GetData("GoOutsideOfHouse", goOutsideOfHouse) && pBlackboard->GetData("TargetHouse", targetHouse);
	//check only the first house in fov

	if (!dataAvailable)
	{
		return false;
	}

	if (goOutsideOfHouse) // going outside of the house so shouldn't check this function
	{
		return true;
	}
	else
	{
		if (abs((agentInfo.Position - targetHouse.Center).Magnitude()) < distance) // check if in center
		{
			//
			goOutsideOfHouse = true;
			pBlackboard->ChangeData("GoOutsideOfHouse", goOutsideOfHouse);
			return true; // AI in center of the house
		}
	}
	return false; // AI not in center of the house
}

bool IsItemInGrabRange(Elite::Blackboard* pBlackboard)
{
	AgentInfo agentInfo = {};
	EntityInfo pickUpEntity = {};

	auto dataAvailable = pBlackboard->GetData("ItemInFov", pickUpEntity) && pBlackboard->GetData("AgentInfo", agentInfo);

	if (!dataAvailable)
	{
		return false;
	}

	if (abs((pickUpEntity.Location - agentInfo.Position).Magnitude()) <= agentInfo.GrabRange) // check if item in grab range
	{
		return true; // item is in grab range
	}
	return false; // is not in grab range
}

bool IfHealthNotMaxAndHasMedkit(Elite::Blackboard* pBlackboard)
{
	//vars
	AgentInfo agentInfo = {};
	IExamInterface* pInterface = nullptr;
	ItemInfo itemInfo = {};
	const int minHealth = { 8 };
	//get vars from blackboard
	auto dataAvailable = pBlackboard->GetData("AgentInfo", agentInfo) && pBlackboard->GetData("Interface", pInterface);

	if (!pInterface || !dataAvailable)
	{
		return false;
	}

	if (agentInfo.Health < minHealth)
	{
		for (size_t i = 0; i < pInterface->Inventory_GetCapacity(); i++)
		{
			if (pInterface->Inventory_GetItem(i, itemInfo))
			{
				if (itemInfo.Type == eItemType::MEDKIT) // check if item is medkit and health below some amount
				{
					return true; // can use the medkit
				}
			}
		}
	}
	return false; // can't use the medkit
}

bool IfEnergyNotMaxAndHasFood(Elite::Blackboard* pBlackboard)
{
	//vars
	AgentInfo agentInfo = {};
	IExamInterface* pInterface = nullptr;
	ItemInfo itemInfo = {};
	const int minEnergy = { 3 };
	//get vars from blackboard
	auto dataAvailable = pBlackboard->GetData("AgentInfo", agentInfo) && pBlackboard->GetData("Interface", pInterface);

	if (!pInterface || !dataAvailable)
	{
		return false;
	}

	if (agentInfo.Energy < minEnergy)
	{
		for (size_t i = 0; i < pInterface->Inventory_GetCapacity(); i++)
		{
			if (pInterface->Inventory_GetItem(i, itemInfo))
			{
				if (itemInfo.Type == eItemType::FOOD) // check if item is food and energy below some amount
				{
					return true; // can use the food
				}
			}
		}
	}
	return false; // can't use the food
}

bool CantAttack(Elite::Blackboard* pBlackboard)
{
	//vars
	UINT pistolIndex = {};
	ItemInfo pistol = {};
	AgentInfo agentInfo = {};
	IExamInterface* pInterface = nullptr;
	//get vars from blackboard
	auto dataAvailable = pBlackboard->GetData("Interface", pInterface) && pBlackboard->GetData("AgentInfo", agentInfo);

	if (!pInterface || !dataAvailable)
	{
		return false;
	}

	for (size_t i = 0; i < pInterface->Inventory_GetCapacity(); i++)
	{
		if (pInterface->Inventory_GetItem(i, pistol))
		{
			if (pistol.Type == eItemType::PISTOL)
			{
				if (pInterface->Weapon_GetAmmo(pistol) > 0)
				{
					pistolIndex = i; // save the inventory index of the pistol
					pBlackboard->ChangeData("PistolIndex", pistolIndex);
					return false; // there is a pistol in inventory with at least 1 bullet
				}
			}
		}
	}
	return true; // there is no pistol in inverntory or no bullets left in it
}

bool IfHasTooManyMedkits(Elite::Blackboard* pBlackboard)
{
	//vars
	IExamInterface* pInterface = nullptr;
	ItemInfo itemInfo = {};
	int medkitAmount = {};
	const int maxAmountMedkits = 3;
	//get vars from blackboard
	auto dataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (!pInterface || !dataAvailable)
	{
		return false;
	}

	for (size_t i = 0; i < pInterface->Inventory_GetCapacity(); i++)
	{
		if (pInterface->Inventory_GetItem(i, itemInfo))
		{
			if (itemInfo.Type == eItemType::MEDKIT) // check if item is medkit and health below some amount
			{
				++medkitAmount;
			}
		}
	}
	//check if it has too many medkitss
	if (medkitAmount >= maxAmountMedkits)
	{
		return true; //has too many medkits
	}
	return false;
}

bool IfHasTooManyFood(Elite::Blackboard* pBlackboard)
{
	//vars
	IExamInterface* pInterface = nullptr;
	ItemInfo itemInfo = {};
	int foodAmount = {};
	const int maxAmountFood = 3;
	//get vars from blackboard
	auto dataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (!pInterface || !dataAvailable)
	{
		return false;
	}

	for (size_t i = 0; i < pInterface->Inventory_GetCapacity(); i++)
	{
		if (pInterface->Inventory_GetItem(i, itemInfo))
		{
			if (itemInfo.Type == eItemType::FOOD) // check if item is medkit and health below some amount
			{
				++foodAmount;
			}
		}
	}
	//check if it has too many medkitss
	if (foodAmount >= maxAmountFood)
	{
		return true; //has too many medkits
	}
	return false;
}

//------------------------------------------------------------BEHAVIORS--------------------------------------------------------------------------------

//------------------------------------------------------------------------------MOVE RELATED

Elite::BehaviorState ChangeToFlee(Elite::Blackboard* pBlackboard)
{
	//vars
	SteeringPlugin_Output* steering = {};
	AgentInfo agentInfo = {};
	Elite::Vector2 fleeTarget = {};
	float offset = 6.f;
	float radius = 4.f; //wander radius
	float fleeAngle = 0.f; //internal
	//
	auto dataAvailable = pBlackboard->GetData("Steering", steering) && pBlackboard->GetData("AgentInfo", agentInfo);
	//

	if (!steering || !dataAvailable)
	{
		return Elite::Failure;
	}

	Elite::Vector2 direction = agentInfo.LinearVelocity;
	direction.Normalize();
	Elite::Vector2 posCircle = agentInfo.Position + direction * offset;

	fleeAngle += Elite::randomFloat(-180, 180);

	fleeTarget.x = posCircle.x + cos(fleeAngle) * radius;
	fleeTarget.y = posCircle.y + sin(fleeAngle) * radius;

	//flee
	steering->LinearVelocity = fleeTarget - agentInfo.Position;
	steering->LinearVelocity.Normalize(); //Normalize desired Velocity
	steering->LinearVelocity *= agentInfo.MaxLinearSpeed; //Rescale to max speed
	steering->RunMode = true;

	return Elite::Success;
}

Elite::BehaviorState ChangeToWander(Elite::Blackboard* pBlackboard)
{
	//vars
	float deltaTime = {};
	IExamInterface* pInterface = nullptr;
	SteeringPlugin_Output* steering = {};
	AgentInfo agentInfo = {};
	Elite::Vector2 target{};
	float offset = 6.f;
	float radius = 4.f; //wander radius
	float angleChange = Elite::ToRadians(360); //max wanderangle change per frame
	float wanderAngle = 0.f; //internal
	bool turnedAround = {};

	auto dataAvailable = pBlackboard->GetData("Steering", steering) && pBlackboard->GetData("AgentInfo", agentInfo)
		&& pBlackboard->GetData("DeltaTime", deltaTime) && pBlackboard->GetData("Interface", pInterface) && pBlackboard->GetData("TurnedAround", turnedAround);

	if (!pInterface || !steering || !dataAvailable)
	{
		return Elite::Failure;
	}

	steering->AutoOrient = true;
	//vars calculations
	Elite::Vector2 direction = agentInfo.LinearVelocity;
	direction.Normalize();
	Elite::Vector2 posCircle = agentInfo.Position + direction * offset;
	wanderAngle += Elite::randomFloat(-angleChange, angleChange);

	//check if outside of the boundaries
	if (abs(agentInfo.Position.x) > pInterface->World_GetInfo().Dimensions.x / 2 || abs(agentInfo.Position.y) > pInterface->World_GetInfo().Dimensions.y / 2)
	{
		target = pInterface->World_GetInfo().Center; // if outside set center of map as target
	}
	else
	{
		target.x = posCircle.x + cos(wanderAngle) * radius;
		target.y = posCircle.y + sin(wanderAngle) * radius;
	}
	//rand movement
	steering->LinearVelocity = target - agentInfo.Position;
	steering->LinearVelocity.Normalize(); //Normalize desired Velocity
	steering->LinearVelocity *= agentInfo.MaxLinearSpeed; //Rescale to max speed
	//run when can
	if (agentInfo.Stamina <= 1)
	{
		steering->RunMode = false;
	}
	else if (agentInfo.Stamina == 10)
	{
		steering->RunMode = true;
	}
	//reset go outside of house
	bool goOutsideOfHouse = {};
	pBlackboard->GetData("GoOutsideOfHouse", goOutsideOfHouse);

	if (goOutsideOfHouse)
	{
		goOutsideOfHouse = false;
		pBlackboard->ChangeData("GoOutsideOfHouse", goOutsideOfHouse);
	}

	if (turnedAround) // if killed an enemy behind you turn towards the center to explore other houses on the way
	{
		steering->LinearVelocity = -(target - agentInfo.Position);
		steering->LinearVelocity.Normalize(); //Normalize desired Velocity
		steering->LinearVelocity *= agentInfo.MaxLinearSpeed; //Rescale to max speed
		pBlackboard->ChangeData("TurnedAround", false);
		pBlackboard->ChangeData("SavedPrevPos", Elite::Vector2{ 0,0 }); // reset prev pos
		return Elite::Success;
	}

	return Elite::Success;
}

Elite::BehaviorState GoOutsideOfPurgeZone(Elite::Blackboard* pBlackboard)
{
	//vars
	PurgeZoneInfo purgeZoneInfo = {};
	EntityInfo purgeZoneEntitiy = {};
	std::vector<EntityInfo> entities = {};
	IExamInterface* pInterface = nullptr;
	SteeringPlugin_Output* steering = {};
	AgentInfo agentInfo = {};
	Elite::Vector2 target = {};

	auto dataAvailable = pBlackboard->GetData("Interface", pInterface)
		&& pBlackboard->GetData("AgentInfo", agentInfo) && pBlackboard->GetData("Steering", steering) && pBlackboard->GetData("EntitiesInFov", entities);

	if (!pInterface || !steering || !dataAvailable)
	{
		return Elite::Failure;
	}
	//get purgezone
	for (size_t i = 0; i < entities.size(); i++)
	{
		if (entities[i].Type == eEntityType::PURGEZONE)
		{
			purgeZoneEntitiy = entities[i];
		}
	}
	pInterface->PurgeZone_GetInfo(purgeZoneEntitiy, purgeZoneInfo);
	//if in house use navmesh to get out of it while in purge zone
	if (agentInfo.IsInHouse)
	{
		target = pInterface->NavMesh_GetClosestPathPoint(pInterface->World_GetInfo().Center);
		steering->LinearVelocity = target - agentInfo.Position;
	}
	else //if not run from the center of purgezone
	{
		target = purgeZoneInfo.Center;
		steering->LinearVelocity = agentInfo.Position - target;
	}
	//steering
	steering->LinearVelocity.Normalize(); //Normalize desired Velocity
	steering->LinearVelocity *= agentInfo.MaxLinearSpeed; //Rescale to max speed
	steering->AutoOrient = true;
	steering->RunMode = true;

	return Elite::Success;
}

Elite::BehaviorState TurnAround(Elite::Blackboard* pBlackboard)
{
	//vars
	const float maxTimeBeforeTurningBack = 0.3f;
	float deltaTime = {};
	SteeringPlugin_Output* steering = {};
	AgentInfo agentInfo = {};
	IExamInterface* pInterface = nullptr;
	std::vector<HouseInfo> housesInFov = {};
	Elite::Vector2 prevPos = {};
	auto dataAvailable = pBlackboard->GetData("Interface", pInterface) &&
		pBlackboard->GetData("AgentInfo", agentInfo) && pBlackboard->GetData("Steering", steering)
		&& pBlackboard->GetData("HousesInFov", housesInFov) && pBlackboard->GetData("DeltaTime", deltaTime) && pBlackboard->GetData("SavedPrevPos", prevPos);

	if (!steering || !pInterface || !dataAvailable)
	{
		return Elite::Failure;
	}
	//check if last frame pos is already filled
	if (prevPos == Elite::Vector2{ 0,0 })
	{
		pBlackboard->ChangeData("SavedPrevPos", agentInfo.Position);
	}
	gTimeBeforeTurningBack += deltaTime;
	steering->RunMode = true;
	if (gTimeBeforeTurningBack >= maxTimeBeforeTurningBack)
	{
		pBlackboard->GetData("SavedPrevPos", prevPos); // take prev pos
		pBlackboard->ChangeData("TurnedAround", true);
		//make it turn around after putting some distance between you and enemy that bit you
		steering->LinearVelocity = prevPos - agentInfo.Position;
		steering->LinearVelocity.Normalize(); //Normalize desired Velocity
		steering->LinearVelocity *= agentInfo.MaxLinearSpeed; //Rescale to max speed
		gTimeBeforeTurningBack -= gTimeBeforeTurningBack;
	}

	return Elite::Success;
}

//------------------------------------------------------------------------------ENEMY RELATED

Elite::BehaviorState FaceTheEnemy(Elite::Blackboard* pBlackboard)
{
	//vars
	SteeringPlugin_Output* steering = {};
	EntityInfo enemy = {};
	AgentInfo agentInfo = {};

	auto dataAvailable = pBlackboard->GetData("Steering", steering) && pBlackboard->GetData("EnemyInFov", enemy) && pBlackboard->GetData("AgentInfo", agentInfo);

	if (!steering || !dataAvailable)
	{
		return Elite::Failure;
	}
	//vectors
	Elite::Vector2 desiredPos = enemy.Location - agentInfo.Position.GetNormalized();
	Elite::Vector2 playerDirection = Elite::OrientationToVector(agentInfo.Orientation).GetNormalized();

	//stop player and face enemy
	steering->LinearVelocity *= 0;
	steering->AutoOrient = false;

	if ((playerDirection - desiredPos).Magnitude() <= 0.0001) // if looking at enemy
	{
		steering->AngularVelocity = 0;
	}
	else if ((playerDirection - desiredPos).Magnitude() > 0.0001)
	{
		if (Cross(playerDirection, (enemy.Location - agentInfo.Position)) < 0) // turn to side where enemy is at
		{
			steering->AngularVelocity = -agentInfo.MaxAngularSpeed;
		}
		else if (Cross(playerDirection, (enemy.Location - agentInfo.Position)) > 0)
		{
			steering->AngularVelocity = agentInfo.MaxAngularSpeed;
		}
	}
	//move back when facing the enemy
	const int divider = 5;
	steering->LinearVelocity = (agentInfo.Position - enemy.Location).GetNormalized() * agentInfo.MaxLinearSpeed / divider;

	return Elite::Success;
}

Elite::BehaviorState AttackTheEnemyIfFacingIt(Elite::Blackboard* pBlackboard)
{
	//vars
	AgentInfo agentInfo = {};
	UINT pistolIndex = {};
	ItemInfo pistol = {};
	EntityInfo enemy = {};
	SteeringPlugin_Output* steering = {};
	IExamInterface* pInterface = nullptr;
	int filledInventoryAmount;

	auto dataAvailable = pBlackboard->GetData("Interface", pInterface) && pBlackboard->GetData("PistolIndex", pistolIndex)
		&& pBlackboard->GetData("EnemyInFov", enemy) && pBlackboard->GetData("Steering", steering) &&
		pBlackboard->GetData("AgentInfo", agentInfo) && pBlackboard->GetData("FilledInventoryAmount", filledInventoryAmount);;

	if (!pInterface || !steering || !dataAvailable)
	{
		return Elite::Failure;
	}
	//get the vectors
	Elite::Vector2 desiredPos = enemy.Location - agentInfo.Position;
	Elite::Vector2 playerDirection = Elite::OrientationToVector(agentInfo.Orientation).GetNormalized();

	if ((playerDirection - Elite::Vector2(desiredPos).GetNormalized()).Magnitude() <= 0.005f) // if looking at enemy
	{
		pInterface->Inventory_UseItem(pistolIndex); // shooting
		steering->LinearVelocity *= 0;

		if (pInterface->Inventory_GetItem(pistolIndex, pistol))
		{
			if (pInterface->Weapon_GetAmmo(pistol) == 0)
			{
				pInterface->Inventory_RemoveItem(pistolIndex); // remove if no ammo left
				filledInventoryAmount--;
				pBlackboard->ChangeData("FilledInventoryAmount", filledInventoryAmount);
			}
		}
	}

	return Elite::Success;
}

//------------------------------------------------------------------------------ITEM RELATED

Elite::BehaviorState PickUpItem(Elite::Blackboard* pBlackboard)
{
	//vars
	EntityInfo pickUpEntity = {};
	ItemInfo itemInfo = {};
	IExamInterface* pInterface = nullptr;
	int filledInventoryAmount;

	auto dataAvailable = pBlackboard->GetData("Interface", pInterface) && pBlackboard->GetData("ItemInFov", pickUpEntity) && pBlackboard->GetData("FilledInventoryAmount", filledInventoryAmount);

	if (!pInterface || !dataAvailable)
	{
		return Elite::Failure;
	}

	if (pInterface->Item_Grab(pickUpEntity, itemInfo))
	{
		for (size_t i = 0; i < pInterface->Inventory_GetCapacity(); i++)
		{
			if (pInterface->Inventory_AddItem(i, itemInfo))
			{
				if (itemInfo.Type == eItemType::GARBAGE)
				{
					pInterface->Inventory_RemoveItem(i); // remove if picked up item is garbage
				}
				else
				{
					filledInventoryAmount++;
					pBlackboard->ChangeData("FilledInventoryAmount", filledInventoryAmount);
				}
				return Elite::Success;
			}
		}
	}
	return Elite::Success;
}

Elite::BehaviorState GoToItem(Elite::Blackboard* pBlackboard)
{
	//vars
	SteeringPlugin_Output* steering = {};
	EntityInfo pickUpEntity = {};
	AgentInfo agentInfo = {};

	auto dataAvailable = pBlackboard->GetData("ItemInFov", pickUpEntity)
		&& pBlackboard->GetData("AgentInfo", agentInfo) && pBlackboard->GetData("Steering", steering);

	if (!steering || !dataAvailable)
	{
		return Elite::Failure;
	}

	//set target to item
	steering->LinearVelocity = pickUpEntity.Location - agentInfo.Position;
	steering->LinearVelocity.Normalize(); //Normalize desired Velocity
	steering->LinearVelocity *= agentInfo.MaxLinearSpeed; //Rescale to max speed

	return Elite::Success;
}

Elite::BehaviorState UseMedkit(Elite::Blackboard* pBlackboard)
{
	//vars
	AgentInfo agentInfo = {};
	IExamInterface* pInterface = nullptr;
	ItemInfo itemInfo = {};
	int filledInventoryAmount;

	auto dataAvailable = pBlackboard->GetData("AgentInfo", agentInfo) && pBlackboard->GetData("Interface", pInterface) && pBlackboard->GetData("FilledInventoryAmount", filledInventoryAmount);

	if (!pInterface || !dataAvailable)
	{
		return Elite::Failure;
	}

	for (size_t i = 0; i < pInterface->Inventory_GetCapacity(); i++)
	{
		if (pInterface->Inventory_GetItem(i, itemInfo))
		{
			if (itemInfo.Type == eItemType::MEDKIT)
			{
				pInterface->Inventory_UseItem(i);

				if (pInterface->Medkit_GetHealth(itemInfo) == 0)
				{
					pInterface->Inventory_RemoveItem(i);
					filledInventoryAmount--;
					pBlackboard->ChangeData("FilledInventoryAmount", filledInventoryAmount);
				}
			}
		}
	}

	return Elite::Success;
}

Elite::BehaviorState UseFood(Elite::Blackboard* pBlackboard)
{
	//vars
	AgentInfo agentInfo = {};
	IExamInterface* pInterface = nullptr;
	ItemInfo itemInfo = {};
	int filledInventoryAmount;

	auto dataAvailable = pBlackboard->GetData("AgentInfo", agentInfo) && pBlackboard->GetData("Interface", pInterface) && pBlackboard->GetData("FilledInventoryAmount", filledInventoryAmount);;

	if (!pInterface || !dataAvailable)
	{
		return Elite::Failure;
	}

	for (size_t i = 0; i < pInterface->Inventory_GetCapacity(); i++)
	{
		if (pInterface->Inventory_GetItem(i, itemInfo))
		{
			if (itemInfo.Type == eItemType::FOOD)
			{
				pInterface->Inventory_UseItem(i);

				if (pInterface->Food_GetEnergy(itemInfo) == 0)
				{
					pInterface->Inventory_RemoveItem(i);
					filledInventoryAmount--;
					pBlackboard->ChangeData("FilledInventoryAmount", filledInventoryAmount);
				}
			}
		}
	}

	return Elite::Success;
}

//------------------------------------------------------------------------------HOUSE RELATED

Elite::BehaviorState ExitHouse(Elite::Blackboard* pBlackboard)
{
	//vars
	SteeringPlugin_Output* steering = {};
	EntityInfo pickUpEntity = {};
	AgentInfo agentInfo = {};
	IExamInterface* pInterface = nullptr;
	Elite::Vector2 target{};
	//
	std::vector<Elite::Vector2> visitedHouseCenters = {};
	HouseInfo targetHouse = {};
	int filledInventoryAmount = {};

	auto dataAvailable = pBlackboard->GetData("Interface", pInterface) &&
		pBlackboard->GetData("AgentInfo", agentInfo) && pBlackboard->GetData("Steering", steering) && pBlackboard->GetData("FilledInventoryAmount", filledInventoryAmount);

	if (!steering || !pInterface || !dataAvailable)
	{
		return Elite::Failure;
	}

	//set target pos to center temporary to exit the house
	target = pInterface->NavMesh_GetClosestPathPoint(Elite::Vector2{ pInterface->World_GetInfo().Center.x , pInterface->World_GetInfo().Center.y + pInterface->World_GetInfo().Dimensions.y / 2.0f });
	steering->LinearVelocity = target - agentInfo.Position;
	steering->LinearVelocity.Normalize(); //Normalize desired Velocity
	steering->LinearVelocity *= agentInfo.MaxLinearSpeed; //Rescale to max speed
	steering->AutoOrient = true;
	steering->RunMode = false;
	if (!agentInfo.IsInHouse) // exited the house so put the has target bool to false
	{
		pBlackboard->ChangeData("HasTargetHouse", false);
		//
		if (filledInventoryAmount != 5) //if inventory full don't do it coz there still might be collectibles in the house
		{
			//put the house in the visited list
			pBlackboard->GetData("VisitedHousesCenterVec", visitedHouseCenters);
			pBlackboard->GetData("TargetHouse", targetHouse);
			visitedHouseCenters.push_back(targetHouse.Center);
			pBlackboard->ChangeData("VisitedHousesCenterVec", visitedHouseCenters);
		}
	}

	return Elite::Success;
}

Elite::BehaviorState EnterHouse(Elite::Blackboard* pBlackboard)
{
	//vars
	SteeringPlugin_Output* steering = {};
	AgentInfo agentInfo = {};
	IExamInterface* pInterface = nullptr;
	HouseInfo targetHouse = {};
	Elite::Vector2 target = {};
	auto dataAvailable = pBlackboard->GetData("Interface", pInterface) &&
		pBlackboard->GetData("AgentInfo", agentInfo) && pBlackboard->GetData("Steering", steering) && pBlackboard->GetData("TargetHouse", targetHouse);

	if (!steering || !pInterface || !dataAvailable)
	{
		return Elite::Failure;
	}

	//set target pos to the center of the house to enter it
	target = pInterface->NavMesh_GetClosestPathPoint(targetHouse.Center);
	steering->LinearVelocity = target - agentInfo.Position;
	steering->LinearVelocity.Normalize(); //Normalize desired Velocity
	steering->LinearVelocity *= agentInfo.MaxLinearSpeed; //Rescale to max speed
	steering->AutoOrient = true;
	steering->RunMode = false;

	return Elite::Success;
}

//------------------------------------------------------------------------------
