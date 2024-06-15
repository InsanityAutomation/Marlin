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

/* ****************************************
 * lcd/extui/lib/Creality/Creality_DWIN.cpp
 * ****************************************
 * Extensible_UI implementation for Creality DWIN
 * 10SPro, Max, CRX, and others
 * Based original Creality release, ported to ExtUI for Marlin 2.0
 * Written by Insanity Automation, sponsored by Tiny Machines 3D
 *
 * ***************************************/


#include "Creality_DWIN.h"
#include "FileNavigator.h"
#include <HardwareSerial.h>
#include <WString.h>
#include <stdio.h>
#include "../ui_api.h"

#if ENABLED(EXTENSIBLE_UI)
namespace ExtUI
{
  static uint16_t fileIndex = 0;
  uint8_t recordcount = 0;
  uint8_t waitway_lock = 0;
  const float manual_feedrate_mm_m[] = MANUAL_FEEDRATE;
  uint8_t startprogress = 0;

  uint16_t rcvdData;

  uint8_t babystepIncrementIndex = 0;

  char waitway = 0;
  int recnum = 0;
  unsigned char Percentrecord = 0;
  float ChangeMaterialbuf[2] = {0};

  char NozzleTempStatus[3] = {0};

  char PrinterStatusKey[2] = {0}; // PrinterStatusKey[1] value: 0 represents to keep temperature, 1 represents  to heating , 2 stands for cooling , 3 stands for printing
                  // PrinterStatusKey[0] value: 0 reprensents 3D printer ready

  unsigned char AxisPagenum = 0; //0 for 10mm, 1 for 1mm, 2 for 0.1mm
  int16_t userConfValidation = 0;

  uint8_t lastPauseMsgState = 0;

  creality_dwin_settings_t Settings;
  uint8_t dwin_settings_version = 1;

  bool reEntryPrevent = false;
  uint8_t reEntryCount = 0;
  uint16_t idleThrottling = 0;

  bool pause_resume_selected = false;

  #if HAS_PID_HEATING
    uint16_t pid_hotendAutoTemp = 150;
    uint16_t pid_bedAutoTemp = 70;
  #endif

void onStartup()
{
	DWIN_SERIAL.begin(115200);
	rtscheck.recdat.head[0] = rtscheck.snddat.head[0] = FHONE;
	rtscheck.recdat.head[1] = rtscheck.snddat.head[1] = FHTWO;
	memset(rtscheck.databuf, 0, sizeof(rtscheck.databuf));

  #if ENABLED(DWINOS_4)
    #define DWIN_BOOTUP_DELAY 1500
  #else
    #define DWIN_BOOTUP_DELAY 500
  #endif
  delay_ms(DWIN_BOOTUP_DELAY); // Delay to allow screen startup
  SetTouchScreenConfiguration();
  rtscheck.RTS_SndData(StartSoundSet, SoundAddr);
  delay_ms(400); // Delay to allow screen to configure
  onStatusChanged(CUSTOM_MACHINE_NAME " Ready");

	rtscheck.RTS_SndData(100, FeedrateDisplay);

	/***************turn off motor*****************/
	rtscheck.RTS_SndData(11, FilenameIcon);

	/***************transmit temperature to screen*****************/
	rtscheck.RTS_SndData(0, NozzlePreheat);
	rtscheck.RTS_SndData(0, BedPreheat);
	rtscheck.RTS_SndData(getActualTemp_celsius(H0), NozzleTemp);
  #if HAS_MULTI_HOTEND
	  rtscheck.RTS_SndData(getActualTemp_celsius(H1), e2Temp);
  #else
    rtscheck.RTS_SndData(0, e2Temp);
  #endif
	rtscheck.RTS_SndData(getActualTemp_celsius(BED), Bedtemp);
	/***************transmit Fan speed to screen*****************/
  rtscheck.RTS_SndData(getActualFan_percent((fan_t)getActiveTool()), FanKeyIcon);


	/***************transmit Printer information to screen*****************/
	for (int j = 0; j < 20; j++) //clean filename
		rtscheck.RTS_SndData(0, MacVersion + j);
	char sizebuf[20] = {0};
	sprintf(sizebuf, "%d X %d X %d", Y_BED_SIZE, X_BED_SIZE, Z_MAX_POS);
  #if defined(CUSTOM_MACHINE_NAME)
	  rtscheck.RTS_SndData(CUSTOM_MACHINE_NAME, MacVersion);
  #else
    rtscheck.RTS_SndData(MACHINE_NAME, MacVersion);
  #endif
	rtscheck.RTS_SndData(SHORT_BUILD_VERSION, SoftVersion);
	rtscheck.RTS_SndData(sizebuf, PrinterSize);
	rtscheck.RTS_SndData(WEBSITE_URL, CorpWebsite);

	/**************************some info init*******************************/
	rtscheck.RTS_SndData(0, PrintscheduleIcon);
	rtscheck.RTS_SndData(0, PrintscheduleIcon + 1);

	/************************clean screen*******************************/
	for (int i = 0; i < MaxFileNumber; i++)
	{
		for (int j = 0; j < 10; j++)
			rtscheck.RTS_SndData(0, SDFILE_ADDR + i * 10 + j);
	}

	for (int j = 0; j < 10; j++)
	{
		rtscheck.RTS_SndData(0, Printfilename + j);  //clean screen.
		rtscheck.RTS_SndData(0, Choosefilename + j); //clean filename
	}
	for (int j = 0; j < 8; j++)
		rtscheck.RTS_SndData(0, FilenameCount + j);
	for (int j = 1; j <= MaxFileNumber; j++)
	{
		rtscheck.RTS_SndData(10, FilenameIcon + j);
		rtscheck.RTS_SndData(10, FilenameIcon1 + j);
	}

	SERIAL_ECHOLNPGM_P(PSTR("==Dwin Init Complete=="));
}

void onIdle()
{

  int dataRec;
   do { dataRec = rtscheck.RTS_RecData(); rcvdData++;} while (dataRec > 0);

  if (reEntryPrevent && reEntryCount < 120) {
    reEntryCount++;
    return;
  }
  reEntryCount = 0;

  idleThrottling++;

  if(idleThrottling == 1) {
    rtscheck.RTS_SndData(getActualTemp_celsius(H0), NozzleTemp);
    rtscheck.RTS_SndData(getActualTemp_celsius(BED), Bedtemp);
    rtscheck.RTS_SndData(getTargetTemp_celsius(H0), NozzlePreheat);
    rtscheck.RTS_SndData(getTargetTemp_celsius(BED), BedPreheat);
  }
  if(idleThrottling == 100) {
#if HAS_MULTI_HOTEND
    rtscheck.RTS_SndData(getActualTemp_celsius(H1), e2Temp);
    rtscheck.RTS_SndData(getTargetTemp_celsius(H1), e2Preheat);

    rtscheck.RTS_SndData(((uint8_t)getActiveTool() + 1), ActiveToolVP);
#else
    rtscheck.RTS_SndData(0, e2Temp);
    rtscheck.RTS_SndData(0, e2Preheat);
#endif
  }

if(idleThrottling == 200) {
  if(awaitingUserConfirm() && (lastPauseMsgState!=ExtUI::pauseModeStatus || userConfValidation > 99))
  {
    switch(ExtUI::pauseModeStatus)
      {
      case PAUSE_MESSAGE_PARKING:  ExtUI::onUserConfirmRequired(GET_TEXT_F(MSG_PAUSE_PRINT_PARKING)); break;
      case PAUSE_MESSAGE_CHANGING: ExtUI::onUserConfirmRequired(GET_TEXT_F(MSG_FILAMENT_CHANGE_INIT)); break;
      case PAUSE_MESSAGE_UNLOAD:   ExtUI::onUserConfirmRequired(GET_TEXT_F(MSG_FILAMENT_CHANGE_UNLOAD)); break;
      case PAUSE_MESSAGE_WAITING:  ExtUI::onUserConfirmRequired(GET_TEXT_F(MSG_ADVANCED_PAUSE_WAITING)); break;
      case PAUSE_MESSAGE_INSERT:   ExtUI::onUserConfirmRequired(GET_TEXT_F(MSG_FILAMENT_CHANGE_INSERT)); break;
      case PAUSE_MESSAGE_LOAD:     ExtUI::onUserConfirmRequired(GET_TEXT_F(MSG_FILAMENT_CHANGE_LOAD)); break;
      case PAUSE_MESSAGE_PURGE:
        #if ENABLED(ADVANCED_PAUSE_CONTINUOUS_PURGE)
          ExtUI::onUserConfirmRequired(GET_TEXT_F(MSG_FILAMENT_CHANGE_CONT_PURGE)); break;
        #else
          ExtUI::onUserConfirmRequired(GET_TEXT_F(MSG_FILAMENT_CHANGE_PURGE)); break;
        #endif
      case PAUSE_MESSAGE_RESUME:   ExtUI::onUserConfirmRequired(GET_TEXT_F(MSG_FILAMENT_CHANGE_RESUME)); break;
      case PAUSE_MESSAGE_HEAT:     ExtUI::onUserConfirmRequired(GET_TEXT_F(MSG_FILAMENT_CHANGE_HEAT)); break;
      case PAUSE_MESSAGE_HEATING:  ExtUI::onUserConfirmRequired(GET_TEXT_F(MSG_FILAMENT_CHANGE_HEATING)); break;
      case PAUSE_MESSAGE_OPTION:   ExtUI::onUserConfirmRequired(GET_TEXT_F(MSG_FILAMENT_CHANGE_OPTION_HEADER)); break;
      case PAUSE_MESSAGE_STATUS: SERIAL_ECHOLNPGM_P(PSTR("PauseStatus")); break;
      default: onUserConfirmRequired(PSTR("Confirm Continue")); break;
    }
    userConfValidation = 0;

  } else if (pause_resume_selected && !awaitingUserConfirm()) {
      rtscheck.RTS_SndData(ExchangePageBase + 53, ExchangepageAddr);
      pause_resume_selected = false;
      userConfValidation = 0;
  }
  else if (awaitingUserConfirm() && !pause_resume_selected)
  {
    userConfValidation++;
  }
  else if (awaitingUserConfirm() && pause_resume_selected)
  {
    pause_resume_selected = false;
    userConfValidation = 100;
  }

}

  reEntryPrevent = true;
if(idleThrottling == 300) {
  if(waitway && !commandsInQueue())
    waitway_lock++;
  else
    waitway_lock = 0;

  if(waitway_lock > 100) {
    waitway_lock = 0;
    waitway = 0; //clear waitway if nothing is going on
  }

	switch (waitway)
	{
		case 1:
      if(isPositionKnown()) {
        SERIAL_ECHOLNPGM_P(PSTR("==waitway 1=="));
        rtscheck.RTS_SndData(ExchangePageBase + 54, ExchangepageAddr);
        waitway = 0;
      }
			break;

		case 2:
      if (isPositionKnown() && !commandsInQueue())
			  waitway = 0;
			break;

		case 3:
      SERIAL_ECHOLNPGM_P(PSTR("==waitway 3=="));
      //if(isPositionKnown() && (getActualTemp_celsius(BED) >= (getTargetTemp_celsius(BED)-1))) {
			  rtscheck.RTS_SndData(ExchangePageBase + 64, ExchangepageAddr);
        waitway = 7;
        //return;
      //}
			break;

		case 4:
			waitway = 0;
			break;
		case 5:
        if(isPositionKnown() && !commandsInQueue()) {
        waitway = 0;
        SERIAL_ECHOLNPGM_P(PSTR("==waitway 5=="));
        rtscheck.RTS_SndData(ExchangePageBase + 78, ExchangepageAddr); //exchange to 78 page
      }
      break;
    case 6:
      if(!commandsInQueue()) {
        setAxisPosition_mm(BED_TRAMMING_HEIGHT, (axis_t)Z);
        waitway = 0;
      }
      break;
    case 7:
      if(!commandsInQueue())
        waitway = 0;
      break;
		}
}
  void yield();

if(idleThrottling == 400) {
  #if HAS_MESH
    if (getLevelingActive())
      rtscheck.RTS_SndData(3, AutoLevelIcon); /*On*/
    else
      rtscheck.RTS_SndData(2, AutoLevelIcon); /*Off*/
  #endif

  #if HAS_FILAMENT_SENSOR
    if(getRunoutMode(getActiveTool()) == 0)
      rtscheck.RTS_SndData(26, RunoutMode);
    if(getRunoutMode(getActiveTool()) == 1)
      rtscheck.RTS_SndData(27, RunoutMode);
    if(getRunoutMode(getActiveTool()) == 2)
      rtscheck.RTS_SndData(28, RunoutMode);
    if(getRunoutMode(getActiveTool()) == 7)
      rtscheck.RTS_SndData(25, RunoutMode);
  #endif

  switch(babystepIncrementIndex)
  {
    case 1:
    {
      rtscheck.RTS_SndData(12, BabystepIncrement);
      break;
    }
    case 2:
    {
      rtscheck.RTS_SndData(13, BabystepIncrement);
      break;
    }
    case 3:
    {
      rtscheck.RTS_SndData(14, BabystepIncrement);
      break;
    }
    case 4:
    {
      rtscheck.RTS_SndData(15, BabystepIncrement);
      break;
    }
    case 5:
    {
      rtscheck.RTS_SndData(16, BabystepIncrement);
      break;
    }
    case 6:
    {
      rtscheck.RTS_SndData(17, BabystepIncrement);
      break;
    }
    case 7:
    {
      rtscheck.RTS_SndData(18, BabystepIncrement);
      break;
    }
    default :
    {
      rtscheck.RTS_SndData(11, BabystepIncrement);
      break;
    }
  }
}
  if(idleThrottling == 500) {

  #if ENABLED(CASE_LIGHT_ENABLE)
    if(getCaseLightState())
      rtscheck.RTS_SndData(3, LedToggle); /*On*/
    else
      rtscheck.RTS_SndData(2, LedToggle); /*Off*/
  #endif

  #if ENABLED(POWER_LOSS_RECOVERY)
    if(getPowerLossRecoveryEnabled())
      rtscheck.RTS_SndData(3, PowerLossToggle); /*On*/
    else
      rtscheck.RTS_SndData(2, PowerLossToggle); /*Off*/
  #endif
}

  if (startprogress == 0)
  {
    startprogress += 25;
          delay_ms(3000); // Delay to show bootscreen
  }
  else if( startprogress < 250)
  {
    //if(isMediaInserted()) //Re init media as it happens too early on STM32 boards often
    //  onMediaInserted();
    //else
    //  injectCommands_P(PSTR("M22\nM21"));
    startprogress = 254;
    //SERIAL_ECHOLNPGM_P(PSTR("  startprogress "));
    rtscheck.RTS_SndData(ExchangePageBase + 45, ExchangepageAddr);
    reEntryPrevent = false;
		return;
    onStatusChanged("Ready");
  }
  if (startprogress <= 100)
    rtscheck.RTS_SndData(startprogress, StartIcon);
  else
  {
    rtscheck.RTS_SndData((startprogress - 100), StartIcon + 1);
  }

  //rtscheck.RTS_SndData((startprogress++) % 5, ExchFlmntIcon);

  if (isPrinting())
	{
    if(idleThrottling == 600) {
    rtscheck.RTS_SndData(getActualFan_percent((fan_t)getActiveTool()), FanKeyIcon);
		rtscheck.RTS_SndData(getProgress_seconds_elapsed() / 3600, Timehour);
		rtscheck.RTS_SndData((getProgress_seconds_elapsed() % 3600) / 60, Timemin);
		if (getProgress_percent() > 0)
		{
			Percentrecord = getProgress_percent() + 1;
			if (Percentrecord <= 50)
			{
				rtscheck.RTS_SndData((unsigned int)Percentrecord * 2, PrintscheduleIcon);
				rtscheck.RTS_SndData(0, PrintscheduleIcon + 1);
			}
			else
	  	{
				rtscheck.RTS_SndData(100, PrintscheduleIcon);
				rtscheck.RTS_SndData((unsigned int)Percentrecord * 2 - 100, PrintscheduleIcon + 1);
			}
		}
		else
		{
			rtscheck.RTS_SndData(0, PrintscheduleIcon);
			rtscheck.RTS_SndData(0, PrintscheduleIcon + 1);
		}
		rtscheck.RTS_SndData((unsigned int)getProgress_percent(), Percentage);
    }
	}
  else { // Not printing settings
  if(idleThrottling == 700) {
    rtscheck.RTS_SndData(map(constrain(Settings.display_volume, 0, 255), 0, 255, 0, 100), VolumeDisplay);
    rtscheck.RTS_SndData(Settings.screen_brightness, DisplayBrightness);
    rtscheck.RTS_SndData(Settings.standby_screen_brightness, DisplayStandbyBrightness);
    rtscheck.RTS_SndData(Settings.standby_time_seconds, DisplayStandbySeconds);
    if(Settings.display_standby)
      rtscheck.RTS_SndData(3, DisplayStandbyEnableIndicator);
    else
      rtscheck.RTS_SndData(2, DisplayStandbyEnableIndicator);
  }
  if(idleThrottling == 800) {
    rtscheck.RTS_SndData((unsigned int)(getAxisSteps_per_mm(X) * 10), StepMM_X);
    rtscheck.RTS_SndData((unsigned int)(getAxisSteps_per_mm(Y) * 10), StepMM_Y);
    rtscheck.RTS_SndData((unsigned int)(getAxisSteps_per_mm(Z) * 10), StepMM_Z);
    rtscheck.RTS_SndData((unsigned int)(getAxisSteps_per_mm(E0) * 10), StepMM_E);
  }
  if(idleThrottling == 900) {
    rtscheck.RTS_SndData((unsigned int)(getAxisMaxAcceleration_mm_s2(X)/100), Accel_X);
    rtscheck.RTS_SndData((unsigned int)(getAxisMaxAcceleration_mm_s2(Y)/100), Accel_Y);
    rtscheck.RTS_SndData((unsigned int)(getAxisMaxAcceleration_mm_s2(Z)/10), Accel_Z);
    rtscheck.RTS_SndData((unsigned int)(getAxisMaxAcceleration_mm_s2(E0)), Accel_E);
  }
  if(idleThrottling == 1000) {
    rtscheck.RTS_SndData((unsigned int)(getAxisMaxFeedrate_mm_s(X)), Feed_X);
    rtscheck.RTS_SndData((unsigned int)(getAxisMaxFeedrate_mm_s(Y)), Feed_Y);
    rtscheck.RTS_SndData((unsigned int)(getAxisMaxFeedrate_mm_s(Z)), Feed_Z);
    rtscheck.RTS_SndData((unsigned int)(getAxisMaxFeedrate_mm_s(E0)), Feed_E);
  }
  if(idleThrottling == 1100) {
    rtscheck.RTS_SndData((unsigned int)(getAxisMaxJerk_mm_s(X)*100), Jerk_X);
    rtscheck.RTS_SndData((unsigned int)(getAxisMaxJerk_mm_s(Y)*100), Jerk_Y);
    rtscheck.RTS_SndData((unsigned int)(getAxisMaxJerk_mm_s(Z)*100), Jerk_Z);
    rtscheck.RTS_SndData((unsigned int)(getAxisMaxJerk_mm_s(E0)*100), Jerk_E);
  }
    #if HAS_HOTEND_OFFSET
    if(idleThrottling == 1200) {
      rtscheck.WriteVariable(T2Offset_X, (uint32_t)(getNozzleOffset_mm(X, E1)*1000));
      rtscheck.WriteVariable(T2Offset_Y, (uint32_t)(getNozzleOffset_mm(Y, E1)*1000));
      rtscheck.WriteVariable(T2Offset_Z, (uint32_t)(getNozzleOffset_mm(Z, E1)*1000));
      rtscheck.RTS_SndData((unsigned int)(getAxisSteps_per_mm(E1) * 10), T2StepMM_E);
    }
    #endif

if(idleThrottling == 1300) {
    #if HAS_BED_PROBE
      rtscheck.RTS_SndData(getProbeOffset_mm(X) * 100, ProbeOffset_X);
      rtscheck.RTS_SndData(getProbeOffset_mm(Y) * 100, ProbeOffset_Y);
    #endif
}
    #if HAS_PID_HEATING
    if(idleThrottling == 1400) {
      rtscheck.RTS_SndData(pid_hotendAutoTemp, HotendPID_AutoTmp);
      rtscheck.RTS_SndData(pid_bedAutoTemp, BedPID_AutoTmp);
      rtscheck.RTS_SndData((unsigned int)(getPID_Kp(E0) * 10), HotendPID_P);
      rtscheck.RTS_SndData((unsigned int)(getPID_Ki(E0) * 10), HotendPID_I);
    }
    if(idleThrottling == 1500) {
      rtscheck.RTS_SndData((unsigned int)(getPID_Kd(E0) * 10), HotendPID_D);
      #if ENABLED(PIDTEMPBED)
        rtscheck.RTS_SndData((unsigned int)(getBedPID_Kp() * 10), BedPID_P);
        rtscheck.RTS_SndData((unsigned int)(getBedPID_Ki() * 10), BedPID_I);
        rtscheck.RTS_SndData((unsigned int)(getBedPID_Kd() * 10), BedPID_D);
      #endif
    }
    #endif

if(idleThrottling == 1600) {
    #if HAS_SHAPING
      rtscheck.RTS_SndData((unsigned int)(getShapingZeta(X) * 1000), ShapingZetaX);
      rtscheck.RTS_SndData((unsigned int)(getShapingZeta(Y) * 1000), ShapingZetaY);
      rtscheck.RTS_SndData((unsigned int)(getShapingFrequency(X) * 100), ShapingFreqX);
      rtscheck.RTS_SndData((unsigned int)(getShapingFrequency(Y) * 100), ShapingFreqY);
    #endif

    #if ENABLED(LIN_ADVANCE)
      rtscheck.RTS_SndData((unsigned int)(getLinearAdvance_mm_mm_s(getActiveTool()) * 1000), LinAdvKFactor);
    #endif
}
  }

if(idleThrottling == 1700) {
	rtscheck.RTS_SndData(getZOffset_mm() * 100, ProbeOffset_Z);
	rtscheck.RTS_SndData((unsigned int)(getFlow_percent(E0)), Flowrate);


	if (NozzleTempStatus[0] || NozzleTempStatus[2]) //statuse of loadfilement and unloadfinement when temperature is less than
	{
		if (getActualTemp_celsius(getActiveTool()) > EXTRUDE_MINTEMP && NozzleTempStatus[0]!=0)
		{
			NozzleTempStatus[0] = 0;
			rtscheck.RTS_SndData(10 * ChangeMaterialbuf[0], FilementUnit1);
			rtscheck.RTS_SndData(10 * ChangeMaterialbuf[1], FilementUnit2);
      //SERIAL_ECHOLNPGM_P(PSTR("==Heating Done Change Filament=="));
			rtscheck.RTS_SndData(ExchangePageBase + 65, ExchangepageAddr);
		}
		else if (getActualTemp_celsius(getActiveTool()) >= getTargetTemp_celsius(getActiveTool()) && NozzleTempStatus[2])
		{
			//SERIAL_ECHOLNPGM("***NozzleTempStatus[2] =", (int)NozzleTempStatus[2]);
			NozzleTempStatus[2] = 0;
			rtscheck.RTS_SndData(4, ExchFlmntIcon);
			rtscheck.RTS_SndData(ExchangePageBase + 83, ExchangepageAddr);
		}
	}
}

if(idleThrottling == 1800) {
  if(rtscheck.recdat.addr != DisplayZaxis && rtscheck.recdat.addr != DisplayYaxis && rtscheck.recdat.addr != DisplayZaxis) {
		rtscheck.RTS_SndData(10 * getAxisPosition_mm((axis_t)X), DisplayXaxis);
	  rtscheck.RTS_SndData(10 * getAxisPosition_mm((axis_t)Y), DisplayYaxis);
		rtscheck.RTS_SndData(10 * getAxisPosition_mm((axis_t)Z), DisplayZaxis);
  }
}

if(idleThrottling == 1900) {
  if(isMediaMounted())
  {
    uint16_t currPage, maxPageAdd;
    if(fileIndex == 0)
      currPage = 1;
    else
      currPage = CEIL((float)((float)fileIndex / (float)DISPLAY_FILES)) +1;

    if(filenavigator.folderdepth!=0)
      maxPageAdd = 1;
    else
      maxPageAdd = 0;
    uint16_t maxPages = CEIL((float)(filenavigator.maxFiles()+ maxPageAdd) / (float)DISPLAY_FILES );

    rtscheck.RTS_SndData(currPage, FilesCurentPage);
    rtscheck.RTS_SndData(maxPages, FilesMaxPage);
  }
  else
  {
    rtscheck.RTS_SndData(0, FilesCurentPage);
    rtscheck.RTS_SndData(0, FilesMaxPage);
  }
}

  void yield();

  #if ENABLED(DWINOS_4)
    int dataRec2;
    do { dataRec2 = rtscheck.RTS_RecData(); rcvdData++;} while (dataRec2 > 0); // Since OS4 returns an ack on an 82 command, receive and purge it now
  #endif

if(idleThrottling == 2000) {
  if(rcvdData < 4010)
  {
    SERIAL_ECHOLNPGM("RX Halt Detected...");
    RTSSHOW::rx_datagram_state = DGUS_IDLE;
    DWIN_SERIAL.flush();
  }
  idleThrottling = 0;
}

  reEntryPrevent = false;
}


rx_datagram_state_t RTSSHOW::rx_datagram_state = DGUS_IDLE;
uint8_t RTSSHOW::rx_datagram_len = 0;
bool RTSSHOW::Initialized = false;
RTSSHOW::RTSSHOW()
{
	recdat.head[0] = snddat.head[0] = FHONE;
	recdat.head[1] = snddat.head[1] = FHTWO;
	memset(databuf, 0, sizeof(databuf));
}

int RTSSHOW::RTS_RecData()
{
  //if(!isPrinting())
    //SERIAL_ECHOLNPGM("Receiving...");
  uint8_t receivedbyte;
  if(!DWIN_SERIAL.connected())
    DWIN_SERIAL.begin(115200);

  //#if ENABLED(DGUS_SERIAL_STATS_RX_BUFFER_OVERRUNS)
    if ((!DWIN_SERIAL.available() && DWIN_SERIAL.buffer_overruns()) || DWIN_SERIAL.dropped() > 0) {
      // Overrun, but reset the flag only when the buffer is empty
      // We want to extract as many as valid datagrams possible...
      SERIAL_ECHOLNPGM("OVFL");
      rx_datagram_state = DGUS_IDLE;
      //DWIN_SERIAL.reset_rx_overun();
      DWIN_SERIAL.flush();
    }
  //#endif

  while (DWIN_SERIAL.available()) {
    switch (rx_datagram_state) {

      case DGUS_IDLE: // Waiting for the first header byte
        receivedbyte = DWIN_SERIAL.read();
        //SERIAL_ECHOLNPGM("< ",receivedbyte);
        if (FHONE == receivedbyte) rx_datagram_state = DGUS_HEADER1_SEEN;
        break;

      case DGUS_HEADER1_SEEN: // Waiting for the second header byte
        receivedbyte = DWIN_SERIAL.read();
        //SERIAL_ECHOLNPGM(" ", receivedbyte);
        rx_datagram_state = (FHTWO == receivedbyte) ? DGUS_HEADER2_SEEN : DGUS_IDLE;
        break;

      case DGUS_HEADER2_SEEN: // Waiting for the length byte
        rx_datagram_len = DWIN_SERIAL.read();
        //DEBUGLCDCOMM_ECHOPAIR(" (", rx_datagram_len, ") ");

        // Telegram min len is 3 (command and one word of payload)
        rx_datagram_state = WITHIN(rx_datagram_len, 3, DGUS_RX_BUFFER_SIZE) ? DGUS_WAIT_TELEGRAM : DGUS_IDLE;
        break;

      case DGUS_WAIT_TELEGRAM: // wait for complete datagram to arrive.
        if (DWIN_SERIAL.available() < rx_datagram_len) return -1;

        Initialized = true; // We've talked to it, so we defined it as initialized.
        uint8_t command = DWIN_SERIAL.read();

       // DEBUGLCDCOMM_ECHOPAIR("# ", command);

        uint8_t readlen = rx_datagram_len - 1;  // command is part of len.
        unsigned char tmp[rx_datagram_len - 1];
        unsigned char *ptmp = tmp;
        while (readlen--) {
          receivedbyte = DWIN_SERIAL.read();
          //DEBUGLCDCOMM_ECHOPAIR(" ", receivedbyte);
          *ptmp++ = receivedbyte;
        }
        //DEBUGLCDCOMM_ECHOPGM(" # ");
        // mostly we'll get this: 5A A5 03 82 4F 4B -- ACK on 0x82, so discard it.
        if (command == VarAddr_W && 'O' == tmp[0] && 'K' == tmp[1]) {
          //DEBUG_ECHOLNPGM(">");
          rx_datagram_state = DGUS_IDLE;
          break;
        }

        /* AutoUpload, (and answer to) Command 0x83 :
        |      tmp[0  1  2  3  4 ... ]
        | Example 5A A5 06 83 20 01 01 78 01 ……
        |          / /  |  |   \ /   |  \     \
        |        Header |  |    |    |   \_____\_ DATA (Words!)
        |     DatagramLen  /  VPAdr  |
        |           Command          DataLen (in Words) */
        if (command == VarAddr_R) {
          const uint16_t vp = tmp[0] << 8 | tmp[1];

         const uint8_t dlen = tmp[2] << 1;  // Convert to Bytes. (Display works with words)
          SERIAL_ECHOLNPGM(" vp=", vp, " dlen=", dlen);
          recdat.addr = vp;
          recdat.len = tmp[2];
          for(unsigned int i = 0;i < dlen; i+=2)
          {
            recdat.data[i/2]= tmp[3+i];
            recdat.data[i/2]= (recdat.data[i/2] << 8 )| tmp[4+i];
          }

          SERIAL_ECHOLNPGM("VP received: ", vp , " - len ", tmp[2]);

          SERIAL_ECHOLNPGM("d1: ", tmp[3] , " - d2 ", tmp[4]);
          SERIAL_ECHOLNPGM("d3: ", tmp[5] , " - d4 ", tmp[6]);

          rx_datagram_state = DGUS_IDLE;
          RTS_HandleData();
          return 2;
          break;
        }

      // discard anything else
      rx_datagram_state = DGUS_IDLE;
      //if(!isPrinting())
        SERIAL_ECHOLNPGM("Discard Return...");
      return -1;
    }
    //if(!isPrinting())
      //SERIAL_ECHOLNPGM("No Case Match...");
  }
  //if(!isPrinting())
    //SERIAL_ECHOLNPGM("No Data...");
  return -1;
}

void RTSSHOW::RTS_SndData(void)
{
	if ((snddat.head[0] == FHONE) && (snddat.head[1] == FHTWO) && snddat.len >= 3)
	{
		databuf[0] = snddat.head[0];
		databuf[1] = snddat.head[1];
		databuf[2] = snddat.len;
		databuf[3] = snddat.command;
		if (snddat.command == 0x80) //to write data to the register
		{
			databuf[4] = snddat.addr;
			for (int i = 0; i < (snddat.len - 2); i++)
				databuf[5 + i] = snddat.data[i];
		}
		else if (snddat.len == 3 && (snddat.command == 0x81)) //to read data from the register
		{
			databuf[4] = snddat.addr;
			databuf[5] = snddat.bytelen;
		}
		else if (snddat.command == 0x82) //to write data to the variate
		{
			databuf[4] = snddat.addr >> 8;
			databuf[5] = snddat.addr & 0xFF;
			for (int i = 0; i < (snddat.len - 3); i += 2)
			{
				databuf[6 + i] = snddat.data[i / 2] >> 8;
				databuf[7 + i] = snddat.data[i / 2] & 0xFF;
			}
		}
		else if (snddat.len == 4 && (snddat.command == 0x83)) //to read data from the variate
		{
			databuf[4] = snddat.addr >> 8;
			databuf[5] = snddat.addr & 0xFF;
			databuf[6] = snddat.bytelen;
		}
		for (int i = 0; i < (snddat.len + 3); i++)
		{
			DWIN_SERIAL.write(databuf[i]);
			delay_us(1);
		}

		memset(&snddat, 0, sizeof(snddat));
		memset(databuf, 0, sizeof(databuf));
		snddat.head[0] = FHONE;
		snddat.head[1] = FHTWO;
	}
}

void RTSSHOW::RTS_SndData(const String &s, unsigned long addr, unsigned char cmd /*= VarAddr_W*/)
{
	if (s.length() < 1)
		return;
	RTS_SndData(s.c_str(), addr, cmd);
}

void RTSSHOW::RTS_SndData(const char *str, unsigned long addr, unsigned char cmd /*= VarAddr_W*/)
{

	int len = strlen(str);
  constexpr int maxlen = SizeofDatabuf - 6;
	if (len > 0)
	{
    if(len>maxlen) len = maxlen;
		databuf[0] = FHONE;
		databuf[1] = FHTWO;
		databuf[2] = 3 + len;
		databuf[3] = cmd;
		databuf[4] = addr >> 8;
		databuf[5] = addr & 0x00FF;
		for (int i = 0; i < len; i++)
			databuf[6 + i] = str[i];

		for (int i = 0; i < (len + 6); i++)
		{
			DWIN_SERIAL.write(databuf[i]);
			delay_us(1);
		}

    uint8_t expected_tx = 6 + len; // 6 bytes header + payload.
    const millis_t try_until = ExtUI::safe_millis() + 1000;

    while (expected_tx > LCD_SERIAL_TX_BUFFER_FREE()) {
      if (ELAPSED(ExtUI::safe_millis(), try_until)) return; // Stop trying after 1 second

      #ifdef ARDUINO_ARCH_STM32
        DWIN_SERIAL.flush();
      #else
        DWIN_SERIAL.flushTX();
      #endif
      delay(50);
    }

    int dataRec2;
    do { dataRec2 = rtscheck.RTS_RecData(); } while (dataRec2 > 0); // Since OS4 returns an ack on an 82 command, receive and purge it now
		memset(databuf, 0, sizeof(databuf));
	}
}

void RTSSHOW::RTS_SndData(char c, unsigned long addr, unsigned char cmd /*= VarAddr_W*/)
{
	snddat.command = cmd;
	snddat.addr = addr;
	snddat.data[0] = (unsigned long)c;
	snddat.data[0] = snddat.data[0] << 8;
	snddat.len = 5;
	RTS_SndData();
}

void RTSSHOW::RTS_SndData(unsigned char *str, unsigned long addr, unsigned char cmd) { RTS_SndData((char *)str, addr, cmd); }

void RTSSHOW::RTS_SndData(int n, unsigned long addr, unsigned char cmd /*= VarAddr_W*/)
{
	if (cmd == VarAddr_W)
	{
    if ((uint8_t)n > 0xFFFF)
		{
			snddat.data[0] = n >> 16;
			snddat.data[1] = n & 0xFFFF;
			snddat.len = 7;
		}
		else
		{
			snddat.data[0] = n;
			snddat.len = 5;
		}
	}
	else if (cmd == RegAddr_W)
	{
		snddat.data[0] = n;
		snddat.len = 3;
	}
	else if (cmd == VarAddr_R)
	{
		snddat.bytelen = n;
		snddat.len = 4;
	}
	snddat.command = cmd;
	snddat.addr = addr;
	RTS_SndData();
}

void RTSSHOW::RTS_SndData(unsigned int n, unsigned long addr, unsigned char cmd) { RTS_SndData((int)n, addr, cmd); }

void RTSSHOW::RTS_SndData(float n, unsigned long addr, unsigned char cmd) { RTS_SndData((int)n, addr, cmd); }

void RTSSHOW::RTS_SndData(long n, unsigned long addr, unsigned char cmd) { RTS_SndData((unsigned long)n, addr, cmd); }

void RTSSHOW::RTS_SndData(unsigned long n, unsigned long addr, unsigned char cmd /*= VarAddr_W*/)
{
	if (cmd == VarAddr_W)
	{
    if (n > 0xFFFF)
		{
      snddat.data[0] = n >> 16;
      snddat.data[1] = n & 0xFFFF;

		  //snddat.data[0] = n >> 24;
			//snddat.data[1] = n >> 16;
			//snddat.data[2] = n >> 8;
			//snddat.data[3] = n;
			snddat.len = 7;
    }
    else
		{
			snddat.data[0] = n;
			snddat.len = 5;
		}
	}
	else if (cmd == VarAddr_R)
	{
		snddat.bytelen = n;
		snddat.len = 4;
	}
	snddat.command = cmd;
	snddat.addr = addr;
	RTS_SndData();
}

void RTSSHOW::WriteVariable(uint16_t adr, long value) {
  union { long l; char lb[4]; } endian;
  char tmp[4];
  endian.l = value;
  tmp[0] = endian.lb[3];
  tmp[1] = endian.lb[2];
  tmp[2] = endian.lb[1];
  tmp[3] = endian.lb[0];
  WriteVariable(adr, static_cast<const void*>(&tmp), sizeof(long));
}

void RTSSHOW::WriteVariable(uint16_t adr, const void* values, uint8_t valueslen, bool isstr, char fillChar) {
  const char* myvalues = static_cast<const char*>(values);
  bool strend = !myvalues;
  DWIN_SERIAL.write(FHONE);
  DWIN_SERIAL.write(FHTWO);
  DWIN_SERIAL.write(valueslen + 3);
  DWIN_SERIAL.write(0x82);
  DWIN_SERIAL.write(adr >> 8);
  DWIN_SERIAL.write(adr & 0xFF);
  while (valueslen--) {
    char x;
    if (!strend) x = *myvalues++;
    if ((isstr && !x) || strend) {
      strend = true;
      x = fillChar;
    }
    DWIN_SERIAL.write(x);
  }

  uint8_t expected_tx = 6 + valueslen; // 6 bytes header + payload.
    const millis_t try_until = ExtUI::safe_millis() + 1000;

    while (expected_tx > LCD_SERIAL_TX_BUFFER_FREE()) {
      if (ELAPSED(ExtUI::safe_millis(), try_until)) return; // Stop trying after 1 second

      #ifdef ARDUINO_ARCH_STM32
        DWIN_SERIAL.flush();
      #else
        DWIN_SERIAL.flushTX();
      #endif
      delay(50);
    }

  int dataRec2;
  do { dataRec2 = rtscheck.RTS_RecData(); } while (dataRec2 > 0); // Since OS4 returns an ack on an 82 command, receive and purge it now
}

void RTSSHOW::RTS_HandleData()
{
	int Checkkey = -1;
	//SERIAL_ECHOLNPGM_P(PSTR("  *******RTS_HandleData******** "));
	if (waitway > 0) //for waiting
	{
		SERIAL_ECHOLNPGM("handle waitway ==", (int)waitway);
		memset(&recdat, 0, sizeof(recdat));
		recdat.head[0] = FHONE;
		recdat.head[1] = FHTWO;
		return;
	}
	//SERIAL_ECHOLNPGM("recdat.data[0] ==", recdat.data[0]);
	//SERIAL_ECHOLNPGM("recdat.addr ==", recdat.addr);
	for (int i = 0; Addrbuf[i] != 0; i++)
	{
		if (recdat.addr == Addrbuf[i])
		{
      if (Addrbuf[i] == NzBdSet || Addrbuf[i] == NozzlePreheat || Addrbuf[i] == BedPreheat || Addrbuf[i] == Flowrate)
				Checkkey = ManualSetTemp;
			else if (Addrbuf[i] >= Stopprint && Addrbuf[i] <= Resumeprint)
				Checkkey = PrintChoice;
			else if (Addrbuf[i] >= AutoZero && Addrbuf[i] <= DisplayZaxis)
				Checkkey = XYZEaxis;
			else if (Addrbuf[i] >= FilementUnit1 && Addrbuf[i] <= FilementUnit2)
				Checkkey = Filement;
			else
				Checkkey = i;
			break;
		}
	}

  switch(recdat.addr) {
    case ProbeOffset_Z :
      {
        Checkkey = Zoffset_Value;
        break;
      }
    case Flowrate :
    case StepMM_X :
    case StepMM_Y :
    case StepMM_Z :
    case StepMM_E :
    case ProbeOffset_X :
    case ProbeOffset_Y :
    case HotendPID_AutoTmp :
    case BedPID_AutoTmp :
    case HotendPID_P :
    case HotendPID_I :
    case HotendPID_D :
    case BedPID_P :
    case BedPID_I :
    case BedPID_D :
    case T2StepMM_E:
    case Accel_X:
    case Accel_Y:
    case Accel_Z:
    case Accel_E:
    case Feed_X:
    case Feed_Y:
    case Feed_Z:
    case Feed_E:
    case Jerk_X:
    case Jerk_Y:
    case Jerk_Z:
    case Jerk_E:
    case ShapingZetaX:
    case ShapingZetaY:
    case ShapingFreqX:
    case ShapingFreqY:
    case LinAdvKFactor:
    case RunoutToggle:
    case RunoutMode:
    case BabystepIncrement:
    case PowerLossToggle:
    case FanKeyIcon:
    case LedToggle:
    case e2Temp:
      Checkkey = ManualSetTemp;
    break;
  }

  if(Checkkey == ManualSetTemp)
    Checkkey = ManualSetTemp;
  else if(recdat.addr == VolumeDisplay)
    Checkkey = VolumeDisplay;
  else if(recdat.addr == T2Offset_X || recdat.addr == T2Offset_Y || recdat.addr == T2Offset_Z)
    Checkkey = Idex_Settings;
  else if(recdat.addr == DisplayBrightness)
    Checkkey = DisplayBrightness;
  else if(recdat.addr == DisplayStandbyBrightness)
    Checkkey = DisplayStandbyBrightness;
  else if(recdat.addr == DisplayStandbySeconds)
    Checkkey = DisplayStandbySeconds;
  else if(recdat.addr >= AutolevelVal && recdat.addr <=  4400)  // ((int)AutolevelVal+(GRID_MAX_POINTS_X*GRID_MAX_POINTS_Y*2)) = 4400 with 5x5 mesh
    Checkkey = AutolevelVal;
	else if (recdat.addr >= SDFILE_ADDR && recdat.addr <= (SDFILE_ADDR + 10 * (FileNum + 1)))
		Checkkey = Filename;

  //SERIAL_ECHOLNPGM_P(PSTR("== Checkkey=="));
	//SERIAL_ECHOLN(Checkkey);

	if (Checkkey < 0)
	{
		memset(&recdat, 0, sizeof(recdat));
		recdat.head[0] = FHONE;
		recdat.head[1] = FHTWO;
		return;
  }

  #if ENABLED(LCD_BED_TRAMMING) && DISABLED(BED_TRAMMING_USE_PROBE)
    constexpr float lfrb[4] = BED_TRAMMING_INSET_LFRB;
  #elif ENABLED(BED_TRAMMING_USE_PROBE)
    float lfrb[4] = { getBedProbeLimits().pos[0] + 0.01f, getBedProbeLimits().pos[1] + 0.01f, getBedProbeLimits().pos[2] - 0.01f, getBedProbeLimits().pos[3] - 0.01f};
  #endif

  //SERIAL_ECHOLNPGM_P(PSTR("BeginSwitch"));

	switch (Checkkey)
	{
    case Printfile:
      if (recdat.data[0] == 1) // card
      {
        //SERIAL_ECHOLNPGM_P(PSTR("Handle Data PrintFile Pre"));
        filenavigator.getFiles(0);
        fileIndex = 0;
        recordcount = 0;
        //SERIAL_ECHOLNPGM_P(PSTR("Handle Data PrintFile Post"));
        RTS_SndData(ExchangePageBase + 46, ExchangepageAddr);
      }
      else if (recdat.data[0] == 2) // return after printing result.
      {
        stopPrint();
        injectCommands_P(PSTR("M84"));
        delay_ms(50);
        RTS_SndData(11, FilenameIcon);
        delay_ms(5);
        RTS_SndData(0, PrintscheduleIcon);
        delay_ms(50);
        RTS_SndData(0, PrintscheduleIcon + 1);
        delay_ms(5);
        RTS_SndData(0, Percentage);
        delay_ms(50);
        RTS_SndData(0, Timehour);
        delay_ms(5);
        RTS_SndData(0, Timemin);
        delay_ms(10);

        //SERIAL_ECHOLNPGM_P(PSTR("Handle Data PrintFile 2 Setting Screen "));
        //RTS_SndData(ExchangePageBase + 45, ExchangepageAddr); //exchange to 45 page
      }
      else if (recdat.data[0] == 3) // Temperature control
      {
        //SERIAL_ECHOLNPGM_P(PSTR("Handle Data PrintFile 3 Setting Screen "));
        RTS_SndData(ExchangePageBase + 57, ExchangepageAddr); //exchange to 57 page, the fans on
      }
      else if (recdat.data[0] == 4) //Settings
      break;

    case Ajust:
      if (recdat.data[0] == 1)
      {
      }
      else if (recdat.data[0] == 2)
      {
        //SERIAL_ECHOLNPGM_P(PSTR("Handle Data Adjust 2 Setting Screen "));
        if (PrinterStatusKey[1] == 3) // during heating
        {
          RTS_SndData(ExchangePageBase + 53, ExchangepageAddr);
        }
        else if (PrinterStatusKey[1] == 4)
        {
          RTS_SndData(ExchangePageBase + 54, ExchangepageAddr);
        }
        else
        {
          RTS_SndData(ExchangePageBase + 53, ExchangepageAddr);
        }
      }
      else if (recdat.data[0] == 3)
      {
        if (getTargetFan_percent((fan_t)getActiveTool())!=0) //turn on the fan
        {
          setTargetFan_percent(100, FAN0);
        }
        else //turn off the fan
        {
          setTargetFan_percent(0, FAN0);
        }
      }

      break;
    #if ENABLED(DUAL_X_CARRIAGE)
      case Idex_Settings:
        if (recdat.addr == T2Offset_X)
        {
          //SERIAL_ECHOLNPGM("T2Offset_X Set 0 : ", recdat.data[0]);
          //SERIAL_ECHOLNPGM("T2Offset_X Set 1 : ", recdat.data[1]);

          union { long l; short lb[2]; } tmpLongBuff;
          tmpLongBuff.lb[0] = recdat.data[1];
          tmpLongBuff.lb[1] = recdat.data[0];
          //SERIAL_ECHOLNPGM("T2Offset_X L : ", tmpLongBuff.l);
          setNozzleOffset_mm((float)tmpLongBuff.l/1000, X, E1);
        }
        else if (recdat.addr == T2Offset_Y)
        {
          union { long l; short lb[2]; } tmpLongBuff;
          tmpLongBuff.lb[0] = recdat.data[1];
          tmpLongBuff.lb[1] = recdat.data[0];
          setNozzleOffset_mm((float)tmpLongBuff.l/1000, Y, E1);
        }
        else if (recdat.addr == T2Offset_Z)
        {
          union { long l; short lb[2]; } tmpLongBuff;
          tmpLongBuff.lb[0] = recdat.data[1];
          tmpLongBuff.lb[1] = recdat.data[0];
          setNozzleOffset_mm((float)tmpLongBuff.l/1000, Z, E1);
        }
      break;
    #endif

    case Feedrate:
      setFeedrate_percent(recdat.data[0]);
      break;

    case PrintChoice:
      if (recdat.addr == Stopprint)
      {
        //SERIAL_ECHOLNPGM_P(PSTR("StopPrint"));
        if (recdat.data[0] == 240) // no
        {
          RTS_SndData(ExchangePageBase + 53, ExchangepageAddr);
          //SERIAL_ECHOLNPGM("Stop No", recdat.data[0] );
        }
        else
        {
          RTS_SndData(ExchangePageBase + 45, ExchangepageAddr);
          RTS_SndData(0, Timehour);
          RTS_SndData(0, Timemin);
          //SERIAL_ECHOLNPGM("Stop Triggered", recdat.data[0] );
          stopPrint();
        }
      }
      else if (recdat.addr == Pauseprint)
      {
        if (recdat.data[0] != 0xF1)
          break;

        RTS_SndData(ExchangePageBase + 54, ExchangepageAddr);
        pausePrint();
      }
      else if (recdat.addr == Resumeprint && recdat.data[0] == 1)
      {
        resumePrint();

        PrinterStatusKey[1] = 0;

        RTS_SndData(ExchangePageBase + 53, ExchangepageAddr);
      }
      if (recdat.addr == Resumeprint && recdat.data[0] == 2) // warming
      {
        resumePrint();
        NozzleTempStatus[2] = 1;
        PrinterStatusKey[1] = 0;
        RTS_SndData(ExchangePageBase + 68, ExchangepageAddr);
      }
      break;

    case Zoffset:
      float tmp_zprobe_offset;
      if (recdat.data[0] >= 32768)
      {
        tmp_zprobe_offset = ((float)recdat.data[0] - 65536) / 100;
      }
      else
      {
        tmp_zprobe_offset = ((float)recdat.data[0]) / 100;
      }
      SERIAL_ECHOLNPGM("Requested Offset ", tmp_zprobe_offset);
      if (WITHIN((tmp_zprobe_offset), PROBE_OFFSET_ZMIN, PROBE_OFFSET_ZMAX))
      {
        int16_t tmpSteps = mmToWholeSteps(getZOffset_mm() - tmp_zprobe_offset, (axis_t)Z);
        if(tmpSteps==0)
        {
          SERIAL_ECHOLNPGM_P(PSTR("Rounding to step"));
          if(getZOffset_mm() < tmp_zprobe_offset)
            tmpSteps = 1;
          else
            tmpSteps = -1;
        }
        smartAdjustAxis_steps(tmpSteps*-1, (axis_t)Z, false);
        char zOffs[20], tmp1[11];
        sprintf_P(zOffs, PSTR("Z Offset : %s"), dtostrf(getZOffset_mm(), 1, 3, tmp1));
        onStatusChanged(zOffs);
      }
      else
      {
        onStatusChanged("Requested Offset Beyond Limits");
        RTS_SndData(getZOffset_mm() * 100, ProbeOffset_Z);
      }

      rtscheck.RTS_SndData(getZOffset_mm() * 100, ProbeOffset_Z);
      break;

    case TempControl:
      if (recdat.data[0] == 0)
      {
      }
      else if (recdat.data[0] == 1)
      {
        RTS_SndData(ExchangePageBase + 59, ExchangepageAddr); //exchange to 59 page, the fans on
      }
      else if (recdat.data[0] == 2)
      {
      }
      else if (recdat.data[0] == 3)
      {
        if (getTargetFan_percent((fan_t)getActiveTool())==0) //turn on the fan
        {
          setTargetFan_percent(100, FAN0);
          RTS_SndData(ExchangePageBase + 57, ExchangepageAddr); //exchange to 57 page, the fans on
        }
        else //turn off the fan
        {
          setTargetFan_percent(0, FAN0);
          RTS_SndData(ExchangePageBase + 57, ExchangepageAddr); //exchange to 57 page, the fans on
        }
      }
      else if (recdat.data[0] == 5) //PLA mode
      {
        setTargetTemp_celsius(PREHEAT_1_TEMP_HOTEND, getActiveTool());
        setTargetTemp_celsius(PREHEAT_1_TEMP_BED, BED);

        RTS_SndData(PREHEAT_1_TEMP_HOTEND, NozzlePreheat);
        RTS_SndData(PREHEAT_1_TEMP_BED, BedPreheat);
      }
      else if (recdat.data[0] == 6) //ABS mode
      {
        setTargetTemp_celsius(PREHEAT_2_TEMP_HOTEND, getActiveTool());
        setTargetTemp_celsius(PREHEAT_2_TEMP_BED, BED);

        RTS_SndData(PREHEAT_2_TEMP_HOTEND, NozzlePreheat);
        RTS_SndData(PREHEAT_2_TEMP_BED, BedPreheat);
      }
      else if (recdat.data[0] == 0xF1)
      {
        #if FAN_COUNT > 0
              for (uint8_t i = 0; i < FAN_COUNT; i++)
                setTargetFan_percent(0, (fan_t)i);
        #endif
        setTargetTemp_celsius(0.0, H0);
        #if HAS_MULTI_HOTEND
          setTargetTemp_celsius(0.0, H1);
        #endif
        setTargetTemp_celsius(0.0, BED);

        RTS_SndData(0, NozzlePreheat);
        delay_ms(1);
        RTS_SndData(0, BedPreheat);
        delay_ms(1);

        RTS_SndData(ExchangePageBase + 57, ExchangepageAddr);
        PrinterStatusKey[1] = 2;
      }
      break;

    case ManualSetTemp:
    //SERIAL_ECHOLNPGM_P(PSTR("ManualSetTemp"));
      if (recdat.addr == NzBdSet)
      {
        if (recdat.data[0] == 0)
        {
          RTS_SndData(ExchangePageBase + 57, ExchangepageAddr); //exchange to 57 page, the fans on
        }
        else if (recdat.data[0] == 1)
        {
          setTargetTemp_celsius(0.0, getActiveTool());
          RTS_SndData(0, NozzlePreheat);
        }
        else if (recdat.data[0] == 2)
        {
          setTargetTemp_celsius(0.0, BED);
          RTS_SndData(0, BedPreheat);
        }
      }
      else if (recdat.addr == NozzlePreheat)
        setTargetTemp_celsius((float)recdat.data[0], H0);
      #if HAS_MULTI_HOTEND
        else if (recdat.addr == e2Temp)
          setTargetTemp_celsius((float)recdat.data[0], H1);
      #endif
      else if (recdat.addr == BedPreheat)
        setTargetTemp_celsius((float)recdat.data[0], BED);
      else if (recdat.addr == Flowrate)
        setFlow_percent((int16_t)recdat.data[0], getActiveTool());
      #if HAS_PID_HEATING
        else if (recdat.addr == HotendPID_AutoTmp)
          pid_hotendAutoTemp = (uint16_t)recdat.data[0];
        else if (recdat.addr == BedPID_AutoTmp)
          pid_bedAutoTemp = (uint16_t)recdat.data[0];
      #endif

      else if (recdat.addr == Accel_X) {
        setAxisMaxAcceleration_mm_s2((uint16_t)recdat.data[0]*100, X);
      }
      else if (recdat.addr == Accel_Y) {
        setAxisMaxAcceleration_mm_s2((uint16_t)recdat.data[0]*100, Y);
      }
      else if (recdat.addr == Accel_Z) {
        setAxisMaxAcceleration_mm_s2((uint16_t)recdat.data[0]*10, Z);
      }
      else if (recdat.addr == Accel_E) {
        setAxisMaxAcceleration_mm_s2((uint16_t)recdat.data[0], E0);
        setAxisMaxAcceleration_mm_s2((uint16_t)recdat.data[0], E1);
      }

      else if (recdat.addr == Feed_X) {
          setAxisMaxFeedrate_mm_s((uint16_t)recdat.data[0], X);
        }
        else if (recdat.addr == Feed_Y) {
          setAxisMaxFeedrate_mm_s((uint16_t)recdat.data[0], Y);
        }
        else if (recdat.addr == Feed_Z) {
          setAxisMaxFeedrate_mm_s((uint16_t)recdat.data[0], Z);
        }
        else if (recdat.addr == Feed_E) {
          setAxisMaxFeedrate_mm_s((uint16_t)recdat.data[0], E0);
          setAxisMaxFeedrate_mm_s((uint16_t)recdat.data[0], E1);
        }
        else if (recdat.addr == FanKeyIcon) {
          setTargetFan_percent((uint16_t)recdat.data[0], (fan_t)getActiveTool());
        }
      else {
        float tmp_float_handling;
        if (recdat.data[0] >= 32768)
        {
          tmp_float_handling = ((float)recdat.data[0] - 65536) / 100;
        }
        else
        {
          tmp_float_handling = ((float)recdat.data[0]) / 100;
        }
        if (recdat.addr == StepMM_X) {
          setAxisSteps_per_mm(tmp_float_handling*10, X);
        }
        else if (recdat.addr == StepMM_Y) {
          setAxisSteps_per_mm(tmp_float_handling*10, Y);
        }
        else if (recdat.addr == StepMM_Z) {
          setAxisSteps_per_mm(tmp_float_handling*10, Z);
        }
        else if (recdat.addr == StepMM_E) {
          setAxisSteps_per_mm(tmp_float_handling*10, E0);
          #if DISABLED(DUAL_X_CARRIAGE)
            setAxisSteps_per_mm(tmp_float_handling*10, E1);
          #endif
        }
        #if ENABLED(DUAL_X_CARRIAGE)
          else if (recdat.addr == T2StepMM_E)
          {
            setAxisSteps_per_mm(tmp_float_handling*10, E1);
          }
        #endif
        #if HAS_BED_PROBE
          else if (recdat.addr == ProbeOffset_X) {
            setProbeOffset_mm(tmp_float_handling, X);
            injectCommands_P(PSTR("M422R"));

          }
          else if (recdat.addr == ProbeOffset_Y) {
            setProbeOffset_mm(tmp_float_handling, Y);
            injectCommands_P(PSTR("M422R"));
          }
          else if (recdat.addr == ProbeOffset_Z) {
            setProbeOffset_mm(tmp_float_handling, Z);
          }
        #endif

        #if ENABLED(CLASSIC_JERK)
          else if (recdat.addr == Jerk_X) {
            setAxisMaxJerk_mm_s(tmp_float_handling, X);
          }
          else if (recdat.addr == Jerk_Y) {
            setAxisMaxJerk_mm_s(tmp_float_handling, Y);
          }
          else if (recdat.addr == Jerk_Z) {
            setAxisMaxJerk_mm_s(tmp_float_handling, Z);
          }
          else if (recdat.addr == Jerk_E) {
            setAxisMaxJerk_mm_s(tmp_float_handling, E0);
            setAxisMaxJerk_mm_s(tmp_float_handling, E1);
          }
        #endif

        #if HAS_SHAPING
          else if (recdat.addr == ShapingZetaX) {
            setShapingZeta(tmp_float_handling/10, X);
          }
          else if (recdat.addr == ShapingZetaY) {
            setShapingZeta(tmp_float_handling/10, Y);
          }
          else if (recdat.addr == ShapingFreqX) {
            setShapingFrequency(tmp_float_handling, X);
          }
          else if (recdat.addr == ShapingFreqY) {
            setShapingFrequency(tmp_float_handling, Y);
          }
        #endif

        #if ENABLED(LIN_ADVANCE)
          else if (recdat.addr == LinAdvKFactor) {
            setLinearAdvance_mm_mm_s(tmp_float_handling/10, getActiveTool());
          }
        #endif

        #if HAS_FILAMENT_SENSOR
          else if(recdat.addr == RunoutToggle){
            if(getFilamentRunoutEnabled())
              setFilamentRunoutEnabled(false);
            else
              setFilamentRunoutEnabled(true);
          }
          else if(recdat.addr == RunoutMode){
            if(getRunoutMode(getActiveTool())==0) {setRunoutMode(1, getActiveTool());}
            else if(getRunoutMode(getActiveTool())==1) {setRunoutMode(2, getActiveTool());}
            else if(getRunoutMode(getActiveTool())==2) {setRunoutMode(7, getActiveTool());}
            else if(getRunoutMode(getActiveTool())==7) {setRunoutMode(0, getActiveTool());}
            SERIAL_ECHOLNPGM("RunoutMode: ", (int)getRunoutMode(getActiveTool()));
          }
        #endif

          else if (recdat.addr == BabystepIncrement) {
            babystepIncrementIndex++;
            if(babystepIncrementIndex>=8)
              babystepIncrementIndex=0;
          }

        #if ENABLED(POWER_LOSS_RECOVERY)
          else if(recdat.addr == PowerLossToggle){
            if(getPowerLossRecoveryEnabled())
              setPowerLossRecoveryEnabled(false);
            else
              setPowerLossRecoveryEnabled(true);
          }
        #endif

        #if ENABLED(CASE_LIGHT_ENABLE)
          else if(recdat.addr == LedToggle){
            if(getCaseLightState())
              setCaseLightState(false);
            else
              setCaseLightState(true);
          }
        #endif




        #if HAS_PID_HEATING
          else if (recdat.addr == HotendPID_P) {
            setPID(tmp_float_handling*10, getPID_Ki(getActiveTool()), getPID_Kd(getActiveTool()), getActiveTool());
          }
          else if (recdat.addr == HotendPID_I) {
            setPID(getPID_Kp(getActiveTool()), tmp_float_handling*10, getPID_Kd(getActiveTool()), getActiveTool());
          }
          else if (recdat.addr == HotendPID_D) {
            setPID(getPID_Kp(getActiveTool()), getPID_Ki(getActiveTool()), tmp_float_handling*10, getActiveTool());
          }
          #if ENABLED(PIDTEMPBED)
            else if (recdat.addr == BedPID_P) {
              setBedPID(tmp_float_handling*10, getBedPID_Ki(), getBedPID_Kd());
            }
            else if (recdat.addr == BedPID_I) {
              setBedPID(getBedPID_Kp(), tmp_float_handling*10, getBedPID_Kd());
            }
            else if (recdat.addr == BedPID_D) {
              setBedPID(getBedPID_Kp(), getBedPID_Ki(), tmp_float_handling*10);
            }
          #endif
        #endif
      }
      break;

    case Setting:
      if (recdat.data[0] == 0) // return to main page
      {
      }
      else if (recdat.data[0] == 1) //Bed Autoleveling
      {
        #if HAS_MESH
          if (getLevelingActive())
            RTS_SndData(3, AutoLevelIcon);
          else
            RTS_SndData(2, AutoLevelIcon);

          if (ExtUI::getLevelingIsValid())
          {
            uint8_t abl_probe_index = 0;
            for(uint8_t outer = 0; outer < GRID_MAX_POINTS_Y; outer++)
            {
              for (uint8_t inner = 0; inner < GRID_MAX_POINTS_X; inner++)
              {
                uint8_t x_Point = inner;
                bool zig = (outer & 1);
                if (zig) x_Point = (GRID_MAX_POINTS_X - 1) - inner;
                xy_uint8_t point = {x_Point, outer};
                rtscheck.RTS_SndData(ExtUI::getMeshPoint(point) * 1000, AutolevelVal + (abl_probe_index * 2));
                ++abl_probe_index;
              }
            }
          }
        #endif
        RTS_SndData(10, FilenameIcon); //Motor Icon
        if (!isPositionKnown())
          injectCommands_P(PSTR("G28\nG1F1000Z0.0"));
        else
          injectCommands_P(PSTR("G1F1000Z0.0"));
        waitway = 2;

        RTS_SndData(ExchangePageBase + 64, ExchangepageAddr);
      }
      else if (recdat.data[0] == 2) // Exchange filement
      {
        memset(ChangeMaterialbuf, 0, sizeof(ChangeMaterialbuf));
        ChangeMaterialbuf[1] = ChangeMaterialbuf[0] = 10;
        RTS_SndData(10 * ChangeMaterialbuf[0], FilementUnit1); //It's ChangeMaterialbuf for show,instead of current_position[E_AXIS] in them.
        RTS_SndData(10 * ChangeMaterialbuf[1], FilementUnit2);
        RTS_SndData(getActualTemp_celsius(H0), NozzleTemp);
        RTS_SndData(getTargetTemp_celsius(H0), NozzlePreheat);
        #if HAS_MULTI_HOTEND
          rtscheck.RTS_SndData(getActualTemp_celsius(H1), e2Temp);
          rtscheck.RTS_SndData(getTargetTemp_celsius(H1), e2Preheat);
        #else
          rtscheck.RTS_SndData(0, e2Temp);
          rtscheck.RTS_SndData(0, e2Preheat);
        #endif
        delay_ms(2);
        RTS_SndData(ExchangePageBase + 65, ExchangepageAddr);
      }
      else if (recdat.data[0] == 3) //Move
      {
        AxisPagenum = 0;
        RTS_SndData(ExchangePageBase + 71, ExchangepageAddr);
      }
      else if (recdat.data[0] == 4) //Language
      {
        //Just loads language screen, now used for tools
      }
      else if (recdat.data[0] == 5) //Printer Information
      {
        RTS_SndData(WEBSITE_URL, CorpWebsite);
      }
      else if (recdat.data[0] == 6) // Diabalestepper
      {
        injectCommands_P(PSTR("M84"));
        RTS_SndData(11, FilenameIcon);
      }
      break;

    case ReturnBack:
      SERIAL_ECHOPGM("Return : ", recdat.data[0]);
      if (recdat.data[0] == 1) // return to the tool page
      {
        RTS_SndData(ExchangePageBase + 63, ExchangepageAddr);
      }
      if (recdat.data[0] == 2) // return to the Level mode page
      {
        RTS_SndData(ExchangePageBase + 64, ExchangepageAddr);
      }
      break;

    case Bedlevel:
      SERIAL_ECHOLNPGM("Bed Level Option ",  recdat.data[0]);
      float tmp_zprobe_adjust;
      switch (babystepIncrementIndex)
      {
      case 1:
      {
        tmp_zprobe_adjust = 0.02;
        break;
      }
      case 2:
      {
        tmp_zprobe_adjust = 0.05;
        break;
      }
      case 3:
      {
        tmp_zprobe_adjust = 0.1;
        break;
      }
      case 4:
      {
        tmp_zprobe_adjust = 0.2;
        break;
      }
      case 5:
      {
        tmp_zprobe_adjust = 0.25;
        break;
      }
      case 6:
      {
        tmp_zprobe_adjust = 0.5;
        break;
      }
      case 7:
      {
        tmp_zprobe_adjust = 1.0f;
        break;
      }
      default:
      {
        tmp_zprobe_adjust = 0.01;
        break;
      }
      }
      #if ENABLED(BED_TRAMMING_USE_PROBE)
        char g30CommandStr[20];
      #endif
      switch(recdat.data[0])
      {
        case 1: // Z-axis to home
        {
          // Disallow Z homing if X or Y are unknown
          if (!isAxisPositionKnown((axis_t)X) || !isAxisPositionKnown((axis_t)Y))
            injectCommands_P(PSTR("G28\nG1F1500Z0.0"));
          else
            injectCommands_P(PSTR("G28Z\nG1F1500Z0.0"));

          RTS_SndData(getZOffset_mm() * 100, ProbeOffset_Z);
          break;
        }
        case 2: // Z-axis to Up
        {
          SERIAL_ECHOLNPGM("Requested Offset ", tmp_zprobe_offset);
          if (WITHIN((getZOffset_mm()+tmp_zprobe_adjust), PROBE_OFFSET_ZMIN, PROBE_OFFSET_ZMAX))
          {
            int16_t tmpSteps = mmToWholeSteps(tmp_zprobe_adjust, (axis_t)Z);
            if(tmpSteps==0)
            {
              SERIAL_ECHOLNPGM_P(PSTR("Rounding to step"));
              tmpSteps = 1;
            }
            smartAdjustAxis_steps(tmpSteps, (axis_t)Z, false);
            char zOffs[20], tmp1[11];
            sprintf_P(zOffs, PSTR("Z Offset : %s"), dtostrf(getZOffset_mm(), 1, 3, tmp1));
            onStatusChanged(zOffs);
          }
          else
          {
            onStatusChanged("Requested Offset Beyond Limits");
            RTS_SndData(getZOffset_mm() * 100, ProbeOffset_Z);
          }

          rtscheck.RTS_SndData(getZOffset_mm() * 100, ProbeOffset_Z);
          break;
        }
        case 3: // Z-axis to Down
        {
          SERIAL_ECHOLNPGM("Requested Offset ", tmp_zprobe_offset);
          if (WITHIN((getZOffset_mm()-tmp_zprobe_adjust), PROBE_OFFSET_ZMIN, PROBE_OFFSET_ZMAX))
          {
            int16_t tmpSteps = mmToWholeSteps((tmp_zprobe_adjust), (axis_t)Z);
            if(tmpSteps==0)
            {
              SERIAL_ECHOLNPGM_P(PSTR("Rounding to step"));
              tmpSteps = 1;
            }
            smartAdjustAxis_steps(tmpSteps*-1, (axis_t)Z, false);
            char zOffs[20], tmp1[11];
            sprintf_P(zOffs, PSTR("Z Offset : %s"), dtostrf(getZOffset_mm(), 1, 3, tmp1));
            onStatusChanged(zOffs);
          }
          else
          {
            onStatusChanged("Requested Offset Beyond Limits");
            RTS_SndData(getZOffset_mm() * 100, ProbeOffset_Z);
          }

          rtscheck.RTS_SndData(getZOffset_mm() * 100, ProbeOffset_Z);
          break;
        }
        case 4: // Assitant Level
        {
          #if HAS_MESH
            setLevelingActive(false);
          #endif
          if (!isPositionKnown())
            injectCommands_P((PSTR("G28\nG1 F1000 Z0.0")));
          else
            injectCommands_P((PSTR("G1 F1000 Z0.0")));
          waitway = 2;
          RTS_SndData(ExchangePageBase + 84, ExchangepageAddr);
          break;
        }
        case 5: // AutoLevel "Measuring" Button
        {
          #if ENABLED(MESH_BED_LEVELING)
            RTS_SndData(ExchangePageBase + 93, ExchangepageAddr);
          #else
            waitway = 3; //only for prohibiting to receive massage
            RTS_SndData(3, AutolevelIcon);
            uint8_t abl_probe_index = 0;
            while (abl_probe_index < 25) {
              rtscheck.RTS_SndData(0, AutolevelVal + abl_probe_index * 2);
              ++abl_probe_index;
            }
            RTS_SndData(ExchangePageBase + 64, ExchangepageAddr);
            injectCommands_P(PSTR(MEASURING_GCODE));
          #endif
          break;
        }

        case 6: // Assitant Level ,  Centre 1
        {
          #if ENABLED(BED_TRAMMING_USE_PROBE)
            sprintf_P(g30CommandStr, PSTR("G30X%sY%s"), X_CENTER, Y_CENTER);
            injectCommands(g30CommandStr);
          #else
            setAxisPosition_mm(BED_TRAMMING_Z_HOP, (axis_t)Z);
            setAxisPosition_mm(X_CENTER, (axis_t)X);
            setAxisPosition_mm(Y_CENTER, (axis_t)Y);
            waitway = 6;
          #endif
          break;
        }
        case 7: // Assitant Level , Front Left 2
        {
          #if ENABLED(BED_TRAMMING_USE_PROBE)
            sprintf_P(g30CommandStr, PSTR("G30X%sY%s"), lfrb[0], lfrb[1]);
            injectCommands(g30CommandStr);
          #else
            setAxisPosition_mm(BED_TRAMMING_Z_HOP, (axis_t)Z);
            setAxisPosition_mm((X_MIN_BED + lfrb[0]), (axis_t)X);
            setAxisPosition_mm((Y_MIN_BED + lfrb[1]), (axis_t)Y);
            waitway = 6;
          #endif
          break;
        }
        case 8: // Assitant Level , Front Right 3
        {
          #if ENABLED(BED_TRAMMING_USE_PROBE)
            sprintf_P(g30CommandStr, PSTR("G30X%sY%s"), lfrb[2], lfrb[1]);
            injectCommands(g30CommandStr);
          #else
            setAxisPosition_mm(BED_TRAMMING_Z_HOP, (axis_t)Z);
            setAxisPosition_mm((X_MAX_BED - lfrb[2]), (axis_t)X);
            setAxisPosition_mm((Y_MIN_BED + lfrb[1]), (axis_t)Y);
            waitway = 6;
          #endif
          break;
        }
        case 9: // Assitant Level , Back Right 4
        {
          #if ENABLED(BED_TRAMMING_USE_PROBE)
            sprintf_P(g30CommandStr, PSTR("G30X%sY%s"), lfrb[2], lfrb[3]);
            injectCommands(g30CommandStr);
          #else
            setAxisPosition_mm(BED_TRAMMING_Z_HOP, (axis_t)Z);
            setAxisPosition_mm((X_MAX_BED - lfrb[2]), (axis_t)X);
            setAxisPosition_mm((Y_MAX_BED - lfrb[3]), (axis_t)Y);
            waitway = 6;
          #endif
          break;
        }
        case 10: // Assitant Level , Back Left 5
        {
          #if ENABLED(BED_TRAMMING_USE_PROBE)
            sprintf_P(g30CommandStr, PSTR("G30X%sY%s"), lfrb[0], lfrb[3]);
            injectCommands(g30CommandStr);
          #else
            setAxisPosition_mm(BED_TRAMMING_Z_HOP, (axis_t)Z);
            setAxisPosition_mm((X_MIN_BED + lfrb[0]), (axis_t)X);
            setAxisPosition_mm((Y_MAX_BED - lfrb[3]), (axis_t)Y);
            waitway = 6;
          #endif
          break;
        }
        case 11: // Autolevel switch
        {
          #if HAS_MESH
            if (!getLevelingActive()) //turn on the Autolevel
            {
              RTS_SndData(3, AutoLevelIcon);
              setLevelingActive(true);
            }
            else //turn off the Autolevel
            {
              RTS_SndData(2, AutoLevelIcon);
              setLevelingActive(false);
            }
          #endif
          RTS_SndData(getZOffset_mm() * 100, ProbeOffset_Z);
          break;
        }
        case 12:
        {
          injectCommands_P(PSTR("G26R255"));
          onStatusChanged("Beginning G26.. Heating");
          break;
        }
        case 13:
        {
          injectCommands_P(PSTR("G29S1"));
          onStatusChanged("Begin Manual Mesh");
          break;
        }
        case 14:
        {
          injectCommands_P(PSTR("G29S2"));
          onStatusChanged("Moving to Next Mesh Point");
          break;
        }
        case 15:
        {
          injectCommands_P(PSTR("M211S0\nG91\nG1Z-0.025\nG90\nM211S1"));
          onStatusChanged("Moved down 0.025");
          break;
        }
        case 16:
        {
          injectCommands_P(PSTR("M211S0\nG91\nG1Z0.025\nG90\nM211S1"));
          onStatusChanged("Moved up 0.025");
          break;
        }
        case 17:
        {
          Settings.display_volume = 0;
          Settings.display_sound = false;
          SetTouchScreenConfiguration();
          break;
        }
        case 18:
        {
          Settings.display_volume = 255;
          Settings.display_sound = true;
          SetTouchScreenConfiguration();
          break;
        }
        case 19:
        {
          Settings.screen_brightness = 10;
          SetTouchScreenConfiguration();
          break;
        }
        case 20:
        {
          Settings.screen_brightness = 100;
          SetTouchScreenConfiguration();
          break;
        }
        case 21:
        {
          if(Settings.display_standby) {
            Settings.display_standby = false;
          }
          else {
            Settings.display_standby = true;
          }
          SetTouchScreenConfiguration();
          break;
        }
        case 22:
        {
          if(Settings.screen_rotation==10) {
            Settings.screen_rotation = 0;
          }
          else {
            Settings.screen_rotation = 10;
          }
          SetTouchScreenConfiguration();
          break;
        }
        case 23: // Set IDEX Autopark
        {
          injectCommands_P(PSTR("M605S1\nG28X\nG1X0"));
          break;
        }
        case 24: // Set IDEX Duplication
        {
          injectCommands_P(PSTR("M605S1\nT0\nG28\nM605S2\nG28X\nG1X0"));
          break;
        }
        case 25: // Set IDEX Mirrored Duplication
        {
          injectCommands_P(PSTR("M605S1\nT0\nG28\nM605S2\nG28X\nG1X0\nM605S3"));
          break;
        }
        case 26: // Set IDEX Full Control
        {
          injectCommands_P(PSTR("M605S0\nG28X"));
          break;
        }
        case 27: // Change Tool
        {
          if(getActiveTool()==E0)
            setActiveTool(E1, !isAxisPositionKnown(X));
          else
            setActiveTool(E0, !isAxisPositionKnown(X));
          break;
        }
        default:
        {
          SERIAL_ECHOLNPGM("Unsupported Option Selected", recdat.data[0]);
          break;
        }
      }

      RTS_SndData(10, FilenameIcon);
      break;

    case XYZEaxis:
    {
      axis_t axis = X;
      float min = 0.0f, max = 0.0f;
      waitway = 4;
      if (recdat.addr == DisplayXaxis)
      {
        axis = X;
        min = X_MIN_POS;
        max = X_MAX_POS;
      }
      else if (recdat.addr == DisplayYaxis)
      {
        axis = Y;
        min = Y_MIN_POS;
        max = Y_MAX_POS;
      }
      else if (recdat.addr == DisplayZaxis)
      {
        axis = Z;
        min = Z_MIN_POS;
        max = Z_MAX_POS;
      }
      else if (recdat.addr == AutoZero)
      {
        if (recdat.data[0] == 3) //autohome
        {
          waitway = 4;
          injectCommands_P((PSTR("G28\nG1 F1000 Z10")));
          RTS_SndData(10, FilenameIcon);
        }
        else
        {
          AxisPagenum = recdat.data[0];
          waitway = 0;
        }
        break;
      }

      float targetPos = ((float)recdat.data[0]) / 10;

      if (targetPos < min)
        targetPos = min;
      else if (targetPos > max)
        targetPos = max;
      setAxisPosition_mm(targetPos, axis);
      waitway = 0;
      RTS_SndData(10, FilenameIcon);
      break;
    }

    case Filement:
      if (recdat.addr == Exchfilement)
      {
        extruder_t tmpTool;
        if (recdat.data[0]<=2)
          tmpTool = E0;
        else if (recdat.data[0]<=4)
          tmpTool = E1;

        if (getActualTemp_celsius(tmpTool) < EXTRUDE_MINTEMP && recdat.data[0] < 5)
        {
          RTS_SndData((int)EXTRUDE_MINTEMP, 0x1020);
          delay_ms(5);
          RTS_SndData(ExchangePageBase + 66, ExchangepageAddr);
          break;
        }

        switch(recdat.data[0])
        {
          case 1 : // Unload filement1
          {
            setAxisPosition_mm((getAxisPosition_mm(E0) - ChangeMaterialbuf[0]), E0);
            break;
          }
          case 2: // Load filement1
          {
            setAxisPosition_mm((getAxisPosition_mm(E0) + ChangeMaterialbuf[0]), E0);
            break;
          }
          case 3: // Unload filement2
          {
            setAxisPosition_mm((getAxisPosition_mm(E1) - ChangeMaterialbuf[1]), E1);
            break;
          }
          case 4: // Load filement2
          {
            setAxisPosition_mm((getAxisPosition_mm(E1) + ChangeMaterialbuf[1]), E1);
            break;
          }
          case 5: // sure to heat
          {
            NozzleTempStatus[0] = 1;
            //InforShowoStatus = true;

            setTargetTemp_celsius((PREHEAT_1_TEMP_HOTEND+10), E0);

            #if HAS_MULTI_HOTEND
              setTargetTemp_celsius((PREHEAT_1_TEMP_HOTEND+10), E1);
            #endif

            RTS_SndData(ExchangePageBase + 68, ExchangepageAddr);
            break;
          }
          case 6: //cancel to heat
          {
            RTS_SndData(ExchangePageBase + 65, ExchangepageAddr);
            break;
          }
          case 0xF1: //Sure to cancel heating
          {
            //InforShowoStatus = true;
            NozzleTempStatus[0] = 0;
            delay_ms(1);
            RTS_SndData(ExchangePageBase + 65, ExchangepageAddr);
            break;
          }
          case 0xF0: // not to cancel heating
            break;
        }
        RTS_SndData(10 * ChangeMaterialbuf[0], FilementUnit1); //It's ChangeMaterialbuf for show,instead of current_position[E_AXIS] in them.
        RTS_SndData(10 * ChangeMaterialbuf[1], FilementUnit2);
      }
      else if (recdat.addr == FilementUnit1)
      {
        ChangeMaterialbuf[0] = ((float)recdat.data[0]) / 10;
      }
      else if (recdat.addr == FilementUnit2)
      {
        ChangeMaterialbuf[1] = ((float)recdat.data[0]) / 10;
      }
      break;

    case LanguageChoice:

      SERIAL_ECHOLNPGM("\n ***recdat.data[0] =", recdat.data[0]);
      /*if(recdat.data[0]==1) {
          settings.save();
        }
        else {
          injectCommands_P(PSTR("M300"));
        }*/
      // may at some point use language change screens to save eeprom explicitly
      //SERIAL_ECHOLNPGM_P(PSTR("InLangChoice"));
      switch(recdat.data[0])
      {
        case 0: {
          //SERIAL_ECHOLNPGM_P(PSTR("Store Settings"));
          injectCommands_P(PSTR("M500"));
          break;
        }
        case 1: {
          RTS_SndData(ExchangePageBase + 94, ExchangepageAddr);
          break;
        }
        #if ENABLED(PIDTEMP)
          case 2: {
            onStatusChanged("Hotend PID Started");
            startPIDTune(static_cast<celsius_t>(pid_hotendAutoTemp), getActiveTool());
            break;
          }
        #endif
        case 3: {
          //SERIAL_ECHOLNPGM_P(PSTR("Init EEPROM"));
          injectCommands_P(PSTR("M502\nM500"));
          break;
        }
        case 4: {
          //SERIAL_ECHOLNPGM_P(PSTR("BLTouch Reset"));
          injectCommands_P(PSTR("M999\nM280P0S160"));
          break;
        }

        case 5: {
          #if ENABLED(PIDTEMPBED)
            onStatusChanged("Bed PID Started");
            startBedPIDTune(static_cast<celsius_t>(pid_bedAutoTemp));
          #else
            SERIAL_ECHOLNPGM_P(PSTR("Bed PID Disabled"));
          #endif
          break;
        }
        case 6: {
          //SERIAL_ECHOLNPGM_P(PSTR("Store Settings"));
          injectCommands_P(PSTR("M500"));
          break;
        }
        default: {
          SERIAL_ECHOLNPGM_P(PSTR("Invalid Option"));
          break;
        }
      }
      break;
    case No_Filement:
      //SERIAL_ECHOLNPGM_P(PSTR("\n No Filament"));

      if (recdat.data[0] == 1) //Filament is out, resume / resume selected on screen
      {
        //SERIAL_ECHOLNPGM_P(PSTR("Resume Yes during print"));
        if (ExtUI::pauseModeStatus != PAUSE_MESSAGE_PURGE && ExtUI::pauseModeStatus != PAUSE_MESSAGE_OPTION)
        {
          //setPauseMenuResponse(PAUSE_RESPONSE_RESUME_PRINT);
          setUserConfirmed();
          //PrinterStatusKey[1] = 3;
          //pause_resume_selected = true;
        }
        else {
          #if ENABLED(FILAMENT_RUNOUT_SENSOR)
            if(getFilamentRunoutState() && getFilamentRunoutEnabled(getActiveTool()))
              ExtUI::setRunoutMode(0, getActiveTool());
            else {
              setPauseMenuResponse(PAUSE_RESPONSE_RESUME_PRINT);
              setUserConfirmed();
              PrinterStatusKey[1] = 3;
              pause_resume_selected = true;
            }

          #else
            setPauseMenuResponse(PAUSE_RESPONSE_RESUME_PRINT);
            setUserConfirmed();
            PrinterStatusKey[1] = 3;
            pause_resume_selected = true;
          #endif
        }
      }
      else if (recdat.data[0] == 0) // Filamet is out, Cancel Selected
      {
        //SERIAL_ECHOLNPGM_P(PSTR(" Filament Response No"));
        if(ExtUI::pauseModeStatus == PAUSE_MESSAGE_PURGE || ExtUI::pauseModeStatus == PAUSE_MESSAGE_OPTION) {
          setPauseMenuResponse(PAUSE_RESPONSE_EXTRUDE_MORE);
          setUserConfirmed();
        }
      }
      break;

  #if ENABLED(POWER_LOSS_RECOVERY)
    case PwrOffNoF:
      if (recdat.data[0] == 1) // Yes:continue to print the 3Dmode during power-off.
      {
        injectCommands_P(PSTR("M1000"));
      }
      else if (recdat.data[0] == 2) // No
      {
        injectCommands_P(PSTR("M1000C"));
      }
      break;
  #endif
    case Volume:
      if (recdat.data[0] < 0)
        Settings.display_volume = 0;
      else if (recdat.data[0] > 255)
        Settings.display_volume = 0xFF;
      else
        Settings.display_volume = recdat.data[0];

      if (Settings.display_volume == 0)
      {
        RTS_SndData(0, VolumeIcon);
        RTS_SndData(9, SoundIcon);
      }
      else
      {
        RTS_SndData((Settings.display_volume + 1) / 32 - 1, VolumeIcon);
        RTS_SndData(8, SoundIcon);
      }
      RTS_SndData(Settings.display_volume << 8, SoundAddr + 1);
      break;

    case Filename:
      //SERIAL_ECHOLNPGM_P(PSTR("Filename Selected"));
      if (isMediaMounted() && recdat.addr == FilenameChs)
      {
        //SERIAL_ECHOLNPGM_P(PSTR("Has Media"));

        recordcount = recdat.data[0] - 1;
        if(filenavigator.currentindex == 0 && filenavigator.folderdepth > 0 && (fileIndex + recordcount) == 0) {
          filenavigator.upDIR();
          //SERIAL_ECHOLNPGM_P(PSTR("GoUpDir"));
          filenavigator.getFiles(0);
          fileIndex = 0;
          return;
        }

        if(filenavigator.currentindex == 0 && filenavigator.folderdepth > 0)
          recordcount = recordcount-1; // account for return dir link in file index

        for (int j = 1; j <= 4; j++) // Clear filename BG Color and Frame
        {
          RTS_SndData((unsigned long)0xFFFF, FilenameNature + j * 16); // white
          RTS_SndData(10, FilenameIcon1 + j);							 //clean
        }
        for (int j = 0; j < 10; j++) // clear current filename
            RTS_SndData(0, Choosefilename + j);

        if(filenavigator.getIndexisDir(fileIndex + recordcount)) {
          SERIAL_ECHOLNPGM("Is Dir ", (fileIndex + recordcount));
          filenavigator.changeDIR((char *)filenavigator.getIndexName(fileIndex + recordcount));
          filenavigator.getFiles(0);
          fileIndex = 0;
          return;
        }
        else{
          SERIAL_ECHOLNPGM("Is File ", (fileIndex + recordcount));
          RTS_SndData(filenavigator.getIndexName(fileIndex + recordcount), Choosefilename);
          RTS_SndData((unsigned long)0x87F0, FilenameNature + recdat.data[0] * 16); // Change BG of selected line to Light Green
          RTS_SndData(6, FilenameIcon1 + recdat.data[0]);							  // show frame
        }
      }
      else if (recdat.addr == FilenamePlay)
      {
        if (recdat.data[0] == 1 && isMediaMounted()) //for sure
        {
          printFile(filenavigator.getIndexName(fileIndex + recordcount));

          for (int j = 0; j < 10; j++) //clean screen.
            RTS_SndData(0, Printfilename + j);

          RTS_SndData(filenavigator.getIndexName(fileIndex + recordcount), Printfilename);
          delay_ms(2);

          delay_ms(2);
          RTS_SndData(ExchangePageBase + 53, ExchangepageAddr);

          PrinterStatusKey[0] = 1;
          PrinterStatusKey[1] = 3;
          fileIndex = 0;
          recordcount = 0;
        }
        else if(recdat.data[0] == 2) //Page Down
        {
          //SERIAL_ECHOLNPGM_P(PSTR("PgDown"));
          if((fileIndex+DISPLAY_FILES) < (filenavigator.maxFiles() + (filenavigator.folderdepth!=0))) {
            fileIndex = fileIndex + DISPLAY_FILES;
            //if(filenavigator.folderdepth!=0 && fileIndex!=0) //Shift to acknowledge Return DIR button on first page
            //  filenavigator.getFiles(fileIndex-1);
           // else
              filenavigator.getFiles(fileIndex);
           //filenavigator.getFiles(filenavigator.currentindex+1);
          }
        }
        else if(recdat.data[0] == 3) //Page Up
        {
          //SERIAL_ECHOLNPGM_P(PSTR("PgUp"));
          if(fileIndex>=DISPLAY_FILES) {
            fileIndex = fileIndex - DISPLAY_FILES;
            //if(filenavigator.folderdepth!=0 && fileIndex!=0) //Shift to acknowledge Return DIR button on first page
              //filenavigator.getFiles(filenavigator.currentindex-DISPLAY_FILES);
            //else
            filenavigator.getFiles(fileIndex);
          }
        }
        else if(recdat.data[0] == 4) //Page Up
        {
          //SERIAL_ECHOLNPGM_P(PSTR("Refresh"));
          injectCommands_P(PSTR("M22\nM21"));
        }
      }
      break;

    case VolumeDisplay:
    {
      //SERIAL_ECHOLN("VolumeDisplay");
      if(recdat.data[0]==0) {
        Settings.display_volume = 0;
        Settings.display_sound = false;
      } else if (recdat.data[0] > 100) {
        Settings.display_volume = 255;
        Settings.display_sound = true;
      } else {

        Settings.display_volume = (uint8_t)map(constrain(recdat.data[0], 0, 100), 0, 100, 0, 255);
        Settings.display_sound = true;
      }
      SetTouchScreenConfiguration();
      break;
    }

    case DisplayBrightness:
    {
      //SERIAL_ECHOLN("DisplayBrightness");
      //SERIAL_ECHOLNPGM("DisplayBrightness LCD: ", recdat.data[0]);
      if(recdat.data[0]<10) {
        Settings.screen_brightness = 10;
      } else if (recdat.data[0] > 100) {
        Settings.screen_brightness = 100;
      } else {
        Settings.screen_brightness = (uint8_t)recdat.data[0];
      }
      //SERIAL_ECHOLNPGM("DisplayBrightness Set: ", Settings.screen_brightness);
      SetTouchScreenConfiguration();
      break;
    }

    case DisplayStandbyBrightness:
    {
      //SERIAL_ECHOLN("DisplayStandbyBrightness");
      if(recdat.data[0]<10) {
        Settings.standby_screen_brightness = 10;
      } else if (recdat.data[0] > 100) {
        Settings.standby_screen_brightness = 100;
      } else {
        Settings.standby_screen_brightness = (uint8_t)recdat.data[0];
      }
      SetTouchScreenConfiguration();
      break;
    }

    case DisplayStandbySeconds:
    {
      //SERIAL_ECHOLN("DisplayStandbySeconds");
      if(recdat.data[0]<5) {
        Settings.standby_time_seconds = 5;
      } else if (recdat.data[0] > 100) {
        Settings.standby_time_seconds = 100;
      } else {
        Settings.standby_time_seconds = (uint8_t)recdat.data[0];
      }
      SetTouchScreenConfiguration();
      break;
    }

    case AutolevelVal:
    {
      uint8_t meshPoint = (recdat.addr - AutolevelVal) / 2;
      uint8_t yPnt = floor(meshPoint / GRID_MAX_POINTS_X);
      uint8_t xPnt;
      if ( yPnt % 2 == 0)
        xPnt = meshPoint - (yPnt*GRID_MAX_POINTS_X);
      else
        xPnt = (GRID_MAX_POINTS_X - 1)- (meshPoint - (yPnt*GRID_MAX_POINTS_X)); //zag row
      float meshVal;
      //SERIAL_ECHOLNPGM("meshPoint ", meshPoint);
      //SERIAL_ECHOLNPGM("xPnt ", xPnt);
      //SERIAL_ECHOLNPGM("yPnt ", yPnt);

      if (recdat.data[0] >= 32768)
        meshVal = ((float)recdat.data[0] - 65536) / 1000;
      else
        meshVal = ((float)recdat.data[0]) / 1000;

      //SERIAL_ECHOLNPGM("meshVal ", meshVal);
      meshVal = constrain(meshVal, Z_PROBE_LOW_POINT, Z_CLEARANCE_BETWEEN_PROBES);
      //SERIAL_ECHOLNPGM("Constrain meshVal ", meshVal);
      xy_uint8_t point = {xPnt, yPnt};
      setMeshPoint(point, meshVal);
      rtscheck.RTS_SndData((meshVal*1000), recdat.addr);
      break;
    }

    default:
      SERIAL_ECHOLNPGM_P(PSTR("No Match :"));
      break;
    }

	memset(&recdat, 0, sizeof(recdat));
	recdat.head[0] = FHONE;
	recdat.head[1] = FHTWO;
}


void SetTouchScreenConfiguration() {
  // Main configuration (System_Config)
  LIMIT(Settings.screen_brightness, 10, 100); // Prevent a possible all-dark screen
  LIMIT(Settings.standby_time_seconds, 10, 655); // Prevent a possible all-dark screen for standby, yet also don't go higher than the DWIN limitation

  //#define LOWRES_DWIN
  unsigned char cfg_bits = 0x0;
  //#if ENABLED(DWINOS_4)
    cfg_bits |= 1UL << 7; // 7: Enable Control
  //#endif
  cfg_bits |= 1UL << 5; // 5: load 22 touch file
  cfg_bits |= 1UL << 4; // 4: auto-upload should always be enabled
  if (Settings.display_sound) cfg_bits |= 1UL << 3; // 3: audio
  if (Settings.display_standby) cfg_bits |= 1UL << 2; // 2: backlight on standby
  if(Settings.screen_rotation==10) cfg_bits |= 1UL << 1; // 1 & 0: Inversion
  #if DISABLED(LOWRES_DWIN)
    cfg_bits |= 1UL << 0; // Portrait Mode or 800x480 display has 0 point rotated 90deg from 480x272 display
  #endif



  #if ENABLED(DWINOS_4)
    const unsigned char config_set[] = { 0x5A, 0x00, (unsigned char) (cfg_bits >> 8U), (unsigned char) (cfg_bits & 0xFFU) };
  #else
    const unsigned char config_set[] = { 0x5A, 0x00, 0xFF, cfg_bits };
  #endif
  rtscheck.WriteVariable(0x80 /*System_Config*/, config_set, sizeof(config_set));

  // Standby brightness (LED_Config)
  uint16_t dwinStandbyTimeSeconds = 100 * Settings.standby_time_seconds;  /* milliseconds, but divided by 10 (not 5 like the docs say) */
  const unsigned char brightness_set[] = {
    Settings.screen_brightness /*% active*/,
    Settings.standby_screen_brightness /*% standby*/,
    static_cast<uint8_t>(dwinStandbyTimeSeconds >> 8),
    static_cast<uint8_t>(dwinStandbyTimeSeconds)
  };
  rtscheck.WriteVariable(0x82 /*LED_Config*/, brightness_set, sizeof(brightness_set));

  if (!Settings.display_sound)
  {
    rtscheck.RTS_SndData(0, VolumeIcon);
    rtscheck.RTS_SndData(9, SoundIcon);
  }
  else
  {
    rtscheck.RTS_SndData((Settings.display_volume + 1) / 32 - 1, VolumeIcon);
    rtscheck.RTS_SndData(8, SoundIcon);
  }
  rtscheck.RTS_SndData(Settings.display_volume, VolumeIcon - 2);
  rtscheck.RTS_SndData(Settings.display_volume << 8, SoundAddr + 1);
  rtscheck.RTS_SndData(map(constrain(Settings.display_volume, 0, 255), 0, 255, 0, 100), VolumeDisplay);
  rtscheck.RTS_SndData(Settings.screen_brightness, DisplayBrightness);
  rtscheck.RTS_SndData(Settings.standby_screen_brightness, DisplayStandbyBrightness);
  rtscheck.RTS_SndData(Settings.standby_time_seconds, DisplayStandbySeconds);
  if(Settings.display_standby)
    rtscheck.RTS_SndData(3, DisplayStandbyEnableIndicator);
  else
    rtscheck.RTS_SndData(2, DisplayStandbyEnableIndicator);
}

void onPrinterKilled(FSTR_P const error, FSTR_P const component) {
  SERIAL_ECHOLNPGM_P(PSTR("***kill***"));
	rtscheck.RTS_SndData(ExchangePageBase + 88, ExchangepageAddr);
  int j = 0;
  char outmsg[40];
  char killMsg[strlen_P(FTOP(error)) + strlen_P(FTOP(component)) + 3];
  sprintf_P(killMsg, PSTR(S_FMT ": " S_FMT), FTOP(error), FTOP(component));
  while (j<4)
	{
    outmsg[j] = '*';
    j++;
  }
  while (const char c = killMsg[j-4]) {
    outmsg[j] = c;
    j++;
  }
  while(j<40)
	{
    outmsg[j] = '*';
    j++;
  }
  rtscheck.RTS_SndData(outmsg, MacVersion);
  delay_ms(10);
}

void onMediaMounted()
{
	//SERIAL_ECHOLNPGM_P(PSTR("***Initing card is OK***"));
  filenavigator.reset();
  filenavigator.getFiles(0);
  fileIndex = 0;
  recordcount = 0;
}

void onMediaError()
{
  filenavigator.reset();
  for (int i = 0; i < MaxFileNumber; i++)
  {
    for (int j = 0; j < 10; j++)
      rtscheck.RTS_SndData(0, SDFILE_ADDR + i * 10 + j);
  }

  for (int j = 0; j < 10; j++)
  {
    rtscheck.RTS_SndData(0, Printfilename + j);  //clean screen.
    rtscheck.RTS_SndData(0, Choosefilename + j); //clean filename
  }
  for (int j = 0; j < 8; j++)
    rtscheck.RTS_SndData(0, FilenameCount + j);
  for (int j = 1; j <= MaxFileNumber; j++)
  {
    rtscheck.RTS_SndData(10, FilenameIcon + j);
    rtscheck.RTS_SndData(10, FilenameIcon1 + j);
  }
  return;
	//SERIAL_ECHOLNPGM_P(PSTR("***Initing card fails***"));
}

void onMediaRemoved()
{
  filenavigator.reset();
  for (int i = 0; i < MaxFileNumber; i++)
  {
    for (int j = 0; j < 10; j++)
      rtscheck.RTS_SndData(0, SDFILE_ADDR + i * 10 + j);
  }

  for (int j = 0; j < 10; j++)
  {
    rtscheck.RTS_SndData(0, Printfilename + j);  //clean screen.
    rtscheck.RTS_SndData(0, Choosefilename + j); //clean filename
  }
  for (int j = 0; j < 8; j++)
    rtscheck.RTS_SndData(0, FilenameCount + j);
  for (int j = 1; j <= MaxFileNumber; j++)
  {
    rtscheck.RTS_SndData(10, FilenameIcon + j);
    rtscheck.RTS_SndData(10, FilenameIcon1 + j);
  }
  return;
	//SERIAL_ECHOLN("***Card Removed***");
}

void onPlayTone(const uint16_t frequency, const uint16_t duration) {
	//SERIAL_ECHOLNPGM_P(PSTR("***CPlay Tone***"));
  rtscheck.RTS_SndData(StartSoundSet, SoundAddr);
}

void onPrintTimerStarted()
{
	//SERIAL_ECHOLNPGM_P(PSTR("==onPrintTimerStarted=="));
  if ( waitway == 7 )
    return;
	PrinterStatusKey[1] = 3;
	delay_ms(1);
	rtscheck.RTS_SndData(ExchangePageBase + 53, ExchangepageAddr);
}

void onPrintTimerPaused()
{
	//SERIAL_ECHOLNPGM_P(PSTR("==onPrintTimerPaused=="));
	rtscheck.RTS_SndData(ExchangePageBase + 78, ExchangepageAddr); //Display Pause Screen
  onStatusChanged("Pausing...");
}
void onPrintTimerStopped()
{
	//SERIAL_ECHOLNPGM_P(PSTR("==onPrintTimerStopped=="));
  if(waitway == 3)
    return;

#if FAN_COUNT > 0
	for (uint8_t i = 0; i < FAN_COUNT; i++)
		setTargetFan_percent(FanOff, (fan_t)i);
#endif

	PrinterStatusKey[0] = 0;
	rtscheck.RTS_SndData(ExchangePageBase + 51, ExchangepageAddr);
}

void onFilamentRunout()
{
	//SERIAL_ECHOLNPGM_P(PSTR("==onFilamentRunout=="));
	PrinterStatusKey[1] = 4;
  rtscheck.RTS_SndData(ExchangePageBase + 78, ExchangepageAddr);
}
void onFilamentRunout(extruder_t extruder)
{
	//SERIAL_ECHOLNPGM_P(PSTR("==onFilamentRunout=="));
  PrinterStatusKey[1] = 4;
  rtscheck.RTS_SndData(ExchangePageBase + 78, ExchangepageAddr);
}
void onUserConfirmRequired(const char *const msg)
{
  PrinterStatusKey[1] = 4;
  if(lastPauseMsgState==ExtUI::pauseModeStatus && msg == (const char*)GET_TEXT_F(MSG_FILAMENT_CHANGE_LOAD))
    return;

  switch(ExtUI::pauseModeStatus)
  {
    case PAUSE_MESSAGE_WAITING:
    {
      rtscheck.RTS_SndData(ExchangePageBase + 78, ExchangepageAddr);
      onStatusChanged("Press Yes to Continue");
      break;
    }
    case PAUSE_MESSAGE_INSERT:
    {
      rtscheck.RTS_SndData(ExchangePageBase + 78, ExchangepageAddr);
      onStatusChanged("Load Filament to           Continue");
      break;
    }
    case PAUSE_MESSAGE_HEAT:
    {
      rtscheck.RTS_SndData(ExchangePageBase + 78, ExchangepageAddr);
      onStatusChanged("Add Filament and Press    Yes to Reheat");
      break;
    }
    #if DISABLED(ADVANCED_PAUSE_CONTINUOUS_PURGE)
      case PAUSE_MESSAGE_PURGE:
      {
        rtscheck.RTS_SndData(ExchangePageBase + 78, ExchangepageAddr);
        char newMsg[40] = "Yes to ";
        if(TERN1(FILAMENT_RUNOUT_SENSOR, (!ExtUI::getFilamentRunoutState() && getFilamentRunoutEnabled())))
          strcat(newMsg, "Continue");
        else
          strcat(newMsg, "Disable ");

        strcat(newMsg, "           No to Purge");
        onStatusChanged(newMsg);
        break;
      }
    #endif


    case PAUSE_MESSAGE_OPTION:
    {
      rtscheck.RTS_SndData(ExchangePageBase + 78, ExchangepageAddr);
      char newMsg[40] = "Yes to ";
        if(TERN1(FILAMENT_RUNOUT_SENSOR, (!ExtUI::getFilamentRunoutState() && getFilamentRunoutEnabled())))
          strcat(newMsg, "Continue");
        else
          strcat(newMsg, "Disable ");

        strcat(newMsg, "           No to Purge");
      onStatusChanged(newMsg);
      break;
    }

    case PAUSE_MESSAGE_PARKING:
    {
      rtscheck.RTS_SndData(ExchangePageBase + 78, ExchangepageAddr);
      onStatusChanged("Parking...");
      break;
    }
    case PAUSE_MESSAGE_CHANGING:{
      rtscheck.RTS_SndData(ExchangePageBase + 78, ExchangepageAddr);
      onStatusChanged("Beginning Filament Change");
      break;
    }
    case PAUSE_MESSAGE_UNLOAD:{
      rtscheck.RTS_SndData(ExchangePageBase + 78, ExchangepageAddr);
      onStatusChanged("Unloading...");
      break;
    }
    case PAUSE_MESSAGE_LOAD:{
      rtscheck.RTS_SndData(ExchangePageBase + 78, ExchangepageAddr);
      onStatusChanged("Reloading...");
      break;
    }
    case PAUSE_MESSAGE_RESUME:
    #if ENABLED(ADVANCED_PAUSE_CONTINUOUS_PURGE)
      case PAUSE_MESSAGE_PURGE:{
        rtscheck.RTS_SndData(ExchangePageBase + 78, ExchangepageAddr);
        onStatusChanged("Press Yes to Stop Purge");
        break;
      }
    #endif

    case PAUSE_MESSAGE_HEATING:
    {
      rtscheck.RTS_SndData(ExchangePageBase + 68, ExchangepageAddr);
      onStatusChanged("Reheating");
      break;
    }

    case PAUSE_MESSAGE_STATUS:
      default:
      {
        setPauseMenuResponse(PAUSE_RESPONSE_RESUME_PRINT);
        setUserConfirmed();
        //SERIAL_ECHOLNPGM_P(PSTR("Pause Mode Status"));
        break;
      }
  }
  lastPauseMsgState = ExtUI::pauseModeStatus;
	//SERIAL_ECHOLNPGM_P(PSTR("==onUserConfirmRequired=="), pauseModeStatus);
}

void onStatusChanged(const char *const statMsg)
{
  for (int j = 0; j < 20; j++) // Clear old message
    rtscheck.RTS_SndData(' ', StatusMessageString+j);
  rtscheck.RTS_SndData(statMsg, StatusMessageString);
}
void onFactoryReset()
{
  Settings.settings_size = sizeof(creality_dwin_settings_t);
  Settings.settings_version = dwin_settings_version;
  Settings.display_standby = true;
  Settings.display_sound = true;
  Settings.display_volume = 32;
  Settings.standby_screen_brightness = 15;
  Settings.screen_brightness = 100;
  Settings.standby_time_seconds = 60;
  Settings.screen_rotation = 0;
  onStartup();
  startprogress = 0;
	//SERIAL_ECHOLNPGM_P(PSTR("==onFactoryReset=="));

}

void onMeshUpdate(const int8_t xpos, const int8_t ypos, probe_state_t state) {

}

void onMeshUpdate(const int8_t xpos, const int8_t ypos, const_float_t zval)
{
  if(waitway==3)
    if(isPositionKnown() && (getActualTemp_celsius(BED) >= (getTargetTemp_celsius(BED)-1)))
			  rtscheck.RTS_SndData(ExchangePageBase + 64, ExchangepageAddr);
  #if HAS_MESH
    uint8_t abl_probe_index = 0;
        for(uint8_t outer = 0; outer < GRID_MAX_POINTS_Y; outer++)
        {
          for (uint8_t inner = 0; inner < GRID_MAX_POINTS_X; inner++)
          {
            uint8_t x_Point = inner;
            bool zig = (outer & 1); // != ((PR_OUTER_END) & 1);
            if (zig) x_Point = (GRID_MAX_POINTS_X - 1) - inner;
            xy_uint8_t point = {x_Point, outer};
            if(x_Point==xpos && outer ==ypos)
              rtscheck.RTS_SndData(ExtUI::getMeshPoint(point) * 1000, AutolevelVal + (abl_probe_index * 2));
            ++abl_probe_index;
          }
        }
  #endif
};

void onStoreSettings(char *buff)
{
	static_assert(
    ExtUI::eeprom_data_size >= sizeof(creality_dwin_settings_t),
    "Insufficient space in EEPROM for UI parameters"
  );

  // Write to buffer
  //SERIAL_ECHOLNPGM("Saving DWIN LCD setting from EEPROM");
  memcpy(buff, &Settings, sizeof(creality_dwin_settings_t));
}

void onLoadSettings(const char *buff)
{
	static_assert(
    ExtUI::eeprom_data_size >= sizeof(creality_dwin_settings_t),
    "Insufficient space in EEPROM for UI parameters"
  );

  creality_dwin_settings_t eepromSettings;
  memcpy(&eepromSettings, buff, sizeof(creality_dwin_settings_t));

  // If size is not the same, discard settings
  if (eepromSettings.settings_size != sizeof(creality_dwin_settings_t)) {
    //SERIAL_ECHOLNPGM("Discarding DWIN LCD setting from EEPROM - size incorrect");

    onFactoryReset();
    return;
  }

  if (eepromSettings.settings_version != dwin_settings_version) {
    //SERIAL_ECHOLNPGM("Discarding DWIN LCD setting from EEPROM - settings version incorrect");

    onFactoryReset();
    return;
  }

  // Copy into final location
  //SERIAL_ECHOLNPGM("Loading DWIN LCD setting from EEPROM");
  memcpy(&Settings, &eepromSettings, sizeof(creality_dwin_settings_t));

  //SERIAL_ECHOLNPGM("Setting Brightness : ", Settings.screen_brightness);
  //SERIAL_ECHOLNPGM("Setting Standby : ", Settings.standby_screen_brightness);
  //SERIAL_ECHOLNPGM("Setting Standby Time : ", Settings.standby_time_seconds);
  //SERIAL_ECHOLNPGM("Setting Rotation : ", Settings.screen_rotation);
  //SERIAL_ECHOLNPGM("Setting Volume : ", Settings.display_volume);

  //SERIAL_ECHOLNPGM("Setting Standby On : ", Settings.display_standby);
  //SERIAL_ECHOLNPGM("Setting Volume On : ", Settings.display_sound);

  SetTouchScreenConfiguration();
}

void onSettingsStored(bool success)
{
	//SERIAL_ECHOLNPGM_P(PSTR("==onSettingsStored=="));
	// This is called after the entire EEPROM has been written,
	// whether successful or not.
}

void onSettingsLoaded(bool success)
{
	//SERIAL_ECHOLNPGM_P(PSTR("==onConfigurationStoreRead=="));
  #if HAS_MESH
    if (ExtUI::getLevelingIsValid())
    {
      uint8_t abl_probe_index = 0;
        for(uint8_t outer = 0; outer < GRID_MAX_POINTS_Y; outer++)
        {
          for (uint8_t inner = 0; inner < GRID_MAX_POINTS_X; inner++)
          {
            uint8_t x_Point = inner;
            bool zig = (outer & 1);
            if (zig) x_Point = (GRID_MAX_POINTS_X - 1) - inner;
            xy_uint8_t point = {x_Point, outer};
            rtscheck.RTS_SndData(ExtUI::getMeshPoint(point) * 1000, AutolevelVal + (abl_probe_index * 2));
            ++abl_probe_index;
          }
        }

      rtscheck.RTS_SndData(3, AutoLevelIcon); //2=On, 3=Off
      setLevelingActive(true);
    }
    else
    {
      rtscheck.RTS_SndData(2, AutoLevelIcon); /*Off*/
      setLevelingActive(false);
    }
  #endif

	//SERIAL_ECHOLNPGM("\n init zprobe_zoffset = ", getZOffset_mm());
	rtscheck.RTS_SndData(getZOffset_mm() * 100, ProbeOffset_Z);
  SetTouchScreenConfiguration();
}

#if ENABLED(POWER_LOSS_RECOVERY)
  void onPowerLossResume() {
    //SERIAL_ECHOLNPGM_P(PSTR("==OnPowerLossResume=="));
    startprogress = 254;
    reEntryPrevent = false;
    rtscheck.RTS_SndData(ExchangePageBase + 76, ExchangepageAddr);
  }
#endif

#if HAS_PID_HEATING
  void onPIDTuning(const pidresult_t rst) {
    // Called for temperature PID tuning result
    rtscheck.RTS_SndData(pid_hotendAutoTemp, HotendPID_AutoTmp);
    rtscheck.RTS_SndData(pid_bedAutoTemp, BedPID_AutoTmp);
    rtscheck.RTS_SndData((unsigned int)(getPID_Kp(E0) * 10), HotendPID_P);
    rtscheck.RTS_SndData((unsigned int)(getPID_Ki(E0) * 10), HotendPID_I);
    rtscheck.RTS_SndData((unsigned int)(getPID_Kd(E0) * 10), HotendPID_D);
    #if ENABLED(PIDTEMPBED)
      rtscheck.RTS_SndData((unsigned int)(getBedPID_Kp() * 10), BedPID_P);
      rtscheck.RTS_SndData((unsigned int)(getBedPID_Ki() * 10), BedPID_I);
      rtscheck.RTS_SndData((unsigned int)(getBedPID_Kd() * 10), BedPID_D);
    #endif
    onStatusChanged("PID Tune Finished");
  }
#endif
void onLevelingStart() {

}

void onLevelingDone() {
  #if HAS_MESH
    if (ExtUI::getLevelingIsValid())
    {
      uint8_t abl_probe_index = 0;
        for(uint8_t outer = 0; outer < GRID_MAX_POINTS_Y; outer++)
        {
          for (uint8_t inner = 0; inner < GRID_MAX_POINTS_X; inner++)
          {
            uint8_t x_Point = inner;
            bool zig = (outer & 1);
            if (zig) x_Point = (GRID_MAX_POINTS_X - 1) - inner;
            xy_uint8_t point = {x_Point, outer};
            rtscheck.RTS_SndData(ExtUI::getMeshPoint(point) * 1000, AutolevelVal + (abl_probe_index * 2));
            ++abl_probe_index;
          }
        }

      rtscheck.RTS_SndData(3, AutoLevelIcon); //2=On, 3=Off
      setLevelingActive(true);
    }
    else
    {
      rtscheck.RTS_SndData(2, AutoLevelIcon); /*Off*/
      setLevelingActive(false);
    }
  #endif
}

void onSteppersEnabled()
{

}

void onPrintDone()
{

}

void onHomingStart()
{

}

void onHomingDone()
{

}

void onSteppersDisabled()
{

}

void onPostprocessSettings()
{

}

void onSetPowerLoss(const bool onoff)
{

}
void onPowerLoss()
{

}

void onHeatingError(signed char)
{

}

void onStartM303(int, signed char, int) {

}

void onAxisEnabled(ExtUI::axis_t) {

}
void onAxisDisabled(ExtUI::axis_t){}
void onSetMinExtrusionTemp(int){}
void onMinTempError(signed char){}
void onMaxTempError(signed char){}

void onPauseMode(PauseMessage, PauseMode, unsigned char) {}

void onSetMinExtrusionTemp(short){}
void onFirmwareFlash(){};
void onMaxTempError(int){};
void onMinTempError(int){};
void onHeatingError(int){};
void onStartM303(int, int, short){};


} // namespace ExtUI

#endif