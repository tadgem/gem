#pragma once

enum class editor_mode : int
{
	no_open_project,
	edit,
	play
};

enum class editor_trigger : int
{
	begin_play_mode,
	exit_play_mode,
	project_loaded
};
