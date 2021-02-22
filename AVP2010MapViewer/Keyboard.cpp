#include "Keyboard.h"
#include "Console.h"
#include "Debug.h"

extern XMMATRIX                g_View;
extern float					pos_x;
extern float					pos_y;
extern float					pos_z;
extern float					g_alpha;
extern float					g_beta;
extern Console*					g_pConsole;

//Move Directions 
bool b_Forward = false;
bool b_Backward = false;
bool b_Right = false;
bool b_Left = false;
bool b_RMB = false; // USE hold RMB mouse to look up
bool b_LMB = false;
bool b_Shift = false;
bool b_show_info = true;
bool b_show_emod = false;

void MovementFunc()
{
	if (b_Forward || b_Backward || b_Left || b_Right)
	{
		float g_SpeedMultiplier = 10.0 + b_Shift * 20.0f;
		float delta_x = g_SpeedMultiplier * cos(g_alpha) * cos(g_beta);
		float delta_y = g_SpeedMultiplier * sin(g_alpha) * cos(g_beta);
		float delta_z = g_SpeedMultiplier * sin(g_beta);

		if (b_Forward)
		{
			pos_x -= delta_x * 0.1f;
			pos_y -= delta_y * 0.1f;
			pos_z -= delta_z * 0.1f;
		}
		if (b_Backward)
		{
			pos_x += delta_x * 0.1f;
			pos_y += delta_y * 0.1f;
			pos_z += delta_z * 0.1f;
		}
		if (b_Left)
		{
			pos_x -= delta_y * 0.1f;
			pos_y += delta_x * 0.1f;
		}
		if (b_Right)
		{
			pos_x += delta_y * 0.1f;
			pos_y -= delta_x * 0.1f;
		}

		XMVECTOR Eye = XMVectorSet(pos_y, pos_z, pos_x, 0.0f);
		XMVECTOR At = XMVectorSet(pos_y + delta_y, pos_z + delta_z, pos_x + delta_x, 0.0f);
		XMVECTOR Up = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
		g_View = XMMatrixLookAtLH(Eye, At, Up);
	}
}

void KeyDown(WPARAM wParam, LPARAM lParam)
{
	
	if (g_pConsole->is_active)
	{
		g_pConsole->Console_input(wParam, 1);
		return;
	}
		
	switch (wParam)
	{
	case 'W':
		b_Forward = true;
		break;
	case 'S':
		b_Backward = true;
		break;
	case 'A':
		b_Left = true;
		break;
	case 'D':
		b_Right = true;
		break;
	case VK_LSHIFT:
		b_Shift = true;
		break;
	case 'H':
		b_show_info = b_show_info ? 0 : 1;
		break;
	case 'C':
		b_show_emod = b_show_emod ? 0 : 1;
		break;
	case 'P':
		g_pConsole->is_active = true;
	default:
		break;
	}
}

void KeyChar(WPARAM wParam, LPARAM lParam)
{
	if (g_pConsole->is_active)
	{
		g_pConsole->Console_input(wParam, 0);
		return;
	}
}

void KeyUp(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case 'W':
		b_Forward = false;
		break;
	case 'S':
		b_Backward = false;
		break;
	case 'A':
		b_Left = false;
		break;
	case 'D':
		b_Right = false;
		break;
	case VK_LSHIFT:
		b_Shift = false;
		break;
	default:
		break;
	}
}

int x_diff = 0;
int y_diff = 0;
int last_x = 0;
int last_y = 0;

void MouseRMB(bool down, WPARAM wParam, LPARAM lParam)
{
	b_RMB = down;
	int xPos = LOWORD(lParam);
	int yPos = HIWORD(lParam);
	if (down)
	{
		x_diff += xPos - last_x;
		y_diff += yPos - last_y;
	}
	else
	{
		last_x = xPos;
		last_y = yPos;
	}
}

bool Show_info()
{
	return b_show_info;
}

void MouseLookUp(WPARAM wParam, LPARAM lParam)
{
	if (b_RMB)
	{
		int xPos = LOWORD(lParam);
		int yPos = HIWORD(lParam);

		int Dx = xPos - x_diff;
		int Dy = -yPos + y_diff;
		g_alpha = Dx / 100.0f;
		g_beta = Dy / 100.0f;

		if (g_beta >= XM_PIDIV2 - 0.1)
			g_beta = XM_PIDIV2 - 0.1;
		if (g_beta <= -XM_PIDIV2 + 0.1)
			g_beta = -XM_PIDIV2 + 0.1;

		float g_SpeedMultiplier = 10.0 + b_Shift * 20.0f;
		float delta_x = g_SpeedMultiplier * cos(g_alpha) * cos(g_beta);
		float delta_y = g_SpeedMultiplier * sin(g_alpha) * cos(g_beta);
		float delta_z = g_SpeedMultiplier * sin(g_beta);

		XMVECTOR Eye = XMVectorSet(pos_y, pos_z, pos_x, 0.0f);
		XMVECTOR At = XMVectorSet(pos_y + delta_y, pos_z + delta_z, pos_x + delta_x, 0.0f);
		XMVECTOR Up = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
		g_View = XMMatrixLookAtLH(Eye, At, Up);
	}
}

