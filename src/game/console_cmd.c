#include "game/console_cmd.h"

#include "config.h"
#include "game/carrier.h"
#include "game/clock.h"
#include "game/console.h"
#include "game/effects/exploding_death.h"
#include "game/game_string.h"
#include "game/gameflow.h"
#include "game/inventory.h"
#include "game/inventory/inventory_vars.h"
#include "game/items.h"
#include "game/lara.h"
#include "game/lara/lara_cheat.h"
#include "game/los.h"
#include "game/objects/common.h"
#include "game/random.h"
#include "game/room.h"
#include "game/sound.h"
#include "global/const.h"
#include "global/types.h"
#include "global/vars.h"
#include "math/math.h"
#include "strings.h"
#include "util.h"

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define ENDS_WITH_ZERO(num) (fabsf((num) - roundf((num))) < 0.0001f)

typedef struct ITEM_NAME {
    const GAME_OBJECT_ID obj_id;
    const char *name;
    const INVENTORY_ITEM *inv_item;
} ITEM_NAME;

static const ITEM_NAME m_ItemNames[] = {
    { .obj_id = O_MEDI_ITEM, .name = "med" },
    { .obj_id = O_MEDI_ITEM, .name = "medi" },
    { .obj_id = O_MEDI_ITEM, .name = "small med" },
    { .obj_id = O_MEDI_ITEM, .name = "small medi" },
    { .obj_id = O_MEDI_ITEM, .name = "small medpack" },
    { .obj_id = O_MEDI_ITEM, .name = "small medipack" },
    { .obj_id = O_BIGMEDI_ITEM, .name = "big med" },
    { .obj_id = O_BIGMEDI_ITEM, .name = "big medi" },
    { .obj_id = O_BIGMEDI_ITEM, .name = "big medpack" },
    { .obj_id = O_BIGMEDI_ITEM, .name = "big medipack" },
    { .obj_id = O_BIGMEDI_ITEM, .name = "large med" },
    { .obj_id = O_BIGMEDI_ITEM, .name = "large medi" },
    { .obj_id = O_BIGMEDI_ITEM, .name = "large medpack" },
    { .obj_id = O_BIGMEDI_ITEM, .name = "large medipack" },
    { .obj_id = O_GUN_ITEM, .name = "pistols" },
    { .obj_id = O_SHOTGUN_ITEM, .name = "shotgun" },
    { .obj_id = O_MAGNUM_ITEM, .name = "magnum" },
    { .obj_id = O_MAGNUM_ITEM, .name = "magnums" },
    { .obj_id = O_UZI_ITEM, .name = "uzi" },
    { .obj_id = O_UZI_ITEM, .name = "uzis" },
    { .obj_id = O_GUN_AMMO_ITEM, .name = "pistol ammo" },
    { .obj_id = O_GUN_AMMO_ITEM, .name = "pistols ammo" },
    { .obj_id = O_SG_AMMO_ITEM, .name = "sg ammo" },
    { .obj_id = O_SG_AMMO_ITEM, .name = "shotgun ammo" },
    { .obj_id = O_SG_AMMO_ITEM, .name = "sg shell" },
    { .obj_id = O_SG_AMMO_ITEM, .name = "shotgun shell" },
    { .obj_id = O_SG_AMMO_ITEM, .name = "sg shells" },
    { .obj_id = O_SG_AMMO_ITEM, .name = "shotgun shells" },
    { .obj_id = O_MAG_AMMO_ITEM, .name = "magnum ammo" },
    { .obj_id = O_MAG_AMMO_ITEM, .name = "magnums ammo" },
    { .obj_id = O_MAG_AMMO_ITEM, .name = "magnum clip" },
    { .obj_id = O_MAG_AMMO_ITEM, .name = "magnums clip" },
    { .obj_id = O_MAG_AMMO_ITEM, .name = "magnum clips" },
    { .obj_id = O_MAG_AMMO_ITEM, .name = "magnums clips" },
    { .obj_id = O_UZI_AMMO_ITEM, .name = "uzi ammo" },
    { .obj_id = O_UZI_AMMO_ITEM, .name = "uzis ammo" },
    { .obj_id = O_UZI_AMMO_ITEM, .name = "uzi clip" },
    { .obj_id = O_UZI_AMMO_ITEM, .name = "uzis clip" },
    { .obj_id = O_UZI_AMMO_ITEM, .name = "uzi clips" },
    { .obj_id = O_UZI_AMMO_ITEM, .name = "uzis clips" },
    { .obj_id = O_KEY_ITEM1, .name = "key1" },
    { .obj_id = O_KEY_ITEM2, .name = "key2" },
    { .obj_id = O_KEY_ITEM3, .name = "key3" },
    { .obj_id = O_KEY_ITEM4, .name = "key4" },
    { .obj_id = O_PUZZLE_ITEM1, .name = "puzzle1" },
    { .obj_id = O_PUZZLE_ITEM2, .name = "puzzle2" },
    { .obj_id = O_PUZZLE_ITEM3, .name = "puzzle3" },
    { .obj_id = O_PUZZLE_ITEM4, .name = "puzzle4" },
    { .obj_id = O_PICKUP_ITEM1, .name = "pickup1" },
    { .obj_id = O_PICKUP_ITEM2, .name = "pickup2" },
    { .obj_id = O_LEADBAR_ITEM, .name = "leadbar" },
    { .obj_id = O_LEADBAR_ITEM, .name = "lead bar" },
    { .obj_id = O_SCION_ITEM, .name = "scion" },
    { .obj_id = O_MEDI_ITEM, .inv_item = &g_InvItemMedi },
    { .obj_id = O_BIGMEDI_ITEM, .inv_item = &g_InvItemBigMedi },
    { .obj_id = O_PUZZLE_ITEM1, .inv_item = &g_InvItemPuzzle1 },
    { .obj_id = O_PUZZLE_ITEM2, .inv_item = &g_InvItemPuzzle2 },
    { .obj_id = O_PUZZLE_ITEM3, .inv_item = &g_InvItemPuzzle3 },
    { .obj_id = O_PUZZLE_ITEM4, .inv_item = &g_InvItemPuzzle4 },
    { .obj_id = O_KEY_ITEM1, .inv_item = &g_InvItemKey1 },
    { .obj_id = O_KEY_ITEM2, .inv_item = &g_InvItemKey2 },
    { .obj_id = O_KEY_ITEM3, .inv_item = &g_InvItemKey3 },
    { .obj_id = O_KEY_ITEM4, .inv_item = &g_InvItemKey4 },
    { .obj_id = O_PICKUP_ITEM1, .inv_item = &g_InvItemPickup1 },
    { .obj_id = O_PICKUP_ITEM2, .inv_item = &g_InvItemPickup2 },
    { .obj_id = O_LEADBAR_ITEM, .inv_item = &g_InvItemLeadBar },
    { .obj_id = O_SCION_ITEM, .inv_item = &g_InvItemScion },
    { .obj_id = O_GUN_ITEM, .inv_item = &g_InvItemPistols },
    { .obj_id = O_SHOTGUN_ITEM, .inv_item = &g_InvItemShotgunAmmo },
    { .obj_id = O_MAGNUM_ITEM, .inv_item = &g_InvItemMagnum },
    { .obj_id = O_UZI_ITEM, .inv_item = &g_InvItemUzi },
    { .obj_id = O_GUN_AMMO_OPTION, .inv_item = &g_InvItemPistolAmmo },
    { .obj_id = O_SG_AMMO_ITEM, .inv_item = &g_InvItemShotgunAmmo },
    { .obj_id = O_MAG_AMMO_ITEM, .inv_item = &g_InvItemMagnumAmmo },
    { .obj_id = O_UZI_AMMO_ITEM, .inv_item = &g_InvItemUziAmmo },
    { .obj_id = NO_OBJECT },
};

static bool Console_Cmd_Fps(const char *const args)
{
    if (String_Equivalent(args, "60")) {
        g_Config.rendering.fps = 60;
        Console_Log(GS(OSD_FPS_SET), g_Config.rendering.fps);
        return true;
    }

    if (String_Equivalent(args, "30")) {
        g_Config.rendering.fps = 30;
        Console_Log(GS(OSD_FPS_SET), g_Config.rendering.fps);
        return true;
    }

    if (String_Equivalent(args, "")) {
        Console_Log(GS(OSD_FPS_GET), g_Config.rendering.fps);
        return true;
    }

    return false;
}

static bool Console_Cmd_Pos(const char *const args)
{
    if (!g_Objects[O_LARA].loaded) {
        return false;
    }

    Console_Log(
        GS(OSD_POS_GET), g_LaraItem->room_number,
        g_LaraItem->pos.x / (float)WALL_L, g_LaraItem->pos.y / (float)WALL_L,
        g_LaraItem->pos.z / (float)WALL_L,
        g_LaraItem->rot.x * 360.0f / (float)PHD_ONE,
        g_LaraItem->rot.y * 360.0f / (float)PHD_ONE,
        g_LaraItem->rot.z * 360.0f / (float)PHD_ONE);
    return true;
}

static bool Console_Cmd_Teleport(const char *const args)
{
    if (!g_Objects[O_LARA].loaded || !g_LaraItem->hit_points) {
        return false;
    }

    {
        float x, y, z;
        if (sscanf(args, "%f %f %f", &x, &y, &z) == 3) {
            if (ENDS_WITH_ZERO(x)) {
                x += 0.5f;
            }
            if (ENDS_WITH_ZERO(z)) {
                z += 0.5f;
            }

            if (Item_Teleport(g_LaraItem, x * WALL_L, y * WALL_L, z * WALL_L)) {
                Console_Log(GS(OSD_POS_SET_POS), x, y, z);
                return true;
            }

            Console_Log(GS(OSD_POS_SET_POS_FAIL), x, y, z);
            return true;
        }
    }

    {
        int16_t room_num = -1;
        if (sscanf(args, "%hd", &room_num) == 1) {
            if (room_num < 0 || room_num >= g_RoomCount) {
                Console_Log(GS(OSD_INVALID_ROOM), room_num, g_RoomCount - 1);
                return true;
            }

            const ROOM_INFO *const room = &g_RoomInfo[room_num];

            const int32_t x1 = room->x + WALL_L;
            const int32_t x2 = (room->y_size << WALL_SHIFT) + room->x - WALL_L;
            const int32_t y1 = room->min_floor;
            const int32_t y2 = room->max_ceiling;
            const int32_t z1 = room->z + WALL_L;
            const int32_t z2 = (room->x_size << WALL_SHIFT) + room->z - WALL_L;

            for (int i = 0; i < 100; i++) {
                int32_t x = x1 + Random_GetControl() * (x2 - x1) / 0x7FFF;
                int32_t y = y1;
                int32_t z = z1 + Random_GetControl() * (z2 - z1) / 0x7FFF;
                if (Item_Teleport(g_LaraItem, x, y, z)) {
                    Console_Log(GS(OSD_POS_SET_ROOM), room_num);
                    return true;
                }
            }

            Console_Log(GS(OSD_POS_SET_ROOM_FAIL), room_num);
            return true;
        }
    }

    return false;
}

static bool Console_Cmd_Fly(const char *const args)
{
    if (!g_Objects[O_LARA].loaded) {
        return false;
    }
    Console_Log(GS(OSD_FLY_MODE_ON));
    Lara_Cheat_EnterFlyMode();
    return true;
}

static bool Console_Cmd_Speed(const char *const args)
{
    if (strcmp(args, "") == 0) {
        Console_Log(GS(OSD_SPEED_GET), Clock_GetTurboSpeed());
        return true;
    }

    int32_t num = -1;
    if (sscanf(args, "%d", &num) == 1) {
        Clock_SetTurboSpeed(num);
        return true;
    }

    return false;
}

static bool Console_Cmd_Braid(const char *const args)
{
    if (String_Equivalent(args, "off")) {
        g_Config.enable_braid = 0;
        Console_Log(GS(OSD_BRAID_OFF));
        return true;
    }

    if (String_Equivalent(args, "on")) {
        g_Config.enable_braid = 1;
        Console_Log(GS(OSD_BRAID_ON));
        return true;
    }

    return false;
}

static bool Console_Cmd_Cheats(const char *const args)
{
    if (String_Equivalent(args, "off")) {
        g_Config.enable_cheats = false;
        Console_Log(GS(OSD_CHEATS_OFF));
        return true;
    }

    if (String_Equivalent(args, "on")) {
        g_Config.enable_cheats = true;
        Console_Log(GS(OSD_CHEATS_ON));
        return true;
    }

    return false;
}

static bool Console_Cmd_GiveItem(const char *args)
{
    if (g_LaraItem == NULL) {
        Console_Log(GS(OSD_INVALID_LEVEL), args);
        return true;
    }

    int32_t num = 1;
    if (sscanf(args, "%d ", &num) == 1) {
        args = strstr(args, " ");
        if (!args) {
            return false;
        }
        args++;
    }

    for (const ITEM_NAME *desc = m_ItemNames; desc->obj_id != NO_OBJECT;
         desc++) {
        const char *desc_name = NULL;
        if (desc->name) {
            desc_name = desc->name;
        } else if (desc->inv_item) {
            desc_name = desc->inv_item->string;
        } else {
            assert(false);
        }
        if (desc_name == NULL) {
            continue;
        }

        if (!String_Equivalent(args, desc_name)) {
            continue;
        }

        if (g_Objects[desc->obj_id].loaded) {
            Inv_AddItemNTimes(desc->obj_id, num);
            Console_Log(GS(OSD_GIVE_ITEM), desc_name);
        } else {
            Console_Log(GS(OSD_UNAVAILABLE_ITEM));
        }

        return true;
    }

    if (String_Equivalent(args, "keys")) {
        Inv_AddItem(O_PUZZLE_ITEM1);
        Inv_AddItem(O_PUZZLE_ITEM2);
        Inv_AddItem(O_PUZZLE_ITEM3);
        Inv_AddItem(O_PUZZLE_ITEM4);
        Inv_AddItem(O_KEY_ITEM1);
        Inv_AddItem(O_KEY_ITEM2);
        Inv_AddItem(O_KEY_ITEM3);
        Inv_AddItem(O_KEY_ITEM4);
        Inv_AddItem(O_PICKUP_ITEM1);
        Inv_AddItem(O_PICKUP_ITEM2);
        Console_Log(GS(OSD_GIVE_ITEM_ALL_KEYS));
        return true;
    }

    if (String_Equivalent(args, "guns")) {
        Inv_AddItem(O_GUN_ITEM);
        Inv_AddItem(O_MAGNUM_ITEM);
        Inv_AddItem(O_UZI_ITEM);
        Inv_AddItem(O_SHOTGUN_ITEM);
        g_Lara.shotgun.ammo = g_GameInfo.bonus_flag & GBF_NGPLUS ? 10001 : 300;
        g_Lara.magnums.ammo = g_GameInfo.bonus_flag & GBF_NGPLUS ? 10001 : 1000;
        g_Lara.uzis.ammo = g_GameInfo.bonus_flag & GBF_NGPLUS ? 10001 : 2000;
        Console_Log(GS(OSD_GIVE_ITEM_ALL_GUNS));
        return true;
    }

    Console_Log(GS(OSD_INVALID_ITEM), args);
    return true;
}

static bool Console_Cmd_FlipMap(const char *args)
{
    bool flip = false;
    if (String_Equivalent(args, "on")) {
        if (g_FlipStatus) {
            Console_Log(GS(OSD_FLIPMAP_FAIL_ALREADY_OFF));
            return true;
        } else {
            flip = true;
        }
    }

    if (String_Equivalent(args, "off")) {
        if (!g_FlipStatus) {
            Console_Log(GS(OSD_FLIPMAP_FAIL_ALREADY_OFF));
            return true;
        } else {
            flip = true;
        }
    }

    if (strcmp(args, "") == 0) {
        flip = true;
    }

    if (flip) {
        Room_FlipMap();
        if (g_FlipStatus) {
            Console_Log(GS(OSD_FLIPMAP_ON));
        } else {
            Console_Log(GS(OSD_FLIPMAP_OFF));
        }
        return true;
    }

    return false;
}

static bool Console_Cmd_Kill(const char *args)
{
    if (String_Equivalent(args, "all")) {
        int32_t num = 0;
        for (int16_t item_num = 0; item_num < g_LevelItemCount; item_num++) {
            struct ITEM_INFO *item = &g_Items[item_num];
            if (Object_IsObjectType(item->object_number, g_EnemyObjects)
                && item->hit_points > 0) {
                Effect_ExplodingDeath(item_num, -1, 0);
                Sound_Effect(SFX_EXPLOSION_CHEAT, &item->pos, SPM_NORMAL);
                Item_Kill(item_num);
                Carrier_TestItemDrops(item_num);
                num++;
            }
        }
        if (num > 0) {
            Sound_Effect(SFX_EXPLOSION_CHEAT, &g_LaraItem->pos, SPM_NORMAL);
            Console_Log(GS(OSD_KILL_ALL), num);
        } else {
            Console_Log(GS(OSD_KILL_ALL_FAIL), num);
        }
        return true;
    }

    if (strcmp(args, "") == 0) {
        // kill all the enemies around Lara within one tile, or a single
        // nearest enemy
        bool found_anything = false;

        while (true) {
            const int16_t best_item_num = Lara_GetNearestEnemy();
            if (best_item_num == NO_ITEM) {
                break;
            }

            struct ITEM_INFO *item = &g_Items[best_item_num];
            const int32_t distance = Item_GetDistance(item, &g_LaraItem->pos);
            Effect_ExplodingDeath(best_item_num, -1, 0);
            Sound_Effect(SFX_EXPLOSION_CHEAT, &item->pos, SPM_NORMAL);
            Item_Kill(best_item_num);
            Carrier_TestItemDrops(best_item_num);
            found_anything = true;
            if (distance >= WALL_L) {
                break;
            }
        }

        if (found_anything) {
            Console_Log(GS(OSD_KILL));
        } else {
            Console_Log(GS(OSD_KILL_FAIL));
        }
        return true;
    }

    return false;
}

static bool Console_Cmd_EndLevel(const char *args)
{
    if (strcmp(args, "") == 0) {
        Lara_Cheat_EndLevel();
        return true;
    }
    return false;
}

static bool Console_Cmd_Level(const char *args)
{
    int32_t level_to_load = -1;

    if (level_to_load == -1) {
        int32_t num = 0;
        if (sscanf(args, "%d", &num) == 1) {
            level_to_load = num;
        }
    }

    if (level_to_load == -1 && strlen(args) >= 2) {
        for (int i = 0; i < g_GameFlow.level_count; i++) {
            if (String_CaseSubstring(g_GameFlow.levels[i].level_title, args)
                != NULL) {
                level_to_load = i;
                break;
            }
        }
    }

    if (level_to_load == -1 && String_Equivalent(args, "gym")) {
        level_to_load = g_GameFlow.gym_level_num;
    }

    if (level_to_load >= g_GameFlow.level_count) {
        Console_Log(GS(OSD_INVALID_LEVEL));
        return true;
    }

    if (level_to_load != -1) {
        g_GameInfo.select_level_num = level_to_load;
        g_LevelComplete = true;
        Console_Log(
            GS(OSD_PLAY_LEVEL), g_GameFlow.levels[level_to_load].level_title);
        return true;
    }

    return false;
}

static bool Console_Cmd_Abortion(const char *args)
{
    if (!g_Objects[O_LARA].loaded) {
        return false;
    }

    if (g_LaraItem->hit_points <= 0) {
        return true;
    }

    Effect_ExplodingDeath(g_Lara.item_number, -1, 0);
    Sound_Effect(SFX_EXPLOSION_CHEAT, &g_LaraItem->pos, SPM_NORMAL);
    Sound_Effect(SFX_LARA_FALL, &g_LaraItem->pos, SPM_NORMAL);
    g_LaraItem->hit_points = 0;
    g_LaraItem->flags |= IS_INVISIBLE;
    return true;
}

CONSOLE_COMMAND g_ConsoleCommands[] = {
    { .prefix = "fps", .proc = Console_Cmd_Fps },
    { .prefix = "pos", .proc = Console_Cmd_Pos },
    { .prefix = "tp", .proc = Console_Cmd_Teleport },
    { .prefix = "fly", .proc = Console_Cmd_Fly },
    { .prefix = "speed", .proc = Console_Cmd_Speed },
    { .prefix = "braid", .proc = Console_Cmd_Braid },
    { .prefix = "cheats", .proc = Console_Cmd_Cheats },
    { .prefix = "give", .proc = Console_Cmd_GiveItem },
    { .prefix = "gimme", .proc = Console_Cmd_GiveItem },
    { .prefix = "flip", .proc = Console_Cmd_FlipMap },
    { .prefix = "flipmap", .proc = Console_Cmd_FlipMap },
    { .prefix = "kill", .proc = Console_Cmd_Kill },
    { .prefix = "endlevel", .proc = Console_Cmd_EndLevel },
    { .prefix = "play", .proc = Console_Cmd_Level },
    { .prefix = "level", .proc = Console_Cmd_Level },
    { .prefix = "abortion", .proc = Console_Cmd_Abortion },
    { .prefix = "natlastinks", .proc = Console_Cmd_Abortion },
    { .prefix = NULL, .proc = NULL },
};
