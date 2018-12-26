/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */

#ifndef MOVEDEF_HANDLER_H
#define MOVEDEF_HANDLER_H

#include <vector>
#include <string>

#include "System/float3.h"
#include "System/UnorderedMap.hpp"
#include "System/creg/creg_cond.h"

class MoveDefHandler;
class CSolidObject;
class LuaTable;


struct MoveDef {
	CR_DECLARE_STRUCT(MoveDef)

	MoveDef();
	MoveDef(const LuaTable& moveDefTable, int moveDefID);

	bool TestMoveSquareRange(
		const CSolidObject* collider,
		const float3 rangeMins,
		const float3 rangeMaxs,
		const float3 testMoveDir,
		bool testTerrain = true,
		bool testObjects = true,
		bool centerOnly = false,
		float* minSpeedModPtr = nullptr,
		int* maxBlockBitPtr = nullptr
	) const;
	bool TestMoveSquare(
		const CSolidObject* collider,
		const float3 testMovePos,
		const float3 testMoveDir,
		bool testTerrain = true,
		bool testObjects = true,
		bool centerOnly = false,
		float* minSpeedModPtr = nullptr,
		int* maxBlockBitPtr = nullptr
	) const {
		return (TestMoveSquareRange(collider, testMovePos, testMovePos, testMoveDir, testTerrain, testObjects, centerOnly, minSpeedModPtr, maxBlockBitPtr));
	}

	// aircraft and buildings defer to UnitDef::floatOnWater
	bool FloatOnWater() const { return (speedModClass == MoveDef::Hover || speedModClass == MoveDef::Ship); }

	float CalcFootPrintRadius(float scale) const;
	float CalcMinimalBoundingFootPrintRadius() const;
	float GetDepthMod(float height) const;
	float CalcFootPrintStretchingFactor() const;

	unsigned int GetCheckSum() const;

	static float GetDefaultMinWaterDepth() { return -1e6f; }
	static float GetDefaultMaxWaterDepth() { return +1e6f; }

	/// determines which of the {tank,kbot,hover,ship}Speed
	/// modifiers this MoveDef receives from a terrain-type
	enum SpeedModClass {
		Tank  = 0,
		KBot  = 1,
		Hover = 2,
		Ship  = 3
	};
	enum TerrainClass {
		Land  = 0, /// we are restricted to "land" (terrain with height >= 0)
		Water = 1, /// we are restricted to "water" (terrain with height < 0)
		Mixed = 2, /// we can exist at heights both greater and smaller than 0
	};
	enum DepthModParams {
		DEPTHMOD_MIN_HEIGHT = 0,
		DEPTHMOD_MAX_HEIGHT = 1,
		DEPTHMOD_MAX_SCALE  = 2,
		DEPTHMOD_QUA_COEFF  = 3,
		DEPTHMOD_LIN_COEFF  = 4,
		DEPTHMOD_CON_COEFF  = 5,
		DEPTHMOD_NUM_PARAMS = 6,
	};
	enum SpeedModMults {
		SPEEDMOD_MOBILE_IDLE_MULT = 0,
		SPEEDMOD_MOBILE_BUSY_MULT = 1,
		SPEEDMOD_MOBILE_MOVE_MULT = 2,
		SPEEDMOD_MOBILE_NUM_MULTS = 3,
	};

	std::string name;

#pragma pack(push, 1)
	SpeedModClass speedModClass;
	TerrainClass terrainClass;

	/// of the footprint
	int xsize, xsizeh;
	int zsize, zsizeh;

	/// minWaterDepth for ships, maxWaterDepth otherwise
	/// controls movement and (un-)loading constraints
	float depth;
	float depthModParams[DEPTHMOD_NUM_PARAMS];
	float maxSlope;
	float slopeMod;
	float crushStrength;

	// PF speedmod-multipliers for squares blocked by mobile units
	// (which can respectively be "idle" == non-moving and have no
	// orders, "busy" == non-moving but have orders, or "moving")
	// NOTE:
	//     includes one extra padding element to make the moveMath
	//     member start on an 8-byte boundary for 64-bit platforms
	float speedModMults[SPEEDMOD_MOBILE_NUM_MULTS + 1];

	unsigned int pathType;

	/// heatmap path-cost modifier
	float heatMod;
	float flowMod;

	/// heat produced by a path
	int heatProduced;

	/// do we stick to the ground when in water?
	bool followGround;
	/// are we supposed to be a purely sub-surface ship?
	bool subMarine;

	/// do we try to pathfind around squares blocked by mobile units?
	///
	/// this also serves as a padding byte for alignment so compiler
	/// does not insert it (GetCheckSum would need to skip such bytes
	/// otherwise, since they are never initialized)
	bool avoidMobilesOnPath;
	bool allowTerrainCollisions;
	bool allowRawMovement;

	/// do we leave heat and avoid any left by others?
	bool heatMapping;
	bool flowMapping;
#pragma pack(pop)
};



class LuaParser;
class MoveDefHandler
{
	CR_DECLARE_STRUCT(MoveDefHandler)
public:
	void Init(LuaParser* defsParser);
	void Kill() {
		moveDefs.clear();
		moveDefNames.clear(); // never iterated
	}

	MoveDef* GetMoveDefByPathType(unsigned int pathType) { return &moveDefs[pathType]; }
	MoveDef* GetMoveDefByName(const std::string& name);

	unsigned int GetNumMoveDefs() const { return moveDefs.size(); }
	unsigned int GetCheckSum() const { return checksum; }

private:
	std::vector<MoveDef> moveDefs;
	spring::unordered_map<std::string, int> moveDefNames;

	unsigned int checksum = 0;
};

extern MoveDefHandler moveDefHandler;

#endif // MOVEDEF_HANDLER_H

