#include "posix.h"

#include "shared/constants/globals.h"
#include "shared/sim/action.h"

const struct global_cfg_t gcfg = {
	.ents = {
		[et_none] = { "ghost", .trav = 0xff },
		[et_worker] = {
			"elf",
			.animate = true,
			.corpse = et_elf_corpse,
			.hp = 100,
			.lifespan = 20000,
			.trav = trav_land,
		},
		[et_elf_corpse] = {
			"elf corpse",
			.hp = 300,
			.lifespan = 100,
			.trav = 0xff
		},
		[et_deer] = {
			"deer",
			.animate = true,
			.corpse = et_resource_meat,
			.group_size = 3,
			.hp = 50,
			.lifespan = 4000,
			.spawn_chance = 10000,
			.spawn_tile = tile_plain,
			.trav = trav_land,
		},
		[et_fish] = {
			"fish",
			.animate = true,
			.corpse = et_resource_meat,
			.group_size = 3,
			.hp = 50,
			.lifespan = 4000,
			.spawn_chance = 10000,
			.spawn_tile = tile_sea,
			.trav = trav_aquatic,
		},
		[et_resource_meat] = {
			"meat",
			.holdable = true,
			.lifespan = 1000,
			.trav = 0xff,
		},
		[et_resource_crop] = {
			"crop",
			.holdable = true,
			.lifespan = 1000,
			.trav = 0xff,
		},
		[et_resource_wood] = {
			"wood",
			.holdable = true,
			.lifespan = 5000,
			.trav = 0xff,
		},
		[et_resource_rock] = {
			"rock",
			.holdable = true,
			.lifespan = 5000,
			.trav = 0xff,
		},
		[et_vehicle_boat] = {
			"boat",
			.lifespan = 0,
			.trav = trav_aquatic,
		},
		[et_storehouse] = {
			"storehouse",
			.phantom = true,
		},
	},
	.tiles = {
		[tile_sea] = {
			"sea",
			.trav_type = trav_aquatic,
		},
		[tile_coast] = {
			"coast",
			.base = tile_coast,
			.foundation = true,
			.next = tile_tree,
			.next_to = tile_sea,
			.trav_type = trav_land,
		},
		[tile_plain] = {
			"plain",
			.base = tile_dirt,
			.foundation = true,
			.next = tile_tree,
			.next_to = tile_tree,
			.trav_type = trav_land,
			.flamable = true,
		},
		[tile_tree] = {
			"tree",
			.base = tile_dirt,
			.drop = et_resource_wood,
			.hardness = 100,
			.next = tile_old_tree,
			.trav_type = trav_land,
			.flamable = true,
		},
		[tile_rock] = {
			"rock",
			.base = tile_dirt,
			.drop = et_resource_rock,
			.hardness = 253,
		},
		[tile_dirt] = {
			"dirt",
			.base = tile_dirt,
			.foundation = true,
			.next = tile_plain,
			.next_to = tile_dirt,
			.trav_type = trav_land,
		},
		[tile_old_tree] = {
			"old tree",
			.base = tile_dirt,
			.hardness = 50,
			.next = tile_dirt,
			.trav_type = trav_land,
			.flamable = true,
		},
		[tile_fire] = {
			"fire",
			.base = tile_ash,
			.function = true,
			.trav_type = trav_land,
			.build = blpt_rect,
		},
		[tile_ash] = {
			"ashes",
			.base = tile_dirt,
			.foundation = true,
			.trav_type = trav_land,
		},
	},
	/* Fields ending in _rate specify the number of ticks before some event
	 * happens.  Fields ending in _chance specify a 1 in that number chance
	 * of an event happening.
	 */
	.misc = {
		.spawnable_ents = { et_deer, et_fish },
		/* Delay before a shrine consumes a resource and spawns an elf */
		.shrine_spawn_rate = 64,
		/* radius of circle a shrine looks for food in */
		.shrine_range = 5,
		/* time it takes to grow */
		.farm_grow_rate = 256,
		/* damage taken by ents in fire */
		.fire_damage = 50,
		/* time before fire can spread */
		.fire_spread_rate = 10,
		/* chance of spreading */
		.fire_spread_chance = 10,
		/* once spreading, per tile chance of igniting */
		.fire_spread_ignite_chance = 2,
		/* random % this number produces starting x and y spawn coords */
		.initial_spawn_range = 100,
		/* amount of starting elves */
		.initial_spawn_amount = 64,
		/* chance an idle ent will move randomly */
		.meander_chance = 55,
		/* maximum amount an ent can age over its lifespan before dying */
		.max_over_age = 1000,

		/* when growing terrain, first adjacent tiles that match the
		 * current tiles .next_to are counted.
		 *
		 * If this number is greater than 0, the chance of growing is
		 * terrain_base_adj_grow_chance / adjacent count
		 */
		.terrain_base_adj_grow_chance = 2000,
		/* If this number is 0, the chance of growing is
		 * terrain_base_not_adj_grow_chance
		 */
		.terrain_base_not_adj_grow_chance = 0x40000,
		/* when generating a new chunk, it is "aged"
		 * terrain_initial_age_multiplier * random() % terrain_initial_age_max
		 * times
		 */
		.terrain_initial_age_multiplier = 10,
		.terrain_initial_age_max = 100,

		.max_hunger = 2000,
		.get_hungry_chance = 10,
		.food_search_cooldown = 64,
	}
};
