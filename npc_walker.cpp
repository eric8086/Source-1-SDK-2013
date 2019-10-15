//============== FOR USE IN VALVe Source 1 SDK 2013=================//
// 
// Purpose:  Prototype Zombie NPC that
//         - Follows player
//         - Melee attacks prop_physics, func_breakable obstacles
//         - Latch onto player to eat (Damage code unfinished. No animation)
//         - Acid vomit attack at player (Unfinished range attack)
//               
//                   !! Prototype !!
//
// SOURCE 1 SDK LICENSE
//
// Source SDK Copyright(c) Valve Corp.
//
// THIS DOCUMENT DESCRIBES A CONTRACT BETWEEN YOU AND VALVE
// CORPORATION("Valve").PLEASE READ IT BEFORE DOWNLOADING OR USING
// THE SOURCE ENGINE SDK("SDK").BY DOWNLOADING AND / OR USING THE
// SOURCE ENGINE SDK YOU ACCEPT THIS LICENSE.IF YOU DO NOT AGREE TO
// THE TERMS OF THIS LICENSE PLEASE DON’T DOWNLOAD OR USE THE SDK.
//
// You may, free of charge, download and use the SDK to develop a modified Valve game
// running on the Source engine.You may distribute your modified Valve game in source and
// object code form, but only for free.Terms of use for Valve games are found in the Steam
// Subscriber Agreement located here : http://store.steampowered.com/subscriber_agreement/ 
//
// You may copy, modify, and distribute the SDK and any modifications you make to the
// SDK in source and object code form, but only for free.Any distribution of this SDK must
// include this LICENSE file and thirdpartylegalnotices.txt.
//
// Any distribution of the SDK or a substantial portion of the SDK must include the above
// copyright notice and the following :
//
//  DISCLAIMER OF WARRANTIES.THE SOURCE SDK AND ANY
//  OTHER MATERIAL DOWNLOADED BY LICENSEE IS PROVIDED
//  "AS IS".VALVE AND ITS SUPPLIERS DISCLAIM ALL
//  WARRANTIES WITH RESPECT TO THE SDK, EITHER EXPRESS
//  OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED
//  WARRANTIES OF MERCHANTABILITY, NON - INFRINGEMENT,
//  TITLE AND FITNESS FOR A PARTICULAR PURPOSE.
//
//  LIMITATION OF LIABILITY.IN NO EVENT SHALL VALVE OR
//  ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL,
//  INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
//  (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF
//	BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
//	BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
//	ARISING OUT OF THE USE OF OR INABILITY TO USE THE
//	ENGINE AND / OR THE SDK, EVEN IF VALVE HAS BEEN
//	ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
//
//
//	If you would like to use the SDK for a commercial purpose, please contact Valve at
//	sourceengine@valvesoftware.com.
//=============================================================================//
#include "cbase.h"
#include "ai_default.h"
#include "ai_task.h"
#include "ai_schedule.h"
#include "ai_hull.h"
#include "movevars_shared.h"
#include "soundent.h"
#include "game.h"
#include "npcevent.h"
#include "entitylist.h"
#include "activitylist.h"
#include "ai_basenpc.h"
#include "engine/IEngineSound.h"
#include "vphysics/constraints.h"

// !
#include "grenade_vomit.h"
#include "props.h"
#include "func_break.h"
#include "simtimer.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define WALKER_MELEE_ATTACK1_REACH 35


ConVar	sk_walker_health("sk_walker_health", "100");
ConVar  g_debug_walker("g_debug_walker", "0");
ConVar  sk_walker_vomit_speed("sk_walker_vomit_speed", "0", FCVAR_NONE, "Speed at which a walker vomit travels.");



//=========================================================
// Private activities
//=========================================================
int	ACT_NOTDONE = -1;



int AE_WALKER_ACID_VOMIT;


//=========================================================
//=========================================================
class CNPC_Walker : public CAI_BaseNPC
{

public:
	DECLARE_CLASS(CNPC_Walker, CAI_BaseNPC);
	void            TestRangeToVomit(void);
	void            LastGrabbedRestoreMovement(void);
	bool            TestRangeToGrab(CHandle<CBasePlayer> LastPlayerToEat);
	int				MeleeAttack1Conditions(float flDot, float flDist);
	float   m_flEatRange = 49.0;
	void	Precache(void);
	void	Spawn(void);
	//void	HandleAnimEvent(animevent_t *pEvent);
	int     SelectSchedule(void);
	void    GatherConditions(void);
	void    RunTask(const Task_t *pTask);
	void    StartTask(const Task_t *pTask);
	Class_T Classify(void);
	virtual int TranslateSchedule(int scheduleType);
	virtual void Event_Killed(const CTakeDamageInfo &info);
	bool		OnObstructionPreSteer(AILocalMoveGoal_t *pMoveGoal, float distClear, AIMoveResult_t *pResult);
	void		HandleAnimEvent(animevent_t *pEvent);
	Activity		NPC_TranslateActivity(Activity eNewActivity);
	DECLARE_DATADESC();

	//Vector VecCheckThrowTolerance(CBaseEntity *pEdict, const Vector &vecSpot1, Vector vecSpot2, float flSpeed, float flTolerance);

	// This is a dummy field. In order to provide save/restore
	// code in this file, we must have at least one field
	// for the code to operate on. Delete this field when
	// you are ready to do your own save/restore for this
	// character.
	int		m_iNotDone;
	DEFINE_CUSTOM_AI;
	

private:
	//virtual bool GetSpitVector(const Vector &vecStartPos, const Vector &vecTarget, Vector *vecOut);

	float m_flGrabTime;
	float m_flVomitTime;
	float m_flResetCountToVomit;
	CHandle<CBaseEntity>	m_hBlockingProp;
	CHandle<CBasePlayer>    m_hLastEnemyToEat;
	IPhysicsConstraint			*m_pConstraint;
	CSimTimer               m_DurationWaitToScanObstacles;
	void OnAnimEventShove(void);
	float m_flCountToVomit;


public:
	enum
	{
		SCHED_WALKER_WALK = BaseClass::NEXT_SCHEDULE,
		SCHED_WALKER_CHASE_ENEMY,
		SCHED_WALKER_SMASH_PROP,
		SCHED_WALKER_GRAB_PLAYER,
		SCHED_WALKER_GRAB_PLAYER_LOOP,
		SCHED_WALKER_ATTACK_VOMIT,
		SCHED_WALKER_ATTACK_THROW,
	};

	enum
	{
		TASK_WALKER_GRAB_LOOP = BaseClass::NEXT_TASK,
		TASK_WALKER_SMASH_PROP,
		TASK_WALKER_ATTACK_VOMIT,
	};

	enum
	{
		COND_WALKER_BLOCKED_BY_PROP = BaseClass::NEXT_CONDITION,
		COND_WALKER_SHOVED,
		COND_WALKER_GRAB_LOOP,
		COND_WALKER_ATTACK_VOMIT,
		COND_WALKER_ATTACK_THROW,
	};

};

int AE_WALKER_GRAB_ENTRY;
int AE_WALKER_GRAB_LOOP;
//=========================================================
// Walker Activities
//=========================================================
int ACT_WALKER_GRAB_LOOP;
int ACT_WALKER_GRAB_ENTRY;


LINK_ENTITY_TO_CLASS(npc_walker, CNPC_Walker);
///IMPLEMENT_CUSTOM_AI(npc_citizen, CNPC_Walker);


//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC(CNPC_Walker)
//DEFINE_FIELD(m_hLastEnemyToEat, FIELD_EHANDLE),
DEFINE_FIELD(m_iNotDone, FIELD_INTEGER),
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: -
// Input  :
// Output :
//-----------------------------------------------------------------------------

void CNPC_Walker::GatherConditions()
{


	BaseClass::GatherConditions();
}

int CNPC_Walker::SelectSchedule()
{

	if (!m_hBlockingProp) {

		ClearCondition(COND_WALKER_BLOCKED_BY_PROP);

	}

	if (HasCondition(COND_NEW_ENEMY) && !HasCondition(COND_CAN_MELEE_ATTACK1)
		&& !m_hBlockingProp) {
		
		return SCHED_WALKER_CHASE_ENEMY;
	
	}

	if (HasCondition(COND_CAN_MELEE_ATTACK1)) {
	
		return SCHED_WALKER_GRAB_PLAYER;
	}
	
	if (HasCondition(COND_WALKER_BLOCKED_BY_PROP) && m_hBlockingProp) {
			SetTarget(m_hBlockingProp);
			//m_hBlockingProp = NULL;
			return SCHED_WALKER_SMASH_PROP;
	}

	// Player might be unreachable (like standing on car roof)
	// Lets use range attacks
	if (HasCondition(COND_WALKER_ATTACK_VOMIT))
	{
		Msg("Gonna vomit\n");
		return SCHED_WALKER_ATTACK_VOMIT;
	}
     
	if (HasCondition(COND_WALKER_ATTACK_THROW))
	{

	}
	
	
	    DevMsg("Going to base schedule\n");
		return BaseClass::SelectSchedule();
	
}


void CNPC_Walker::StartTask(const Task_t *pTask) {

	switch (pTask->iTask) {
	
	case TASK_WALKER_ATTACK_VOMIT:
	{
		SetActivity(ACT_IDLE_ANGRY);
		break;
	}
	case TASK_WALKER_GRAB_LOOP:
	{
		
		SetActivity((Activity)ACT_WALKER_GRAB_LOOP);

		break;
	}

	case TASK_WALKER_SMASH_PROP:
	{
		SetActivity(ACT_MELEE_ATTACK_SWING);

		break;
	}
	
	default:
	{
		BaseClass::StartTask(pTask);
	}
	}
	


}

void CNPC_Walker::RunTask(const Task_t *pTask) {
	
	switch (pTask->iTask) {

	case TASK_WALKER_ATTACK_VOMIT:
	{

		if (IsActivityFinished())
		{
			// Play animation again
			ResetIdealActivity(ACT_IDLE_ANGRY);
		}
		break;
	}
	case TASK_WALKER_GRAB_LOOP:
	{
		// Test if there still in front of me to Grab
		if (TestRangeToGrab(m_hLastEnemyToEat) == false)
		{
			TaskComplete();
			LastGrabbedRestoreMovement();
		}

			if (IsActivityFinished())
			{
				// Play animation again
				ResetIdealActivity((Activity)ACT_WALKER_GRAB_LOOP);
			}
		
		break;
	}
	
	case TASK_WALKER_SMASH_PROP:
	{
		if (!m_hBlockingProp)
		{
			TaskComplete();
		}

		if (IsActivityFinished())
		{
			ResetIdealActivity(ACT_MELEE_ATTACK_SWING);
		}
		break;
	}

	default:
	
		BaseClass::RunTask(pTask);
	}

}
//-----------------------------------------------------------------------------
// Purpose: Restore back player movement to whom we attempted to Eat
//-----------------------------------------------------------------------------
void CNPC_Walker::LastGrabbedRestoreMovement(void)
{
	if(m_hLastEnemyToEat != NULL){
		if (!m_hLastEnemyToEat->IsNPC()) {

			//TODO: Get rid of unrealistic stored kick/punch velocity while also preserving it if
			// both the player and walker died in an explosion

			m_hLastEnemyToEat->SetMoveType(MOVETYPE_WALK);
			m_hLastEnemyToEat = NULL;
		}
	}
}

int CNPC_Walker::TranslateSchedule(int scheduleType)
{
	switch (scheduleType)
	{
		case SCHED_ESTABLISH_LINE_OF_FIRE:
		{
			return SCHED_WALKER_CHASE_ENEMY;
			break;
		}
		case SCHED_CHASE_ENEMY:
		{
			return SCHED_WALKER_CHASE_ENEMY;
			break;
		}
	
		default:
		{
			return BaseClass::TranslateSchedule(scheduleType);
			break;
		}
	}
}
//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CNPC_Walker::Precache(void)
{
	PrecacheModel("models/humans/group01/female_01.mdl");


	UTIL_PrecacheOther("grenade_vomit");
	PrecacheScriptSound("NPC_Barnacle.FinalBite");
	//PrecacheScriptSound("NPC_Antlion.PoisonShoot");
	//PrecacheScriptSound("NPC_Antlion.PoisonBall");

	BaseClass::Precache();
}


enum SuggestedRange
{
	DEFAULT_RANGE = 20,
	WARM_RANGE = 50,
	HOT_RANGE = 150,

};
//-----------------------------------------------------------------------------
// Purpose: Detect for Physics obstacle conditions where the walker should smash things, vomit attack, throw rocks
//-----------------------------------------------------------------------------
bool CNPC_Walker::OnObstructionPreSteer(AILocalMoveGoal_t *pMoveGoal, float distClear, AIMoveResult_t *pResult)
{


	
	DevMsg("Being obstructed: %f\n", m_flCountToVomit);
	m_flCountToVomit += 1;
    // Player may be unreachable standing on an obstruction
	// When 20 counts of obstruction. Test conditions to range attack vomit
	if (m_flCountToVomit > DEFAULT_RANGE)
	{
		TestRangeToVomit();
		m_flCountToVomit = 0;
		return BaseClass::OnObstructionPreSteer(pMoveGoal, distClear, pResult);
	}


	// Smash smash
	if (pMoveGoal->directTrace.pObstruction)
	{
		// What is blocking us? 
		// Need to add doors to this
		CPhysicsProp *pProp = dynamic_cast<CPhysicsProp*>(pMoveGoal->directTrace.pObstruction);
		CBreakable   *pBreakable = dynamic_cast< CBreakable* >(pMoveGoal->directTrace.pObstruction);
	

		if (pProp && pProp->GetHealth() > 0)
		{
			m_hBlockingProp = pProp;
			// Node may get marked bits_LINK_STALE_SUGGESTED 
			// This might help avoid that
			*pResult = AIMR_OK; 
			GetNavigator()->ClearNavFailCounter();
			SetCondition(COND_WALKER_BLOCKED_BY_PROP);

		}
		else if (pBreakable && pBreakable->GetHealth() > 0) {
			// Duplicate code, refactor 
			GetNavigator()->ClearNavFailCounter();
			*pResult = AIMR_OK;
			m_hBlockingProp = pBreakable;
			SetCondition(COND_WALKER_BLOCKED_BY_PROP);
			
		} else {

			// Must not be something the NPC can break.
			m_hBlockingProp = NULL;

		}

	}

	return BaseClass::OnObstructionPreSteer(pMoveGoal, distClear, pResult);
}


//-----------------------------------------------------------------------------
// Purpose: Override base class activiites
//-----------------------------------------------------------------------------
Activity CNPC_Walker::NPC_TranslateActivity(Activity activity) {



	return BaseClass::NPC_TranslateActivity(activity);

}

void CNPC_Walker::TestRangeToVomit(void)
{
	
	// Want to check how far away the enemy is
	float flDist = GetEnemy()->WorldSpaceCenter().DistTo(WorldSpaceCenter());

	if (flDist <= 100) {

		// Then  check how far below or above my eyes the Enemy is
		SetCondition(COND_WALKER_ATTACK_VOMIT);
	}
	
}


//-----------------------------------------------------------------------------
// Purpose: Test if this walker is close enough to player to grab them while Standing
//-----------------------------------------------------------------------------
bool CNPC_Walker::TestRangeToGrab(CHandle<CBasePlayer> LastPlayerToEat)
{
	if (!LastPlayerToEat) {
		AssertMsg( 0, "Expected a CHandle to a player in CNPC_Walker::TestRangeToGrab!\n");
		return false;
	}

	float flDist = LastPlayerToEat->WorldSpaceCenter().DistTo(WorldSpaceCenter());

	if (flDist >= WALKER_MELEE_ATTACK1_REACH) {

		return false;
	}
	else {

		return true;
	}

}
//-----------------------------------------------------------------------------
// Purpose: Handle Animation "Event AE_NAME #Frame" in studiomdls
//-----------------------------------------------------------------------------
void CNPC_Walker::HandleAnimEvent(animevent_t *pEvent)
{

	// Walker has no actual weapon entity to handle this, so do damage here
	if (pEvent->event == 3001) // 3001 = AE_MELEE_ATTACK_SWING
	{
		OnAnimEventShove();
		return;
	}

	// We are midway through the grab animation
	// Test if the enemy is still close enough or moved
	if (pEvent->event == AE_WALKER_GRAB_ENTRY) {

		// Get CHandle to Enemy
		m_hLastEnemyToEat = static_cast<CBasePlayer*>(GetEnemy());

		// Can't support grabbing NPCs yet.
		if (m_hLastEnemyToEat->IsNPC())
		{
			
			SetSequenceByName("stopwoman");
			TaskComplete();
			return;
		}
		
		
		// We know its not a NPC now, test now for Distance
		if (m_hLastEnemyToEat && TestRangeToGrab(m_hLastEnemyToEat)) {

			// if player is on ladder, disengage
			if (m_hLastEnemyToEat->GetMoveType() == MOVETYPE_LADDER)
			{
				m_hLastEnemyToEat->ExitLadder();

			}

			// HACKHACK: Constrain them
			// Need a better way then this that works on NPCs..
			m_hLastEnemyToEat->SetMoveType(MOVETYPE_NONE);

			// Begin Eating loop
			SetSchedule(SCHED_WALKER_GRAB_PLAYER_LOOP);

		}
		else {
			// The Grab missed!
			SetSequenceByName("stopwoman");
			TaskComplete();
		}
			
			
		
		return;
	}

	if (pEvent->event == AE_WALKER_GRAB_LOOP) {

		EmitSound("NPC_barnacle.FinalBite");
		return;
	}
	
	BaseClass::HandleAnimEvent(pEvent);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Walker::OnAnimEventShove(void)
{
	CBaseEntity *pHurt = CheckTraceHullAttack(16, Vector(-16, -16, -16), Vector(16, 16, 16), 15, DMG_CLUB, 1.0f, false);

	if (pHurt)
	{
		Vector vecForceDir = (pHurt->WorldSpaceCenter() - WorldSpaceCenter());

		CBasePlayer *pPlayer = ToBasePlayer(pHurt);

		if (pPlayer != NULL)
		{
			//Kick the player angles
			pPlayer->ViewPunch(QAngle(8, 14, 0));

			Vector	dir = pHurt->GetAbsOrigin() - GetAbsOrigin();
			VectorNormalize(dir);

			QAngle angles;
			VectorAngles(dir, angles);
			Vector forward, right;
			AngleVectors(angles, &forward, &right, NULL);

			//If not on ground, then don't make them fly!
			if (!(pHurt->GetFlags() & FL_ONGROUND))
				forward.z = 0.0f;

			//Push the target back
			pHurt->ApplyAbsVelocityImpulse(forward * 250.0f);

			// Force the player to drop anyting they were holding
			pPlayer->ForceDropOfCarriedPhysObjects();
		}

		// Play a random attack hit sound
		EmitSound("NPC_Metropolice.Shove");
	}
}

int CNPC_Walker::MeleeAttack1Conditions(float flDot, float flDist)
{

					if (flDist <= WALKER_MELEE_ATTACK1_REACH)
					{
						return COND_CAN_MELEE_ATTACK1;
					}

					if (flDot < 0.7)
					{
						return COND_NOT_FACING_ATTACK;
					}
				
		return COND_TOO_FAR_TO_ATTACK;
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CNPC_Walker::Spawn(void)
{
	Precache();

	SetModel("models/humans/group01/female_01.mdl");
	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);
	SetBloodColor(BLOOD_COLOR_RED);
	m_flFieldOfView = 0.5;
	m_NPCState = NPC_STATE_NONE;
	m_flCountToVomit = 0;
	SetNavType(NAV_GROUND);


	CapabilitiesClear();
	//CapabilitiesAdd( bits_CAP_NONE );

	//CapabilitiesAdd(bits_CAP_INNATE_RANGE_ATTACK1);
	CapabilitiesAdd(bits_CAP_INNATE_MELEE_ATTACK1);
	CapabilitiesAdd(bits_CAP_TURN_HEAD);
	CapabilitiesAdd(bits_CAP_MOVE_GROUND);
	m_iHealth = sk_walker_health.GetFloat();
	m_flFieldOfView = -0.2;
	m_hLastEnemyToEat = NULL;
	m_NPCState = NPC_STATE_NONE;

	NPCInit();

}

void CNPC_Walker::Event_Killed(const CTakeDamageInfo &info)
{
	// We are dead, let go of player/NPC if we're eating them
	LastGrabbedRestoreMovement();

	BaseClass::Event_Killed(info);
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
// Output : 
//-----------------------------------------------------------------------------
Class_T	CNPC_Walker::Classify(void)
{
	return	CLASS_WALKER;
}


AI_BEGIN_CUSTOM_NPC(npc_walker, CNPC_Walker)
DECLARE_ANIMEVENT(AE_WALKER_GRAB_ENTRY);
DECLARE_ANIMEVENT(AE_WALKER_GRAB_LOOP);
DECLARE_ACTIVITY(ACT_WALKER_GRAB_ENTRY);
DECLARE_ACTIVITY(ACT_WALKER_GRAB_LOOP);
DECLARE_CONDITION(COND_WALKER_SHOVED);
DECLARE_CONDITION(COND_WALKER_BLOCKED_BY_PROP);
DECLARE_CONDITION(COND_WALKER_ATTACK_VOMIT);
DECLARE_CONDITION(COND_WALKER_ATTACK_THROW);
DECLARE_TASK(TASK_WALKER_GRAB_LOOP);
DECLARE_TASK(TASK_WALKER_SMASH_PROP);
DECLARE_TASK(TASK_WALKER_ATTACK_VOMIT);



//=========================================================
// > ChaseEnemy
//=========================================================
DEFINE_SCHEDULE
(
	SCHED_WALKER_CHASE_ENEMY,

	"	Tasks"
	"		TASK_STOP_MOVING				0"
	"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_ALERT_STAND"
	"		TASK_SET_TOLERANCE_DISTANCE		28"
	"		TASK_GET_CHASE_PATH_TO_ENEMY	300"
	"		TASK_SPEAK_SENTENCE				6"	// METROPOLICE_SENTENCE_MOVE_INTO_POSITION
	"		TASK_WALK_PATH					0"
	//"		TASK_METROPOLICE_RESET_LEDGE_CHECK_TIME 0"
	"		TASK_WAIT_FOR_MOVEMENT			0"
	"		TASK_FACE_ENEMY					0"
	"	"
	"	Interrupts"
	"		COND_NEW_ENEMY"
	"       COND_WALKER_BLOCKED_BY_PROP"
	"       COND_CAN_MELEE_ATTACK1"

);

//=========================================================
// > Pre Grab
//=========================================================
DEFINE_SCHEDULE
(
	SCHED_WALKER_GRAB_PLAYER,

	"	Tasks"
	"		TASK_STOP_MOVING		0"
	"		TASK_FACE_ENEMY			0"
	"		TASK_ANNOUNCE_ATTACK	1"	// 1 = primary attack
	"		TASK_PLAY_SEQUENCE			ACTIVITY:ACT_WALKER_GRAB_ENTRY"
	""
	"	Interrupts"
	"		COND_NEW_ENEMY"
	"		COND_ENEMY_DEAD"
	"		COND_HEAVY_DAMAGE"
//	"		COND_ENEMY_OCCLUDED"
);


//=========================================================
// > Post Grab
//=========================================================
DEFINE_SCHEDULE
(
	SCHED_WALKER_GRAB_PLAYER_LOOP,

	"	Tasks"
	"       TASK_FACE_ENEMY	             0"
	"		TASK_WALKER_GRAB_LOOP		 0"
	""
	"	Interrupts"
	"		COND_ENEMY_DEAD"
	"       COND_WALKER_SHOVED"
	"       COND_WALKER_BLOCKED_BY_PROP"
	"		COND_ENEMY_OCCLUDED"
	"		COND_TOO_FAR_TO_ATTACK"
);

DEFINE_SCHEDULE
(
	SCHED_WALKER_ATTACK_VOMIT,

	"	Tasks"
	"		TASK_FACE_ENEMY		         0"
	"		TASK_WALKER_ATTACK_VOMIT     0"
	""
	"	Interrupts"
	//"		COND_NEW_ENEMY"
	"		COND_ENEMY_DEAD"
);

DEFINE_SCHEDULE
(
	SCHED_WALKER_SMASH_PROP,

	"	Tasks"
	"		TASK_GET_PATH_TO_TARGET		0"
	"		TASK_MOVE_TO_TARGET_RANGE	50"
	"		TASK_STOP_MOVING			0"
	"		TASK_FACE_TARGET			0"
	"		TASK_ANNOUNCE_ATTACK		1"	// 1 = primary attack
	"		TASK_WALKER_SMASH_PROP		0"
	""
	"	Interrupts"
	"		COND_NEW_ENEMY"
	"		COND_ENEMY_DEAD"
);

AI_END_CUSTOM_NPC()
