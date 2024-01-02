// dllmain.cpp : Defines the entry point for the DLL application.
#define _USE_MATH_DEFINES
#include <cmath>
#define NOMINMAX
#include <Windows.h>
#include <iostream>
#include <vector>
#include <tlhelp32.h>
#include <iomanip>
#include <unordered_map>
#include <chrono>

#include "dllmain.h"



HMODULE myhModule;

#undef goto

#pragma region DEFINES

//#define AUTO_ROTATE

#pragma endregion


#pragma region Position
struct Position
{
	int x{}, y{}, z{};
};


double GetDistance(const Position& p1, const Position& p2)
{
	unsigned long long dx = (p1.x < p2.x) ? (p2.x - p1.x) : (p1.x - p2.x);
	unsigned long long dy = (p1.y < p2.y) ? (p2.y - p1.y) : (p1.y - p2.y);
	unsigned long long dz = (p1.z < p2.z) ? (p2.z - p1.z) : (p1.z - p2.z);

	return std::sqrt(dx * dx + dy * dy + dz * dz);
};

#pragma endregion

#pragma region EnemyTypes

enum EnemyTypes
{
	MT_POSSESSED = 1,
	MT_SHOTGUY = 2,
	MT_VILE = 3,
	MT_UNDEAD = 5,
	MT_FATSO = 8,
	MT_CHAINGUY = 10,
	MT_TROOP = 11,
	MT_SERGEANT = 12,
	MT_SHADOWS = 13,
	MT_HEAD = 14,
	MT_BRUISER = 15,
	MT_KNIGHT = 17,
	MT_SKULL = 18,
	MT_SPIDER = 19,
	MT_BABY = 20,
	MT_CYBORG = 21,
	MT_PAIN = 22,
};


std::string EnemyTypeToString(EnemyTypes enemyType) {
	static std::unordered_map<int, std::string> typeMap = {
		{MT_POSSESSED, "MT_POSSESSED"},
		{MT_SHOTGUY, "MT_SHOTGUY"},
		{MT_VILE, "MT_VILE"},
		{MT_UNDEAD, "MT_UNDEAD"},
		{MT_FATSO, "MT_FATSO"},
		{MT_CHAINGUY, "MT_CHAINGUY"},
		{MT_TROOP, "MT_TROOP"},
		{MT_SERGEANT, "MT_SERGEANT"},
		{MT_SHADOWS, "MT_SHADOWS"},
		{MT_HEAD, "MT_HEAD"},
		{MT_BRUISER, "MT_BRUISER"},
		{MT_KNIGHT, "MT_KNIGHT"},
		{MT_SKULL, "MT_SKULL"},
		{MT_SPIDER, "MT_SPIDER"},
		{MT_BABY, "MT_BABY"},
		{MT_CYBORG, "MT_CYBORG"},
		{MT_PAIN, "MT_PAIN"}
	};

	auto it = typeMap.find(static_cast<int>(enemyType));
	return (it != typeMap.end()) ? it->second : "UnknownEnemyType";
}

#pragma endregion

#pragma region Adress Offsets
//Player Obj
constexpr DWORD gPlayerPointerOffset = 0x42F5B0;
constexpr DWORD gHealthOffset = 0x2C;

//MOBJ
constexpr DWORD gMobjThinkOffset = 0x10;
constexpr DWORD gMobjCNextOffset = 0x20;
constexpr DWORD gMobjXOffset = 0x30;
constexpr DWORD gMobjYOffset = 0x34;
constexpr DWORD gMobjZOffset = 0x38;
constexpr DWORD gMobjAngleOffset = 0x50;
constexpr DWORD gMobjMobjTypeOffset = 0x98;
constexpr DWORD gMobjHealthOffset = 0xC4;
constexpr DWORD gMobjMadeSoundOffset = 0x191;


#pragma endregion

#pragma region Const Vars
//Player MOBJ
constexpr unsigned gMaxAngle = static_cast<unsigned>(UINT_MAX);
constexpr unsigned gMinAngle = 0;

constexpr unsigned gMinDistance{ 50000000 };

const std::vector<int> gViableTypes{ MT_POSSESSED ,MT_SHOTGUY, MT_VILE ,MT_UNDEAD ,MT_FATSO ,MT_CHAINGUY ,MT_TROOP, MT_SERGEANT ,MT_SHADOWS ,MT_HEAD ,MT_BRUISER ,MT_KNIGHT ,MT_SKULL ,MT_SPIDER ,MT_BABY ,MT_CYBORG ,MT_PAIN };
#pragma endregion

#pragma region Addresses
//Player Obj
uintptr_t playerBaseAddress{};

//Player MOBJ
uintptr_t playerMobjBaseAddress{};
uintptr_t playerMobjThinkAdress{};

#pragma endregion

#pragma region Usable Pointers
//Player Obj
int* pHealthPointer{ nullptr };

//Player MOBJ
unsigned int* pAimDirectionPointer{ nullptr };

int* pPlayerX{ nullptr };
int* pPlayerY{ nullptr };
int* pPlayerZ{ nullptr };
#pragma endregion

#pragma region Toggles
bool IsAutoHeal{ false };
bool IsAutoAim{ false };
#pragma endregion

DWORD __stdcall EjectThread(LPVOID) {
	Sleep(100);
	FreeLibraryAndExitThread(myhModule, 0);
	return 0;
}


#pragma region Setup Functions

void SetupPlayerPointer()
{
	// Get the base address of the current process
	HMODULE hModule = GetModuleHandle(NULL);
	if (hModule == NULL) {
		std::cout << "Error getting module handle." << std::endl;
		return;
	}

	uintptr_t baseAddress = reinterpret_cast<uintptr_t>(hModule);
	std::cout << "Game Base Address: 0x" << std::hex << baseAddress << std::dec << std::endl;

	// Calculate the player base address using the base address and offset
	uintptr_t playerPointerAddress = baseAddress + gPlayerPointerOffset;
	std::cout << "Player Base Address: 0x" << std::hex << playerPointerAddress << std::dec << std::endl;

	//Set the base address of the player
	playerBaseAddress = *reinterpret_cast<uintptr_t*>(playerPointerAddress);
}

void SetupPlayerMOBJPointer()
{
	playerMobjBaseAddress = *reinterpret_cast<uintptr_t*>(playerBaseAddress);
	std::cout << "Player MOBJ Base Address: 0x" << std::hex << playerMobjBaseAddress << std::dec << std::endl;

	playerMobjThinkAdress = *reinterpret_cast<uintptr_t*>(playerMobjBaseAddress + gMobjThinkOffset);
	std::cout << "Player MOBJ Think Function Address: 0x" << std::hex << playerMobjThinkAdress << std::dec << std::endl;
}

void SetupPlayerHealthPointer()
{
	// Dereference the player base address to get the health address
	uintptr_t healthAddress = playerBaseAddress + gHealthOffset;
	std::cout << "Health Address: 0x" << std::hex << healthAddress << std::dec << std::endl;

	// Assuming health is an integer (adjust the type accordingly)
	pHealthPointer = reinterpret_cast<int*>(healthAddress);
	std::cout << "Current Health: " << *pHealthPointer << std::endl;
}

void SetupAimPointer()
{
	uintptr_t directionAddress = playerMobjBaseAddress + gMobjAngleOffset;
	std::cout << "Aim Dir Address: 0x" << std::hex << directionAddress << std::dec << std::endl;

	pAimDirectionPointer = reinterpret_cast<unsigned int*>(directionAddress);
}

void SetupPlayerPositionPointers()
{
	//Player X
	uintptr_t xAddress = playerMobjBaseAddress + gMobjXOffset;

	std::cout << "X Pos Address: 0x" << std::hex << xAddress << std::dec << std::endl;

	pPlayerX = reinterpret_cast<int*>(xAddress);

	//Player Y
	uintptr_t yAddress = playerMobjBaseAddress + gMobjYOffset;

	std::cout << "Y Pos Address: 0x" << std::hex << yAddress << std::dec << std::endl;

	pPlayerY = reinterpret_cast<int*>(yAddress);

	//Player Z
	uintptr_t zAddress = playerMobjBaseAddress + gMobjZOffset;

	std::cout << "Z Pos Address: 0x" << std::hex << zAddress << std::dec << std::endl;

	pPlayerZ = reinterpret_cast<int*>(zAddress);
}
#pragma endregion

#pragma region Aimbot Functions

void SetHealthTo200()
{
	if (!pHealthPointer)
		SetupPlayerHealthPointer();

	// Set health to 200
	*pHealthPointer = 200;
}

unsigned remap(unsigned value, unsigned fromLow, unsigned fromHigh, unsigned toLow, unsigned toHigh)
{
	// Check if the value is outside the original range
	if (value < fromLow || value > fromHigh) {
		std::cout << "Error: Value outside the original range." << std::endl;
		return 0; // or handle the error in a way suitable for your application
	}

	double result = toLow + static_cast<double>(value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow);

	// Map the value from the original range to the new range
	return static_cast<unsigned>(result);
}

#ifdef AUTO_ROTATE
void AutoRotate(unsigned int& aimDir)
{
	aimDir -= 100000000;


	*pAimDirectionPointer = aimDir;

	std::cout << "Aim Dir: " << aimDir << std::endl;

	unsigned aimDirRemap = remap(aimDir, minAngle, maxAngle, 0, 100);

	std::cout << "Aim Dir: " << aimDirRemap << std::endl;
}
#endif // AUTO_ROTATE



unsigned CalculateHorizontalAimDirection(const Position& playerPos, const Position& enemyPos)
{	
	// get delta x and y
	int deltaX = enemyPos.x - playerPos.x;
	int deltaY = enemyPos.y - playerPos.y;

	// get the angle in radians via atan2
	double angleRad = std::atan2(deltaY, deltaX);

	// Convert the angle to degrees
	double angleDeg = angleRad * (180.0 / M_PI);

	double aimDirection = (angleDeg < 0 ? angleDeg + 360.0 : angleDeg);

	// Remap the value from 0-360 to 0-intmax
	unsigned remappedAimDirection = static_cast<unsigned>(remap(aimDirection, 0u, 360u, gMinAngle, gMaxAngle));

	return remappedAimDirection;
};

void AutoAim()
{
	if (!pAimDirectionPointer)
		SetupAimPointer();


	//AutoRotate Block
#ifdef AUTO_ROTATE
	unsigned aimDir = *pAimDirectionPointer;
	AutoRotate(aimDir);
#endif // AUTO_ROTATE



	uintptr_t nextThinkAdress = *reinterpret_cast<uintptr_t*>(playerMobjBaseAddress + gMobjCNextOffset);

	uintptr_t nextThinkAdressThink = *reinterpret_cast<uintptr_t*>(nextThinkAdress + gMobjThinkOffset);



	if (!(nextThinkAdressThink == playerMobjThinkAdress))
	{
		return;
	}

	if (!pPlayerX || !pPlayerY || !pPlayerZ)
		SetupPlayerPositionPointers();

	Position playerPos{ *pPlayerX, *pPlayerY, *pPlayerZ };
	int loops = 0;
	double minDistance = DBL_MAX;
	Position minPosition{};

	while (nextThinkAdressThink == playerMobjThinkAdress)
	{
		int* pType = reinterpret_cast<int*>(nextThinkAdress + gMobjMobjTypeOffset);

		auto it = std::find(gViableTypes.begin(), gViableTypes.end(), *pType);
		int* pNextThinkerHealth = reinterpret_cast<int*>(nextThinkAdress + gMobjHealthOffset);

		if (it != gViableTypes.end() && *pNextThinkerHealth > 0)
		{
			int* pNextThinkerX = reinterpret_cast<int*>(nextThinkAdress + gMobjXOffset);
			int* pNextThinkerY = reinterpret_cast<int*>(nextThinkAdress + gMobjYOffset);
			int* pNextThinkerZ = reinterpret_cast<int*>(nextThinkAdress + gMobjZOffset);


			Position EnemyPos{ *pNextThinkerX, *pNextThinkerY, *pNextThinkerZ };

			double distance = GetDistance(playerPos, EnemyPos);


			if (distance < gMinDistance && distance < minDistance)
			{

				minDistance = distance;

				minPosition = EnemyPos;
			}

		}

		nextThinkAdress = *reinterpret_cast<uintptr_t*>(nextThinkAdress + gMobjCNextOffset);

		nextThinkAdressThink = *reinterpret_cast<uintptr_t*>(nextThinkAdress + gMobjThinkOffset);


		++loops;
	}


	if (minDistance == DBL_MAX)
		return;

	unsigned newAimDir = CalculateHorizontalAimDirection(playerPos, minPosition);
	*pAimDirectionPointer = newAimDir;



	//*nextThinkerX = *playerX;
	//*nextThinkerY = *playerY;
	//*nextThinkerZ = *playerZ;
	//
	//
	//std::cout << "Next Thinker New pos: X :" << *nextThinkerX << ", Y: " << *nextThinkerY << ", Z: " << *nextThinkerZ << std::endl;


}


#pragma endregion


void Reset()
{

	SetupPlayerPointer();
	SetupPlayerMOBJPointer();

	pPlayerX = nullptr;
	pPlayerY = nullptr;
	pPlayerZ = nullptr;

	IsAutoAim = false;

	IsAutoHeal = false;
}

DWORD WINAPI Menue() {


	AllocConsole();

	FILE* fp;

	freopen_s(&fp, "CONOUT$", "w", stdout); // output only

	std::cout << "Press Y to Exit\nPress U To Toggle Auto Heal\nPress I To Toggle Aimbot\nPress O To Reset\n" << std::endl;

	Reset();

	std::vector<std::chrono::milliseconds> durations{};
	std::chrono::time_point<std::chrono::system_clock> start{}, end{};

	while (1) {
		start = std::chrono::system_clock::now();
		Sleep(50);

		

		if (IsAutoHeal)
			SetHealthTo200();

		if (IsAutoAim)
			AutoAim();

		if (GetAsyncKeyState('Y'))
			break;

		if (GetAsyncKeyState('U'))
		{
			IsAutoHeal = !IsAutoHeal;
			std::cout << "Auto heal:" << (IsAutoHeal ? "On" : "Off") << std::endl;
		}

		if (GetAsyncKeyState('O'))
			Reset();

		if (GetAsyncKeyState('I'))
		{
			IsAutoAim = !IsAutoAim;
			std::cout << "Auto aim:" << (IsAutoAim ? "On" : "Off") << std::endl;
		}

		end = std::chrono::system_clock::now();

		durations.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(end - start));

	}

	size_t trimSize = durations.size() / 10;
	durations.erase(durations.begin(), durations.begin() + trimSize);
	durations.erase(durations.end() - trimSize, durations.end());

	std::chrono::milliseconds sumDuration(0);
	for (const auto& duration : durations) {
		sumDuration += duration;
	}

	std::chrono::milliseconds averageDuration = sumDuration / durations.size();


	std::cout << "Average Duration: " << averageDuration.count() << " milliseconds\n";

	while (1) {
		if (GetAsyncKeyState('P'))
			break;
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