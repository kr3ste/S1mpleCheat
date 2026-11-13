#include "overlay.h"
#include "MemoryManager.hpp"

uintptr_t GetPlayer(uint32_t i) {
	uintptr_t entityList = mem.Read<uintptr_t>(module_base + 0x2709010);


	uintptr_t listEntry = mem.Read<uintptr_t>(entityList + (8 * ((i & 0x7FFF) >> 9)) + 16);
	if (!listEntry) return 0;

	uintptr_t controller = mem.Read<uintptr_t>(listEntry + 112 * (i & 0x1FF));

	if (controller < 0x1000000 || (controller >> 48) != 0) return 0;

	// m_hPlayerPawn
	uint32_t pawnHandle = mem.Read<uint32_t>(controller + 0x92C);
	if (!pawnHandle) return 0;


	uintptr_t listEntry2 = mem.Read<uintptr_t>(entityList + (8 * ((pawnHandle & 0x7FFF) >> 9)) + 16);
	if (!listEntry2) return 0;


	uintptr_t pawn = mem.Read<uintptr_t>(listEntry2 + 112 * (pawnHandle & 0x1FF));

	return pawn;
}

Vector3 GetLocationOfPlayer(uintptr_t player) {
	return mem.Read<Vector3>(player + 0x14A4);
}

Vector2 WorldToScreen(Vector3 pos)
{

	Matrix4x4 matrix = mem.Read<Matrix4x4>(module_base + 0x2559640);


	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	Vector2 screen{};

	float w = matrix.m[3][0] * pos.x + matrix.m[3][1] * pos.y + matrix.m[3][2] * pos.z + matrix.m[3][3];
	if (w < 0.01f)
		return Vector2{};

	float invW = 1.0f / w;
	float x = (matrix.m[0][0] * pos.x + matrix.m[0][1] * pos.y + matrix.m[0][2] * pos.z + matrix.m[0][3]) * invW;
	float y = (matrix.m[1][0] * pos.x + matrix.m[1][1] * pos.y + matrix.m[1][2] * pos.z + matrix.m[1][3]) * invW;

	screen.x = (screenWidth / 2.0f) * (1.0f + x);
	screen.y = (screenHeight / 2.0f) * (1.0f - y);

	return screen;
}

void MyDrawings()
{
	
	for (size_t i = 0; i < 64; i++)
	{
		
		auto player = GetPlayer(i);

		Vector3 enemy_position_3d = GetLocationOfPlayer(player);
		
		Vector2 line_start_position(960, 1060);
		Vector2 enemyPosition = WorldToScreen(enemy_position_3d);

		DrawLine(line_start_position, enemyPosition, Colors::Red, 3.0f);
		DrawBox(enemyPosition - Vector2(25, 75), 50, 75, Colors::Red, 3.0f);
		DrawFilledBox(enemyPosition - Vector2(25, 75), 50, 75, Color(255, 0, 0, 100));
	}
}

int main()
{

	if (!mem.Attach(L"cs2.exe")) {
		std::wcerr << L"Failed to find or open process!" << std::endl;
		return 1;
	}
	std::wcout << L"Successfully attached!" << std::endl;

	// 2. Find the module base address
	module_base = mem.GetModuleBaseAddress(L"client.dll");
	std::cout << "Module Base Address: 0x" << std::hex << module_base << std::endl;

    RunOverlay(MyDrawings);
    return 0;
}