#pragma once
#include <SDL.h>

#include "gf3d_vector.h"

/*
	File defining concept Entity system. 
	Mostly based on code from engineerOfLies's gf2d project, with adjustments to follow the requirements of the game
*/


typedef enum{
	ES_Idle,
	ES_Moving,
	ES_Inactive,
	ES_TrickStart,		
	ES_Trick,				//In Process of Performing Trick
	ES_TrickFail,			//Trick Failed
	ES_TrickSucc			//Trick Successful
}EntityState;


typedef enum {
	BOT,
	PLAYER,
	WEAPON,
}EntityType;


typedef struct Entity_S {
	
	/*entity info*/
	Uint64 id;
	EntityState state;
	EntityType	type;

	/*position and physix*/
	Vector3D point;			//position of entity
	Vector3D velocity;		//movement direction
	Vector3D acceleration; 

	/*grafix*/
	//Not implemented yet

	/*sound*/
	//Not implemented yet

	/*operations*/
	
	void(*draw)(struct Entity_S *self);													/**<called after system entity drawing for custom effects*/
	void(*think)(struct Entity_S *self);												/**<called before system updates to make decisions / hand input*/
	void(*update)(struct Entity_S *self);												/**<called after system entity update*/
	int(*touch)(struct Entity_S *self, struct Entity_S *other);							/**<when this entity touches another entity*/
	int(*damage)(struct Entity_S *self, int amount, struct Entity_S *source);			/**<when this entity takes damage*/
	void(*die)(struct Entity_S *self);													/**<when this entity dies*/
	void(*free)(struct Entity_S *self);													/**<called when the entity is freed for any custom cleanup*/
	int dead;																			/**<when true, the entity system will delete the entity on the next update*/

	/*specific parameters*/

	float	maxSpeed;
	float	speed;
	int		powerAbil;			//Tracks the amount of "power chunks" the entity has (applicable only for players and bots)
	int		health;
	int		maxHealth;

}Entity;


/*
	begin entity system
	maxEntities limits amount of active entities
*/
void entity_sys_start(Uint32 maxEntities);

/*
	adds new entity to the system
	returns NULL when no more entities
*/
Entity *new_entity();

/*
	cleans up entity and puts it back in system
*/
void free_entity(Entity *self);

/*
	draw a single entity
*/
void draw_entity(Entity *self);

/*
	update all active entities
*/
void update_all_ent();

/*
	draws all active entities
*/
void draw_all_ent();

/*
	calls think fucntion for all active entities
*/
void think_all_ent();

/**
	deal damage to target entity (if applicable)
	
	* "target":		the entity to take damage
	* "inflictor":	the entity doing the damage
	* "attacker":	the entity who gets credit for damaging
*/
int entity_damage(Entity *target, Entity *inflictor, Entity *attacker, int damage);