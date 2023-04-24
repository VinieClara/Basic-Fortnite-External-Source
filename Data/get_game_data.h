#include "calculate_data.h"

namespace Hopesar
{

	namespace Settings
	{
		namespace Aimbot
		{

			bool Aimbotopen = true;
			bool triggerbot = false;
			bool prediction = false;
			bool memoryaim = false;
			bool showfov = true;
			bool cross = true;
			bool targetline = true;
			bool shootafteraim = false;
			bool onlyvisible = true;
			bool randombone = false;
			float AimFOV = 100;
			float smooth = 7;
			float max_distance = 200;
			static int aimkey;
			static int hitbox;
			static int aimkeypos = 3;
			static int hitboxpos = 0;
		}


		namespace Visuals
		{
			bool Esp_box = true;
			bool Esp_boxnormal = false;
			bool DwayneJhonson = false;
			bool Radar = false;
			bool WindowStreamProof = false;
			bool Esp_line = true;
			bool Esp_Name = true;
			bool Esp_Skeleton = false;
			bool Esp_Dist = false;
			bool Esp_Threed = false;
			bool Esp_Weapon = false;
			static float VisDist = 200;
		}

		namespace Misc
		{
			float fovval = 150;
			bool fovchange = false;
			bool debug = false;
			bool speedhack = false;
		}

		namespace GUI
		{
			bool ShowGui = false;
		}
	}

	namespace Data
	{

		DWORD_PTR Uworld;
		DWORD_PTR GameInstances;
		DWORD_PTR GameState;
		DWORD_PTR LocalPlayers;
		DWORD_PTR LocalPlayer;
		DWORD_PTR PlayerController;
		DWORD_PTR PlayerState;
		DWORD_PTR LocalPawn;
		DWORD_PTR Rootcomp;
		Vector3 RootPos;
		DWORD_PTR Persistentlevel;
		DWORD_PTR AActor;
		Vector3 LocalActorPos;
		DWORD ActorCount;
	}


	namespace Overlay
	{
		RECT GRect = { NULL };
		D3DPRESENT_PARAMETERS Parameters;

		DWORD CenterX;
		DWORD CenterY;

		int Width;
		int Height;

	}

	namespace Game
	{
		uint64_t base_addy;
		HWND hwnd = NULL;
		DWORD processID;
	}

}


uintptr_t realdbno;
uintptr_t realworld = 0x1078a7d8;
#define GAMEINST_OFFSET 0x1B8
#define LOCALPLYR_OFFSET 0x38
#define CONTROLLER_OFFSET 0x30
#define ACKNOWLEDGEPWN_OFFSET 0x330
#define PLAYERSTATE_OFFSET 0x2A8
#define ROOTCMP_OFFSET 0x190
#define RELATIVELOC_OFFSET 0x128
#define PersistentLVL_OFFSET 0x30
#define AACTORS_OFFSET 0x98
#define BONEARRY_OFFSET 0x5F8
#define COMPONETWRLD_OFFSET 0x240
#define MESH_OFFSET 0x310
#define VELLOCITY_OFFSET 0x170

char* wchar_to_char(const wchar_t* pwchar)
{
	int currentCharIndex = 0;
	char currentChar = pwchar[currentCharIndex];

	while (currentChar != '\0')
	{
		currentCharIndex++;
		currentChar = pwchar[currentCharIndex];
	}

	const int charCount = currentCharIndex + 1;

	char* filePathC = (char*)malloc(sizeof(char) * charCount);

	for (int i = 0; i < charCount; i++)
	{
		char character = pwchar[i];

		*filePathC = character;

		filePathC += sizeof(char);

	}
	filePathC += '\0';

	filePathC -= (sizeof(char) * charCount);

	return filePathC;
}

kernel::driver driver;

Vector3 CallBoneFunction(uintptr_t mesh, int id)
{
	int isCached = driver.read<int>(mesh + 0x638);//	CachedCategoryPageView
	uintptr_t bonearray = driver.read<uintptr_t>(mesh + 0x5F0 + (isCached * 0x10));

	FTransform comptoworld = driver.read<FTransform>(mesh + COMPONETWRLD_OFFSET);

		FTransform bone = driver.read<FTransform>(bonearray + (id * 0x60));
		D3DMATRIX Matrix = MatrixMultiplication(bone.ToMatrixWithScale(), comptoworld.ToMatrixWithScale());
		return Vector3(Matrix._41, Matrix._42, Matrix._43);
	
}

std::string GetPlayerName(__int64 PlayerState)
{
	__int64 FString = driver.read<__int64>(PlayerState + 0xac0);
	int Length = driver.read<int>(FString + 16i64);
	__int64 v6 = Length; // rbx
	if (!v6) return std::string("");
	uintptr_t FText = driver.read<__int64>(FString + 8);

	wchar_t* NameBuffer = new wchar_t[Length];
	driver.read_buffer(FText, (uint8_t*)NameBuffer, (Length * 2));
	
	char v21; // al
	int v22; // r8d
	int i; // ecx
	int v25; // eax
	_WORD* v23;

	v21 = v6 - 1;
	if (!(_DWORD)v6)
		v21 = 0;
	v22 = 0;
	v23 = (_WORD*)NameBuffer;
	for (i = (v21) & 3; ; *v23++ += i & 7)
	{
		v25 = v6 - 1;
		if (!(_DWORD)v6)
			v25 = 0;
		if (v22 >= v25)
			break;
		i += 3;
		++v22;
	}

	std::wstring Temp{ NameBuffer };
	return std::string(Temp.begin(), Temp.end());
}

struct HopesariCam
{
	Vector3 Location;
	Vector3 Rotation;
	float FieldOfView;
};




boolean bIsInRectangle(double centerX, double centerY, double radius, double x, double y) {
	return x >= centerX - radius && x <= centerX + radius &&
		y >= centerY - radius && y <= centerY + radius;
}

bool isVisible(uint64_t mesh)
{
	float fLastSubmitTime =driver.read<float>(mesh + 0x360);
	float fLastRenderTimeOnScreen =driver.read<float>(mesh + 0x368);

	const float fVisionTick = 0.06f;
	bool bVisible = fLastRenderTimeOnScreen + fVisionTick >= fLastSubmitTime;
	return bVisible;
}

HopesariCam W2SCam;
uintptr_t viewmatrix;


struct w2s_padding {
	char pad_0[2288]; // 2288
	double sin;

	char pad_1[24]; // 2320 
	double cos;

	char pad_2[168]; // 2496 
	double rotx;

};

void w2scam()
{
	 
	   //w2s_padding w2s_data =driver.read<w2s_padding>(viewmatrix);
	
	   // FOV And Viewmatrix are being readed at cache thread there not that important
	    W2SCam.FieldOfView = driver.read<float>(Hopesar::Data::PlayerController + 0x38C) * 90.f;//80.f / (read<double>(viewmatrix + 0x7F0) / 1.19f);
		W2SCam.Rotation.x = driver.read<double>(viewmatrix + 0x9C0);

		double sin = driver.read<double>(viewmatrix + 2288);
		double cos = driver.read<double>(viewmatrix + 2320);
		double yaw = atan2(sin, cos) * 180 / M_PI;
		yaw *= -1;
		W2SCam.Rotation.y = yaw;

		W2SCam.Location = driver.read<Vector3>(driver.read<uint64_t>(Hopesar::Data::Uworld + 0x110));
}

//std::mutex w2smut;
Vector3 ProjectWorldToScreen(Vector3 WorldLocation)
{
	//w2smut.lock();
	w2scam();
	HopesariCam vCamera = W2SCam;
	vCamera.Rotation.x = (asin(vCamera.Rotation.x)) * (180.0 / M_PI);


	D3DMATRIX HopesariMatrix = Matrix(vCamera.Rotation);
	Vector3 vAxisX = Vector3(HopesariMatrix.m[0][0], HopesariMatrix.m[0][1], HopesariMatrix.m[0][2]);
	Vector3 vAxisY = Vector3(HopesariMatrix.m[1][0], HopesariMatrix.m[1][1], HopesariMatrix.m[1][2]);
	Vector3 vAxisZ = Vector3(HopesariMatrix.m[2][0], HopesariMatrix.m[2][1], HopesariMatrix.m[2][2]);

	Vector3 vDelta = WorldLocation - vCamera.Location;
	Vector3 vTransformed = Vector3(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));

	if (vTransformed.z < 1.f)
		vTransformed.z = 1.f;
	//w2smut.unlock();
	return Vector3((Hopesar::Overlay::Width / 2.0f) + vTransformed.x * (((Hopesar::Overlay::Width / 2.0f) / tanf(vCamera.FieldOfView * (float)M_PI / 360.f))) / vTransformed.z, (Hopesar::Overlay::Height / 2.0f) - vTransformed.y * (((Hopesar::Overlay::Width / 2.0f) / tanf(vCamera.FieldOfView * (float)M_PI / 360.f))) / vTransformed.z, 0);
}

static std::string ReadFNamePool(int key)
{
	uint64_t NamePoolChunk = driver.read<uint64_t>(Hopesar::Game::base_addy + 0xef89f80 + (8 * (uint32_t)((int)(key) >> 16)) + 16) + (unsigned int)(4 * (uint16_t)key);
	int nameLength = driver.read<uint16_t>(NamePoolChunk) >> 6;
	char buff[1024];

	if ((uint32_t)nameLength)
	{
		for (int x = 0; x < nameLength; ++x)
		{
			buff[x] = driver.read<char>(NamePoolChunk + 4 + x);
		}

		char* v2 = buff; // rbx
		int v4; // ecx
		unsigned int v5 = nameLength; // eax
		__int64 v6; // rdx
		char v7; // al

		v4 = 17;
		if (v5)
		{
			v6 = v5;
			do
			{
				v7 = *v2++;
				v5 = v4 + ~v7;
				v4 += 7947;
				*(v2 - 1) = v5;
				--v6;
			} while (v6);
		} buff[nameLength] = '\0'; return std::string(buff);
	}
	else return "";
}

static std::string GetNameFromFName(int key)
{
	uint64_t NamePoolChunk = driver.read<uint64_t>(Hopesar::Game::base_addy + 0xef89f80 + (8 * (uint32_t)((int)(key) >> 16)) + 16) + (unsigned int)(4 * (uint16_t)key);

	if (driver.read<uint16_t>( NamePoolChunk) < 64)
	{
		auto a1 = driver.read<DWORD>(NamePoolChunk + 4);
		return ReadFNamePool(a1);
	}

	else
	{
		return ReadFNamePool(key);
	}
}