/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
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

#include "../DGUSScreenHandlerBase.h"

enum DGUS_ScreenID : uint8_t;

class DGUSScreenHandlerMKS : public DGUSScreenHandler {
public:
  DGUSScreenHandlerMKS() = default;

  #if 0
  static void sendinfoscreen_ch(const uint16_t *line1, const uint16_t *line2, const uint16_t *line3, const uint16_t *line4);
  static void sendinfoscreen_en(PGM_P const line1, PGM_P const line2, PGM_P const line3, PGM_P const line4);
  static void sendInfoScreen(const void *line1, const void *line2, const void *line3, const void *line4, uint16_t language);
  #endif

  static void screenBackChange(DGUS_VP_Variable &var, void *val_ptr);

  static void eepromControl(DGUS_VP_Variable &var, void *val_ptr);
  static void languageChange(DGUS_VP_Variable &var, void *val_ptr);
  static void getOffsetValue(DGUS_VP_Variable &var, void *val_ptr);
  static void levelControl(DGUS_VP_Variable &var, void *val_ptr);
  static void meshLevel(DGUS_VP_Variable &var, void *val_ptr);
  static void meshLevelDistanceConfig(DGUS_VP_Variable &var, void *val_ptr);
  static void manualAssistLeveling(DGUS_VP_Variable &var, void *val_ptr);
  static void zOffsetConfirm(DGUS_VP_Variable &var, void *val_ptr);
  static void zOffsetSelect(DGUS_VP_Variable &var, void *val_ptr);
  static void getManualMovestep(DGUS_VP_Variable &var, void *val_ptr);
  static void getZoffsetDistance(DGUS_VP_Variable &var, void *val_ptr);
  static void getMinExtrudeTemp(DGUS_VP_Variable &var, void *val_ptr);
  static void getParkPos(DGUS_VP_Variable &var, void *val_ptr);
  #if ENABLED(PREVENT_COLD_EXTRUSION)
    static void handleGetExMinTemp(DGUS_VP_Variable &var, void *val_ptr);
  #endif
  static void languageDisplay(uint8_t var);
  static void tmcChangeConfig(DGUS_VP_Variable &var, void *val_ptr);
  static void getTurnOffCtrl(DGUS_VP_Variable &var, void *val_ptr);
  static void languagePInit();
  static void runoutIdle();
  static void runoutInit();
  static void extrudeLoadInit();
  static void lcdBLKAdjust(DGUS_VP_Variable &var, void *val_ptr);
  static void sdFileBack(DGUS_VP_Variable &var, void *val_ptr);

  static void handleStepPerMMChanged(DGUS_VP_Variable &var, void *val_ptr);
  static void handleStepPerMMExtruderChanged(DGUS_VP_Variable &var, void *val_ptr);
  static void handleMaxSpeedChange(DGUS_VP_Variable &var, void *val_ptr);
  static void handleExtruderMaxSpeedChange(DGUS_VP_Variable &var, void *val_ptr);
  static void handleAccChange(DGUS_VP_Variable &var, void *val_ptr);
  static void handleMaxAccChange(DGUS_VP_Variable &var, void *val_ptr);
  static void handleExtruderAccChange(DGUS_VP_Variable &var, void *val_ptr);
  static void handleChangeLevelPoint(DGUS_VP_Variable &var, void *val_ptr);
  static void handleTravelAccChange(DGUS_VP_Variable &var, void *val_ptr);
  static void handleFeedRateMinChange(DGUS_VP_Variable &var, void *val_ptr);
  static void handleMin_T_F(DGUS_VP_Variable &var, void *val_ptr);

  #if HAS_PID_HEATING
    static void filamentLoadUnload(DGUS_VP_Variable &var, void *val_ptr, const int filamentDir);
    static void filamentLoad(DGUS_VP_Variable &var, void *val_ptr);
    static void filamentUnload(DGUS_VP_Variable &var, void *val_ptr);
    static void getManualFilament(DGUS_VP_Variable &var, void *val_ptr);
    static void getManualFilamentSpeed(DGUS_VP_Variable &var, void *val_ptr);
  #endif

  #if HAS_MEDIA
    // Marlin informed us about SD print completion.
    static void sdPrintingFinished();
  #else
    static void printReturn(DGUS_VP_Variable &var, void *val_ptr);
  #endif

  static void sendPrintTimeToDisplay(DGUS_VP_Variable &var);
  static void sendBabyStepToDisplay(DGUS_VP_Variable &var);
  static void sendFanToDisplay(DGUS_VP_Variable &var);
  static void sendGbkToDisplay(DGUS_VP_Variable &var);
  static void sendStringToDisplay_Language(DGUS_VP_Variable &var);
  static void sendTMCStepValue(DGUS_VP_Variable &var);

  static void setUint8(DGUS_VP_Variable &var, void *val_ptr);

<<<<<<< HEAD
  // Recall the remembered screen.
  static void PopToOldScreen();

  // Make the display show the screen and update all VPs in it.
  static void GotoScreen(DGUSLCD_Screens screen, bool ispopup = false);

  static void UpdateScreenVPData();

  // Helpers to convert and transfer data to the display.
  static void DGUSLCD_SendWordValueToDisplay(DGUS_VP_Variable &var);
  static void DGUSLCD_SendStringToDisplay(DGUS_VP_Variable &var);
  static void DGUSLCD_SendStringToDisplayPGM(DGUS_VP_Variable &var);
  static void DGUSLCD_SendTemperaturePID(DGUS_VP_Variable &var);
  static void DGUSLCD_SendPercentageToDisplay(DGUS_VP_Variable &var);
  static void DGUSLCD_SendPrintProgressToDisplay(DGUS_VP_Variable &var);
  static void DGUSLCD_SendPrintTimeToDisplay(DGUS_VP_Variable &var);

  static void DGUSLCD_SendPrintTimeToDisplay_MKS(DGUS_VP_Variable &var);
  static void DGUSLCD_SendBabyStepToDisplay_MKS(DGUS_VP_Variable &var);
  static void DGUSLCD_SendFanToDisplay(DGUS_VP_Variable &var);
  static void DGUSLCD_SendGbkToDisplay(DGUS_VP_Variable &var);
  static void DGUSLCD_SendStringToDisplay_Language_MKS(DGUS_VP_Variable &var);
  static void DGUSLCD_SendTMCStepValue(DGUS_VP_Variable &var);

  #if ENABLED(PRINTCOUNTER)
    static void DGUSLCD_SendPrintAccTimeToDisplay(DGUS_VP_Variable &var);
    static void DGUSLCD_SendPrintsTotalToDisplay(DGUS_VP_Variable &var);
  #endif
  #if HAS_FAN
    static void DGUSLCD_SendFanStatusToDisplay(DGUS_VP_Variable &var);
  #endif
  static void DGUSLCD_SendHeaterStatusToDisplay(DGUS_VP_Variable &var);
  #if ENABLED(DGUS_UI_WAITING)
    static void DGUSLCD_SendWaitingStatusToDisplay(DGUS_VP_Variable &var);
  #endif

  // Send a value from 0..100 to a variable with a range from 0..255
  static void DGUSLCD_PercentageToUint8(DGUS_VP_Variable &var, void *val_ptr);

  static void DGUSLCD_SetUint8(DGUS_VP_Variable &var, void *val_ptr);

  template<typename T>
  static void DGUSLCD_SetValueDirectly(DGUS_VP_Variable &var, void *val_ptr) {
    if (!var.memadr) return;
    union { unsigned char tmp[sizeof(T)]; T t; } x;
    unsigned char *ptr = (unsigned char*)val_ptr;
    LOOP_L_N(i, sizeof(T)) x.tmp[i] = ptr[sizeof(T) - i - 1];
    *(T*)var.memadr = x.t;
  }

  // Send a float value to the display.
  // Display will get a 4-byte integer scaled to the number of digits:
  // Tell the display the number of digits and it cheats by displaying a dot between...
  template<unsigned int decimals>
  static void DGUSLCD_SendFloatAsLongValueToDisplay(DGUS_VP_Variable &var) {
    if (var.memadr) {
      float f = *(float *)var.memadr;
      f *= cpow(10, decimals);
      dgusdisplay.WriteVariable(var.VP, (long)f);
    }
  }

  // Send a float value to the display.
  // Display will get a 2-byte integer scaled to the number of digits:
  // Tell the display the number of digits and it cheats by displaying a dot between...
  template<unsigned int decimals>
  static void DGUSLCD_SendFloatAsIntValueToDisplay(DGUS_VP_Variable &var) {
    if (var.memadr) {
      float f = *(float *)var.memadr;
      DEBUG_ECHOLNPGM(" >> ", f, 6);
      f *= cpow(10, decimals);
      dgusdisplay.WriteVariable(var.VP, (int16_t)f);
    }
  }

  // Force an update of all VP on the current screen.
  static void ForceCompleteUpdate() { update_ptr = 0; ScreenComplete = false; }
  // Has all VPs sent to the screen
  static bool IsScreenComplete() { return ScreenComplete; }

  static DGUSLCD_Screens getCurrentScreen() { return current_screen; }

  static void SetupConfirmAction( void (*f)()) { confirm_action_cb = f; }

private:
  static DGUSLCD_Screens current_screen;  //< currently on screen
  static constexpr uint8_t NUM_PAST_SCREENS = 4;
  static DGUSLCD_Screens past_screens[NUM_PAST_SCREENS]; //< LIFO with past screens for the "back" button.

  static uint8_t update_ptr;      //< Last sent entry in the VPList for the actual screen.
  static uint16_t skipVP;         //< When updating the screen data, skip this one, because the user is interacting with it.
  static bool ScreenComplete;     //< All VPs sent to screen?

  static uint16_t ConfirmVP;      //< context for confirm screen (VP that will be emulated-sent on "OK").

  #if ENABLED(SDSUPPORT)
    static int16_t top_file;      //< file on top of file chooser
    static int16_t file_to_print; //< touched file to be confirmed
  #endif

  static void (*confirm_action_cb)();
=======
  static bool loop();
>>>>>>> bugfix-2.1.x
};

enum MKS_Choose : uint8_t { MKS_Language_Choose, MKS_Language_NoChoose };
enum MKS_Language : uint8_t { MKS_SimpleChinese, MKS_English };

extern MKS_Language mks_language_index;
extern bool DGUSAutoTurnOff;

#if ENABLED(POWER_LOSS_RECOVERY)
  #define PLR_SCREEN_RECOVER MKSLCD_SCREEN_PRINT
  #define PLR_SCREEN_CANCEL MKSLCD_SCREEN_HOME
#endif

typedef DGUSScreenHandlerMKS DGUSScreenHandlerClass;
