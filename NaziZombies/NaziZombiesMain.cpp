// dllmain.cpp : Defines the entry point for the DLL application.

#define _USE_MATH_DEFINES
#include "cmath"
#include "vector"

#define NOMINMAX
#include <Windows.h>
#include <iomanip>
#include <iostream>


#pragma region Defines
#define ZOMBIE_DEAD_VALUE 0
#define MAX_NR_ZOMBIES 25
#define MIN_DISTANCE 400
#define OPCODE_LENGTH 33
#pragma endregion


#pragma region Constants
char gOriginalOpcodes[] = "\xF3\x0F\x5C\xC1\xF3\x0F\x59\x05\x3C\x9F\x83\x00\xF3\x0F\x11\x84\xB7\x24\x01\x00\x00\x83\xC6\x01\x83\xFE\x03\x0F\x8C\x36\xFF\xFF\xFF";
char gAimbotOpcodes[] = "\xF3\x0F\x10\x05\x8C\xD1\x8E\x01\xF3\x0F\x11\x87\x24\x01\x00\x00\xF3\x0F\x10\x05\x90\xD1\x8E\x01\xF3\x0F\x11\x87\x28\x01\x00\x00\x90";
#pragma endregion


#pragma region Structs

struct Position
{
	float x{}, y{}, z{};
};
#pragma endregion


#pragma region Adress Offsets
//Functions
constexpr DWORD gAimFunctionBase = 0x00418545;

//Zombies Array
constexpr DWORD gZombieArrayOffset = 0x014E7448;
constexpr DWORD gZombieArrayDeathMarkerOffset = 0x7C;
constexpr DWORD gZombieArrayNextZombieOffset = 0x88;

//Zombies
constexpr DWORD gZombiePointerXOffset = 0x18;
constexpr DWORD gZombiePointerYOffset = 0x1C;
constexpr DWORD gZombiePointerZOffset = 0x20;
constexpr DWORD gZombieHealthOffset = 0x1C8;
constexpr DWORD gZombieMaxHealthOffset = 0x1CC;

//Player
constexpr DWORD gPlayerHealthOffset = 0x136C8B8;

constexpr DWORD gPlayerXOffset = 0x14ED090 - 0x8;
constexpr DWORD gPlayerYOffset = 0x14ED090 - 0x4;
constexpr DWORD gPlayerZOffset = 0x14ED090;

constexpr DWORD gPlayerGrenadesOffset = 0x14ED674;
constexpr DWORD gPlayerPointsOffset = 0x14EF124;
constexpr DWORD gPlayerVerticalOffset = 0x14ED18C;
constexpr DWORD gPlayerHorizontalOffset = 0x14ED18C + 0x4;


#pragma endregion


#pragma region Usable Pointers
//Game
uintptr_t gBaseAddress{};
//Player Obj
int* pHealthPointer{ nullptr };
float* pPlayerX{ nullptr };
float* pPlayerY{ nullptr };
float* pPlayerZ{ nullptr };
int* pScorePointer{ nullptr };
byte* pGrenadesPointer{ nullptr };

float* pPlayerLookHorizontal{ nullptr };
float* pPlayerLookVertical{ nullptr };

#pragma endregion


#pragma region Toggles
bool IsAutoHeal{ false };
bool IsAutoAim{ false };
bool IsInfinitePoints{ false };
bool IsInfiniteGrenades{ false };
bool IsInstaKill{ false };
#pragma endregion


HMODULE myhModule;

#undef goto


#pragma region Setup

void SetupBase()
{
	HMODULE hModule = GetModuleHandle(NULL);
	if (hModule == NULL) {
		std::cout << "Error getting module handle." << std::endl;
		return;
	}

	gBaseAddress = reinterpret_cast<uintptr_t>(hModule);

	std::cout << "Game Base Address: 0x" << std::hex << gBaseAddress << std::dec << std::endl;
}

void SetupPlayer()
{

	pHealthPointer = reinterpret_cast<int*>(gBaseAddress + gPlayerHealthOffset);
	pPlayerX = reinterpret_cast<float*>(gBaseAddress + gPlayerXOffset);
	pPlayerY = reinterpret_cast<float*>(gBaseAddress + gPlayerYOffset);
	pPlayerZ = reinterpret_cast<float*>(gBaseAddress + gPlayerZOffset);
	pScorePointer = reinterpret_cast<int*>(gBaseAddress + gPlayerPointsOffset);
	pGrenadesPointer = reinterpret_cast<byte*>(gBaseAddress + gPlayerGrenadesOffset);

	pPlayerLookHorizontal = reinterpret_cast<float*>(gBaseAddress + gPlayerHorizontalOffset);
	pPlayerLookVertical = reinterpret_cast<float*>(gBaseAddress + gPlayerVerticalOffset);

}

#pragma endregion

#pragma region Aimbot Functions
void WriteMemory(DWORD address, LPVOID data, int size)
{
	DWORD oldProtection{};
	VirtualProtect(reinterpret_cast<LPVOID>(address), size, PAGE_EXECUTE_READWRITE, &oldProtection);

	memcpy(reinterpret_cast<LPVOID>(address), data, size);

	VirtualProtect(reinterpret_cast<LPVOID>(address), size, oldProtection, NULL);
}

void EnableHack()
{
	std::cout << "enable Hack" << std::endl;
	WriteMemory(gAimFunctionBase, gAimbotOpcodes, OPCODE_LENGTH);
}

void DisableHack()
{
	std::cout << "disable Hack" << std::endl;
	WriteMemory(gAimFunctionBase, gOriginalOpcodes, OPCODE_LENGTH);
}


float Calculate3dDistance(Position p1, Position p2) {

	float xDist = p1.x - p2.x;
	float yDist = p1.y - p2.y;
	float zDist = p1.z - p2.z;

	return std::sqrt(
		xDist * xDist +
		yDist * yDist +
		zDist * zDist);
}

void CalculateAngles(float* angles, float x1, float y1, float z1, float x2, float y2, float z2) {
	float delta[3] = { (x1 - x2), (y1 - y2), (z1 - z2) };
	double dist = sqrt(pow(x1 - x2, 2.0) + pow(y1 - y2, 2.0));

	angles[0] = (float)(atanf(delta[2] / dist) * 180 / M_PI) + 800 / dist;
	angles[1] = (float)(atanf(delta[1] / delta[0]) * 180 / M_PI) + 100 / dist;

	if (delta[0] >= 0.0)
	{
		angles[1] += 180.0f;
	}
}

void AimTowardsAngles(float vertical, float horizontal) {
	*pPlayerLookHorizontal = horizontal;
	*pPlayerLookVertical = vertical;
}

void AutoHeal()
{
	// Set health to 200
	*pHealthPointer = 100;
}

void InfinitePoints()
{
	// Set health to 200
	*pScorePointer = 9000000;
}

void InfiniteGrenades()
{
	// Set health to 200
	*pGrenadesPointer = 127;
}

void InstaKill()
{
	for (int i = 0; i < MAX_NR_ZOMBIES; i++)
	{
		uintptr_t zombieBaseAddress = gBaseAddress + gZombieArrayOffset + (gZombieArrayNextZombieOffset * i);

		int* pZombieIsDead = reinterpret_cast<int*>(zombieBaseAddress + gZombieArrayDeathMarkerOffset);

		if (*pZombieIsDead == ZOMBIE_DEAD_VALUE)
			continue;

		uintptr_t zombiePointerAddress = *reinterpret_cast<uintptr_t*>(zombieBaseAddress);

		int* pHealth = reinterpret_cast<int*>(zombiePointerAddress + gZombieHealthOffset);
		*pHealth = 0;
	}
}

void AutoAim()
{
	Position playerPos{ *pPlayerX, *pPlayerY, *pPlayerZ };

	Position closestZombie{};
	float closestZombieDist{ FLT_MAX };

	for (int i = 0; i < MAX_NR_ZOMBIES; i++)
	{
		uintptr_t zombieBaseAddress = gBaseAddress + gZombieArrayOffset + (gZombieArrayNextZombieOffset * i);

		int* pZombieIsDead = reinterpret_cast<int*>(zombieBaseAddress + gZombieArrayDeathMarkerOffset);

		if (*pZombieIsDead == ZOMBIE_DEAD_VALUE)
			continue;

		uintptr_t zombiePointerAddress = *reinterpret_cast<uintptr_t*>(zombieBaseAddress);


		float* pXPointer = reinterpret_cast<float*>(zombiePointerAddress + gZombiePointerXOffset);
		float* pYPointer = reinterpret_cast<float*>(zombiePointerAddress + gZombiePointerYOffset);
		float* pZPointer = reinterpret_cast<float*>(zombiePointerAddress + gZombiePointerZOffset);

		int* pHealth = reinterpret_cast<int*>(zombiePointerAddress + gZombieHealthOffset);

		Position zombiePos{ *pXPointer, *pYPointer, *pZPointer };

		float dist = Calculate3dDistance(playerPos, zombiePos);

		if (dist < closestZombieDist)
		{
			closestZombie = zombiePos;
			closestZombieDist = dist;
		}
	}

	if (closestZombieDist <= MIN_DISTANCE)
	{
		float angles[2]{};

		playerPos;
		closestZombie;

		std::cout << "Player:\n	X:" << playerPos.x << ", Y: " << playerPos.y << ", Z: " << playerPos.z << std::endl;

		std::cout << "Closest Zombie:\n	X:" << closestZombie.x << ", Y: " << closestZombie.y << ", Z: " << closestZombie.z << std::endl;

		CalculateAngles(angles, playerPos.x, playerPos.y, playerPos.z, closestZombie.x, closestZombie.y, closestZombie.z);

		AimTowardsAngles(angles[0], angles[1]);
		EnableHack();
	}
	else
	{
		DisableHack();
	}
}

#pragma endregion


DWORD __stdcall EjectThread(LPVOID) {
	Sleep(100);
	FreeLibraryAndExitThread(myhModule, 0);
	return 0;
}

DWORD WINAPI Menue() {


	AllocConsole();

	FILE* fp;

	freopen_s(&fp, "CONOUT$", "w", stdout); // output only

	std::cout << "Wating for player to start playing\nPress F1 to start the program\nPress Y to Exit\n" << std::endl;

	bool hasExited{ false };

	while (1) {
		Sleep(100);

		if (GetAsyncKeyState(VK_F1))
			break;

		if (GetAsyncKeyState('Y'))
		{
			hasExited = true;
			break;
		}
	}

	std::cout << "Press Y to Exit\nPress U To Toggle Infinite Health\nPress I To Toggle Aimbot\nPress O To Toggle Infinite Points\nPress P To Toggle Infinite Grenades\nPress L To Toggle Insta Kill Zombies\n" << std::endl;

	SetupBase();
	SetupPlayer();

	if (!hasExited)
		while (1) {
			Sleep(20);
#pragma region Input Checks
			if (GetAsyncKeyState('Y'))
				break;

			if (GetAsyncKeyState('U'))
			{
				IsAutoHeal = !IsAutoHeal;
				std::cout << "Auto heal:" << (IsAutoHeal ? "On" : "Off") << std::endl;
			}

			if (GetAsyncKeyState('I'))
			{
				IsAutoAim = !IsAutoAim;
				DisableHack();
				std::cout << "Auto aim:" << (IsAutoAim ? "On" : "Off") << std::endl;
			}

			if (GetAsyncKeyState('O'))
			{
				IsInfinitePoints = !IsInfinitePoints;
				std::cout << "Infinite Points:" << (IsInfinitePoints ? "On" : "Off") << std::endl;
			}

			if (GetAsyncKeyState('P'))
			{
				IsInfiniteGrenades = !IsInfiniteGrenades;
				std::cout << "Infinte Grenades:" << (IsInfiniteGrenades ? "On" : "Off") << std::endl;
			}

			if (GetAsyncKeyState('L'))
			{
				IsInstaKill = !IsInstaKill;
				std::cout << "Insta Kill:" << (IsInstaKill ? "On" : "Off") << std::endl;
			}
#pragma endregion


#pragma region Aimbot Funcs

			if (IsInstaKill)
				InstaKill();

			if (IsAutoAim)
				AutoAim();

			if (IsAutoHeal)
				AutoHeal();

			if (IsInfinitePoints)
				InfinitePoints();

			if (IsInfiniteGrenades)
				InfiniteGrenades();
#pragma endregion
		}

	fclose(fp);

	FreeConsole();

	CreateThread(0, 0, EjectThread, 0, 0, 0);

	return 0;
}



BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		myhModule = hModule;
		CreateThread(NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(Menue), NULL, 0, NULL);
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

