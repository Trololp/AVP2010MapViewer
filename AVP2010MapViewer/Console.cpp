#include "Console.h"
#include <SpriteFont.h>

extern std::unique_ptr<DirectX::SpriteFont> m_font;
extern std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
extern int						g_framecount;
extern Console* g_pConsole;

const char* Console_name = "CONSOLE \n";


DWORD Console_command_set_console_font_size(DWORD* args)
{
	float new_size = ((float*)args)[0];
	
	if (new_size < 0.25f)
		new_size = 0.25f;
	if (new_size > 4.0f)
		new_size = 4.0f;
	g_pConsole->font_size = new_size;
	return 0;
}

Command cmd_set_console_font_size = { "font_size", 
	CONCMD_RET_ZERO,
	CONARG_FLOAT,
	Console_command_set_console_font_size,
	nullptr };

DWORD Console_command_set_console_color(DWORD* args)
{
	int r = args[0];
	int g = args[1];
	int b = args[2];

	if (r > 255 || r < 0 || g > 255 || g < 0 || b > 255 || b < 0)
	{
		r = 255;
		g = 255;
		b = 255;
	}

	g_pConsole->color = { (float)r / 255, (float)g / 255, (float)b / 255, 1.0f };
	return 0;
}

VIEWER_CONSOLE_ARG cmd_set_color_argshema[4] = { CONARG_INT , CONARG_INT , CONARG_INT , CONARG_END };
Command cmd_set_console_color = { "font_color",
	CONCMD_RET_ZERO,
	CONARG_INT,
	Console_command_set_console_color,
	cmd_set_color_argshema };

Console::Console(int line_count, float distance, XMVECTOR console_color, float font_size_console)
{
	this->line_count = line_count;
	color = console_color;
	font_size = font_size_console;
	distance_betwen_lines = distance;
	pos = { 150.0f, 250.0f };
	for (int i = 0; i < line_count; i++)
	{
		char* line = (char*)malloc(256);
		memset(line, ' ', 256);
		line[255] = '\0';
		lines.push_back(line);
	}
	CurrentLine = (char*)malloc(256);
	Last_Command = (char*)malloc(256);
	Clear_current_line();
	memset(Last_Command, ' ', 256);
	Last_Command[255] = '\0';
	Add_command(&cmd_set_console_font_size); // console font size
	Add_command(&cmd_set_console_color); // console color
	dbgprint("Console", "Created !!! \n");
}

Console::~Console()
{
	for (std::vector<char*>::iterator it = lines.begin(); it != lines.end(); ++it)
	{
		free((*it));
	}
	free(Last_Command);
	free(CurrentLine);
	std::vector<char*>().swap(lines);
}


void Console::Console_input(WPARAM wParam, bool key_or_char)
{
	char* temp = CurrentLine;
	int temp2 = cursor_pos;
	VIEWER_CONSOLE_ERROR_TYPE er1 = CONERR_NULL;
	if (key_or_char)
	{
		switch (wParam)
		{
		case VK_ESCAPE:
			is_active = false;
			return;
		case VK_RETURN:
			er1 = Detect_and_execute_command();
			memcpy(Last_Command, CurrentLine, 256);
			cursor_pos2 = cursor_pos;
			Print_error(er1);
			return;
		case VK_BACK:
			if (cursor_pos < 1)
				return;
			cursor_pos--;
			CurrentLine[cursor_pos] = ' ';
			return;
		case VK_LEFT:
			if (cursor_pos < 1)
				return;
			cursor_pos--;
			return;
		case VK_RIGHT:
			if (cursor_pos > 254)
				return;
			cursor_pos++;
			return;
		case VK_UP:
		case VK_DOWN:
			CurrentLine = Last_Command;
			Last_Command = temp;
			cursor_pos = cursor_pos2;
			cursor_pos2 = temp2;
			break;
			//case VK_TAB:
			//To do auto fill commands
			//	break;
		default:
			break;
		}
	}
	

	//dbgprint("Console", "In Console_input wParam = %c (%x) cursor_pos = %d \n", wParam, wParam, cursor_pos);

	if (!key_or_char)
	{
		if ((isalnum((char)wParam) || isspace((char)wParam) || wParam == '_' || wParam == '.' || wParam == '-') \
			&& (wParam != '\r' && wParam != '\n'))
		{
			CurrentLine[cursor_pos] = (char)wParam;
			if (cursor_pos > 254)
				return;
			cursor_pos++;
		}
	}
}



void Console::On_Enter()
{
	memcpy(lines[line_count - 1], CurrentLine, 256);
	Move_lines();
	Clear_current_line();
}



void Console::Add_command(Command * pCommand)
{
	this->commands.push_back(pCommand);
}

void Console::Console_printf(const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	vsprintf(CurrentLine, fmt, args);
	On_Enter();
	va_end(args);
}

void Console::Move_lines()
{
	char* start = lines[0];
	lines[0] = lines[line_count-1];
	char* temp = lines[1];
	lines[1] = start;
	for (int i = 1; i < line_count-1; i++)
	{
		start = temp;
		temp = lines[i];
		lines[i] = start;
	}
}

void Console::Clear_current_line()
{
	memset(this->CurrentLine, ' ', 256);
	CurrentLine[255] = '\0';
	cursor_pos = 0;
}

void Console::Draw()
{
	if (!is_active)
		return;
	
	bool show_pointer = (g_framecount % 32) < 16;
	char symbol_under_pointer = CurrentLine[cursor_pos];

	if (show_pointer)
	{
		CurrentLine[cursor_pos] = '_';
	}

	XMVECTOR color2 = XMVectorScale(color, 0.7f);

	m_font->DrawString(m_spriteBatch.get(), (const char*)CurrentLine, pos, color, 0.f, XMVectorZero(), font_size);
	for (float i = 0; i < line_count; i++)
	{
		XMVECTOR pos2 = { pos.m128_f32[0], pos.m128_f32[1] - distance_betwen_lines * (float)(i+1) };
		m_font->DrawString(m_spriteBatch.get(), (const char*)lines[i], pos2, color2, 0.f, XMVectorZero(), font_size);
	}
	XMVECTOR pos2 = { pos.m128_f32[0], pos.m128_f32[1] - distance_betwen_lines * (float)(line_count+1)};
	m_font->DrawString(m_spriteBatch.get(), Console_name, pos2, color2, 0.f, XMVectorZero(), font_size);
	//dbgprint("Console", "In Draw4\n");
	if (show_pointer)
	{
		CurrentLine[cursor_pos] = symbol_under_pointer;
	}
}

void Console::Print_error(VIEWER_CONSOLE_ERROR_TYPE type)
{
	switch (type)
	{
	case CONERR_NULL:
		break;
	case CONERR_ARG:
		Console_printf("Incorrect arg !!! \n");
		break;
	case CONERR_TOOMANYARG:
		Console_printf("Too many args !!! \n");
		break;
	case CONERR_LESSARG:
		Console_printf("Not enough arguments !!! \n");
		break;
	case CONERR_NOCMD:
		break;
	case CONERR_INTERNAL:
		Console_printf("Error in command handler !!! \n");
		break;
	case CONERR_UNKNOWN:
		Console_printf("Unknown error !!! \n");
		break;
	default:
		break;
	}
}

VIEWER_CONSOLE_ERROR_TYPE Console::Detect_and_execute_command()
{
	for (std::vector<Command*>::iterator it = commands.begin(); it != commands.end(); ++it)
	{
		const char* cmd_name = (*it)->name;
		size_t cmd_name_size = strlen(cmd_name);
		if (!strncmp(cmd_name, CurrentLine, cmd_name_size))
		{
			DWORD* args = (DWORD*)malloc(16 * 4);
			ZeroMemory(args, 16 * 4);
			VIEWER_CONSOLE_ARG * pArgschema = (*it)->pArgschema;
			char* curr_p = CurrentLine + cmd_name_size;
			char* end_p = CurrentLine + 255;
			int i = 0;
			VIEWER_CONSOLE_ARG argtype = pArgschema[0];
			if ((*it)->argtype == CONARG_END)
				goto execute;
			
			if (!((*it)->pArgschema))
			{
				while ((curr_p != end_p) && (*curr_p == ' '))
				{
					curr_p++;
				}
				if (curr_p == end_p)
				{
					dbgprint("Console", "No arg in argument parse cycle !!! \n");
					return CONERR_LESSARG;
				}
				switch ((*it)->argtype)
				{
				case CONARG_INT:
					((int*)args)[0] = (int)strtol(curr_p, NULL, 10);
					break;
				case CONARG_FLOAT:
					((float*)args)[0] = (float)strtof(curr_p, NULL);
					break;
				case CONARG_STR:
					//str_p = strchr(CurrentLine + cmd_name_size + 1, ' '); // to do make str arg parsing
					//
					break;
				default:
					dbgprint("Console", "bug in ARG parse %d \n", (*it)->argtype);
					break;
				}
				goto execute;
			}

			for (;  i < 16; i++)
			{
				//dbgprint("Console", "curr_p: %s \n", curr_p);
				VIEWER_CONSOLE_ARG argtype = pArgschema[i];
				if (argtype == CONARG_END)
					break;
				while ((curr_p != end_p) && (*curr_p == ' '))
				{
					curr_p++;
				}
				if (curr_p == end_p)
				{
					dbgprint("Console", "No arg in argument parse cycle !!! \n");
					return CONERR_LESSARG;
				}
				switch (argtype)
				{
				case CONARG_INT:
					((int*)args)[i] = (int)strtol(curr_p, NULL, 10);
					break;
				case CONARG_FLOAT:
					((float*)args)[i] = (float)strtof(curr_p, NULL);
					break;
				case CONARG_STR:
					//str_p = strchr(CurrentLine + cmd_name_size + 1, ' '); // to do make str arg parsing
					//
					break;
				default:
					dbgprint("Console", "bug in CONARG_MULTIPLE parse %d \n", argtype);
					break;
				}
				while ((curr_p != end_p) && (*curr_p != ' '))
				{
					curr_p++;
				}
			}
			
			execute:
			VIEWER_CONSOLE_COMMAND_TYPE cmd_type = (*it)->type;
			DWORD(*pFunc)(DWORD*) = (*it)->pFunc;
			switch (cmd_type)
			{
			case CONCMD_RET_INT:
				Console_printf("%d\n", (*pFunc)(args));
				break;
			case CONCMD_RET_FLOAT:
				Console_printf("%f\n", (*pFunc)(args));
				break;
			case CONCMD_RET_STR:
				Console_printf("%s\n", (*pFunc)(args));
				break;
			case CONCMD_RET_ZERO:
				(*pFunc)(args);
				break;
			default:
				dbgprint("Console", "bug in cmd_type parse %d \n", cmd_type);
				break;
			}
			free(args);
		}
	}
	return CONERR_NOCMD;
}

