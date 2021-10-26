#include "esp.h"
#include <iostream>

#include "dx_overlay.h"
#include "math.h"

//OFFSETS
#define isLocal 0xDA544C
#define flagsCS 0x104
#define flFlashDuration 0x10470
#define viewmatrix 0x4DB10A4
#define dwForceJump 0x52695A0
#define iTeamNum 0xF4
#define radarbase 0x51F4504
#define dwClientState 0x588FEC
#define dwEntityList 0x4DBF78C
#define dwClientState_State 0x108
#define iCrosshairId 0x11838
#define dwForceAttack 0x31EFD04
#define s_EntityLoop 0x10
#define dwGlowObjectManager 0x5307C78
#define dwClientState_MaxPlayer 0x388
#define m_bDormant 0xED
#define m_iGlowIndex 0x10488
#define m_bIsDefusing 0x997C
#define m_bSpotted 0x93D
#define m_clrRender  0x70
constexpr auto ihealthj = 0x100;
constexpr auto dwBoneMatrix = 0x26A8;

sdk::vec3_t screen_size(1920, 1080, 0);




void log12(std::string_view message) {
	std::cout << "[+] flash in your vicinity! " << message << std::endl;
}

void log1(std::string_view message) {
	std::cout << "[+]  " << message << std::endl;
}

bool ct = false;
bool t = false;
bool ingame = false;

namespace esp {
	std::vector< esp_entity_t > entities(64);



	void getstate(forceinline::memory_manager* memory)
	{
		static DWORD engine_dll = memory->get_module_base("engine.dll");

		DWORD enginePtr = memory->read<DWORD>(engine_dll + dwClientState);

		int gamestate = memory->read<int>(enginePtr + dwClientState_State);

		if (gamestate == 6) {
			ingame = true;
		}


	}
	int delay = 10;
	void trigger(forceinline::memory_manager* memory, forceinline::dx_renderer* renderer)
	{
		
		//local info
		static DWORD client_dll = memory->get_module_base("client.dll");

		DWORD localplayer = memory->read<DWORD >(client_dll + isLocal);

		uint32_t health = memory->read<uint32_t>(localplayer + ihealthj);

		uint32_t crosshairid = memory->read<uint32_t>(localplayer + iCrosshairId);

		uint32_t team = memory->read<uint32_t>(localplayer + iTeamNum);

		//enemy info

		DWORD enemyplayer = memory->read<DWORD>(client_dll + dwEntityList + ((crosshairid - 1) * s_EntityLoop));


		uint32_t healthE = memory->read<uint32_t>(enemyplayer + ihealthj);

		uint32_t teamE = memory->read<uint32_t>(enemyplayer + iTeamNum);

		int offset = 9;
		if (GetAsyncKeyState(VK_ADD)) {
			Beep(525, 100);
			delay += 10;
			

		}
		
		if (GetAsyncKeyState(VK_SUBTRACT)) {
			Beep(425, 100);
			delay -= 10;

		}

		if (delay > 100) {
			offset = 10;
		}

		renderer->draw_text(std::to_string(delay), offset, 1040, 0xFFFFFFFF);
		if (health > 0 && healthE > 0) {
			if (team != teamE) {
				std::this_thread::sleep_for(std::chrono::milliseconds(delay));



				memory->write<uint32_t>(client_dll + dwForceAttack, sizeof(std::uint32_t) + 1);
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				memory->write<uint32_t>(client_dll + dwForceAttack, sizeof(std::uint32_t));
			}
		}
	}

	void glow(forceinline::memory_manager* memory)
	{

	}

	void getteam(forceinline::memory_manager* memory, forceinline::dx_renderer* renderer) {

		static DWORD client_dll = memory->get_module_base("client.dll");
		DWORD playeraddr = memory->read<DWORD>(client_dll + isLocal);
		int team = memory->read<INT>(playeraddr + iTeamNum);

		if (team == 2) {
			ct = 1;
			t = 0;
			renderer->draw_text("terrorist", 22, 1058, 0xFFFFFFFF);
		}
		if (team == 3) {
			ct = 0;
			t = 1;
			renderer->draw_text("counter-terrorist", 42, 1058, 0xFFFFFFFF);
		}

	}

	void getplayerhp( forceinline::memory_manager* memory, forceinline::dx_renderer* renderer)
	{
		static DWORD client_dll5 = memory->get_module_base("client.dll");
		DWORD playeraddr = memory->read<DWORD>(client_dll5 + isLocal);
		int HP = memory->read<int>(playeraddr + ihealthj);
		if (ingame == true) {
			if (HP) {
				if (HP < 0) {
					renderer->draw_text("0", 9, 1050, 0xFFFFFFFF);
				}
				if (HP > 100) {
					renderer->draw_text("over 100", 11, 1050, 0xFFFFFFFF);
				}
				else {
					renderer->draw_text(std::to_string(HP), 9, 1050, 0xFFFFFFFF);
				}
			}
		}
	}


	void bhop(forceinline::memory_manager* memory, forceinline::dx_renderer* renderer) {
		//get module
		
			static DWORD client_dll3 = memory->get_module_base("client.dll");
			//get current flag
			// 
			//FOR NOW, FLAGS ARE DISABLED AS I CANT FIGURE IT OUT, WORKS FINE WITHOUT
			// 
			//DWORD flags = memory->read<BYTE>(isLocal + flagsCS);
			//gets local player
			static DWORD localPlayer = memory->read<BYTE>(client_dll3 + isLocal);

			//actual bhop
			if (GetAsyncKeyState(VK_SPACE)/* && flags & (1 << 0)*/) {
			
				memory->write<BYTE>(client_dll3 + dwForceJump, 6);
				Sleep(1);
			}
		
	}

	void noflash(forceinline::memory_manager* memory, forceinline::dx_renderer* renderer) {
		//get module
		bool flash = false;
			static DWORD client_dll2 = memory->get_module_base("client.dll");
			//get local player
			DWORD localplayer1 = memory->read<DWORD >(client_dll2 + isLocal);
			//create variable to store flashDuration
			int flashDuration = 0;
			//set flashDuration to the current flashDuration
			flashDuration = memory->read<int>(localplayer1 + flFlashDuration);
			//antiflash (rethink this)
			if (flashDuration > 0) {
				log12("");
				memory->write<int>(localplayer1 + flFlashDuration, 0);
				
			}

	
	}


	void draw(forceinline::memory_manager* memory, forceinline::dx_renderer* renderer) {
		//create our variables for screnpos
		sdk::vec3_t screen_bottom, screen_top;

		//get base module
		static std::uintptr_t client_dll = memory->get_module_base("client.dll");

		//grab view matrix
		sdk::view_matrix_t view_matrix = memory->read< sdk::view_matrix_t >(client_dll + viewmatrix); //view_matrix

	
		//Doesn't work; flickers a LOT for some reason.
		//static std::uintptr_t crender_bytes = memory->find_pattern( "engine.dll", "B9 ? ? ? ? A1 ? ? ? ? FF 60 38" ) + 1;
		//static std::uintptr_t crender_offset = memory->read< std::uintptr_t >( crender_bytes );

		//sdk::view_matrix_t view_matrix = memory->read< sdk::view_matrix_t >( crender_offset + 39 * 4 );

			//Loop our vector containing the entities
			for (auto& ent : entities) {
				//If our entity isn't valid, skip it
				if (!ent.valid)
					continue;

				//Convert 3D space coordinates to 2D screen coordinates
				if (!sdk::world_to_screen(screen_size, ent.origin, screen_bottom, view_matrix) || !sdk::world_to_screen(screen_size, ent.top_origin, screen_top, view_matrix))
					continue;

				//Box properties
				int box_height = screen_bottom.y - screen_top.y;
				int box_width = box_height / 2;

				//Draw the ESP
				renderer->draw_outlined_rect(screen_top.x - box_width / 2, screen_top.y, box_width, box_height, D3DCOLOR_RGBA(119, 136, 153, 255));
				renderer->draw_text(ent.name, screen_top.x, screen_top.y - 14, 0xFFFFFFFF, true);
				renderer->draw_text(std::to_string(ent.health) + " HP", screen_top.x, screen_bottom.y, 0xFFFFFFFF);

			}
		
	}
}