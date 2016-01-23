#include "fluidsynthDLL.h"
#include "../log.h"
#include "../PluginFramework/dynamicLibrary.h"
#include <string>

static DynamicLibrary* s_fluidsynthDLL = NULL;

xl_new_fluid_settings_def		xl_new_fluid_settings;
xl_fluid_settings_setstr_def	xl_fluid_settings_setstr;
xl_new_fluid_synth_def			xl_new_fluid_synth;
xl_new_fluid_sequencer2_def		xl_new_fluid_sequencer2;
xl_fluid_settings_getnum_def	xl_fluid_settings_getnum;
xl_fluid_synth_sfload_def		xl_fluid_synth_sfload;
xl_fluid_sequencer_register_fluidsynth_def xl_fluid_sequencer_register_fluidsynth;
xl_new_fluid_player_def			xl_new_fluid_player;
xl_delete_fluid_settings_def	xl_delete_fluid_settings;
xl_delete_fluid_synth_def		xl_delete_fluid_synth;
xl_delete_fluid_sequencer_def	xl_delete_fluid_sequencer;
xl_delete_fluid_player_def		xl_delete_fluid_player;
xl_fluid_player_stop_def		xl_fluid_player_stop;
xl_fluid_player_add_def			xl_fluid_player_add;
xl_fluid_player_get_status_def	xl_fluid_player_get_status;
xl_fluid_synth_write_s16_def	xl_fluid_synth_write_s16;
xl_fluid_player_play_def		xl_fluid_player_play;
xl_fluid_player_set_loop_def	xl_fluid_player_set_loop;

bool loadFluidsythDLL()
{
	std::string errorString;
	s_fluidsynthDLL = DynamicLibrary::load("libfluidsynth", errorString);
	if (!s_fluidsynthDLL)
	{
		return false;
	}

	new_fluid_settings					= (xl_new_fluid_settings_def)s_fluidsynthDLL->getSymbol("new_fluid_settings");
	new_fluid_synth						= (xl_new_fluid_synth_def)s_fluidsynthDLL->getSymbol("new_fluid_synth");
	new_fluid_sequencer2				= (xl_new_fluid_sequencer2_def)s_fluidsynthDLL->getSymbol("new_fluid_sequencer2");
	new_fluid_player					= (xl_new_fluid_player_def)s_fluidsynthDLL->getSymbol("new_fluid_player");
	delete_fluid_settings				= (xl_delete_fluid_settings_def)s_fluidsynthDLL->getSymbol("delete_fluid_settings");
	delete_fluid_synth					= (xl_delete_fluid_synth_def)s_fluidsynthDLL->getSymbol("delete_fluid_synth");
	delete_fluid_sequencer				= (xl_delete_fluid_sequencer_def)s_fluidsynthDLL->getSymbol("delete_fluid_sequencer");
	delete_fluid_player					= (xl_delete_fluid_player_def)s_fluidsynthDLL->getSymbol("delete_fluid_player");
	fluid_settings_setstr				= (xl_fluid_settings_setstr_def)s_fluidsynthDLL->getSymbol("fluid_settings_setstr");
	fluid_settings_getnum				= (xl_fluid_settings_getnum_def)s_fluidsynthDLL->getSymbol("fluid_settings_getnum");
	fluid_synth_sfload					= (xl_fluid_synth_sfload_def)s_fluidsynthDLL->getSymbol("fluid_synth_sfload");
	fluid_synth_write_s16				= (xl_fluid_synth_write_s16_def)s_fluidsynthDLL->getSymbol("fluid_synth_write_s16");
	fluid_sequencer_register_fluidsynth = (xl_fluid_sequencer_register_fluidsynth_def)s_fluidsynthDLL->getSymbol("fluid_sequencer_register_fluidsynth");
	fluid_player_stop					= (xl_fluid_player_stop_def)s_fluidsynthDLL->getSymbol("fluid_player_stop");
	fluid_player_add					= (xl_fluid_player_add_def)s_fluidsynthDLL->getSymbol("fluid_player_add");
	fluid_player_get_status				= (xl_fluid_player_get_status_def)s_fluidsynthDLL->getSymbol("fluid_player_get_status");
	fluid_player_play					= (xl_fluid_player_play_def)s_fluidsynthDLL->getSymbol("fluid_player_play");
	fluid_player_set_loop				= (xl_fluid_player_set_loop_def)s_fluidsynthDLL->getSymbol("fluid_player_set_loop");

	return true;
}

void unloadFluidsynthDLL()
{
	delete s_fluidsynthDLL;
	s_fluidsynthDLL = NULL;
}

s32 fillFluidBuffer(u32 requestedChunkSize, u8* chunkData, f64 sampleRate, fluid_synth_t* synth, fluid_player_t* player)
{
	if (fluid_player_get_status(player) != FLUID_PLAYER_PLAYING)
	{
		return 0;
	}

	s32 res = fluid_synth_write_s16(synth, requestedChunkSize/4, chunkData, 0, 2, chunkData, 1, 2);
	if (res != FLUID_OK)
	{
		LOG( LOG_WARNING, "error during midi synthesis: %d", res );
		return 0;
	}

	return requestedChunkSize;
}