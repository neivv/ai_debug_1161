#ifndef UNIT_H
#define UNIT_H

#include "dat.h"
#include "types.h"
#include "list.h"
#include "offsets.h"
#include "sprite.h"
#include "unit_type.h"
#include "pathing.h"

#include "constants/unit.h"

#include <atomic>

namespace UnitStatus
{
    const unsigned int Completed = 0x1;
    const unsigned int Building = 0x2;
    const unsigned int Air = 0x4;
    const unsigned int Disabled2 = 0x8;
    const unsigned int Burrowed = 0x10;
    const unsigned int InBuilding = 0x20;
    const unsigned int InTransport = 0x40; // Also bunker, even if it has 0x20 as well
    const unsigned int Unk80 = 0x80;
    const unsigned int BeginInvisibility = 0x100;
    const unsigned int InvisibilityDone = 0x200;
    const unsigned int Disabled = 0x400;
    const unsigned int FreeInvisibility = 0x800; // That is, energy doesn't decrease
    const unsigned int UninterruptableOrder = 0x1000;
    const unsigned int Nobrkcodestart = 0x2000;
    const unsigned int HasDisappearingCreep = 0x4000; // Set only if creeap disappear array runs out of entries
    const unsigned int UnderDweb = 0x8000;
    const unsigned int FullAutoAttack = 0x00010000;
    const unsigned int Reacts = 0x00020000;
    const unsigned int Unstacking = 0x00040000;
    const unsigned int MovePosUpdated = 0x00080000;
    const unsigned int Collides = 0x00100000;
    const unsigned int NoCollision = 0x00200000;
    const unsigned int NoEnemyCollide = 0x00400000; // No clue at all
    const unsigned int Harvesting = 0x00800000;
    const unsigned int Invincible = 0x04000000;
    const unsigned int CanSwitchTarget = 0x08000000;
    const unsigned int Unk01000000 = 0x01000000;
    const unsigned int SpeedUpgrade = 0x10000000;
    const unsigned int AttackSpeedUpgrade = 0x20000000;
    const unsigned int Hallucination = 0x40000000;
    const unsigned int SelfDestructing = 0x80000000;
}

namespace MovementState
{
    const int Subunit = 0x3;
    const int Bunker = 0x4;
    const int Flyer = 0x7;
    const int FollowPath = 0x19;
}

#pragma pack(push)
#pragma pack(1)

struct struct230
{
    uint8_t dc0[0x1d];
    uint8_t building_was_hit;
};

 // Derived from BWAPI's unit.h

class Unit
{
    friend class UnitIscriptContext;
    public:
        static const size_t offset_of_allocated = 0xb8;

        // Sc data
        RevListEntry<Unit, 0x0> list;
        int32_t hitpoints;
        ptr<Sprite> sprite;

        Point move_target; // 0x10
        Unit *move_target_unit; // 0x14

        Point next_move_waypoint;
        Point unk_move_waypoint;

        // 0x20
        uint8_t flingy_flags;
        uint8_t facing_direction;
        uint8_t flingy_turn_speed;
        uint8_t movement_direction;
        uint16_t flingy_id;
        uint8_t _unknown_0x026;
        uint8_t flingy_movement_type;
        Point position;
        Point32 exact_position; // 0x2c
        uint32_t flingy_top_speed; // 0x34
        int32_t current_speed;
        int32_t next_speed; // 0x3c

        int32_t speed[2]; // 0x40
        uint16_t acceleration; // 0x48
        uint8_t new_direction;
        uint8_t target_direction; // 0x4b

        // Flingy end

        uint8_t player;
        uint8_t order;
        uint8_t order_state;
        uint8_t order_signal;
        uint16_t order_fow_unit;
        uint8_t unused52;
        uint8_t unused53;
        uint8_t order_timer;
        uint8_t ground_cooldown;
        uint8_t air_cooldown;
        uint8_t spell_cooldown;
        Point order_target_pos;
        Unit *target;

        // Entity end

        int32_t shields;   // 0x60
        uint16_t unit_id;      // 0x64
        uint16_t unused66;

        RevListEntry<Unit, 0x68> player_units;
        Unit *subunit; // 0x70
        RevListHead<Order, 0x0> order_queue_begin;
        ListHead<Order, 0x0>  order_queue_end;
        Unit *previous_attacker;

        Unit *related;
        uint8_t highlighted_order_count; // 0x84
        uint8_t order_wait; // 0x85
        uint8_t unk86;
        uint8_t attack_notify_timer;
        uint16_t previous_unit_id;
        uint8_t lastEventTimer;
        uint8_t lastEventColor;
        uint16_t unused8c;
        uint8_t rankIncrease;
        uint8_t old_kills;

        uint8_t last_attacking_player;
        uint8_t secondary_order_wait;
        uint8_t ai_spell_flags;
        uint8_t order_flags;
        uint16_t buttons;
        uint8_t invisibility_effects;
        uint8_t movement_state;
        uint16_t build_queue[5]; // 0x98
        uint16_t energy; // 0xa2
        uint8_t current_build_slot; // 0xa4
        uint8_t targetOrderSpecial; // 0xa5
        uint8_t secondary_order; // 0xa6
        uint8_t buildingOverlayState; // 0xa7
        uint16_t build_hp_gain; // 0xa8
        uint16_t build_shield_gain;
        uint16_t remaining_build_time;
        uint16_t previous_hp;

        uint16_t loadedUnitIndex[8]; // 0xb0

        /// NOTE: While some of these union fields, such as spider mines or nuke,
        /// are easy to use with a unit which is not supposed to have them,
        /// saving the game will have issues unless Unit::serialize is modified
        /// to know which units use which variant. However, as a safety measure,
        /// the serialization function saves everything for units which should not
        /// use any variants at all. If variant uses pointers then it *must* be explicitly
        /// fixed, otherwise it just makes things clearer/safer (and saves some bytes)
        /// (Ideally this union would not be extended, but used less?)
        union // c0 union
        {
            struct {
                uint8_t spiderMineCount; // 0
            } vulture;

            struct {
                ListHead<Unit, 0xc4> in_child;
                ListHead<Unit, 0xc4> out_child;
                uint8_t in_hangar_count;
                uint8_t out_hangar_count;
            } carrier; // also applies to reaver

            struct {
                Unit *parent;   // 0x0
                ListEntry<Unit, 0xc4> list;
                uint8_t is_outside_hangar;  // 0xC
            } interceptor;

            struct {
                Unit *addon;
                uint16_t addonBuildType;
                uint16_t upgradeResearchTime;
                uint8_t tech;
                uint8_t upgrade;
                uint8_t larva_timer;
                uint8_t is_landing;
                uint8_t creep_timer;
                uint8_t upgrade_level;
            } building;

            struct
            {
                Unit *powerup;
                uint16_t target_resource_pos[2];
                Unit *current_harvest_target;
                uint16_t repair_resource_loss_timer;
                uint8_t is_harvesting;
                uint8_t carried_resource_count;
            } worker;
        };
        union // d0 union
        {
            struct {
                uint16_t resource_amount;
                uint8_t resourceIscript;
                uint8_t awaiting_workers;
                RevListHead<Unit, 0xd4> first_awaiting_worker;
                uint8_t resource_area;
                uint8_t ai_unk;
            } resource;

            struct {
                Unit *harvest_target;
                RevListEntry<Unit, 0xd4> harvesters;
            } harvester;

            struct { Unit *exit; } nydus;
            /// Owned and animated by LoneSpriteSystem
            struct { Sprite *nukedot; } ghost;
            /// Owned by this unit, not animated.
            /// Really messy as this is inside union.
            struct { ptr<Sprite> aura; } pylon;

            struct {
                Unit *nuke; // d0
                uint32_t has_nuke; // d4
            } silo;

            struct { Rect16 preferred_larvaspawn; } hatchery;

            struct {
                Point origin_point;
                Unit *carrying_unit;
            } powerup;
        };

        uint32_t flags; // 0xdc
        uint8_t carried_powerup_flags; // 0xe0, 1 = gas, 2 = minerals
        uint8_t wireframe_randomizer;
        uint8_t secondary_order_state;
        uint8_t move_target_update_timer;
        uint32_t detection_status;
        uint16_t unke8;    // Secondary order smth
        uint16_t unkea;    // Secondary order smth
        Unit *currently_building; // 0xec

        RevListEntry<Unit, 0xf0> invisible_list; // 0xf0
        union
        {
            struct {
                Point position;
                Unit *unit;
            } rally;

            struct { RevListEntry<Unit, 0xf8> list; } pylon_list;
        };

        ptr<Path> path; // 0x100
        uint8_t path_frame; // 0x104
        uint8_t pathing_flags;
        uint8_t _unused_0x106;
        uint8_t is_being_healed;
        Rect16 contourBounds;          // a rect that specifies the closest contour (collision) points

        uint16_t death_timer;
        uint16_t matrix_hp;
        uint8_t matrix_timer;
        uint8_t stim_timer;
        uint8_t ensnare_timer;
        uint8_t lockdown_timer;
        uint8_t irradiate_timer;
        uint8_t stasis_timer;
        uint8_t plague_timer;
        uint8_t is_under_storm;
        Unit *irradiated_by;
        uint8_t irradiate_player; // 0x120
        uint8_t parasites;
        uint8_t master_spell_timer;
        uint8_t blind;
        uint8_t mael_timer;
        uint8_t _unused_125;
        uint8_t acid_spore_count;
        uint8_t acid_spore_timers[9];

        uint16_t bullet_spread_seed;
        uint16_t _padding_0x132;
        Ai::UnitAi *ai;
        uint16_t air_strength;
        uint16_t ground_strength;
        int32_t search_left;
        int32_t search_right;
        int32_t unit_search_top;
        int32_t unit_search_bottom;
        uint8_t _repulseUnknown;
        uint8_t repulseAngle;
        uint8_t driftPosX;
        uint8_t driftPosY;

        // Funcs etc
        Unit *&next() { return list.next; }
        Unit *&prev() { return list.prev; }

        UnitType Type() const { return UnitType(unit_id); }
        class OrderType OrderType() const {
            class OrderType order_type(order);
            return order_type;
        }
        class OrderType SecondaryOrderType() const {
            class OrderType order_type(secondary_order);
            return order_type;
        }

        bool IsDying() const { return order == 0 && order_state == 1; }

        bool IsUnreachable(const Unit *other) const;
        bool IsFlying() const { return flags & UnitStatus::Air; }
        bool IsInvincible() const { return flags & UnitStatus::Invincible; }

        bool IsOnBurningHealth() const;
        bool IsOnYellowHealth() const;
        int GetMaxShields() const;
        int GetShields() const;
        int GetMaxHealth() const;
        int GetHealth() const;
        int GetMaxHitPoints() const { return Type().HitPoints() >> 8; }
        int GetHitPoints() const { return hitpoints >> 8; }
        int GetArmor() const { return GetArmorUpgrades() + Type().Armor(); }
        int GetArmorUpgrades() const;
        class OrderType GetIdleOrder() const;
        int GetWeaponRange(bool ground) const;
        int GetSightRange(bool dont_check_blind) const;
        int GetTargetAcquisitionRange() const;
        int GetMaxEnergy() const;

        bool CanDetect() const;
        bool IsKnownHallucination() const;
        const Point &GetPosition() const;
        // Right and bottom are first non-included coords, as one would expect
        Rect16 GetCollisionRect() const;

        void ReduceEnergy(int amt);

        bool IsResourceDepot() const;
        bool IsMorphingBuilding() const;
        bool IsUpgrading() const;
        bool IsBuildingAddon() const;

        bool CanCollideWith(const Unit *other) const;
        bool NeedsToDodge(const Unit *other) const;
        bool DoesCollideAt(const Point &own_pos, const Unit *other, const Point &other_pos) const;
        bool IsMovingAwayFrom(const Unit *other) const;

        bool DoesAcceptRClickCommands() const;
        int GetRClickAction() const;

        bool CanRClickGround() const;
        bool CanMove() const;
        bool IsDisabled() const;
        bool HasSubunit() const;
        Unit *GetTurret();
        const Unit *GetTurret() const;

        int GetRegion() const;
        bool CanLocalPlayerControl() const;
        bool CanLocalPlayerSelect() const;

        bool IsInUninterruptableState() const { return flags & 0x3000; }
        bool IsCarryingFlag() const;

        void IscriptToIdle();
        void OrderDone();
        void DoNextQueuedOrder();
        void ForceOrderDone();

        int GetUsedSpace() const;
        bool CanLoadUnit(const Unit *unit) const;
        void LoadUnit(Unit *unit);
        bool UnloadUnit(Unit *unit);

        void TargetedOrder(class OrderType new_order, Unit *target, const Point &pos, UnitType fow_unit, bool queued);
        void IssueOrderTargetingNothing(class OrderType order) {
            IssueOrder(order, nullptr, Point(0, 0), UnitId::None);
        }
        void IssueOrderTargetingGround(class OrderType order_id, const Point &pos) {
            IssueOrder(order_id, nullptr, pos, UnitId::None);
        }
        void IssueOrderTargetingUnit(class OrderType order, Unit *target) {
            IssueOrder(order, target, target->sprite->position, UnitId::None);
        }
        /// IssueOrder changes the current order - at least once uninterruptable state is over.
        void IssueOrder(class OrderType new_order, Unit *target, const Point &pos, UnitType fow_unit);
        /// PrependOrder/AppendOrder/InsertOrder do not change the currently executed order
        void PrependOrderTargetingNothing(class OrderType order) {
            PrependOrder(order, nullptr, Point(0, 0), UnitId::None);
        }
        void PrependOrderTargetingGround(class OrderType order_id, const Point &pos) {
            PrependOrder(order_id, nullptr, pos, UnitId::None);
        }
        void PrependOrderTargetingUnit(class OrderType order, Unit *target) {
            PrependOrder(order, target, target->sprite->position, UnitId::None);
        }
        void PrependOrder(class OrderType order, Unit *target, const Point &pos, UnitType fow_unit);
        void AppendOrderTargetingNothing(class OrderType order_id) {
            AppendOrder(order_id, nullptr, Point(0, 0), UnitId::None, false);
        }
        void AppendOrderTargetingGround(class OrderType order_id, const Point &pos) {
            AppendOrder(order_id, nullptr, pos, UnitId::None, false);
        }
        void AppendOrderTargetingUnit(class OrderType order_id, Unit *target) {
            AppendOrder(order_id, target, target->sprite->position, UnitId::None, false);
        }
        void AppendOrder(class OrderType order_id, Unit *target, const Point &pos,
                UnitType fow_unit, bool clear_others);
        void InsertOrderAfter(class OrderType order_id, Unit *target, const Point &pos,
                UnitType fow_unit, Order *insert_after);
        void InsertOrderBefore(class OrderType order_id, Unit *target, const Point &pos,
                UnitType fow_unit, Order *insert_before);

        void Recall(Unit *other);

        int Order_AttackMove_ReactToAttack(class OrderType order);
        void Order_AttackMove_TryPickTarget(class OrderType order);

        void AllowSwitchingTarget() {
            flags |= UnitStatus::CanSwitchTarget;
            if (subunit)
                subunit->flags |= UnitStatus::CanSwitchTarget;
        }

        void IssueSecondaryOrder(class OrderType order_id);
        void DeleteOrder(Order *order);
        void DeleteSpecificOrder(class OrderType order_id);
        bool IsTransport() const;

        bool IsEnemy(const Unit *other) const;
        int GetOriginalPlayer() const;
        void IncrementKills();
        void SetButtons(int buttonset);

        void UpdateStrength();

        void RemoveOverlayFromSelf(ImageType first_id, int id_amount);
        void RemoveOverlayFromSelfOrSubunit(ImageType first_id, int id_amount);
        void AddSpellOverlay(ImageType small_overlay_id);

        WeaponType GetAirWeapon() const;
        WeaponType GetGroundWeapon() const;
        int GetCooldown(WeaponType weapon_id) const;

        /// Returns 0 if moving, 1 if not and 2 if can't tell???
        int IsStandingStill() const;
        bool IsAtHome() const;

        bool Reaver_CanAttackUnit(const Unit *enemy) const;
        bool CanBeInfested() const;
        bool CanAttackFowUnit(UnitType unit_id) const;
        bool CanTargetSelf(class OrderType order) const;
        bool CanUseTargetedOrder(class OrderType order) const;

        void Attack(Unit *enemy) {
            IssueOrderTargetingUnit(Type().AttackUnitOrder(), enemy);
        }
        void ReactToHit(Unit *attacker);

        bool HasWayOfAttacking() const;
        bool IsBetterTarget(const Unit *other) const;
        /// Returns true if cmp is better target than prev.
        /// Used by ai code, but is not depending on any ai structures.
        bool Ai_IsBetterTarget(const Unit *cmp, const Unit *prev) const {
            return Ai_ChooseBetterTarget(cmp, prev) == cmp;
        }
        const Unit *Ai_ChooseBetterTarget(const Unit *cmp, const Unit *prev) const;

        bool IsInvisible() const { return flags & (UnitStatus::BeginInvisibility | UnitStatus::InvisibilityDone); }
        bool IsInvisibleTo(const Unit *unit) const;
        bool CanSeeUnit(const Unit *other) const;
        bool IsInAttackRange(const Unit *target) const;
        bool WillBeInAreaAtStop(const Unit *other, int range) const;
        int GetDistanceToUnit(const Unit *other) const;

        void ClearTarget();
        void SetPreviousAttacker(Unit *unit);

        void AskForHelp(Unit *attacker);
        Unit *GetActualTarget(Unit *target) const;

        void Cloak(TechType tech);

        bool CanBeAttacked() const;
        bool CanAttackUnit(const Unit *other, bool check_detection = true) const;
        /// Micro-optimization :|
        bool CanAttackUnit_ChooseTarget(const Unit *other, bool check_detection) const;
        bool IsThreat(const Unit *other) const;

        uint32_t GetHaltDistance() const;

        bool ChangeMovementTargetToUnit(Unit *target);
        bool ChangeMovementTarget(const Point &pos);

        Ai::BuildingAi *AiAsBuildingAi();

        std::string DebugStr() const;
        const char *GetName() const;
};

#pragma pack(pop)

#endif
