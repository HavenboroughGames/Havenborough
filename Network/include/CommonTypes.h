/**
 * File comment.
 */

#pragma once

#include <Utilities/XMFloatUtil.h>

#include <cstdint>

/**
 * Opaque Package type. Only to be used as a reference while using the network API.
 */
typedef unsigned int Package;

/**
 * Supported package types, used to prevent collisions.
 */
enum class PackageType : uint16_t
{
	RESERVED = 0,
	PLAYER_READY,
	CREATE_OBJECTS,
	REMOVE_OBJECTS,
	UPDATE_OBJECTS,
	GAME_RESULT,
	OBJECT_ACTION,
	ASSIGN_PLAYER,
	PLAYER_CONTROL,
	DONE_LOADING,
	JOIN_GAME,
	LEAVE_GAME,
	LEVEL_DATA,
};

struct ObjectInstance
{
	Vector3 m_Position;
	Vector3 m_Rotation;
	uint16_t m_DescriptionIdx;
	uint16_t m_Id;
};

struct UpdateObjectData
{
	Vector3 m_Position;
	Vector3 m_Velocity;
	Vector3 m_Rotation;
	Vector3 m_RotationVelocity;
	uint16_t m_Id;
	uint16_t _padding_;
};

struct PlayerControlData
{
	Vector3 m_Velocity;
	Vector3 m_Rotation;
};

/**
 * Result codes for API use.
 */
enum class Result
{
	SUCCESS,
	FAILURE,
};
/**
 * Callback type for reporting that an action has been completed.
 */
typedef void (*actionDoneCallback)(Result, void* p_UserData);
