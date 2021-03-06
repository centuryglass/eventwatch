#include <pebble.h>
#include "display_handler.h"
#include "display_core.h"
#include "storage_keys.h"
#include "util.h"
#include "debug.h"

//-----LOCAL VALUE DEFINITIONS-----
//#define DEBUG_DISPLAY //uncomment to enable display debug logging
#define DEFAULT_DATE_FORMAT "%d %b %Y" //default strftime date format


//----------LOCAL VARIABLES----------
char date_format[12] = DEFAULT_DATE_FORMAT;
  //strftime date format

int weatherCondition = 0;//weather condition code
//upper bounds(exclusive) for each weather condition category

Theme displayTheme = THEME_CORINTHIAN;//watch theme

//----------STATIC FUNCTION DECLARATIONS----------
static void update_progress(LayerID progressID,int percent);
  //Re-sizes a progress bar
static void update_weather_condition();
  //Updates weather condition display


//----------PUBLIC FUNCTIONS----------
//initializes all display functionality
void display_init(){
  if(persist_exists(PERSIST_KEY_THEME))
    displayTheme =  persist_read_int(PERSIST_KEY_THEME);
  set_theme(displayTheme);
  if(persist_exists(PERSIST_KEY_WEATHER_COND))
    weatherCondition = persist_read_int(PERSIST_KEY_WEATHER_COND);
  if(persist_exists(PERSIST_KEY_DATE_FORMAT))
      persist_read_string(PERSIST_KEY_DATE_FORMAT,date_format,sizeof(date_format));//load date format
  update_weather_condition();
}

//shuts down all display functionality
void display_deinit(){
  persist_write_string(PERSIST_KEY_DATE_FORMAT,date_format);
  persist_write_int(PERSIST_KEY_WEATHER_COND,weatherCondition);
  persist_write_int(PERSIST_KEY_THEME,displayTheme);
  display_destroy();
}

//sets the display theme
void set_theme(Theme theme){
  displayTheme = theme;
  int fontIDs[NUM_FONTS];
  int fontMargins[NUM_FONTS];
  int themeID;
  switch(theme){
    case THEME_BITWISE:
      fontIDs[FONT_SMALL] = RESOURCE_ID_CODERS_CRUX_16;
      fontIDs[FONT_LARGE] = RESOURCE_ID_CODERS_CRUX_48;
      fontMargins[FONT_SMALL] = 8;
      fontMargins[FONT_LARGE] = 24;
      themeID = RESOURCE_ID_BITWISE;
      break;
    case THEME_CORINTHIAN:
      fontIDs[FONT_SMALL] = RESOURCE_ID_INCONSOLATA_12;
      fontIDs[FONT_LARGE] = RESOURCE_ID_INCONSOLATA_36;
      fontMargins[FONT_SMALL] = 4;
      fontMargins[FONT_LARGE] = 8;
      themeID = RESOURCE_ID_CORINTHIAN;
      break;
    default:
      #ifdef DEBUG_DISPLAY
      APP_LOG(APP_LOG_LEVEL_ERROR,"set_theme: invalid theme!");
      #endif
      return;
  }
  #ifdef DEBUG_DISPLAY
  APP_LOG(APP_LOG_LEVEL_DEBUG,"set_theme: destroying window");
  #endif
  display_destroy();
  
  #ifdef DEBUG_DISPLAY
  APP_LOG(APP_LOG_LEVEL_DEBUG,"set_theme: re-creating window");
  #endif
  display_create(themeID,displayTheme,fontIDs,fontMargins);
  
  #ifdef DEBUG_DISPLAY
  APP_LOG(APP_LOG_LEVEL_DEBUG,"set_theme: window created");
  #endif
  update_weather_condition();
}

//Updates display data for events
void update_event_display(int eventNum, char * event_title, char * event_time,
                          int eventPercent, char * event_color){
  #ifdef DEBUG_DISPLAY
  APP_LOG(APP_LOG_LEVEL_DEBUG,"update_event_display: starting update");
  #endif
  
  char eventStr[64];
  strncpy(eventStr,event_title,sizeof(eventStr));
  strcat(eventStr,"\n");
  strncat(eventStr,event_time,sizeof(eventStr)-strlen(eventStr)-1);
  
  #ifdef DEBUG_DISPLAY
  APP_LOG(APP_LOG_LEVEL_DEBUG,"update_event_display:Setting event text to %s",eventStr);
  #endif   
  
  update_display_text(eventStr,eventNum == 0 ? 
                      TEXTLAYER_EVENT_0 : TEXTLAYER_EVENT_1);
  //finally, update event progress bar,
  update_progress(eventNum == 0 ? IMAGE_LAYER_EVENT_0_PROGRESS:IMAGE_LAYER_EVENT_1_PROGRESS,
                    eventPercent); 
  //set progress bar color if display is in color
  #ifdef PBL_COLOR
  #ifdef DEBUG_DISPLAY
  APP_LOG(APP_LOG_LEVEL_DEBUG,"update_event_display:Setting progress bar color to %s",event_color);
  #endif
  update_color(event_color,eventNum == 0 ? EVENT_0_COLOR : EVENT_1_COLOR);
  #endif
}


//Sets the displayed time and date
void set_time(time_t newTime){
  #ifdef DEBUG_DISPLAY
    APP_LOG(APP_LOG_LEVEL_DEBUG,"set_time: updating time display");
  #endif
  struct tm *tick_time = localtime(&newTime);
  //set display time
  char buffer[24];
  strftime(buffer, sizeof(buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);
  #ifdef DEBUG_DISPLAY
    APP_LOG(APP_LOG_LEVEL_DEBUG,"set_time: setting time string to %s",buffer);
  #endif
  
  update_display_text(buffer,TEXTLAYER_TIME);
  //set display date
  strftime(buffer, sizeof(buffer),date_format, tick_time);
  #ifdef DEBUG_DISPLAY
  APP_LOG(APP_LOG_LEVEL_DEBUG,"set_time: setting date string to %s",buffer);
  #endif
  update_display_text(buffer,TEXTLAYER_DATE);
  //Update day progress bar
  int minutes_passed_today = tick_time->tm_hour*60 + tick_time->tm_min;
  int dayPercent = minutes_passed_today*100/(60*24);
  update_progress(IMAGE_LAYER_DAY_PROGRESS, dayPercent);
}

//Updates weather display
void update_weather(int degrees, int condition){
  #ifdef DEBUG_DISPLAY
    APP_LOG(APP_LOG_LEVEL_DEBUG,"update_weather:Data received: %d degrees, condition code %d",degrees,condition);
  #endif
  
  //set degrees
  char weatherStr[16];
  snprintf(weatherStr,16,"%d°",degrees);
  
  #ifdef DEBUG_DISPLAY
    APP_LOG(APP_LOG_LEVEL_DEBUG,"update_weather:final weather string is %s",weatherStr);
  #endif
  
  update_display_text(weatherStr,TEXTLAYER_WEATHERTEXT);
  weatherCondition = condition;
  update_weather_condition();
}

/**
*change non-event color values to the ones stored in a color array
*@param colorArray an array of NUM_COLOR color strings,
*each 7 bits long
*/
void update_colors(char colorArray[NUM_COLORS][7]){
  for(int i = 0; i < NUM_COLORS; i++){
    if(strcmp(colorArray[i],"") != 0){
      update_color(colorArray[i],i);
    }
  }
  #ifdef PBL_BW//if monochrome, make sure progress bars are set to text color 
  update_color(colorArray[TEXT_COLOR],EVENT_0_COLOR);
  update_color(colorArray[TEXT_COLOR],EVENT_1_COLOR);
  #endif
}

//Directly updates display text
void update_text(char * newText, DisplayTextType textType){
  #ifdef DEBUG_DISPLAY
    APP_LOG(APP_LOG_LEVEL_DEBUG,"update_display_text: New text =%s",newText);
  #endif
  int textLayerIndex = -1;
  switch(textType){
    case TEXT_INFOTEXT:
      textLayerIndex = TEXTLAYER_INFOTEXT;
      break;
    case TEXT_PHONE_BATTERY:
      textLayerIndex = TEXTLAYER_PHONE_BATTERY;
      break;
    case TEXT_PEBBLE_BATTERY:
      textLayerIndex = TEXTLAYER_PEBBLE_BATTERY;
      break;
  }
  if(textLayerIndex == -1){
    APP_LOG(APP_LOG_LEVEL_ERROR,"update_display_text:invalid text field %d!",textType);
    return;
  }
  update_display_text(newText, textLayerIndex);
}

//set the strftime date format
void set_date_format(char * format){
  strncpy(date_format, format, sizeof(date_format));
}

//----------STATIC FUNCTIONS----------

/**
*Re-sizes a progress bar
*@param bar the progress bar
*@param percent new bar percentage
*@post the progress bar is re-sized
*/
static void update_progress(LayerID progressID,int percent){
  #ifdef DEBUG_DISPLAY
    APP_LOG(APP_LOG_LEVEL_DEBUG,"update_progress:Updating progress bar to %d percent",percent);
  #endif
  GRect frame = get_default_frame(progressID);
  if(percent < 3) percent = 3; //always show a bit of the event color
  int newW = frame.size.w * percent /100;
  //set progress bar width to x% of the default
  set_frame(GRect(frame.origin.x,frame.origin.y,newW, frame.size.h),progressID);
  #ifdef DEBUG_DISPLAY
    APP_LOG(APP_LOG_LEVEL_DEBUG,"update_progress:Changing width from %d to %d",frame.size.w,newW);
  #endif
}

/**
*Updates weather condition display
*/
static void update_weather_condition(){
   int weatherCondCodeBounds [] = {
    300,//thunderstorm-2xx
    400,//drizzle-3xx
    600,//rain-5xx
    700,//snow-6xx
    800,//atmosphere-7xx
    801,//clear-800
    810,//clouds-80x
    910,//extreme-90x
    1000,//additional-9xx
  };
  
  //Weather Condition definitions
  enum{
    THUNDERSTORM, //GROUP 2XX
    DRIZZLE,  //GROUP 3XX
    RAIN,  //GROUP 5XX
    SNOW,  //GROUP 6XX
    ATMOSPHERE,  //GROUP 7XX
    CLEAR,  //GROUP 800
    CLOUDS,  //GROUP 80X
    EXTREME,  //GROUP 90X
    ADDITIONAL,  //GROUP 9XX
    NUM_WEATHER_CONDITIONS = 9
  };
  #ifdef DEBUG_DISPLAY
    APP_LOG(APP_LOG_LEVEL_DEBUG,"update_weather_condition:Setting weather condition icon for id %d",weatherCondition);
  #endif
  //set condition icon
  GRect bounds = get_bounds(IMAGE_LAYER_WEATHER_ICONS);
  GRect frame = get_frame(IMAGE_LAYER_WEATHER_ICONS);
  #ifdef DEBUG_DISPLAY
  APP_LOG(APP_LOG_LEVEL_DEBUG,"update_weather_condition:bounds found: x=%d, y=%d, w=%d, h=%d",bounds.origin.x,bounds.origin.y,bounds.size.w,bounds.size.h);
  APP_LOG(APP_LOG_LEVEL_DEBUG,"update_weather_condition:frame found: x=%d, y=%d, w=%d, h=%d",frame.origin.x,frame.origin.y,frame.size.w,frame.size.h);
  #endif
  for(int i = 0; i < NUM_WEATHER_CONDITIONS; i++){
    if(weatherCondition < weatherCondCodeBounds[i]){
      //condition found, set bitmap bounds to section i of 9
      int x = (i % 3) * bounds.size.w;
      int y = (i / 3) * bounds.size.h;
      #ifdef DEBUG_DISPLAY
      APP_LOG(APP_LOG_LEVEL_DEBUG,"update_weather_condition:weather condition %d/9,%d<%d, x:%d y:%d",i+1,weatherCondition,weatherCondCodeBounds[i],-x,-y);
      #endif
      set_bounds(GRect(-x,-y,bounds.size.w,bounds.size.h),IMAGE_LAYER_WEATHER_ICONS);
      break;
    }
  }
  
}