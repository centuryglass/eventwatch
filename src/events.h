/**
*@File events.h
*Handles all functionality related to
*storing, retrieving, and formatting event data
*/

#pragma once
#include <pebble.h>

#define MAX_EVENT_LENGTH 24 //Maximum number of characters allowed in an event title
#define NUM_EVENTS 2  //Number of events stored

/**
*initializes event functionality 
*/
void events_init();

/**
*Shuts down event functionality
*/
void events_deinit();

/**
*Stores an event
*@param numEvent the event slot to set
*@param title the event title
*@param start the event start time
*@param end the event end time
*@param color the event color string
*/
void add_event(int numEvent,char *title,long start,long end,char* color);

/**
*Gets one of the stored event titles
*@param numEvent the event number
*@param buffer the buffer where the event string will be stored
*@param bufSize number of bytes allocated to the buffer
*@return buffer if operation succeeds, NULL otherwise
*/
char *get_event_title(int numEvent,char *buffer,int bufSize);

/**
*Gets the percent complete of an event
*@param numEvent the event to access
*@return the percent completed, or -1 if event has
*not yet started
*/
int get_percent_complete(int numEvent);

/**
*Gets one of the stored events' time info as a formatted string
*Either Days/Hours/Minutes until event, or percent complete
*@param numEvent the event number
*@param buffer the buffer where the time string will be stored
*@param bufSize number of bytes allocated to the buffer
*@return buffer if operation succeeds, NULL otherwise
*/
char *get_event_time_string(int numEvent,char *buffer,int bufSize);

/**
*Gets an event's display color
*@param numEvent the event to access
*@param buffer color string is copied into this, must be at least 7 bytes
*/
void get_event_color(int numEvent,char * buffer);

typedef enum{
  TIME_REMAINING_ONLY,
  INCLUDE_DATE_MONTH_FIRST,
  INCLUDE_DATE_DAY_FIRST
} FutureEventFormat;

/**
*Sets the format for upcoming event time strings
*@param format the selected format
*/
void setFutureEventTimeFormat(FutureEventFormat format);


