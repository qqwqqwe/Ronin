/***
 * Demonstrike Core
 */

#include "StdAfx.h"

AIInterface::AIInterface(Creature *creature, UnitPathSystem *unitPath, Unit *owner) : m_Creature(creature), m_path(unitPath), m_AISeed(RandomUInt()), m_waypointCounter(0), m_waypointMap(NULL),
m_AIState(creature->isAlive() ? AI_STATE_IDLE : AI_STATE_DEAD), // Initialize AI state idle if unit is not dead
m_AIFlags(AI_FLAG_NONE)
{

}

AIInterface::~AIInterface()
{

}

void AIInterface::Init()
{
    if(m_Creature->isTrainingDummy())
        m_AIFlags |= AI_FLAG_DISABLED;
}

void AIInterface::Update(uint32 p_time)
{
    if(m_AIState == AI_STATE_DEAD || m_AIFlags & AI_FLAG_DISABLED)
        return;

    if(m_waypointWaitTimer > p_time)
        m_waypointWaitTimer -= p_time;
    else m_waypointWaitTimer = 0;

    switch(m_AIState)
    {
    case AI_STATE_IDLE:
        {
            if(m_Creature->hasStateFlag(UF_EVADING))
            {
                if(!m_path->hasDestination())
                    m_Creature->clearStateFlag(UF_EVADING);
                return;
            }

            if(FindTarget() == false)
            {
                if(!m_path->hasDestination())
                {
                    if(m_pendingWaitTimer)
                    {
                        m_waypointWaitTimer = m_pendingWaitTimer;
                        m_pendingWaitTimer = 0;
                    }
                    else if(m_waypointWaitTimer == 0)
                        FindNextPoint();
                } // The update block should stop here
                return;
            }
        }
    case AI_STATE_COMBAT:
        {
            m_AIState = AI_STATE_COMBAT;
            _HandleCombatAI();
        }break;
    case AI_STATE_SCRIPT:
        {
            if(!m_targetGuid.empty())
                _HandleCombatAI();
        }break;
    }
}

void AIInterface::OnDeath()
{
    m_targetGuid.Clean();
    m_path->StopMoving();
    m_AIState = AI_STATE_DEAD;
    m_Creature->EventAttackStop();
    m_Creature->clearStateFlag(UF_EVADING);
}

void AIInterface::OnPathChange()
{

}

void AIInterface::OnTakeDamage(Unit *attacker, uint32 damage)
{
    if(m_AIFlags & AI_FLAG_DISABLED)
        return;

    if(m_targetGuid.empty() || m_AIState == AI_STATE_IDLE)
    {
        // On enter combat

    }

    // Set into combat AI State
    if(m_AIState <= AI_STATE_COMBAT)
        m_AIState = AI_STATE_COMBAT;
    // Set target guid for combat
    if(attacker && m_targetGuid.empty())
    {
        m_targetGuid = attacker->GetGUID();
        _HandleCombatAI();
    }
}

bool AIInterface::FindTarget()
{
    if(m_AIFlags & AI_FLAG_DISABLED)
        return false;
    if(m_Creature->hasStateFlag(UF_EVADING) || !m_targetGuid.empty() || !m_Creature->HasInrangeHostiles())
        return false;

    Unit *target = NULL;
    float baseAggro = m_Creature->GetAggroRange(), targetDist = 0.f;

    // Begin iterating through our inrange units
    for(WorldObject::InRangeSet::iterator itr = m_Creature->GetInRangeHostileSetBegin(); itr != m_Creature->GetInRangeHostileSetEnd(); itr++)
    {
        if(Unit *unitTarget = m_Creature->GetInRangeObject<Unit>(*itr))
        {   // Cut down on checks by skipping dead creatures
            if(unitTarget->isDead())
                continue;
            float dist = m_Creature->GetDistanceSq(unitTarget);
            float aggroRange = unitTarget->ModDetectedRange(m_Creature, baseAggro);
            aggroRange *= aggroRange; // Distance is squared so square our range
            if(dist >= MAX_COMBAT_MOVEMENT_DIST || dist >= aggroRange)
                continue;
            if(target && targetDist <= dist)
                continue;
            if(!sFactionSystem.CanEitherUnitAttack(m_Creature, unitTarget))
                continue;
            // LOS is a big system hit so do it last
            if(!m_Creature->IsInLineOfSight(unitTarget))
                continue;

            target = unitTarget;
            targetDist = dist;
        }
    }

    if(target)
    {
        m_targetGuid = target->GetGUID();
        return true;
    }
    return false;
}

void AIInterface::FindNextPoint()
{
    // Waiting before processing next waypoint
    if(m_waypointWaitTimer)
        return;

    if(m_waypointMap && !m_waypointMap->empty())
    {   // Process waypoint map travel

        return;
    }

    float distance = m_Creature->GetDistanceSq(m_Creature->GetSpawnX(), m_Creature->GetSpawnY(), m_Creature->GetSpawnZ());

    if(false)//m_Creature->HasRandomMovement())
    {
        if(distance < MAX_RANDOM_MOVEMENT_DIST)
        {   // Process random movement point generation

            return;
        } // Else we just return to spawn point and generate a random point then
    }
    if(distance == 0.f)
        return;

    // Move back to our original spawn point
    m_path->MoveToPoint(m_Creature->GetSpawnX(), m_Creature->GetSpawnY(), m_Creature->GetSpawnZ(), m_Creature->GetSpawnO());
}

void AIInterface::_HandleCombatAI()
{
    bool tooFarFromSpawn = false;
    Unit *unitTarget = m_Creature->GetInRangeObject<Unit>(m_targetGuid);
    if (unitTarget == NULL || unitTarget->isDead() || (tooFarFromSpawn = (unitTarget->GetDistance2dSq(m_Creature->GetSpawnX(), m_Creature->GetSpawnY()) > MAX_COMBAT_MOVEMENT_DIST))
        || !sFactionSystem.CanEitherUnitAttack(m_Creature, unitTarget))
    {
        // If we already have a target but he doesn't qualify back us out of combat
        if(unitTarget)
        {
            m_path->StopMoving();
            m_targetGuid.Clean();
            m_Creature->EventAttackStop();
        }

        if(tooFarFromSpawn || FindTarget() == false)
        {
            m_AIState = AI_STATE_IDLE;
            if(!m_path->hasDestination())
            {
                m_Creature->addStateFlag(UF_EVADING);
                FindNextPoint();
            }
            return;
        }
    }

    SpellEntry *sp = NULL;
    float attackRange = 0.f, minRange = 0.f, x, y, z, o;
    if (m_Creature->calculateAttackRange(m_Creature->GetPreferredAttackType(&sp), minRange, attackRange, sp))
        attackRange *= 0.8f; // Cut our attack range down slightly to prevent range issues

    m_Creature->GetPosition(x, y, z);
    m_path->GetDestination(x, y);
    if (unitTarget->GetDistance2dSq(x, y) >= attackRange*attackRange)
    {
        unitTarget->GetPosition(x, y, z);
        o = m_Creature->calcAngle(m_Creature->GetPositionX(), m_Creature->GetPositionY(), x, y) * M_PI / 180.f;
        x -= attackRange * cosf(o);
        y -= attackRange * sinf(o);
        m_path->MoveToPoint(x, y, z, o);
    } else m_Creature->SetOrientation(m_Creature->GetAngle(unitTarget));

    if(!m_Creature->ValidateAttackTarget(m_targetGuid))
        m_Creature->EventAttackStart(m_targetGuid);
}