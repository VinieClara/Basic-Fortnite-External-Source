

#include <dwmapi.h>
#include <vector>
#include "Headers/overlay.h"
#include "Headers/aimbot.h"
#define STB_IMAGE_IMPLEMENTATION
#include <Headers/stb_image.h>

#include "Headers/auth.hpp"
#include "Headers/Protect.h"
#include <string>
std::string tm_to_readable_time(tm ctx);
static std::time_t string_to_timet(std::string timestamp);
static std::tm timet_to_tm(time_t timestamp);

int normalsleep = 2500;
int airsleep = 300;


bool activate = 0; // variable for action (for example activation)
bool vsyncenable = true;
int center_x;
int center_y;
DWORD Pid;

struct player
{

	uint64_t Actor;
	uint64_t Mesh;
	string Name;
	uint64_t RootComp;
	bool valid;
	bool IsOnScreen;
	bool enemy;
	bool isbot;
};

struct item
{

	uint64_t Actor;
	uint64_t Mesh;
	string Name;
	uint64_t RootComp;
	bool IsOnScreen;

};
struct actor_data {
	char pad_0[680]; // 0x2A8
	uintptr_t Sate;

	char pad_1[96]; // 0x310 
	uintptr_t Mesh;

};

std::vector<player> player_pawns;
std::vector<item> item_pawns;
inline bool operator==(const player& a, const player& b) {
	if (a.Actor == b.Actor)
		return true;

	return false;
}

int my_image_width = 0;
int my_image_height = 0;
ID3D11ShaderResourceView* my_texture = NULL;

float enemydistlimit = 100;

/*float lootrenderdist = 150;
void CacheLevels()
{
	for(;;)
	{


		if (Hopesar::Data::LocalPawn)
		{

			if (!Hopesar::Data::Uworld) continue;
			std::vector<item> mrxd;
			uintptr_t ItemLevels = driver.read<uintptr_t>(Hopesar::Data::Uworld + 0x170);

			for (int i = 0; i < driver.read<DWORD>(Hopesar::Data::Uworld + (0x170 + sizeof(PVOID))); ++i) {

				uintptr_t ItemLevel = driver.read<uintptr_t>(ItemLevels + (i * sizeof(uintptr_t)));

				for (int i = 0; i < driver.read<DWORD>(ItemLevel + (0x98 + sizeof(PVOID))); ++i) {
					uintptr_t ItemsPawns = driver.read<uintptr_t>(ItemLevel + 0x98);
					uintptr_t CurrentItemPawn = driver.read<uintptr_t>(ItemsPawns + (i * sizeof(uintptr_t)));
					uintptr_t ItemRootComponent = driver.read<uintptr_t>(CurrentItemPawn + ROOTCMP_OFFSET);
					Vector3 ItemPosition = driver.read<Vector3>(ItemRootComponent + RELATIVELOC_OFFSET);
					float ItemDist = Hopesar::Data::RootPos.Distance(ItemPosition) / 100.f;


						int ItemIndex = driver.read<int>(CurrentItemPawn + 0x18);
						auto CurrentItemPawnName = GetNameFromFName(ItemIndex);

						if ((strstr(CurrentItemPawnName.c_str(), ("Tiered_Chest"))))
						{
							item Item{ };
							Item.Actor = CurrentItemPawn;
							Item.Mesh = driver.read<uint64_t>(CurrentItemPawn + MESH_OFFSET);
							Item.Name = CurrentItemPawnName;
							Item.RootComp = ItemRootComponent;
							Item.IsOnScreen = true;
							Item.valid = true;


							mrxd.push_back(Item);

						}


				}
			}
			item_pawns.clear();
			item_pawns = mrxd;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
}
*/
bool InLobby;
bool skydiving;
bool SleepLess;

bool cheatfps;
bool playercounts = true;
bool sleepdelays;
bool enemyworning;


void CacheWorld()
{
	SPOOF_FUNC;


	uintptr_t ItemLevels = driver.read<uintptr_t>(Hopesar::Data::Uworld + 0x170);

	for (int i = 0; i < driver.read<DWORD>(Hopesar::Data::Uworld + (0x170 + sizeof(PVOID))); ++i) {

		uintptr_t ItemLevel = driver.read<uintptr_t>(ItemLevels + (i * sizeof(uintptr_t)));
		int LevelsSize = driver.read<DWORD>(ItemLevel + (0x98 + sizeof(PVOID)));
		for (int uwur = 0; uwur < LevelsSize; ++uwur) {
		
			uintptr_t ItemsPawns = driver.read<uintptr_t>(ItemLevel + 0x98);
			uintptr_t CurrentItemPawn = driver.read<uintptr_t>(ItemsPawns + (uwur * sizeof(uintptr_t)));
				int ItemIndex = driver.read<int>(CurrentItemPawn + 0x18);
				std::string CurrentItemPawnName = GetNameFromFName(ItemIndex);

				std::cout << CurrentItemPawnName << skCrypt(" || On Level: ") << i << skCrypt(" || Size Of Level: ") << LevelsSize << endl;
 		}
	}

}

// Simple helper function to load an image into a DX11 texture with common settings
bool LoadTextureFromFile(const char* filename, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height)
{
	// Load from disk into a raw RGBA buffer
	int image_width = 0;
	int image_height = 0;
	unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
	if (image_data == NULL)
		return false;

	// Create texture
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = image_width;
	desc.Height = image_height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;

	ID3D11Texture2D* pTexture = NULL;
	D3D11_SUBRESOURCE_DATA subResource;
	subResource.pSysMem = image_data;
	subResource.SysMemPitch = desc.Width * 4;
	subResource.SysMemSlicePitch = 0;
	pD3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);
	
	// Create texture view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	pD3dDevice->CreateShaderResourceView(pTexture, &srvDesc, out_srv);
	pTexture->Release();

	*out_width = image_width;
	*out_height = image_height;
	stbi_image_free(image_data);

	return true;
}

void actor_cahce()
{
	SPOOF_FUNC;

	bool Debug = true;
	while (true)
	{
		try
		{

			
			

			HWND abdul = FindWindowA_Spoofed(0, skCrypt("Fortnite  "));
			if (!abdul)
			{
				exit(0);
				*(uintptr_t*)(0) = 0;

			}
			std::vector<player> tmpList;

			if (Debug) { printf(skCrypt("\n Base Addy: %p"), Hopesar::Game::base_addy); }

			Hopesar::Data::Uworld = driver.read<DWORD_PTR>(Hopesar::Game::base_addy + realworld);
			if (Debug) { printf(skCrypt("\n Uworld: %p"), Hopesar::Data::Uworld); }

			Hopesar::Data::GameInstances = driver.read<DWORD_PTR>(Hopesar::Data::Uworld + GAMEINST_OFFSET);
			if (Debug) { printf(skCrypt("\n GameInstances: %p"), Hopesar::Data::GameInstances); }


			Hopesar::Data::LocalPlayers = driver.read<DWORD_PTR>(Hopesar::Data::GameInstances + LOCALPLYR_OFFSET);
			if (Debug) { printf(skCrypt("\n LocalPlayers: %p"), Hopesar::Data::LocalPlayers); }


			Hopesar::Data::LocalPlayer = driver.read<DWORD_PTR>(Hopesar::Data::LocalPlayers);
			if (Debug) { printf(skCrypt("\n LocalPlayer: %p"), Hopesar::Data::LocalPlayer); }


			Hopesar::Data::PlayerController = driver.read<DWORD_PTR>(Hopesar::Data::LocalPlayer + CONTROLLER_OFFSET);
			if (Debug) { printf(skCrypt("\n PlayerController: %p"), Hopesar::Data::PlayerController); }


			Hopesar::Data::LocalPawn = driver.read<DWORD_PTR>(Hopesar::Data::PlayerController + ACKNOWLEDGEPWN_OFFSET);
			if (Debug) { printf(skCrypt("\n LocalPawn: %p"), Hopesar::Data::LocalPawn); }


			Hopesar::Data::Rootcomp = driver.read<DWORD_PTR>(Hopesar::Data::LocalPawn + ROOTCMP_OFFSET);
			if (Debug) { printf(skCrypt("\n Rootcomp: %p"), Hopesar::Data::Rootcomp); }


			Hopesar::Data::PlayerState = driver.read<DWORD_PTR>(Hopesar::Data::LocalPawn + PLAYERSTATE_OFFSET);
			if (Debug) { printf(skCrypt("\n PlayerState: %p"), Hopesar::Data::PlayerState); }


			InLobby = false;
			if (!Hopesar::Data::LocalPawn) {
				InLobby = true;
				skydiving = false;
			}
			else
			{
				skydiving = driver.read<bool>(Hopesar::Data::LocalPawn + 0x1c2f);
			}




			Hopesar::Data::Persistentlevel = driver.read<DWORD_PTR>(Hopesar::Data::Uworld + PersistentLVL_OFFSET);
			if (Debug) { printf(skCrypt("\n Persistentlevel: %p"), Hopesar::Data::Persistentlevel); }

			Hopesar::Data::RootPos = driver.read<Vector3>(Hopesar::Data::Rootcomp + RELATIVELOC_OFFSET);
			if (Debug) { printf(skCrypt("\n RootPos: %p"), Hopesar::Data::RootPos); }

			Hopesar::Data::ActorCount = driver.read<DWORD>(Hopesar::Data::Persistentlevel + (AACTORS_OFFSET + 0x8));
			if (Debug) { printf(skCrypt("\n ActorCount: %i"), Hopesar::Data::ActorCount); }



			Hopesar::Data::AActor = driver.read<DWORD_PTR>(Hopesar::Data::Persistentlevel + AACTORS_OFFSET);
			if (Debug) { printf(skCrypt("\n AActor: %p"), Hopesar::Data::AActor); }

			viewmatrix = driver.read<int64>(Hopesar::Data::LocalPlayer + 0xd0);
			viewmatrix = driver.read<int64>(viewmatrix + 0x8);//
			int MyTeamId = driver.read<int>(Hopesar::Data::PlayerState + 0x10B0);

			/*if (GetAsyncKeyState(VK_F3))
			{
				CacheWorld();
			}

			if (GetAsyncKeyState(VK_F4))
			{
				system("cls");
			}*/
			for (int uwu = 0; uwu < Hopesar::Data::ActorCount; uwu++) {


				auto CurrentActor = driver.read<uintptr_t>(Hopesar::Data::AActor + uwu * 0x8);
				if (Debug) { printf(skCrypt("\n CurrentActor: %p"), CurrentActor); }
				if (!CurrentActor) continue;
				float check = driver.read<float>(CurrentActor + 0x4480);
				if (check == 10)
				{
					int enemy;
					DWORD64 otherPlayerState = driver.read<uint64_t>(CurrentActor + PLAYERSTATE_OFFSET);
					string uwuaaaa = GetPlayerName(otherPlayerState);
					
					int ActorTeamId = driver.read<int>(otherPlayerState + 0x10B0);
					if (MyTeamId == ActorTeamId)
					{
						enemy = false;
					}
					else
					{
						enemy = true;
					}
					BYTE bIsDying = driver.read<BYTE>(CurrentActor + 0x710);
					bool bIsDead = bIsDying == 0x30;
					if (bIsDead)
					{

						continue;
					}


					uint64_t RootCompenent = driver.read<uint64_t>(CurrentActor + ROOTCMP_OFFSET);

					player Actor{ };

					if ((driver.read<BYTE>(otherPlayerState + 0x292) & 8))
					{
						Actor.isbot = true;
					}
					else
					{
						Actor.isbot = false;
					}




					Actor.Actor = CurrentActor;
					Actor.Mesh = driver.read<uint64_t>(CurrentActor + MESH_OFFSET);
					Actor.Name = uwuaaaa;
					Actor.RootComp = RootCompenent;
					Actor.IsOnScreen = true;
					Actor.valid = true;
					Actor.enemy = enemy;



					tmpList.push_back(Actor);


				}
			}

			player_pawns.clear();
			player_pawns = tmpList;

			if (InLobby || skydiving)
			{
				SleepLess = true;
				std::this_thread::sleep_for(std::chrono::milliseconds(airsleep));
			}
			else
			{
				SleepLess = false;
				std::this_thread::sleep_for(std::chrono::milliseconds(normalsleep));
			}
		}
		catch (...) { continue; }

	}
}


/*void Actors()
{
	bool Debug = false;
	bool loby = false;

	while (true) {
		Sleep(250); //delay must be on top of loop, otherwise it would be useless. delay is 250ms rn

		if (Debug) { printf("\n Base Addy: %p", Hopesar::Game::base_addy); }//
		if (!Hopesar::Game::base_addy) continue;


		Hopesar::Data::Uworld =driver.read<DWORD_PTR>(Hopesar::Game::base_addy + UWORLD_OFFSET);//
		if (Debug) { printf("\n Uworld: %p", Hopesar::Data::Uworld); }
		if (!Hopesar::Data::Uworld) continue;


		Hopesar::Data::GameInstances =driver.read<DWORD_PTR>(Hopesar::Data::Uworld + GAMEINST_OFFSET);//
		if (Debug) { printf("\n GameInstances: %p", Hopesar::Data::GameInstances); }
		if (!Hopesar::Data::GameInstances) continue;

		Hopesar::Data::GameState =driver.read<DWORD_PTR>(Hopesar::Data::Uworld + GAMESTS_OFFSET);
		if (Debug) { printf("\n GameState: %p", Hopesar::Data::GameState); }
		if (!Hopesar::Data::GameState) continue;

		Hopesar::Data::Persistentlevel =driver.read<DWORD_PTR>(Hopesar::Data::Uworld + PersistentLVL_OFFSET);
		if (Debug) { printf("\n Persistentlevel: %p", Hopesar::Data::Persistentlevel); }
		if (!Hopesar::Data::Persistentlevel) continue;


		Hopesar::Data::LocalPlayers =driver.read<DWORD_PTR>(Hopesar::Data::GameInstances + LOCALPLYR_OFFSET);//
		if (Debug) { printf("\n LocalPlayers: %p", Hopesar::Data::LocalPlayers); }
		if (!Hopesar::Data::LocalPlayers) continue;


		Hopesar::Data::LocalPlayer =driver.read<DWORD_PTR>(Hopesar::Data::LocalPlayers);//
		if (Debug) { printf("\n LocalPlayer: %p", Hopesar::Data::LocalPlayer); }
		if (!Hopesar::Data::LocalPlayer) continue;


		Hopesar::Data::PlayerController =driver.read<DWORD_PTR>(Hopesar::Data::LocalPlayer + CONTROLLER_OFFSET);//
		if (Debug) { printf("\n PlayerController: %p", Hopesar::Data::PlayerController); }
		if (!Hopesar::Data::PlayerController) continue;


		Hopesar::Data::LocalPawn =driver.read<DWORD_PTR>(Hopesar::Data::PlayerController + ACKNOWLEDGEPWN_OFFSET);//
		if (Debug) { printf("\n LocalPawn: %p", Hopesar::Data::LocalPawn); }
		if (!Hopesar::Data::LocalPawn)
		{
			loby = true;
		}
		else
		{
			loby = false;
		}


		if (!loby)
		{
			Hopesar::Data::PlayerState = driver.read<DWORD_PTR>(Hopesar::Data::LocalPawn + PLAYERSTATE_OFFSET);//
			if (Debug) { printf("\n PlayerState: %p", Hopesar::Data::PlayerState); }
			if (!Hopesar::Data::PlayerState)


				Hopesar::Data::Rootcomp = driver.read<DWORD_PTR>(Hopesar::Data::LocalPawn + ROOTCMP_OFFSET);//
			if (Debug) { printf("\n Rootcomp: %p", Hopesar::Data::Rootcomp); }
			if (!Hopesar::Data::Rootcomp)




		}



		Hopesar::Data::ActorCount =driver.read<DWORD>(Hopesar::Data::Persistentlevel + (AACTORS_OFFSET + 0x8));
		if (Debug) { printf("\n ActorCount: %i", Hopesar::Data::ActorCount); }
		if (!Hopesar::Data::ActorCount) continue;


		Hopesar::Data::AActor =driver.read<DWORD_PTR>(Hopesar::Data::Persistentlevel + AACTORS_OFFSET);
		if (Debug) { printf("\n AActor: %p", Hopesar::Data::AActor); }
		if (!Hopesar::Data::AActor) continue;



		viewmatrix =driver.read<int64>(Hopesar::Data::LocalPlayer + 0xd0);
		viewmatrix =driver.read<int64>(viewmatrix + 0x8);//


		for (int uwu = 0; uwu < Hopesar::Data::ActorCount; uwu++) {


			auto CurrentActor =driver.read<uintptr_t>(Hopesar::Data::AActor + uwu * 0x8);
			if (Debug) { printf("\n CurrentActor: %p", CurrentActor); }
			if (!CurrentActor) continue;



			int CurrentActorId = driver.read<int>(CurrentActor + 0x18);
			string fname = GetNameFromFName(CurrentActorId);
			std::cout << fname << endl;
			if (strstr(fname.c_str(), ("PlayerPawn_Athena_C")) || strstr(fname.c_str(), ("PlayerPawn_Athena_Phoebe_C")) || strstr(fname.c_str(), ("BP_MangPlayerPawn")) || strstr(fname.c_str(), ("PlayerPawn")))
			{
				std::cout << "BABABABBABABABB" << endl;
				DWORD_PTR RootComp = driver.read<DWORD_PTR>(CurrentActor + ROOTCMP_OFFSET);
				if (Debug) { printf("\n RootComp: %p", RootComp); }//changed
				if (!RootComp) continue;

				if (CurrentActor == Hopesar::Data::LocalPawn) continue;
				if (RootComp == Hopesar::Data::Rootcomp) continue;


				player this_player{
					CurrentActor,
					fname,
					RootComp,
					5,
					true,
					true,
					true,
				};

				if (!player_pawns.empty()) {
					auto found_player = std::find(player_pawns.begin(), player_pawns.end(), this_player);
					if (found_player == player_pawns.end())
					{
						std::cout << "\n[player] add -> 1";
						player_pawns.push_back(this_player);
					}


				}
				else
				{
					std::cout << "\n[player] create -> 1";
					player_pawns.push_back(this_player);
				}
			}
			else if (strstr(fname.c_str(), ("FortPickupAthena")) || strstr(fname.c_str(), ("Tiered_Chest")) || strstr(fname.c_str(), ("Vehicl")) || strstr(fname.c_str(), ("Valet_Taxi")) || strstr(fname.c_str(), ("Valet_BigRig")) || strstr(fname.c_str(), ("Valet_BasicTr")) || strstr(fname.c_str(), ("Valet_SportsC")) || strstr(fname.c_str(), ("Valet_BasicC")) || strstr(fname.c_str(), ("Tiered_Ammo")))
			{
				DWORD_PTR RootComp = driver.read<DWORD_PTR>(CurrentActor + ROOTCMP_OFFSET);
				if (Debug) { printf("\n RootComp: %p", RootComp); }//changed
				if (!RootComp) continue;

				if (RootComp == Hopesar::Data::Rootcomp) continue;


				item this_item{
					CurrentActor,
					fname,
					RootComp,
					5,
					true,
					true,
					true,
				};

				if (!item_pawns.empty()) {
					auto found_item = std::find(item_pawns.begin(), item_pawns.end(), this_item);
					if (found_item == item_pawns.end())
					{
						std::cout << "\n[item] add -> 1";
						item_pawns.push_back(this_item);
					}


				}
				else
				{
					std::cout << "\n[item] create -> 1";
					item_pawns.push_back(this_item);
				}
			}
		}



	}
}*/

int xart;
int yart;

float abdul;
float closestDistance = FLT_MAX;
DWORD_PTR closestPawn = NULL;
float closestDistanceenmy = FLT_MAX;

void Players() {
	SPOOF_FUNC;

	abdul =  Hopesar::Overlay::Width / 2;
	if(Hopesar::Settings::Aimbot::showfov)
	{
		DrawCircle(center_x, center_y, Hopesar::Settings::Aimbot::AimFOV, &Col.white, 300);
	}

	if (Hopesar::Settings::Visuals::Radar)
	{
		render_radar_main();
	}
	
	int virginia = 0;
	if (playercounts)
	{
		char players_found[256];
		sprintf_s(players_found, (skCrypt("players found: %d")), player_pawns.size());
		ImGui::GetBackgroundDrawList()->AddText(ImVec2(5, virginia), ImGui::GetColorU32({ 1.f, 0.f, 0.f, 1.f }), players_found);
	}

	if (SleepLess && sleepdelays)
	{
		virginia += 20;
		char sleep[256];
		sprintf_s(sleep, (skCrypt("Sleep Air Delay Activated")));
		ImGui::GetBackgroundDrawList()->AddText(ImVec2(5, virginia), ImGui::GetColorU32({ 1.f, 0.f, 0.f, 1.f }), sleep);
	}
	

	if (cheatfps)
	{
		virginia += 20;
		char fpps[256];
		sprintf_s(fpps, (skCrypt("Cheat FPS %.3f ms/frame (%.1f FPS)")), 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::GetBackgroundDrawList()->AddText(ImVec2(5, virginia), ImGui::GetColorU32({ 1.f, 0.f, 0.f, 1.f }), fpps);

	}

	if (vsyncenable)
	{
		virginia += 20;
		char fpps[256];
		sprintf_s(fpps, (skCrypt("VSYNC Enabled (Performance Mode)")), 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::GetBackgroundDrawList()->AddText(ImVec2(5, virginia), ImGui::GetColorU32({ 1.f, 0.f, 0.f, 1.f }), fpps);
	}

	/*if (GetAsyncKeyState(VK_F2) & 1) {
		printf("\nRotation: %lf",driver.read<double>(Hopesar::Data::Rootcomp + 0x148));
	}
	if (GetAsyncKeyState(VK_F3) & 1) {
		for (int i = 0; i < 4096; i++) {
			printf("\nOffset: %d, Value: %lf", i,driver.read<double>(viewmatrix + i));
		}
	}
	*/

	

	try
	{

		for (const player& this_player : player_pawns)
		{

			int screen;
			if (this_player.Actor == Hopesar::Data::LocalPawn) continue;

			
			if (!this_player.enemy && !InLobby) continue;


			Vector3 BottomBone = CallBoneFunction(this_player.Mesh, 0);
			Vector3 zero_r = Vector3(BottomBone.x, BottomBone.y, BottomBone.z - 15);
			Vector3 BottomScreen = ProjectWorldToScreen(zero_r);
			Vector3 Top = CallBoneFunction(this_player.Mesh, 68);
			Vector3 w2shead = ProjectWorldToScreen(Top);
			Vector3 Chest = CallBoneFunction(this_player.Mesh, 7);
			Vector3 w2sChest = ProjectWorldToScreen(Chest);
			float distance = W2SCam.Location.Distance(BottomBone) / 100.f;

			if (distance < Hopesar::Settings::Visuals::VisDist || InLobby)
			{
				bool isvisible;

				if (isVisible(this_player.Mesh))
				{
					isvisible = true;
				}
				else
				{
					isvisible = false;
				}
				if ((bIsInRectangle(Hopesar::Overlay::CenterX, Hopesar::Overlay::CenterY, 2000, w2shead.x, w2shead.y) && bIsInRectangle(960, 540, 2000, BottomScreen.x, BottomScreen.y)) || InLobby)
				{

					Vector3 head_r = Vector3(Top.x, Top.y, Top.z + 20);
					Vector3 TopScreen = ProjectWorldToScreen(head_r);

					
				

					float BoxHeight = BottomScreen.y - TopScreen.y;
					float BoxWidth = BoxHeight / 2.f;

					RGBA* finalcolor;
					char name[64];

					if (this_player.isbot)
					{
						sprintf_s(name, skCrypt("Bot | %2.f "), distance);
					}
					else
					{
						sprintf_s(name, skCrypt("Player | %2.f "), distance);
					}

				



					if (isvisible)
					{
						finalcolor = &Col.green;
					}
					else
					{
						finalcolor = &Col.red;
					}

					if (Hopesar::Settings::Visuals::Esp_box)
					{
						DrawCorneredBox(BottomScreen.x - (BoxWidth / 2), TopScreen.y, BoxWidth, BoxHeight, IM_COL32(255, 128, 0, 255), 1.5f);
					}

					if (Hopesar::Settings::Visuals::DwayneJhonson)
					{
					

						// Define the size of the image in pixels
						float imagePixels = 100.0f; // Adjust this value to control the size of the image

						// Compute the scaling factor based on the distance
						float scalingFactor = imagePixels / distance;

						// Define the position of the image
						ImVec2 position = ImVec2(w2shead.x - xart, w2shead.y - yart); // Define the image position

						// Cast the texture pointer to ImTextureID
						ImTextureID textureID = (ImTextureID)my_texture;

						// Add the image to the draw list with a constant size
						ImGui::GetBackgroundDrawList()->AddImage(textureID, position, position + ImVec2(imagePixels, imagePixels), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255));


					}

					if (Hopesar::Settings::Visuals::Esp_boxnormal)
					{
						DrawNormalBox(BottomScreen.x - (BoxWidth / 2), TopScreen.y, BoxWidth, BoxHeight, 1.5f, &Col.orange, &Col.orange);

					}

					if (Hopesar::Settings::Visuals::WindowStreamProof) {
						SetWindowDisplayAffinity(Window, WDA_EXCLUDEFROMCAPTURE);
					}
					if (!Hopesar::Settings::Visuals::WindowStreamProof) {
						SetWindowDisplayAffinity(Window, !WDA_EXCLUDEFROMCAPTURE);
					}
					

					if (Hopesar::Settings::Visuals::Esp_line)
					{
						DrawLine(Hopesar::Overlay::Width / 2, Hopesar::Overlay::Height, BottomScreen.x, BottomScreen.y, &Col.blue, 0.5);
					}

					if (Hopesar::Settings::Visuals::Esp_Dist)
					{
						ImVec2 DistanceTextSize2 = ImGui::CalcTextSize(name);
						ImGui::GetBackgroundDrawList()->AddText(ImVec2(BottomScreen.x - DistanceTextSize2.x / 2, BottomScreen.y + 5), ImGui::ColorConvertFloat4ToU32(ImVec4(finalcolor->R / 255.0, finalcolor->G / 255.0, finalcolor->B / 255.0, finalcolor->A / 255.0)), name);
					}

					if (Hopesar::Settings::Visuals::Esp_Threed)
					{
						ImU32 ESPSkeleton;

						ESPSkeleton = ImGui::GetColorU32({ 255, 255, 255, 255 });

						Vector3 bottom1 = ProjectWorldToScreen(Vector3(BottomBone.x + 40, BottomBone.y - 40, BottomBone.z));
						Vector3 bottom2 = ProjectWorldToScreen(Vector3(BottomBone.x - 40, BottomBone.y - 40, BottomBone.z));
						Vector3 bottom3 = ProjectWorldToScreen(Vector3(BottomBone.x - 40, BottomBone.y + 40, BottomBone.z));
						Vector3 bottom4 = ProjectWorldToScreen(Vector3(BottomBone.x + 40, BottomBone.y + 40, BottomBone.z));

						Vector3 top1 = ProjectWorldToScreen(Vector3(Top.x + 40, Top.y - 40, Top.z + 15));
						Vector3 top2 = ProjectWorldToScreen(Vector3(Top.x - 40, Top.y - 40, Top.z + 15));
						Vector3 top3 = ProjectWorldToScreen(Vector3(Top.x - 40, Top.y + 40, Top.z + 15));
						Vector3 top4 = ProjectWorldToScreen(Vector3(Top.x + 40, Top.y + 40, Top.z + 15));

						ImGui::GetBackgroundDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(top1.x, top1.y), ESPSkeleton, 0.1f);
						ImGui::GetBackgroundDrawList()->AddLine(ImVec2(bottom2.x, bottom2.y), ImVec2(top2.x, top2.y), ESPSkeleton, 0.1f);
						ImGui::GetBackgroundDrawList()->AddLine(ImVec2(bottom3.x, bottom3.y), ImVec2(top3.x, top3.y), ESPSkeleton, 0.1f);
						ImGui::GetBackgroundDrawList()->AddLine(ImVec2(bottom4.x, bottom4.y), ImVec2(top4.x, top4.y), ESPSkeleton, 0.1f);

						ImGui::GetBackgroundDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(bottom2.x, bottom2.y), ESPSkeleton, 0.1f);
						ImGui::GetBackgroundDrawList()->AddLine(ImVec2(bottom2.x, bottom2.y), ImVec2(bottom3.x, bottom3.y), ESPSkeleton, 0.1f);
						ImGui::GetBackgroundDrawList()->AddLine(ImVec2(bottom3.x, bottom3.y), ImVec2(bottom4.x, bottom4.y), ESPSkeleton, 0.1f);
						ImGui::GetBackgroundDrawList()->AddLine(ImVec2(bottom4.x, bottom4.y), ImVec2(bottom1.x, bottom1.y), ESPSkeleton, 0.1f);

						ImGui::GetBackgroundDrawList()->AddLine(ImVec2(top1.x, top1.y), ImVec2(top2.x, top2.y), ESPSkeleton, 0.1f);
						ImGui::GetBackgroundDrawList()->AddLine(ImVec2(top2.x, top2.y), ImVec2(top3.x, top3.y), ESPSkeleton, 0.1f);
						ImGui::GetBackgroundDrawList()->AddLine(ImVec2(top3.x, top3.y), ImVec2(top4.x, top4.y), ESPSkeleton, 0.1f);
						ImGui::GetBackgroundDrawList()->AddLine(ImVec2(top4.x, top4.y), ImVec2(top1.x, top1.y), ESPSkeleton, 0.1f);

					}

					/*if (Hopesar::Settings::Misc::fovchange)
					{
						driver.write<float>(Hopesar::Data::Rootcomp + 0x2A0, Hopesar::Settings::Misc::fovval);
					}*/
					
					
					if (Hopesar::Settings::Visuals::Esp_Name && !this_player.isbot)
					{
						ImVec2 DistanceTextSize2 = ImGui::CalcTextSize(this_player.Name.c_str());
						ImGui::GetBackgroundDrawList()->AddText(ImVec2((w2sChest.x - 10) - DistanceTextSize2.x / 2, w2sChest.y), ImGui::ColorConvertFloat4ToU32(ImVec4(finalcolor->R / 255.0, finalcolor->G / 255.0, finalcolor->B / 255.0, finalcolor->A / 255.0)), this_player.Name.c_str());
					}

					if (Hopesar::Settings::Visuals::Esp_Weapon)
					{

						auto CurrentWeapon = driver.read<uintptr_t>(this_player.Actor + 0x8F8);
						auto ItemData = driver.read<DWORD_PTR>(CurrentWeapon + 0x3f8);
						BYTE tieraa = driver.read<BYTE>(ItemData + 0x73);


						auto DisplayName = driver.read<uint64_t>(ItemData + 0x90);
						auto WeaponLength = driver.read<uint32_t>(DisplayName + 0x38);
						wchar_t* WeaponName = new wchar_t[uint64_t(WeaponLength) + 1];

						driver.read_buffer((ULONG64)driver.read<PVOID>(DisplayName + 0x30), (uint8_t*)WeaponName, WeaponLength * sizeof(wchar_t));
						std::string Text = wchar_to_char(WeaponName);

					

						BYTE tier = tieraa;
						ImColor Color;
						if (tier == 2)
						{
							Color = IM_COL32(0, 255, 0, 255);
						}
						else if ((tier == 3))
						{
							Color = IM_COL32(0, 0, 255, 255);
						}
						else if ((tier == 4))
						{
							Color = IM_COL32(128, 0, 128, 255);
						}
						else if ((tier == 5))
						{
							Color = IM_COL32(255, 255, 0, 255);
						}
						else if ((tier == 6))
						{
							Color = IM_COL32(255, 255, 0, 255);
						}
						else if ((tier == 0) || (tier == 1))
						{
							Color = IM_COL32(255, 255, 255, 255);
						}

						string lastweapon;
						auto bIsReloadingWeapon = driver.read<bool>(CurrentWeapon + 0x331);
						auto AmmoCount = driver.read<int>(CurrentWeapon + 0xb8c);

						if (AmmoCount)
						{
							char buffer[128];
							sprintf_s(buffer, " %i", AmmoCount);
							if (buffer != "?")
							{
								lastweapon = Text + " | " + buffer;
							}
							else
							{
								lastweapon = Text;
							}
						}
						else
						{
							lastweapon = Text;
						}

						if (bIsReloadingWeapon)
						{
							ImGui::GetBackgroundDrawList()->AddText(ImVec2(w2shead.x - 30, w2shead.y - 15), IM_COL32(255, 255, 255, 255), skCrypt("Reloading"));
						}
						else
						{
							if (lastweapon.length() >= 3 && lastweapon.find('?') == string::npos && Text.find('?') == string::npos && !InLobby)
							{
								ImVec2 DistanceTextSize2 = ImGui::CalcTextSize(lastweapon.c_str());
								ImGui::GetBackgroundDrawList()->AddText(ImVec2((w2shead.x - 30) - DistanceTextSize2.x / 2, w2shead.y - 15), Color, lastweapon.c_str());

							}
						}

					}



					auto dx = w2shead.x - (Hopesar::Overlay::Width / 2);
					auto dy = w2shead.y - (Hopesar::Overlay::Height / 2);
					auto dist = sqrtf(dx * dx + dy * dy);



					if ((dist < Hopesar::Settings::Aimbot::AimFOV && dist < closestDistance) && (isvisible || !Hopesar::Settings::Aimbot::onlyvisible)) {
						if (distance < Hopesar::Settings::Aimbot::max_distance)
						{
							closestDistance = dist;
							closestPawn = this_player.Actor;
						}

					}

					if (Hopesar::Settings::Aimbot::triggerbot && in_rect(center_x, center_y, 7, w2sChest.x, w2sChest.y) && (isvisible || !Hopesar::Settings::Aimbot::onlyvisible))
					{
						LeftClick();
					}

					if (Hopesar::Settings::Aimbot::triggerbot && in_rect(center_x, center_y, 7, w2shead.x, w2shead.y) && (isvisible || !Hopesar::Settings::Aimbot::onlyvisible))
					{
						LeftClick();
					}


				}
				if (distance > 20 && !InLobby && Hopesar::Settings::Visuals::Radar)
				{
					add_to_radar(BottomBone, 187, isvisible);
				}


				if (distance < closestDistanceenmy) {
					closestDistanceenmy = distance;
				}
			}

		

		}

	
		if (closestDistanceenmy < enemydistlimit && enemyworning && !InLobby)
		{
		
			char uwuenemy[256];
			sprintf_s(uwuenemy, skCrypt("Enemy Near By : %2.f Meters"), closestDistanceenmy);
			ImGui::GetBackgroundDrawList()->AddText(ImVec2(abdul - 50, 20), ImGui::GetColorU32({ 1.f, 0.f, 0.f, 1.f }), uwuenemy);
			closestDistanceenmy = FLT_MAX;
			
		}

		if (!InLobby && closestPawn != DWORD_PTR())
		{
			uintptr_t mesha = driver.read<uintptr_t>(closestPawn + MESH_OFFSET);
		
			auto rootbone = CallBoneFunction(mesha, hitbox);
			Vector3 rootBoneOut = ProjectWorldToScreen(rootbone);
		

			if (Hopesar::Settings::Aimbot::targetline && in_rect(center_x, center_y, Hopesar::Settings::Aimbot::AimFOV, rootBoneOut.x, rootBoneOut.y))
				ImGui::GetBackgroundDrawList()->AddLine(ImVec2(center_x, center_y), ImVec2(rootBoneOut.x, rootBoneOut.y), ImGui::GetColorU32({ 1.f, 0.f, 0.f, 1.f }), 1.f);

			if (Hopesar::Settings::Aimbot::Aimbotopen && GetAsyncKeyState(aimkey) < 0)
			{


					AimAt(mesha, closestPawn);
					if (Hopesar::Settings::Aimbot::shootafteraim)
					{
						LeftClick();
					}
					
				
			}
			else
			{
				closestDistance = FLT_MAX;
				closestPawn = DWORD_PTR();
			}
		}

	
	}
	catch (...) {}




}

ImVec2 mp;
static int menutab;
ImVec2 IndicatorPos;
static int destination = 0;
static bool cb_var1;
static bool cb_var2;
static float sf_var;
int hotkey;

bool loadedimg = false;
bool imagetesttt = false;
void render() {
	SPOOF_FUNC;

	
	if (GetAsyncKeyState(VK_INSERT) & 1)
	{
		showmenu = !showmenu;
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (Hopesar::Settings::Aimbot::cross) {
		DrawLine(center_x, center_y - 8, center_x, center_y + 8, &Col.orange, 1);
		DrawLine(center_x - 8, center_y, center_x + 8, center_y, &Col.orange, 1);
	}
	

	if (!loadedimg && imagetesttt)
	{
		bool ret = LoadTextureFromFile("test.jpg", &my_texture, &my_image_width, &my_image_height);
		IM_ASSERT(ret);
		loadedimg = true;
	}


	ImGuiIO& io = ImGui::GetIO();
	if (showmenu)
	{


	}
	else
	{
		io.MouseDrawCursor = false;
	}
	
		Players();

	

	
	
	



	ImGui::EndFrame();
	const ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 0.00f);
		ImGui::Render();
		ImGui::Render();
		const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
		pD3dDeviceContext->OMSetRenderTargets(1, &pMainRenderTargetView, nullptr);
		pD3dDeviceContext->ClearRenderTargetView(pMainRenderTargetView, clear_color_with_alpha);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

		if (vsyncenable)
		{
			pSwapChain->Present(1, 0);

		}
		else
		{
			pSwapChain->Present(0, 0);

		}




	if (Hopesar::Settings::Aimbot::aimkeypos == 0)
	{
		aimkey = 0x01;//left mouse button
	}
	else if (Hopesar::Settings::Aimbot::aimkeypos == 1)
	{
		aimkey = 0x02;//right mouse button
	}
	else if (Hopesar::Settings::Aimbot::aimkeypos == 2)
	{
		aimkey = 0x04;//middle mouse button
	}
	else if (Hopesar::Settings::Aimbot::aimkeypos == 3)
	{
		aimkey = 0x05;//x1 mouse button
	}
	else if (Hopesar::Settings::Aimbot::aimkeypos == 4)
	{
		aimkey = 0x06;//x2 mouse button
	}
	else if (Hopesar::Settings::Aimbot::aimkeypos == 5)
	{
		aimkey = 0x03;//control break processing
	}
	else if (Hopesar::Settings::Aimbot::aimkeypos == 6)
	{
		aimkey = 0x08;//backspace
	}
	else if (Hopesar::Settings::Aimbot::aimkeypos == 7)
	{
		aimkey = 0x09;//tab
	}
	else if (Hopesar::Settings::Aimbot::aimkeypos == 8)
	{
		aimkey = 0x0c;//clear
	}
	else if (Hopesar::Settings::Aimbot::aimkeypos == 9)
	{
		aimkey == 0x0D;//enter
	}
	else if (Hopesar::Settings::Aimbot::aimkeypos == 10)
	{
		aimkey = 0x10;//shift
	}
	else if (Hopesar::Settings::Aimbot::aimkeypos == 11)
	{
		aimkey = 0x11;//ctrl
	}
	else if (Hopesar::Settings::Aimbot::aimkeypos == 12)
	{
		aimkey == 0x12;//alt
	}
	else if (Hopesar::Settings::Aimbot::aimkeypos == 13)
	{
		aimkey == 0x14;//caps lock
	}
	else if (Hopesar::Settings::Aimbot::aimkeypos == 14)
	{
		aimkey == 0x1B;//esc
	}
	else if (Hopesar::Settings::Aimbot::aimkeypos == 15)
	{
		aimkey == 0x20;//space
	}
	


}
bool test = true;
bool malw = false;

int main(int argc, const char* argv[])
{
	SPOOF_FUNC;
	
	hide_imports();
	
	
		if (!driver.init())
		{
	
			exit(0);

		}
	
	printf((skCrypt("Please Launch Fortnite. \n")));
	while (!Hopesar::Game::hwnd)
	{

		Hopesar::Game::hwnd = FindWindowA_Spoofed(0, skCrypt("Fortnite  "));//FindWindowA_Spoofed
		Sleep(5);
	}
	
	

	printf((skCrypt("Found Fortnite! Loading.. \n")));


	Pid = _GetProcessId((string)skCrypt("FortniteClient-Win64-Shipping.exe"));
	driver.attach(Pid);

	Hopesar::Game::base_addy = driver.get_process_base(Pid);
	if (!Hopesar::Game::base_addy)
	{
		printf((skCrypt("no base addres \n")));
	}
	

	HANDLE player = CreateThread_Spoofed(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(actor_cahce), nullptr, NULL, nullptr);//CreateThread_Spoofed
	HideThread(player);



	center_x = GetSystemMetrics(0) / 2 - 3;
	center_y = GetSystemMetrics(1) / 2 - 3;


	printf((skCrypt("Attaching In Fortnite.. Have Fun!\n")));
	if (!test)
	{
		Sleep(4000);

	}
	if (!test)
	{
		::ShowWindow(::GetConsoleWindow(), SW_HIDE);

	}
	SetUp();
	
	

	return 0;
}
