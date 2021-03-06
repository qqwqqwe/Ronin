/*
 * Sandshroud Project Ronin
 * Copyright (C) 2005-2008 Ascent Team <http://www.ascentemu.com/>
 * Copyright (C) 2008-2009 AspireDev <http://www.aspiredev.org/>
 * Copyright (C) 2009-2017 Sandshroud <https://github.com/Sandshroud>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

class Group;
class Player;
class Arena;
class SpellTargetClass;
struct PlayerInfo;

enum PartyErrors
{
    ERR_PARTY_NO_ERROR                  = 0,
    ERR_PARTY_CANNOT_FIND               = 1,
    ERR_PARTY_IS_NOT_IN_YOUR_PARTY      = 2,
    ERR_PARTY_UNK                       = 3,
    ERR_PARTY_IS_FULL                   = 4,
    ERR_PARTY_ALREADY_IN_GROUP          = 5,
    ERR_PARTY_YOU_ARENT_IN_A_PARTY      = 6,
    ERR_PARTY_YOU_ARE_NOT_LEADER        = 7,
    ERR_PARTY_WRONG_FACTION             = 8,
    ERR_PARTY_IS_IGNORING_YOU           = 9,
};

enum GroupTypes
{
    GROUP_TYPE_PARTY    = 0x00,
    GROUP_TYPE_BG       = 0x01,
    GROUP_TYPE_RAID     = 0x02,
    GROUP_TYPE_BGRAID   = GROUP_TYPE_BG | GROUP_TYPE_RAID,
    GROUP_TYPE_LFD      = 0x04 | 0x08,
    GROUP_TYPE_MASK     = 0x0F,
    // If we have either of these flags we're secondary
    GROUP_TYPE_SECONDARY= GROUP_TYPE_BG | GROUP_TYPE_LFD,

    GROUP_TYPE_HEROIC   = 0x20,
};

enum MaxGroupCount
{
    MAX_GROUP_SIZE_PARTY    = 5,
    MAX_GROUP_SIZE_RAID     = 40,
};

enum QuickGroupUpdateFlags
{
    PARTY_UPDATE_FLAG_LOCATION          = 1,
//  GROUP_UPDATE_FLAG_VEHICLE_SEAT
};

enum GroupFlags
{
    GROUP_FLAG_DONT_DISBAND_WITH_NO_MEMBERS         = 1,
    GROUP_FLAG_REMOVE_OFFLINE_PLAYERS               = 2,
    GROUP_FLAG_BATTLEGROUND_GROUP                   = 4,
};

typedef struct
{
    PlayerInfo * player_info;
    Player* player;
}GroupMember;

typedef std::set<PlayerInfo*> GroupMembersSet;

class SERVER_DECL SubGroup    // Most stuff will be done through here, not through the "Group" class.
{
public:
    friend class Group;

    SubGroup(Group* parent, uint32 id):m_Parent(parent),m_Id(id)
    {
    }

    ~SubGroup();

    RONIN_INLINE GroupMembersSet::iterator GetGroupMembersBegin(void) { return m_GroupMembers.begin(); }
    RONIN_INLINE GroupMembersSet::iterator GetGroupMembersEnd(void)   { return m_GroupMembers.end();   }

    bool AddPlayer(PlayerInfo * info);
    void RemovePlayer(PlayerInfo * info);

    RONIN_INLINE bool IsFull(void)                { return m_GroupMembers.size() >= MAX_GROUP_SIZE_PARTY; }
    RONIN_INLINE size_t GetMemberCount(void)      { return m_GroupMembers.size(); }

    RONIN_INLINE uint32 GetID(void)              { return m_Id; }
    RONIN_INLINE void SetID(uint32 newid)      { m_Id = newid; }

    RONIN_INLINE void   SetParent(Group* parent)  { m_Parent = parent; }
    RONIN_INLINE Group* GetParent(void)          { return m_Parent; }

    void   Disband();
    bool HasMember(WoWGuid guid);

protected:

    GroupMembersSet  m_GroupMembers;
    Group*            m_Parent;
    uint32            m_Id;

};

class SERVER_DECL Group
{
public:
    friend class SubGroup;

    static Group *Create();
    static Group *Init(uint32 groupId, uint32 dungeonId, uint16 groupType);

    Group(bool Assign);
    Group(uint32 groupId, uint32 dungeonId, uint16 groupType);
    ~Group();

    // Adding/Removal Management
    bool AddMember(PlayerInfo * info, int32 subgroupid=-1, bool silent = false);
    void RemovePlayer(PlayerInfo * info);

    // Leaders and Looting
    void SetLeader(Player* pPlayer,bool silent);
    void SetLooter(Player* pPlayer, uint8 method, uint16 threshold);

    // Transferring data to clients
    void Update();

    void AddMemberTargets(SpellTargetClass *spell, uint32 effIndex, bool subGroupOnly, Player *plr, uint32 TargetType, float range, uint32 maxTargets, uint32 classMask = 0);

    RONIN_INLINE void SendPacketToAll(WorldPacket *packet) { SendPacketToAllButOne(packet, NULL); }
    void SendPacketToAllButOne(WorldPacket *packet, Player* pSkipTarget);

    RONIN_INLINE void OutPacketToAll(uint16 op, uint16 len, const void* data) { OutPacketToAllButOne(op, len, data, NULL); }
    void OutPacketToAllButOne(uint16 op, uint16 len, const void* data, Player* pSkipTarget);

    void SendNullUpdate(Player* pPlayer);

    void SendLFGLockInfo(Player *pPlayer);

    bool QualifiesForGuildXP(Creature *cVictim);

    // Group Combat
    void SendPartyKillLog(WorldObject* player, WorldObject* Unit);

    // Destroying/Converting
    void Disband();
    Player* FindFirstPlayer();
    bool HasAcceptableDisenchanters(int32 requiredskill);

    // Accessing functions
    RONIN_INLINE SubGroup* GetSubGroup(uint32 Id)
    {
        if(Id >= 8)
            return 0;

        return m_SubGroups[Id];
    }

    RONIN_INLINE uint32 GetSubGroupCount(void) { return m_SubGroupCount; }

    RONIN_INLINE uint8 GetMethod(void) { return m_LootMethod; }
    RONIN_INLINE uint16 GetThreshold(void) { return m_LootThreshold; }
    RONIN_INLINE PlayerInfo* GetLeader(void) { return m_Leader; }
    RONIN_INLINE PlayerInfo* GetLooter(void) { return m_Looter; }

    void MovePlayer(PlayerInfo* info, uint8 subgroup);

    bool HasMember(Player* pPlayer);
    bool HasMember(PlayerInfo * info);
    RONIN_INLINE uint32 MemberCount(void) { return m_MemberCount; }
    RONIN_INLINE bool IsFull() { return ((m_GroupType == GROUP_TYPE_PARTY && m_MemberCount >= MAX_GROUP_SIZE_PARTY) || (m_GroupType == GROUP_TYPE_RAID && m_MemberCount >= MAX_GROUP_SIZE_RAID)); }
    //RONIN_INLINE uint32 GetOnlineMemberCount();
    uint32 GetOnlineMemberCount();

    SubGroup* FindFreeSubGroup();

    void ExpandToRaid();

    void SaveToDB();
    void LoadFromDB(Field *fields);

    RONIN_INLINE uint8 GetGroupType() { return m_GroupType; }
    RONIN_INLINE uint32 GetID() { return m_Id; }
    RONIN_INLINE WoWGuid GetGuid() { return MAKE_NEW_GUID(m_Id, 0, HIGHGUID_TYPE_GROUP); }

    void UpdateOutOfRangePlayer(PlayerInfo *info, uint32 Flags, bool Distribute, WorldPacket * Packet);
    void UpdateAllOutOfRangePlayersFor(PlayerInfo *info);
    void HandleUpdateFieldChange(uint32 Index, Player* pPlayer);
    void HandlePartialChange(uint32 Type, Player* pPlayer);

    void FillLFDMembers(Loki::AssocVector<PlayerInfo*, uint8> *members);

    uint64 m_targetIcons[8];
    RONIN_INLINE Mutex& getLock() { return m_groupLock; }
    RONIN_INLINE void Lock() { m_groupLock.Acquire(); }
    RONIN_INLINE void Unlock() { return m_groupLock.Release(); }
    bool m_isqueued;

    void SetAssistantLeader(PlayerInfo * pMember);
    void SetMainTank(PlayerInfo * pMember);
    void SetMainAssist(PlayerInfo * pMember);

    RONIN_INLINE PlayerInfo * GetAssistantLeader() { return m_assistantLeader; }
    RONIN_INLINE PlayerInfo * GetMainTank() { return m_mainTank; }
    RONIN_INLINE PlayerInfo * GetMainAssist() { return m_mainAssist; }

    bool IsGroupFinderInstance(uint32 mapId);

public:
    RONIN_INLINE void SetFlag(uint8 groupflag) { m_groupFlags |= groupflag; }
    RONIN_INLINE void RemoveFlag(uint8 groupflag) { m_groupFlags &= ~groupflag; }
    RONIN_INLINE bool HasFlag(uint8 groupflag) { return (m_groupFlags & groupflag) > 0 ? true : false; }
    RONIN_INLINE int8 GetDifficulty() { return m_difficulty; }
    RONIN_INLINE int8 GetRaidDifficulty() { return m_raiddifficulty; }
    RONIN_INLINE void SetDifficulty(uint8 diff) { m_difficulty = diff; }
    RONIN_INLINE void SetRaidDifficulty(uint8 diff) { m_raiddifficulty = diff; }
    uint8 m_difficulty;
    uint8 m_raiddifficulty;

protected:
    PlayerInfo * m_Leader;
    PlayerInfo * m_Looter;
    PlayerInfo * m_assistantLeader;
    PlayerInfo * m_mainTank;
    PlayerInfo * m_mainAssist;

    uint8 m_LootMethod;
    uint16 m_LootThreshold;
    uint8 m_GroupType;
    uint32 m_Id;

    SubGroup* m_SubGroups[8];
    uint8 m_SubGroupCount;
    uint32 m_MemberCount;
    Mutex m_groupLock;
    bool m_dirty;
    bool m_updateblock;
    uint8 m_groupFlags;

    struct LFDDungeonData
    {
        uint32 dungeonId;
        uint32 encounterMask;
        Loki::AssocVector<WoWGuid, uint8> memberRoles;
    } *m_lfdData;
};
