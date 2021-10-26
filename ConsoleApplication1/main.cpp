#define _CRT_SECURE_NO_WARNINGS
#include "../ConsoleApplication1/dx_renderer.h"
#include "../ConsoleApplication1/dx_overlay.h"
#include "../ConsoleApplication1/csgo_sdk.h"
#include "../ConsoleApplication1/esp.h"

#include <mutex>
#include <iostream>
#include <cassert>

//OFFSETS
#define DwClientState 0x588FEC
#define entitylist 0x4DBF78C
#define localPlayer 0x180

//Custom logging functions
void log(std::string_view message) {
	std::cout << "[+] " << message << std::endl;
}


std::string path()
{
	char shitter[_MAX_PATH]; // defining the path
	GetModuleFileNameA(NULL, shitter, _MAX_PATH); // getting the path
	return std::string(shitter); //returning the path
}

void log_and_exit(std::string_view message) {
	log(message);
	std::this_thread::sleep_for(std::chrono::seconds(3));
	exit(-1);
}


int main(_In_ HINSTANCE instance, _In_opt_ HINSTANCE prev_instance, _In_ LPWSTR p_cmd_line, _In_ int cmd_show) {
	wWinMain(instance, prev_instance, p_cmd_line, cmd_show);
}

void clear() {
	COORD topLeft = { 0, 0 };
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO screen;
	DWORD written;

	GetConsoleScreenBufferInfo(console, &screen);
	FillConsoleOutputCharacterA(
		console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written
	);
	FillConsoleOutputAttribute(
		console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
		screen.dwSize.X * screen.dwSize.Y, topLeft, &written
	);
	SetConsoleCursorPosition(console, topLeft);
}
int APIENTRY wWinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE prev_instance, _In_ LPWSTR p_cmd_line, _In_ int cmd_show) {
	AllocConsole();
	freopen("conout$", "w", stdout);
	SetConsoleTitleA("kollusion EXTERNAL v0.1");
	try {
		//Create overlay and memory objects
		forceinline::memory_manager memory("csgo.exe");
		forceinline::dx_overlay overlay(L"Valve001", L"Counter-Strike: Global Offensive", false);

		//If our constructor didn't throw, these will have to be true
		assert(memory.is_attached());
		assert(overlay.is_initialized());

	
	

		log("attaching to process.");

		Sleep(1500);
		clear();
		log("created overlay successfully, enjoy!");

		Sleep(1500);
		//Create a mutex so we can multithread safely
		std::mutex ent_mtx;

		//Grab the base of the modules
		sdk::modules_t modules(&memory);

		//Create a thread to read info so we don't slow down our rendering part
		std::thread read_ent_info([&]() -> void {
			std::vector< sdk::ent_info_t > ent_info(64);

			while (1) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));

				//Lock our mutex so our ESP thread doesn't do dumb shit
				std::unique_lock lock(ent_mtx);

				//Invalidate all ESP entities as we're updating them
				for (auto& esp_ent : esp::entities)
					esp_ent.valid = false;

				//Get clientstate for the entitylist
				std::uintptr_t client_state = memory.read< std::uintptr_t >(modules.engine_dll + DwClientState); //m_dwClientState

				//Read the whole entity list at once
				memory.read_ex< sdk::ent_info_t >(ent_info.data(), modules.client_dll + entitylist, ent_info.size()); //m_dwEntityList

				//Get our local player ptr
				int local_player_idx = memory.read< int >(client_state + localPlayer); //m_dwClientState_GetLocalPlayer
				std::uintptr_t local_ptr = ent_info[local_player_idx].entity_ptr; // ERROR HERE

				//Is our local player ptr valid?
				if (!local_ptr)
					continue;

				//Get our local player
				sdk::entity_t local(&memory, &modules, local_ptr);

				//Gather entity information for our ESP
				for (std::size_t i = 0; i < ent_info.size(); i++) {
					std::uintptr_t ent_ptr = ent_info[i].entity_ptr;

					//Entity is invalid, don't draw on ESP
					if (!ent_ptr)
						continue;

					//Create an entity object so we can get information the easy way
					sdk::entity_t entity(&memory, &modules, ent_ptr);

					//Continue if entity is dormant or dead
					if (entity.dormant() || !entity.is_alive())
						continue;

					//We don't want to draw ESP on our team
					if (entity.team() == local.team())
						continue;

					//We have a valid entity, get a reference to it for ease of use
					esp::esp_entity_t& esp_entity = esp::entities[i];

					//Get entity information for our ESP
					esp_entity.health = entity.health();
					entity.get_name(esp_entity.name);
					esp_entity.origin = entity.origin();
					esp_entity.top_origin = esp_entity.origin + sdk::vec3_t(0.f, 0.f, 75.f);

					//Our ESP entity is now valid to draw
					esp_entity.valid = true;
				}
			}
			});
		clear();
		log("successfully read memory, rendering!");
		Sleep(2500); 
		clear();
		log("welcome to kollusion!!\n");
		log("current status: ONLINE\n");
		log("features: \n");
		log("     - bhop\n");
		log("     - antiflash\n");
		log("     - stream-proof esp\n");
		log("     - external information on overlay\n");
		log("     - triggerbot with delay\n");

		//MSG struct for WndProc
		MSG m;
		ZeroMemory(&m, sizeof m);

		//Get our overlay renderer
		forceinline::dx_renderer renderer = overlay.create_renderer();

		//Message and rendering loop
		do {
			if (PeekMessage(&m, overlay.get_overlay_wnd(), NULL, NULL, PM_REMOVE)) {
				TranslateMessage(&m);
				DispatchMessage(&m);
			}



			//Lock the mutex so we don't fuck shit up
			std::unique_lock lock(ent_mtx);
			DWORD PlayerAddr = NULL;
			const uint32_t boneId = 8;
			//Render our ESP
			renderer.begin_rendering();
			renderer.draw_text("kollusion external (October 25, 2021)", 92, 1067, 0xFFFFFFFF, true, true);
			esp::draw(&memory, &renderer);
			esp::bhop(&memory, &renderer);
			esp::noflash(&memory, &renderer);
			esp::getteam(&memory, &renderer);
			esp::getplayerhp(&memory, &renderer);
			esp::getstate(&memory);
			esp::trigger(&memory, &renderer);
			esp::glow(&memory);
	
			renderer.end_rendering();


			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		} while (m.message != WM_QUIT);
		clear();
		log("exiting...");
		std::this_thread::sleep_for(std::chrono::seconds(3));

		return 0;
	}
	catch (const std::exception& e) {
		//Catch and log any exceptions
		log_and_exit(e.what());
	}
}