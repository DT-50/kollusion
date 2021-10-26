#pragma once
#include <vector>
#include "../kollusion/csgo_sdk.h"
#include "../kollusion/dx_overlay.h"

namespace esp {
	struct esp_entity_t {
		bool valid = false;
		BYTE flag;
		int health = 0;
		std::string name = "";
		sdk::vec3_t origin, top_origin;
	};
	extern std::vector< esp_entity_t > entities;

	struct GlowStruct
	{
		BYTE pad_0000[8]; //0x0000
		float Red; //0x0008
		float Green; //0x000C
		float Blue; //0x0010
		float Alpha; //0x0014
		BYTE pad_0018[16]; //0x0018
		bool renderOccluded; //0x0028
		bool renderUnoccluded; //0x0029
	};
	static_assert(sizeof(GlowStruct) == 0x2C);

	struct ChamStruct
	{
		UINT8 Red, Green, Blue;
	};
	//sdk
	
	void getteam(forceinline::memory_manager* memory, forceinline::dx_renderer* renderer);
	void getplayerhp(forceinline::memory_manager* memory, forceinline::dx_renderer* renderer);
	void getstate(forceinline::memory_manager* memory);

	//features
	extern void noflash(forceinline::memory_manager* memory, forceinline::dx_renderer* renderer);
	extern void bhop(forceinline::memory_manager* memory, forceinline::dx_renderer* renderer);
	extern void draw(forceinline::memory_manager* memory, forceinline::dx_renderer* renderer);
	void trigger(forceinline::memory_manager* memory, forceinline::dx_renderer* renderer);
	void glow(forceinline::memory_manager* memory);
}