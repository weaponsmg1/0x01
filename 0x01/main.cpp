#include <windows.h>
#include <vector>
#include <cstring>
#include <string>
#include "memory.h"
#include "overlay.h"
#include "math.h"
#include "offsets.h"

int screenWidth, screenHeight;
Overlay* overlay = NULL;

Memory memory;
DWORD64 hw = 0;
DWORD64 client = 0;

void* viewMatrixBuffer = nullptr;
void* entityListBuffer = nullptr;

struct Player
{
	float screenPosition[2];
	float head2d[2];
	float feet2d[2];
	char name[32];
	bool dead;
	int state;
};

Player players[64];
bool esp = true;
bool enemy_name = true;
bool enemy_box = true;
bool show_only_enemies = true;
int local_player_team = 0;

int item = 0;
int showmenu = 0;

COLORREF box;

COLORREF name;

COLORREF unload;

void Menu()
{
	if (showmenu)
	{
		overlay->DrawBox(50, 50, 160, 160, RGB(255, 255, 255), 2, false);
		overlay->DrawFilledBox(50, 50, 160, 160, RGB(255, 255, 255), 2, false);
		overlay->DrawTextA(L"0x01", 115, 50, RGB(1, 1, 1));
		overlay->DrawTextA(L"Box", 115, 80, box);
		overlay->DrawTextA(L"Name", 115, 110, name);
		overlay->DrawTextA(L"Unload", 115, 140, unload);
	}

	if (item == 0)
	{
		box = RGB(0, 255, 0);
	}
	else
	{
		box = RGB(1, 1, 1);
	}

	if (item == 1)
	{
		name = RGB(0, 255, 0);
	}
	else
	{
		name = RGB(1, 1, 1);
	}

	if (item == 2)
	{
		unload = RGB(0, 255, 0);
	}
	else
	{
		unload = RGB(1, 1, 1);
	}
}

void MenuInput()
{
	if (GetAsyncKeyState(VK_INSERT) & 1)
		showmenu = !showmenu;

	if (GetAsyncKeyState(VK_DOWN) & 1)
		item = item++;

	if (GetAsyncKeyState(VK_UP) & 1)
		item = item--;

	if (GetAsyncKeyState(VK_RIGHT) & 1)
	{
		if (item == 0)
		{
			enemy_box = !enemy_box;
		}
		else if (item == 1)
		{
			enemy_name = !enemy_name;
		}
		else if (item == 2)
		{
			exit(0);
		}
	}
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void Matrix()
{
	if (esp && viewMatrixBuffer)
	{
		memory.ReadHugeMemory(hw + Offsets::view, viewMatrixBuffer, 0x40);
		memcpy(gWorldToScreen, viewMatrixBuffer, sizeof(gWorldToScreen));
	}
}

void Players()
{
	if (!esp || !entityListBuffer) return;

	memory.ReadHugeMemory(hw + Offsets::ent_list, entityListBuffer, 0x940C);

	for (int i = 0; i < 64; i++)
	{
		float playerX = memory.ReadModuleBuffer<float>(entityListBuffer, i * Offsets::ent_stride + Offsets::pos_x);

		if (!playerX)
		{
			players[i].dead = true;
			players[i].name[0] = '\0';
			continue;
		}

		float playerY = memory.ReadModuleBuffer<float>(entityListBuffer, i * Offsets::ent_stride + Offsets::pos_y);
		float playerZ = memory.ReadModuleBuffer<float>(entityListBuffer, i * Offsets::ent_stride + Offsets::pos_z);

		if (show_only_enemies)
		{
			DWORD modelOffset = i * Offsets::ent_stride + 0x012C;
			std::string model;
			char ch;

			do 
			{
				ch = memory.ReadModuleBuffer<char>(entityListBuffer, modelOffset);
				model.push_back(ch);
				modelOffset++;
			} while (ch != '\0');

			local_player_team = memory.read<int>(client + 0x100DF4);

			bool isFriendly = false;

			if (local_player_team == 2)
			{
				if (model.find("urban") != std::string::npos ||
					model.find("gign") != std::string::npos ||
					model.find("gsg9") != std::string::npos ||
					model.find("sas") != std::string::npos) {
					isFriendly = true;
				}
			}
			else if (local_player_team == 1) 
			{
				if (model.find("terror") != std::string::npos ||
					model.find("leet") != std::string::npos ||
					model.find("arctic") != std::string::npos ||
					model.find("guerilla") != std::string::npos) {
					isFriendly = true;
				}
			}

			if (isFriendly)
			{
				players[i].dead = true;
				players[i].name[0] = '\0';
				continue;
			}
		}

		Vector3 headPos = { playerX, playerY, playerZ + 72.0f };
		Vector3 feetPos = { playerX, playerY, playerZ };

		float head2d[2], feet2d[2];

		bool headVisible = WorldToScreen(headPos, head2d);
		bool feetVisible = WorldToScreen(feetPos, feet2d);

		if (headVisible && feetVisible)
		{
			players[i].head2d[0] = head2d[0];
			players[i].head2d[1] = head2d[1];
			players[i].feet2d[0] = feet2d[0];
			players[i].feet2d[1] = feet2d[1];
			players[i].dead = false;
		}
		else
		{
			players[i].dead = true;
			players[i].name[0] = '\0';
			continue;
		}

		if (enemy_name)
		{
			DWORD nameOffset = i * Offsets::ent_stride + Offsets::name;
			char ch;
			int nameIndex = 0;

			do
			{
				ch = memory.ReadModuleBuffer<char>(entityListBuffer, nameOffset);
				if (ch != '\0' && nameIndex < 31) {
					players[i].name[nameIndex++] = ch;
				}
				nameOffset++;
			} while (ch != '\0' && nameIndex < 31);

			players[i].name[nameIndex] = '\0';
		}
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0, 0, hInstance, NULL, NULL, NULL, NULL, "OverlayWindow", NULL }; RegisterClassEx(&wc);

	HWND hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT, wc.lpszClassName, "0x01", WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), NULL, NULL, hInstance, NULL);

	SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
	ShowWindow(hwnd, SW_SHOW);

	overlay = new Overlay(hwnd);

	if (!memory.AttachProcess("hl.exe"))
	{
		MessageBox(NULL, "Start game", "Game not found", MB_ICONERROR);
		return 0;
	}

	hw = memory.GetModuleAddress("hw.dll");
	client = memory.GetModuleAddress("client.dll");

	viewMatrixBuffer = malloc(0x40);
	entityListBuffer = malloc(0x940C);

	memset(players, 0, sizeof(players));

	MSG msg;
	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) break;
			continue;
		}

		Matrix();
		Players();

		RECT rect;
		GetClientRect(hwnd, &rect);
		screenWidth = rect.right;
		screenHeight = rect.bottom;
		width = screenWidth;
		height = screenHeight;

		overlay->BeginDraw();

		Menu();
		MenuInput();

		for (int i = 0; i < 64; i++)
		{
			if (players[i].dead) continue;

			float boxHeight = fabsf(players[i].feet2d[1] - players[i].head2d[1]);
			if (boxHeight < 1.0f) continue;

			float boxWidth = boxHeight * 0.45f;
			
			float boxSize = 8.0f; 
			float boxX = players[i].feet2d[0] - boxSize / 2.0f;
			float boxY = players[i].feet2d[1] - boxSize / 2.0f;

			if (enemy_box)
			{
				overlay->DrawBox(boxX, boxY, boxSize, boxSize, RGB(255, 0, 0), 2);
			}

			if (enemy_name && players[i].name[0] != '\0')
			{
				wchar_t wname[32];
				MultiByteToWideChar(CP_UTF8, 0, players[i].name, -1, wname, 32);

				HFONT font = overlay->CreateFontScaled(14);
				SIZE textSize = overlay->GetTextSize(wname, font);
				DeleteObject(font);

				float textY = boxY + boxSize + 2.0f; 

				float textX = boxX + boxSize / 2.0f - textSize.cx / 2.0f;

				overlay->DrawText(wname, textX, textY, RGB(255, 255, 255));
			}
		}

		overlay->EndDraw();
		Sleep(1);
	}

	free(viewMatrixBuffer);
	free(entityListBuffer);
	delete overlay;
	return 0;
}