/*
 *  network_dialog_widgets_sdl.h

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

 *  Custom widgets for network-related dialogs in the SDL version.
 *
 *  Created by Woody Zenfell, III on Fri Sep 28 2001.
 */

#ifndef	NETWORK_DIALOG_WIDGETS_SDL_H
#define	NETWORK_DIALOG_WIDGETS_SDL_H

#include "sdl_widgets.h"
#include "SSLP_API.h"

#include "player.h"	// for MAXIMUM_PLAYER_NAME_LENGTH
#include "PlayerImage_sdl.h"
#include "network_dialogs.h" // for net_rank



//////// w_found_players ////////
// This lists the "players on network" in the Gather box.  It manages the SSLP callback information
// to produce/update its list.  (Well, I guess it depends on someone setting up simple callbacks
// that forward to w_found_players::found_player(), etc.)
struct player_info;

class w_found_players;

typedef void (*player_selected_callback_t)(w_found_players*, const SSLP_ServiceInstance*);

class w_found_players : public w_list<const SSLP_ServiceInstance*> {
public:
    w_found_players(int width, int numRows) :
        w_list<const SSLP_ServiceInstance*>(listed_players, width, numRows, 0), player_selected_callback(NULL)
        { num_items = 0; }
        // must update num_items here since listed_players had not been initialized earlier when passed to w_list<>()
    
    void found_player(const SSLP_ServiceInstance* player);
    void lost_player(const SSLP_ServiceInstance* player);
    void player_name_changed(const SSLP_ServiceInstance* player);
    
    void hide_player(const SSLP_ServiceInstance* player);
    
    void item_selected();

    // currently, this depends on the callback removing the item passed to it.  yuck, but it works for
    // our purposes and is a little easier to implement ;)
    void callback_on_all_items();

    void set_player_selected_callback(player_selected_callback_t callback) { player_selected_callback = callback; }
    
private:
    vector<const SSLP_ServiceInstance*>	found_players;		// players that are out there
    vector<const SSLP_ServiceInstance*>	hidden_players;		// players we don't want displayed - may include some not in found_players.
    vector<const SSLP_ServiceInstance*>	listed_players;		// {found_players} - {hidden_players} (keyed by particular found instance, not name
                                                                // nor address)

	// Note: no need to clean up serviceInstances when done; SSLP will free the storage as
	// services become unavailable or when lookup is stopped.

    player_selected_callback_t		player_selected_callback;	// called when a player is clicked on

    void list_player(const SSLP_ServiceInstance* player);
    void unlist_player(const SSLP_ServiceInstance* player);
    
    void draw_item(vector<const SSLP_ServiceInstance*>::const_iterator i, SDL_Surface* s, int x, int y, int width, bool selected) const;
};




//////// w_chat_history ////////
// This lists things people have said in a chat history scrollback buffer thingy.
struct chat_entry {
    uint32		player_pixel_color;
    uint32		team_pixel_color;
    char*		player_name;
    char*		chat_text;
};

class w_chat_history : public w_list<chat_entry> {
private:    
    vector<chat_entry>	chat_lines;

public:
    w_chat_history(int width, int numRows) :
        w_list<chat_entry>(chat_lines, width, numRows, 0)
        { num_items = 0; }
        // must update num_items since chat_lines was not initialized when w_list<> acted on it.
        
	// Widget selectable?
	virtual bool is_selectable(void) const {return false;}

    void item_selected() {}
    
    // player may be NULL, to put informational messages into the chat buffer.
    void append_chat_entry(const player_info* player, const char* chat_text);
    
	~w_chat_history();
private:
    void append_chat_entry(const chat_entry& entry);

    void draw_item(vector<chat_entry>::const_iterator i, SDL_Surface* s, int x, int y, int width, bool selected) const;
};



//////// w_players_in_game2 ////////
// This serves both as a "who's in the game?" widget for gather/join AND as the graph widget in
// the Postgame Carnage Report.  Yes, there WAS a w_players_in_game, that was used just for the
// former purpose, but it's no longer relevant.
struct player_entry2 {
    char		player_name[MAXIMUM_PLAYER_NAME_LENGTH + 1];
    uint32		name_pixel_color;
    int16		name_width;
    PlayerImage*	player_image;
};

struct	bar_info;
class	TextLayoutHelper;
class	w_players_in_game2;

typedef void (*element_clicked_callback_t)(w_players_in_game2* inWPIG2, // widget clicked
                                           bool inTeam,                 // team?  (or player)
                                           bool inGraph,                // postgame carnage report?
                                           bool inScores,               // showing scores?  (or carnage)
                                           int inDrawIndex,             // starting with 0 at left
                                           int inPlayerIndexOrTeamColor); // meaning depends on inTeam

class w_players_in_game2 : public widget {
public:
        // pass "true" for a widget that takes more vertical space (for postgame carnage report)
	w_players_in_game2(bool inPostgameLayout);
        
        // Update from dynamic world (e.g. postgame)?  (else from topology)
        void update_display(bool inFromDynamicWorld = false);

        // Call this at least once when there is valid topology data (no need if update_display fromDynamicWorld)
        void start_displaying_actual_information() { displaying_actual_information = true; }

	virtual void draw(SDL_Surface *s) const;

	// User clicked in widget - element_clicked_callback, if set, will be invoked
    // if user clicked reasonably close to a player icon.  NOTE currently, despite
    // appearances, callback will NOT be invoked if not showing a postgame report.
	virtual void click(int x, int y);

	// Widget selectable?
	virtual bool is_selectable(void) const {return false;}

    void    set_graph_data(const net_rank* inRankings, int inNumRankings, int inSelectedPlayer,
                            bool inClumpPlayersByTeam, bool inDrawScoresNotCarnage);

    void    set_element_clicked_callback(element_clicked_callback_t inCallback)
                { element_clicked_callback = inCallback; }

        ~w_players_in_game2();

protected:
    // Local storage
    vector<player_entry2>	player_entries;
	bool			displaying_actual_information;
    bool			postgame_layout;
    element_clicked_callback_t  element_clicked_callback;

    // Stuff in support of postgame carnage report
    bool        draw_carnage_graph;
    vector<int>	players_on_team[NUMBER_OF_TEAM_COLORS];	// (note array of vectors) hold indices into player_entries
    net_rank    net_rankings[MAXIMUM_NUMBER_OF_PLAYERS];
    int         num_valid_net_rankings;
    int         selected_player;
    bool        clump_players_by_team;
    bool        draw_scores_not_carnage;

    // Local methods
    void draw_player_icon(SDL_Surface* s, int rank_index, int center_x) const;
    void draw_player_icons_separately(SDL_Surface* s) const;
    void draw_player_icons_clumped(SDL_Surface* s) const;
    void draw_player_names_separately(SDL_Surface* s, TextLayoutHelper& ioTextLayoutHelper) const;
    void draw_player_names_clumped(SDL_Surface* s, TextLayoutHelper& ioTextLayoutHelper) const;
    int  find_maximum_bar_value() const;
    void draw_bar_or_bars(SDL_Surface* s, int rank_index, int center_x, int maximum_value, vector<bar_info>& outBarInfos) const;
    void draw_bars_separately(SDL_Surface* s, vector<bar_info>& outBarInfos) const;
    void draw_bars_clumped(SDL_Surface* s, vector<bar_info>& outBarInfos) const;
    void draw_bar_labels(SDL_Surface* s, const vector<bar_info>& inBarInfos, TextLayoutHelper& ioTextLayoutHelper) const;
    void draw_carnage_totals(SDL_Surface* s) const;
    void draw_carnage_legend(SDL_Surface* s) const;

    void draw_bar(SDL_Surface* s, int inCenterX, int inBarColorIndex, int inBarValue, int inMaxValue, bar_info& outBarInfo) const;

    void clear_vector();
        
    // Class (static) methods
};

#endif//NETWORK_DIALOG_WIDGETS_SDL_H
