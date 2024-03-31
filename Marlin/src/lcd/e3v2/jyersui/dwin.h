/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2021 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#pragma once

/**
 * lcd/e3v2/jyersui/dwin.h
 */

#include "dwin_lcd.h"
#include "../common/dwin_set.h"
#include "../common/dwin_font.h"
#include "../common/dwin_color.h"
#include "../common/encoder.h"
#include "../common/limits.h"
#include "../../../libs/BL24CXX.h"

#include "../../../inc/MarlinConfigPre.h"

//#define DWIN_CREALITY_LCD_CUSTOM_ICONS

enum processID : uint8_t {
<<<<<<< HEAD
  Main, Print, Menu, Value, Option, File, Popup, Confirm, Keyboard, Wait
=======
  Proc_Main, Proc_Print, Proc_Menu, Proc_Value, Proc_Option,
  Proc_File, Proc_Popup, Proc_Confirm, Proc_Wait
>>>>>>> bugfix-2.1.x
};

enum PopupID : uint8_t {
  Popup_Pause,
  Popup_Stop,
  Popup_Resume,
  Popup_SaveLevel,
  Popup_ETemp,
  Popup_ConfFilChange,
  Popup_PurgeMore,
  Popup_MeshSlot,
  Popup_Level,
  Popup_Home,
  Popup_MoveWait,
  Popup_Heating,
  Popup_FilLoad,
  Popup_FilChange,
  Popup_TempWarn,
  Popup_Runout,
  Popup_PIDWait,
  Popup_MPCWait,
  Popup_Resuming,
  Popup_ManualProbing,
  Popup_FilInsert,
  Popup_HeaterTime,
  Popup_UserInput,
  Popup_LevelError,
  Popup_InvalidMesh,
  Popup_UI,
  Popup_Complete,
  Popup_Custom
};

enum menuID : uint8_t {
<<<<<<< HEAD
  MainMenu,
    Prepare,
      Move,
      HomeMenu,
      ManualLevel,
      ZOffset,
      Preheat,
      ChangeFilament,
      HostActions,
    Control,
      TempMenu,
        PID,
          HotendPID,
          BedPID,
        Preheat1,
        Preheat2,
        Preheat3,
        Preheat4,
        Preheat5,
      Motion,
        HomeOffsets,
        MaxSpeed,
        MaxAcceleration,
        MaxJerk,
        Steps,
      Visual,
        ColorSettings,
      HostSettings,
        ActionCommands,
      Advanced,
        ProbeMenu,
      Info,
    Leveling,
      LevelManual,
      LevelView,
      MeshViewer,
      LevelSettings,
      ManualMesh,
      UBLMesh,
    InfoMain,
  Tune,
  PreheatHotend
=======
  ID_MainMenu,
    ID_Prepare,
      ID_Move,
      ID_HomeMenu,
      ID_ManualLevel,
      ID_ZOffset,
      ID_Preheat,
      ID_ChangeFilament,
      ID_MenuCustom,
      OPTITEM(FWRETRACT, ID_FWMenu)
    ID_Control,
      ID_TempMenu,
        ID_PID,
          ID_HotendPID,
          ID_BedPID,
        #if HAS_PREHEAT
          #define _PREHEAT_ID(N) ID_Preheat##N,
          REPEAT_1(PREHEAT_COUNT, _PREHEAT_ID)
        #endif
        #if ANY(MPC_EDIT_MENU, MPC_AUTOTUNE_MENU)
          ID_MPC,
        #endif
      ID_Motion,
        ID_HomeOffsets,
        ID_MaxSpeed,
        ID_MaxAcceleration,
        ID_MaxJerk,
        ID_Steps,
      ID_Visual,
        ID_ColorSettings,
      ID_Advanced,
        ID_ProbeMenu,
        OPTITEM(HAS_TRINAMIC_CONFIG, ID_TMCMenu)
      ID_Info,
    ID_Leveling,
      ID_LevelManual,
      ID_LevelView,
      ID_MeshViewer,
      ID_LevelSettings,
      ID_ManualMesh,
      ID_UBLMesh,
    ID_InfoMain,
  ID_Tune,
  ID_PreheatHotend
>>>>>>> bugfix-2.1.x
};

// Custom icons
#if ENABLED(DWIN_CREALITY_LCD_CUSTOM_ICONS)
  // index of every custom icon should be >= CUSTOM_ICON_START
  #define CUSTOM_ICON_START         ICON_Checkbox_F
  #define ICON_Checkbox_F           200
  #define ICON_Checkbox_T           201
  #define ICON_Fade                 202
  #define ICON_Mesh                 203
  #define ICON_Tilt                 204
  #define ICON_Brightness           205
  #define ICON_AxisD                249
  #define ICON_AxisBR               250
  #define ICON_AxisTR               251
  #define ICON_AxisBL               252
  #define ICON_AxisTL               253
  #define ICON_AxisC                254
#else
  #define ICON_Fade                 ICON_Version
  #define ICON_Mesh                 ICON_Version
  #define ICON_Tilt                 ICON_Version
  #define ICON_Brightness           ICON_Version
  #define ICON_AxisD                ICON_Axis
  #define ICON_AxisBR               ICON_Axis
  #define ICON_AxisTR               ICON_Axis
  #define ICON_AxisBL               ICON_Axis
  #define ICON_AxisTL               ICON_Axis
  #define ICON_AxisC                ICON_Axis
#endif

enum colorID : uint8_t {
  Default, White, Green, Cyan, Blue, Magenta, Red, Orange, Yellow, Brown, Black
};

#define Custom_Colors       10
#define COLOR_AQUA          RGB(0x00,0x3F,0x1F)
#define COLOR_LIGHT_WHITE   0xBDD7
#define COLOR_GREEN         RGB(0x00,0x3F,0x00)
#define COLOR_LIGHT_GREEN   0x3460
#define COLOR_CYAN          0x07FF
#define COLOR_LIGHT_CYAN    0x04F3
#define COLOR_BLUE          0x015F
#define COLOR_LIGHT_BLUE    0x3A6A
#define COLOR_MAGENTA       0xF81F
#define COLOR_LIGHT_MAGENTA 0x9813
#define COLOR_LIGHT_RED     0x8800
#define COLOR_ORANGE        0xFA20
#define COLOR_LIGHT_ORANGE  0xFBC0
#define COLOR_LIGHT_YELLOW  0x8BE0
#define COLOR_BROWN         0xCC27
#define COLOR_LIGHT_BROWN   0x6204
#define COLOR_BLACK         0x0000
#define COLOR_GREY          0x18E3
#define COLOR_CHECKBOX      0x4E5C  // Check-box check color
#define COLOR_CONFIRM       0x34B9
#define COLOR_CANCEL        0x3186

class JyersDWIN {
public:
  static constexpr size_t eeprom_data_size = 48;
  static struct EEPROM_Settings { // use bit fields to save space, max 48 bytes
    bool time_format_textual : 1;
    #if ENABLED(AUTO_BED_LEVELING_UBL)
      uint8_t tilt_grid_size : 3;
    #endif
    uint16_t corner_pos : 10;
    uint8_t cursor_color : 4;
    uint8_t menu_split_line : 4;
    uint8_t menu_top_bg : 4;
    uint8_t menu_top_txt : 4;
    uint8_t highlight_box : 4;
    uint8_t progress_percent : 4;
    uint8_t progress_time : 4;
    uint8_t status_bar_text : 4;
    uint8_t status_area_text : 4;
    uint8_t coordinates_text : 4;
    uint8_t coordinates_split_line : 4;
    #if ENABLED(HOST_ACTION_COMMANDS)
      uint64_t host_action_label_1 : 48;
      uint64_t host_action_label_2 : 48;
      uint64_t host_action_label_3 : 48;
    #endif
  } eeprom_settings;

  static constexpr const char * const color_names[11] = { "Default", "White", "Green", "Cyan", "Blue", "Magenta", "Red", "Orange", "Yellow", "Brown", "Black" };
  static constexpr const char * const preheat_modes[3] = { "Both", "Hotend", "Bed" };

<<<<<<< HEAD
  static void Clear_Screen(uint8_t e=3);
  static void Draw_Float(float value, uint8_t row, bool selected=false, uint8_t minunit=10);
  static void Draw_Option(uint8_t value, const char * const * options, uint8_t row, bool selected=false, bool color=false);
  static void Draw_String(char * string, uint8_t row, bool selected=false, bool below=false);
  static const uint64_t Encode_String(const char * string);
  static void Decode_String(const uint64_t num, char string[8]);
  static uint16_t GetColor(uint8_t color, uint16_t original, bool light=false);
  static void Draw_Checkbox(uint8_t row, bool value);
  static void Draw_Title(const char * title);
  static void Draw_Title(FSTR_P const title);
  static void Draw_Menu_Item(uint8_t row, uint8_t icon=0, const char * const label1=nullptr, const char * const label2=nullptr, bool more=false, bool centered=false);
  static void Draw_Menu_Item(uint8_t row, uint8_t icon=0, FSTR_P const flabel1=nullptr, FSTR_P const flabel2=nullptr, bool more=false, bool centered=false);
  static void Draw_Menu(uint8_t menu, uint8_t select=0, uint8_t scroll=0);
  static void Redraw_Menu(bool lastprocess=true, bool lastselection=false, bool lastmenu=false);
  static void Redraw_Screen();
=======
  static void clearScreen(const uint8_t e=3);
  static void drawFloat(const_float_t value, const uint8_t row, const bool selected=false, const uint8_t minunit=10);
  static void drawOption(const uint8_t value, const char * const * options, const uint8_t row, const bool selected=false, const bool color=false);
  static uint16_t getColor(const uint8_t color, const uint16_t original, const bool light=false);
  static void drawCheckbox(const uint8_t row, const bool value);
  static void drawTitle(const char * const title);
  static void drawTitle(FSTR_P const title);
  static void drawMenuItem(const uint8_t row, uint8_t icon=0, const char * const label1=nullptr, const char * const label2=nullptr, const bool more=false, const bool centered=false);
  static void drawMenuItem(const uint8_t row, uint8_t icon=0, FSTR_P const flabel1=nullptr, FSTR_P const flabel2=nullptr, const bool more=false, const bool centered=false);
  static void drawMenu(const uint8_t menu, const uint8_t select=0, const uint8_t scroll=0);
  static void redrawMenu(const bool lastproc=true, const bool lastsel=false, const bool lastmenu=false);
  static void redrawScreen();
>>>>>>> bugfix-2.1.x

  static void mainMenuIcons();
  static void drawMainMenu(uint8_t select=0);
  static void printScreenIcons();
  static void drawPrintScreen();
  static void drawPrintFilename(const bool reset=false);
  static void drawPrintProgressBar();
  #if ENABLED(SET_REMAINING_TIME)
    static void drawPrintProgressRemain();
  #endif
<<<<<<< HEAD
  static void Draw_Print_ProgressElapsed();
  static void Draw_Print_confirm();
  static void Draw_SD_Item(uint8_t item, uint8_t row);
  static void Draw_SD_List(bool removed=false);
  static void Draw_Status_Area(bool icons=false);
  static void Draw_Popup(FSTR_P const line1, FSTR_P const line2, FSTR_P const line3, uint8_t mode, uint8_t icon=0);
  static void Popup_Select();
  static void Update_Status_Bar(bool refresh=false);
  static void Draw_Keyboard(bool restrict, bool numeric, uint8_t selected=0, bool uppercase=false, bool lock=false);
  static void Draw_Keys(uint8_t index, bool selected, bool uppercase=false, bool lock=false);
=======
  static void drawPrintProgressElapsed();
  static void drawPrintConfirm();
  static void drawSDItem(const uint8_t item, const uint8_t row);
  static void drawSDList(const bool removed=false);
  static void drawStatusArea(const bool icons=false);
  static void drawPopup(FSTR_P const line1, FSTR_P const line2, FSTR_P const line3, uint8_t mode, uint8_t icon=0);
  static void popupSelect();
  static void updateStatusBar(const bool refresh=false);
>>>>>>> bugfix-2.1.x

  static FSTR_P getMenuTitle(const uint8_t menu);
  static uint8_t getMenuSize(const uint8_t menu);
  static void menuItemHandler(const uint8_t menu, const uint8_t item, bool draw=true);

  static void popupHandler(const PopupID popupid, bool option=false);
  static void confirmHandler(const PopupID popupid);

  static void mainMenuControl();
  static void menuControl();
  static void valueControl();
  static void optionControl();
  static void fileControl();
  static void printScreenControl();
  static void popupControl();
  static void confirmControl();

<<<<<<< HEAD
  static void Main_Menu_Control();
  static void Menu_Control();
  static void Value_Control();
  static void Option_Control();
  static void File_Control();
  static void Print_Screen_Control();
  static void Popup_Control();
  static void Confirm_Control();
  static void Keyboard_Control();

  static void Setup_Value(float value, float min, float max, float unit, uint8_t type);
  static void Modify_Value(float &value, float min, float max, float unit, void (*f)()=nullptr);
  static void Modify_Value(uint8_t &value, float min, float max, float unit, void (*f)()=nullptr);
  static void Modify_Value(uint16_t &value, float min, float max, float unit, void (*f)()=nullptr);
  static void Modify_Value(int16_t &value, float min, float max, float unit, void (*f)()=nullptr);
  static void Modify_Value(uint32_t &value, float min, float max, float unit, void (*f)()=nullptr);
  static void Modify_Value(int8_t &value, float min, float max, float unit, void (*f)()=nullptr);
  static void Modify_Option(uint8_t value, const char * const * options, uint8_t max);
  static void Modify_String(char * string, uint8_t maxlength, bool restrict);

  static void Update_Status(const char * const text);
  static void Start_Print(bool sd);
  static void Stop_Print();
  static void Update();
  static void State_Update();
  static void Screen_Update();
  static void AudioFeedback(const bool success=true);
  static void Save_Settings(char *buff);
  static void Load_Settings(const char *buff);
  static void Reset_Settings();
=======
  static void setupValue(const_float_t value, const_float_t min, const_float_t max, const_float_t unit, const uint8_t type);
  static void modifyValue(float &value, const_float_t min, const_float_t max, const_float_t unit, void (*f)()=nullptr);
  static void modifyValue(uint8_t &value, const_float_t min, const_float_t max, const_float_t unit, void (*f)()=nullptr);
  static void modifyValue(uint16_t &value, const_float_t min, const_float_t max, const_float_t unit, void (*f)()=nullptr);
  static void modifyValue(int16_t &value, const_float_t min, const_float_t max, const_float_t unit, void (*f)()=nullptr);
  static void modifyValue(uint32_t &value, const_float_t min, const_float_t max, const_float_t unit, void (*f)()=nullptr);
  static void modifyValue(int8_t &value, const_float_t min, const_float_t max, const_float_t unit, void (*f)()=nullptr);
  static void modifyOption(const uint8_t value, const char * const * options, const uint8_t max);

  static void updateStatus(const char * const text);
  static void startPrint(const bool sd);
  static void stopPrint();
  static void update();
  static void stateUpdate();
  static void screenUpdate();
  static void audioFeedback(const bool success=true);
  static void saveSettings(char * const buff);
  static void loadSettings(const char * const buff);
  static void resetSettings();
>>>>>>> bugfix-2.1.x
};

extern JyersDWIN jyersDWIN;
