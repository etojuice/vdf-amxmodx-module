/*
 *	------------------------------------------------------------------
 *	  This is part of Vdf module examples, designed only to test and 
 *	  demonstrate this module functionality.
 *	  For more information check this topic:
 *	  http://forums.alliedmods.net/showthread.php?t=51662
 *	------------------------------------------------------------------
 */
 
//	
//
//	Daily mapcycle plugin
//	-----------------------
//	It uses on single file as a database to set copy a list of
//	maps into mapcycle.
//	This file uses days abbreviaton as tags, and 3 different periods
//	for each day. It also includes a "Any" tag, as so to make a list
//	to include no matter the day period.
//	The config file, daily_mapcycle.txt, is much clearer than this
//	this raw description.
//

#include <amxmodx>
#include <amxmisc>
#include <vdf>

#define PLUGIN "daily_mapcycle"
#define VERSION "1.04"
#define AUTHOR "commonbullet"

#define CONFIG_FILE "daily_maps.txt"
#define LOG_NO_CONFIG "No available maps config for this time/day."
#define LOG_NO_FILE "Could not open config file."

public plugin_init() 
{
	register_plugin(PLUGIN, VERSION, AUTHOR)	
	set_map_cycle()
}

get_config_file(filestr[], const filename[], maxlen)
{
	new len
	get_configsdir(filestr, maxlen)
	len = strlen(filestr)
	format(filestr[len], maxlen - len, "/%s", filename)
}

add_node_to_cycle(VdfNode:node, &pfile, &mapsadded)
{
	new VdfNode:child	
	new key[48]
	
	child = vdf_get_child_node(node)
	
	while(child) {
		
		if(!pfile)
			pfile = fopen("mapcycle.txt", "w+")	
		
		vdf_get_node_key(child, key, 47)
		
		fputs(pfile, key)
		fputs(pfile, "^n")
		child = vdf_get_next_node(child)
		mapsadded++
	}	
}

public set_map_cycle()
{
	new VdfTree:mapstree
	new VdfNode:cursor
	new VdfNode:currentnode
	new VdfSearch:search
	new today[5]
	new hourstr[5]
	new hour
	new periods[3][]  = {"Morning", "Day", "Later"}
	new period
	new configfile[72]	
	new pfile
	new count
	
	get_config_file(configfile, CONFIG_FILE, 72)	
	
	if(!(mapstree = vdf_open(configfile))) {
		log_message(LOG_NO_FILE)
		return
	}
	
	get_time("%H", hourstr, 4)
	get_time("%a", today, 4)
	
	hour = str_to_num(hourstr)	
	
	if(hour < 9)
		period = 0
	else if(hour < 18)
		period = 1
	else
		period = 2
	
	search = vdf_create_search()
	vdf_set_search(search, mapstree, "Current", _, 0)
	
	
	// To avoid creating lists whenever a map is launched
	// a "Current" tag is created any time the mapcycle is updated.
	
	if((currentnode = vdf_find_next_match(search))) {
		
		new compday[5]
		new compperiod[12]
		new value[20]
		
		vdf_get_node_value(currentnode, value, 19)
		
		strbreak(value, compday, 4, compperiod, 11)		
		
		if(equali(compperiod, periods[period]) &&
		    equali(compday, today)) {
			vdf_remove_tree(mapstree)
			vdf_close_search(search)
			return
		}
	}	
	
	vdf_set_search(search, mapstree, today, _, 0)
	
	if((cursor = vdf_find_next_match(search))) {
		
		cursor = vdf_get_child_node(cursor)
		
		if((cursor = vdf_find_in_branch(cursor, periods[period])))
			add_node_to_cycle(cursor, pfile, count)
		
		if((cursor = vdf_find_in_branch(cursor, "Any")))
			add_node_to_cycle(cursor, pfile, count)
	}
	else {
		log_message(LOG_NO_CONFIG)		
		vdf_close_search(search)
		vdf_remove_tree(mapstree)
		
		return
	}
	
	if(count) {
		new curstr[23]
		
		format(curstr, 22, "%s %s", today, periods[period])
		
		if(!currentnode) {
			new VdfNode:rootnode
			rootnode = vdf_get_root_node(mapstree)
			currentnode = vdf_append_node(mapstree, rootnode, "Current", curstr)
		}
		else
			vdf_set_node_value(currentnode, curstr)
		
		vdf_save(mapstree)
	}
	else 		
		log_message(LOG_NO_CONFIG)	
	
	vdf_close_search(search)
	vdf_remove_tree(mapstree)
	
	if(pfile)
		fclose(pfile)
}

