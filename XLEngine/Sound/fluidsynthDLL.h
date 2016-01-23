#pragma once
#include "../types.h"

#include <fluidsynth.h>
#include <fluidsynth/midi.h>
#include <fluidsynth/seq.h>
#include <fluidsynth/seqbind.h>

bool loadFluidsythDLL();
void unloadFluidsynthDLL();
s32  fillFluidBuffer(u32 requestedChunkSize, u8* chunkData, f64 sampleRate, fluid_synth_t* synth, fluid_player_t* player);

typedef fluid_settings_t* (*xl_new_fluid_settings_def)(void);
typedef int (*xl_fluid_settings_setstr_def)(fluid_settings_t*, const char*, const char*);
typedef fluid_synth_t* (*xl_new_fluid_synth_def)(fluid_settings_t*);
typedef fluid_sequencer_t* (*xl_new_fluid_sequencer2_def)(int);
typedef int (*xl_fluid_settings_getnum_def)(fluid_settings_t*, const char*, double*);
typedef int (*xl_fluid_synth_sfload_def)(fluid_synth_t*, const char*, int);
typedef short (*xl_fluid_sequencer_register_fluidsynth_def)(fluid_sequencer_t*, fluid_synth_t*);
typedef fluid_player_t* (*xl_new_fluid_player_def)(fluid_synth_t*);
typedef void (*xl_delete_fluid_settings_def)(fluid_settings_t*);
typedef int (*xl_delete_fluid_synth_def)(fluid_synth_t*);
typedef void (*xl_delete_fluid_sequencer_def)(fluid_sequencer_t*);
typedef int (*xl_delete_fluid_player_def)(fluid_player_t*);
typedef int (*xl_fluid_player_stop_def)(fluid_player_t*);
typedef int (*xl_fluid_player_add_def)(fluid_player_t*, const char*);
typedef int (*xl_fluid_player_get_status_def)(fluid_player_t*);
typedef int (*xl_fluid_synth_write_s16_def)(fluid_synth_t*, int, void*, int, int, void*, int, int);
typedef int (*xl_fluid_player_play_def)(fluid_player_t*);
typedef int (*xl_fluid_player_set_loop_def)(fluid_player_t*, int);

extern xl_new_fluid_settings_def					xl_new_fluid_settings;
extern xl_fluid_settings_setstr_def					xl_fluid_settings_setstr;
extern xl_new_fluid_synth_def						xl_new_fluid_synth;
extern xl_new_fluid_sequencer2_def					xl_new_fluid_sequencer2;
extern xl_fluid_settings_getnum_def					xl_fluid_settings_getnum;
extern xl_fluid_synth_sfload_def					xl_fluid_synth_sfload;
extern xl_fluid_sequencer_register_fluidsynth_def	xl_fluid_sequencer_register_fluidsynth;
extern xl_new_fluid_player_def						xl_new_fluid_player;
extern xl_delete_fluid_settings_def					xl_delete_fluid_settings;
extern xl_delete_fluid_synth_def					xl_delete_fluid_synth;
extern xl_delete_fluid_sequencer_def				xl_delete_fluid_sequencer;
extern xl_delete_fluid_player_def					xl_delete_fluid_player;
extern xl_fluid_player_stop_def						xl_fluid_player_stop;
extern xl_fluid_player_add_def						xl_fluid_player_add;
extern xl_fluid_player_get_status_def				xl_fluid_player_get_status;
extern xl_fluid_synth_write_s16_def					xl_fluid_synth_write_s16;
extern xl_fluid_player_play_def						xl_fluid_player_play;
extern xl_fluid_player_set_loop_def					xl_fluid_player_set_loop;

#define new_fluid_settings		xl_new_fluid_settings
#define fluid_settings_setstr	xl_fluid_settings_setstr
#define new_fluid_synth			xl_new_fluid_synth
#define new_fluid_sequencer2	xl_new_fluid_sequencer2
#define fluid_settings_getnum	xl_fluid_settings_getnum
#define fluid_synth_sfload		xl_fluid_synth_sfload
#define fluid_sequencer_register_fluidsynth		xl_fluid_sequencer_register_fluidsynth
#define new_fluid_player		xl_new_fluid_player
#define delete_fluid_settings	xl_delete_fluid_settings
#define delete_fluid_synth		xl_delete_fluid_synth
#define delete_fluid_sequencer	xl_delete_fluid_sequencer
#define delete_fluid_player		xl_delete_fluid_player
#define	fluid_player_stop		xl_fluid_player_stop
#define	fluid_player_add		xl_fluid_player_add
#define fluid_player_get_status	xl_fluid_player_get_status
#define	fluid_synth_write_s16	xl_fluid_synth_write_s16
#define fluid_player_play		xl_fluid_player_play
#define	fluid_player_set_loop	xl_fluid_player_set_loop