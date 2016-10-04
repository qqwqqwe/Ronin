/***
* Demonstrike Core
*/

#include "StdAfx.h"

bool PaladinBlessingApplicator(SpellEntry *sp, uint32 effIndex, WorldObject *caster, WorldObject *target, int32 &amount)
{
    Group *grp = NULL;
    uint32 triggerSpell = (sp->Id == 19740 ? 79101 : 79062);
    if(caster->IsPlayer() && target->IsPlayer() && (grp = castPtr<Player>(caster)->GetGroup()) && grp->HasMember(castPtr<Player>(target)))
    {
        grp->Lock();
        GroupMembersSet::iterator itr;
        for(uint8 i = 0; i < grp->GetSubGroupCount(); i++)
            for(itr = grp->GetSubGroup(i)->GetGroupMembersBegin(); itr != grp->GetSubGroup(i)->GetGroupMembersEnd(); itr++)
                if((*itr)->m_loggedInPlayer && caster->IsInRangeSet((*itr)->m_loggedInPlayer))
                    caster->CastSpell((*itr)->m_loggedInPlayer, triggerSpell, true);
        grp->Unlock();
    } else if(target->IsUnit())
        caster->CastSpell(target, triggerSpell, true);
    return true;
}

bool PaladinJudgementDummyHandler(SpellEntry *sp, uint32 effIndex, WorldObject *caster, WorldObject *target, int32 &amount)
{
    Unit *unitCaster = caster->IsUnit() ? castPtr<Unit>(caster) : NULL, *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if(unitCaster && unitTarget)
    {
        SpellEntry *sealTrigger = NULL;
        // Insight and justice trigger the same seal release
        if(unitCaster->m_AuraInterface.FindActiveAuraWithNameHash(SPELL_HASH_SEAL_OF_INSIGHT) || unitCaster->m_AuraInterface.FindActiveAuraWithNameHash(SPELL_HASH_SEAL_OF_JUSTICE))
            sealTrigger = dbcSpell.LookupEntry(54158);
        else if(unitCaster->m_AuraInterface.FindActiveAuraWithNameHash(SPELL_HASH_SEAL_OF_RIGHTEOUSNESS))
            sealTrigger = dbcSpell.LookupEntry(20187);
        else if(unitCaster->m_AuraInterface.FindActiveAuraWithNameHash(SPELL_HASH_SEAL_OF_TRUTH))
            sealTrigger = dbcSpell.LookupEntry(31804);

        if(sealTrigger && sealTrigger != sp)
            unitCaster->CastSpell(unitTarget, sealTrigger, true);
    }
    return true;
}

void PaladinJudgementTriggerDamageHandler(SpellEntry *sp, uint32 effIndex, WorldObject *caster, Unit *target, int32 &amount)
{
    if(Unit *unitCaster = caster->IsUnit() ? castPtr<Unit>(caster) : NULL)
    {
        switch(sp->Id)
        {
            // Righteousness is a 0.32 and 0.2 coefficient
        case 20187: amount += float2int32(((float)unitCaster->GetDamageDoneMod(SCHOOL_HOLY))*0.32f + ((float)unitCaster->GetAttackPower())*0.2f); break;
            // Truth is a 0.22 and 0.14 coefficient
        case 31804: amount += float2int32(((float)unitCaster->GetDamageDoneMod(SCHOOL_HOLY))*0.22f + ((float)unitCaster->GetAttackPower())*0.14f); break;
            // Raw judgement is a 0.25 and 0.16 coefficient
        case 54158: amount += float2int32(((float)unitCaster->GetDamageDoneMod(SCHOOL_HOLY))*0.25f + ((float)unitCaster->GetAttackPower())*0.16f); break;
        }
    }

    Aura *censure = NULL;
    if(sp->Id == 31804 && (censure = target->m_AuraInterface.FindActiveAuraWithNameHash(SPELL_HASH_CENSURE)))
        amount += ((float)amount) * 0.2f * censure->getStackSize();
}

void SpellManager::_RegisterPaladinFixes()
{
    // Register applicator for blessing of might and blessing of kings
    _RegisterDummyEffect(19740, SP_EFF_INDEX_0, PaladinBlessingApplicator);
    _RegisterDummyEffect(20217, SP_EFF_INDEX_0, PaladinBlessingApplicator);

    // Register the dummy handler for casting judgement spells
    _RegisterDummyEffect(20271, SP_EFF_INDEX_0, PaladinJudgementDummyHandler);

    // Register the damage modifier for judgement triggers
    _RegisterDamageEffect(20187, SP_EFF_INDEX_0, PaladinJudgementTriggerDamageHandler);
    _RegisterDamageEffect(31804, SP_EFF_INDEX_0, PaladinJudgementTriggerDamageHandler);
    _RegisterDamageEffect(54158, SP_EFF_INDEX_0, PaladinJudgementTriggerDamageHandler);
}