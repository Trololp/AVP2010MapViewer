// Internal console that can be accessed trought "~" key
#pragma once
#include "AVP2010MapViewer.h"
#include "Debug.h"

enum VIEWER_CONSOLE_ARG
{
	CONARG_INT,
	CONARG_FLOAT,
	CONARG_STR,
	CONARG_MULTIPLE,
	CONARG_END
};

enum VIEWER_CONSOLE_COMMAND_TYPE
{
	CONCMD_RET_INT,
	CONCMD_RET_FLOAT,
	CONCMD_RET_STR,
	CONCMD_RET_ZERO
};

enum VIEWER_CONSOLE_ERROR_TYPE
{
	CONERR_NULL,
	CONERR_ARG,
	CONERR_TOOMANYARG,
	CONERR_LESSARG,
	CONERR_NOCMD,
	CONERR_INTERNAL,
	CONERR_UNKNOWN
};

//Argschema is array of VIEWER_CONSOLE_ARG values but not CONARG_MULTIPLE included. 
//Terminated with CONARG_END. Use it only when argtype set to CONARG_MULTIPLE.

struct Command
{
	const char* name;
	VIEWER_CONSOLE_COMMAND_TYPE type;
	VIEWER_CONSOLE_ARG argtype;
	DWORD (*pFunc)(DWORD*);
	VIEWER_CONSOLE_ARG* pArgschema;
};

class Console
{
public:
	char* CurrentLine;
	char* Last_Command;
	int cursor_pos;
	int cursor_pos2;
	bool is_active = false;
	int line_count;
	std::vector <char*> lines;
	std::vector <Command*> commands;
	XMVECTOR pos;
	XMVECTOR color;
	float font_size;
	float distance_betwen_lines;
	// constructor destructor
	Console(int line_count, float distance, XMVECTOR console_color, float font_size_console);
	~Console();
	// methods
	void Console_input(WPARAM wParam, bool key_or_char);
	void On_Enter();
	void Add_command(Command* pCommand);
	void Console_printf(const char* fmt, ...);
	void Draw();
private:
	void Move_lines();
	void Clear_current_line();
	void Print_error(VIEWER_CONSOLE_ERROR_TYPE type);
	VIEWER_CONSOLE_ERROR_TYPE  Detect_and_execute_command();
};

