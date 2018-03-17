#include "unit.h"

#include <functional>
#include <algorithm>
#include <string.h>

#include "console/assert.h"

#include "constants/image.h"
#include "constants/order.h"
#include "constants/sprite.h"
#include "constants/tech.h"
#include "constants/upgrade.h"
#include "constants/weapon.h"

#include "ai.h"
#include "image.h"
#include "offsets.h"
#include "order.h"
#include "pathing.h"
#include "player.h"
#include "rng.h"
#include "selection.h"
#include "sprite.h"
#include "strings.h"
#include "tech.h"
#include "text.h"
#include "upgrade.h"
#include "yms.h"
#include "warn.h"

using std::get;
using std::max;
using std::min;

bool Unit::IsDisabled() const
{
    if (flags & UnitStatus::Disabled || mael_timer || lockdown_timer || stasis_timer)
        return true;
    return false;
}

int Unit::GetMaxShields() const
{
    if (!Type().HasShields())
        return 0;
    return Type().Shields();
}

int Unit::GetShields() const
{
    if (!Type().HasShields())
        return 0;
    return shields >> 8;
}

int Unit::GetMaxHealth() const
{
    int health = GetMaxShields() + GetMaxHitPoints();
    if (!health)
        return 1;
    return health;
}

int Unit::GetHealth() const
{
    return GetShields() + (hitpoints >> 8);
}

int Unit::GetMaxEnergy() const
{
    return bw::GetMaxEnergy(this);
}

int Unit::GetArmorUpgrades() const
{
    if (*bw::is_bw && (Type() == UnitId::Ultralisk || Type() == UnitId::Torrasque))
    {
        if (Type().IsHero())
            return GetUpgradeLevel(Type().ArmorUpgrade(), player) + 2;
        else
        {
            return GetUpgradeLevel(Type().ArmorUpgrade(), player) + 2 *
                GetUpgradeLevel(UpgradeId::ChitinousPlating, player);
        }
    }
    return GetUpgradeLevel(Type().ArmorUpgrade(), player);
}

int Unit::GetOriginalPlayer() const
{
    if (player == 11)
        return sprite->player;
    return player;
}

bool Unit::IsEnemy(const Unit *other) const
{
    if (player >= Limits::Players || other->player >= Limits::Players)
        return false;
    return bw::alliances[player][other->GetOriginalPlayer()] == 0;
}

bool Unit::IsOnBurningHealth() const
{
    int max_hp = GetMaxHitPoints();
    int hp = (hitpoints + 0xff) >> 8;
    return (hp * 100 / max_hp) <= 33;
}

bool Unit::IsOnYellowHealth() const
{
    int max_hp = GetMaxHitPoints();
    int hp = (hitpoints + 0xff) >> 8;
    int percentage = (hp * 100 / max_hp);
    return percentage < 66 && percentage > 33;
}

int Unit::GetWeaponRange(bool ground) const
{
    using namespace UnitId;

    int range;
    if (ground)
        range = GetTurret()->GetGroundWeapon().MaxRange();
    else
        range = GetTurret()->GetAirWeapon().MaxRange();

    if (flags & UnitStatus::InBuilding)
        range += 0x40;
    switch (unit_id)
    {
        case Marine:
            return range + GetUpgradeLevel(UpgradeId::U_238Shells, player) * 0x20;
        case Hydralisk:
            return range + GetUpgradeLevel(UpgradeId::GroovedSpines, player) * 0x20;
        case Dragoon:
            return range + GetUpgradeLevel(UpgradeId::SingularityCharge, player) * 0x40;
        case FenixDragoon: // o.o
            if (*bw::is_bw)
                return range + 0x40;
            return range;
        case Goliath:
        case GoliathTurret:
            if (ground || *bw::is_bw == 0)
                return range;
            else
                return range + GetUpgradeLevel(UpgradeId::CharonBooster, player) * 0x60;
        case AlanSchezar:
        case AlanTurret:
            if (ground || *bw::is_bw == 0)
                return range;
            else
                return range + 0x60;
        default:
            return range;
    }
}

int Unit::GetSightRange(bool dont_check_blind) const
{
    if (flags & UnitStatus::Building && ~flags & UnitStatus::Completed && !IsMorphingBuilding())
        return 4;

    if (!dont_check_blind && blind)
        return 2;

    auto upgrade = Type().SightUpgrade();
    if (upgrade != UpgradeId::None && GetUpgradeLevel(upgrade, player) != 0)
        return 11;
    else
        return Type().SightRange();
}

int Unit::GetTargetAcquisitionRange() const
{
    using namespace UnitId;

    int base_range = Type().TargetAcquisitionRange();
    switch (unit_id)
    {
        case Ghost:
        case AlexeiStukov:
        case SamirDuran:
        case SarahKerrigan:
        case InfestedDuran:
            if (IsInvisible() && OrderType() == OrderId::HoldPosition)
                return 0;
            else
                return base_range;
        break;
        case Marine:
            return base_range + GetUpgradeLevel(UpgradeId::U_238Shells, player);
        break;
        case Hydralisk:
            return base_range + GetUpgradeLevel(UpgradeId::GroovedSpines, player);
        break;
        case Dragoon:
            return base_range + GetUpgradeLevel(UpgradeId::SingularityCharge, player) * 2;
        break;
        case FenixDragoon: // o.o
            if (*bw::is_bw)
                return base_range + 2;
            else
                return base_range;
        break;
        case Goliath:
        case GoliathTurret:
            if (*bw::is_bw)
                return base_range + GetUpgradeLevel(UpgradeId::CharonBooster, player) * 3;
            else
                return base_range;
        break;
        case AlanSchezar:
        case AlanTurret:
            if (*bw::is_bw)
                return base_range + 3;
            else
                return base_range;
        break;
        default:
            return base_range;
    }
}
WeaponType Unit::GetGroundWeapon() const
{
    if ((Type() == UnitId::Lurker) && !(flags & UnitStatus::Burrowed))
        return WeaponId::None;
    return Type().GroundWeapon();
}

WeaponType Unit::GetAirWeapon() const
{
    return Type().AirWeapon();
}

int Unit::GetCooldown(WeaponType weapon) const
{
    int cooldown = weapon.Cooldown();
    if (acid_spore_count)
    {
        if (cooldown / 8 < 3)
            cooldown += 3 * acid_spore_count;
        else
            cooldown += (cooldown / 8) * acid_spore_count;
    }
    int bonus = 0;
    if (stim_timer)
        bonus++;
    if (flags & UnitStatus::AttackSpeedUpgrade)
        bonus++;
    if (ensnare_timer)
        bonus--;
    if (bonus > 0)
        cooldown /= 2;
    else if (bonus < 0)
        cooldown += cooldown / 4;

    if (cooldown > 250)
        return 250;
    if (cooldown < 5)
        return 5;
    return cooldown;
}

bool Unit::CanAttackFowUnit(UnitType fow_unit) const
{
    if (fow_unit == UnitId::None)
        return false;
    if (fow_unit.Flags() & UnitFlags::Invincible)
        return false;
    if (Type().HasHangar())
    {
        return true;
    }
    if (GetGroundWeapon() == WeaponId::None)
    {
        if (!subunit || subunit->GetGroundWeapon() == WeaponId::None)
            return false;
    }
    return true;
}

bool Unit::HasSubunit() const
{
    if (!subunit)
        return false;
    if (subunit->Type().IsSubunit())
        return true;
    return false;
}

Unit *Unit::GetTurret()
{
    if (HasSubunit())
        return subunit;
    return this;
}

const Unit *Unit::GetTurret() const
{
    if (HasSubunit())
        return subunit;
    return this;
}

bool Unit::HasWayOfAttacking() const
{
    if (GetAirWeapon() != WeaponId::None || GetGroundWeapon() != WeaponId::None)
        return true;

    switch (Type().Raw())
    {
        case UnitId::Carrier:
        case UnitId::Gantrithor:
        case UnitId::Reaver:
        case UnitId::Warbringer:
            if (carrier.in_hangar_count || carrier.out_hangar_count)
                return true;
        break;
    }
    if (subunit)
    {
        if (subunit->GetAirWeapon() != WeaponId::None || subunit->GetGroundWeapon() != WeaponId::None)
            return true;
    }
    return false;
}

bool Unit::IsAtHome() const
{
    if (!ai || ai->type != 1)
        return true;
    return ((Ai::GuardAi *)ai)->home == sprite->position;
}

bool Unit::IsInvisibleTo(const Unit *unit) const
{
    if (!IsInvisible())
        return false;
    return (detection_status & (1 << unit->player)) == 0;
}

bool Unit::IsMorphingBuilding() const
{
    if (flags & UnitStatus::Completed)
        return false;
    return UnitType(build_queue[current_build_slot]).IsBuildingMorphUpgrade();
}

bool Unit::IsResourceDepot() const
{
    if (~flags & UnitStatus::Building)
        return false;
    if (~flags & UnitStatus::Completed && !IsMorphingBuilding())
        return false;
    return Type().Flags() & UnitFlags::ResourceDepot;
}

void Unit::IssueSecondaryOrder(class OrderType order_id)
{
    if (secondary_order == order_id.Raw())
        return;
    secondary_order = order_id.Raw();
    secondary_order_state = 0;
    currently_building = nullptr;
    unke8 = 0;
    unkea = 0;
    // Not touching the secondary order timer..
}

bool Unit::CanDetect() const
{
    if (~Type().Flags() & UnitFlags::Detector)
        return false;
    if (~flags & UnitStatus::Completed)
        return false;
    if (IsDisabled())
        return false;
    if (blind)
        return false;
    return true;
}

int Unit::GetDistanceToUnit(const Unit *other) const
{
    //STATIC_PERF_CLOCK(Unit_GetDistanceToUnit);
    Rect16 other_rect;
    const Rect16 &other_dbox = other->Type().DimensionBox();
    other_rect.left = other->sprite->position.x - other_dbox.left;
    other_rect.top = other->sprite->position.y - other_dbox.top;
    other_rect.right = other->sprite->position.x + other_dbox.right + 1;
    other_rect.bottom = other->sprite->position.y + other_dbox.bottom + 1;
    const Unit *self = this;
    if (Type().IsSubunit())
        self = subunit;

    Rect16 self_rect;
    const Rect16 &own_dbox = self->Type().DimensionBox();
    self_rect.left = self->sprite->position.x - own_dbox.left;
    self_rect.top = self->sprite->position.y - own_dbox.top;
    self_rect.right = self->sprite->position.x + own_dbox.right + 1;
    self_rect.bottom = self->sprite->position.y + own_dbox.bottom + 1;

    int x = self_rect.left - other_rect.right;
    if (x < 0)
    {
        x = other_rect.left - self_rect.right;
        if (x < 0)
            x = 0;
    }
    int y = self_rect.top - other_rect.bottom;
    if (y < 0)
    {
        y = other_rect.top - self_rect.bottom;
        if (y < 0)
            y = 0;
    }
    return Distance(Point32(0,0), Point32(x, y));
}

const Point &Unit::GetPosition() const
{
    return sprite->position;
}

Rect16 Unit::GetCollisionRect() const
{
    const Rect16 dbox = Type().DimensionBox();
    const Point &pos = GetPosition();
    return Rect16(pos.x - dbox.left, pos.y - dbox.top, pos.x + dbox.right + 1, pos.y + dbox.bottom + 1);
}

int Unit::GetRegion() const
{
    return ::GetRegion(sprite->position);
}

bool Unit::IsUpgrading() const
{
    return building.upgrade != UpgradeId::None;
}

bool Unit::IsBuildingAddon() const
{
    return SecondaryOrderType() == OrderId::BuildAddon && flags & UnitStatus::Building &&
        currently_building != nullptr && ~currently_building->flags & UnitStatus::Completed;
}

const char *Unit::GetName() const
{
    return (*bw::stat_txt_tbl)->GetTblString(unit_id + 1);
}

std::string Unit::DebugStr() const
{
    char buf[256];
    snprintf(buf, sizeof buf, "%s (id %x)", GetName(), unit_id);
    return std::string(buf);
}
bool Unit::CanLocalPlayerControl() const
{
    if (IsReplay())
        return false;
    return player == *bw::local_player_id;
}

bool Unit::CanLocalPlayerSelect() const
{
    int visions = *bw::player_visions;
    if (IsReplay())
    {
        if (*bw::replay_show_whole_map)
            return true;
        visions = *bw::replay_visions;
    }
    if ((sprite->visibility_mask & visions) == 0)
        return false;
    if (IsInvisible() && (detection_status & visions) == 0)
        return false;
    return true;
}

Ai::BuildingAi *Unit::AiAsBuildingAi()
{
    if (ai != nullptr && ai->type == 3)
        return (Ai::BuildingAi *)ai;
    return nullptr;
}
