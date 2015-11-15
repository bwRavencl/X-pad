/* Copyright (C) 2015  Matteo Hausner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "XPLMDataAccess.h"
#include "XPLMDisplay.h"
#include "XPLMMenus.h"
#include "XPLMPlanes.h"
#include "XPLMPlugin.h"
#include "XPLMProcessing.h"
#include "XPLMUtilities.h"
#include "XPStandardWidgets.h"
#include "XPWidgets.h"

#include <fstream>
#include <sstream>
#include <stack>

#if IBM
#include <Windows.h>
#elif APL
#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>
#elif LIN
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#endif

// define name
#define NAME "X-pad"
#define NAME_LOWERCASE "x_pad"

// define version
#define VERSION "0.9"

// define config file path
#if IBM
#define CONFIG_PATH ".\\Resources\\plugins\\" NAME_LOWERCASE "\\" NAME_LOWERCASE ".ini"
struct ControllerStruct
{
    unsigned long eventCount;
    unsigned short up : 1, down : 1, left : 1, right : 1, start : 1, back : 1, l3 : 1, r3 : 1, lButton : 1, rButton : 1, guideButton : 1, unknown : 1, aButton : 1, bButton : 1, xButton : 1, yButton : 1;
    unsigned char lTrigger;
    unsigned char rTrigger;
    short lJoyY;
    short lJoyx;
    short rJoyY;
    short rJoyX;
};
#else
#define CONFIG_PATH "./Resources/plugins/" NAME_LOWERCASE "/" NAME_LOWERCASE ".ini"
#endif

// define controller types
#define CONTROLLER_TYPE_DS3 0
#define CONTROLLER_TYPE_XBOX360 1

// define joystick axis
#define JOYSTICK_AXIS_ABSTRACT_LEFT_X 0
#define JOYSTICK_AXIS_ABSTRACT_LEFT_Y 1
#define JOYSTICK_AXIS_ABSTRACT_RIGHT_X 2
#define JOYSTICK_AXIS_ABSTRACT_RIGHT_Y 3

// define dualshock 3 joystick buttons
#define JOYSTICK_AXIS_DS3_LEFT_X 0
#define JOYSTICK_AXIS_DS3_LEFT_Y 1
#define JOYSTICK_AXIS_DS3_RIGHT_X 2
#define JOYSTICK_AXIS_DS3_RIGHT_Y 3

// define xbox 360 joystick buttons
#define JOYSTICK_AXIS_XBOX360_LEFT_X 1
#define JOYSTICK_AXIS_XBOX360_LEFT_Y 0
#define JOYSTICK_AXIS_XBOX360_RIGHT_X 3
#define JOYSTICK_AXIS_XBOX360_RIGHT_Y 2
#define JOYSTICK_AXIS_XBOX360_TRIGGERS 4

// define abstract joystick buttons
#define JOYSTICK_BUTTON_ABSTRACT_DPAD_LEFT 0
#define JOYSTICK_BUTTON_ABSTRACT_DPAD_RIGHT 1
#define JOYSTICK_BUTTON_ABSTRACT_DPAD_UP 2
#define JOYSTICK_BUTTON_ABSTRACT_DPAD_DOWN 3
#define JOYSTICK_BUTTON_ABSTRACT_FACE_LEFT 4
#define JOYSTICK_BUTTON_ABSTRACT_FACE_RIGHT 5
#define JOYSTICK_BUTTON_ABSTRACT_FACE_UP 6
#define JOYSTICK_BUTTON_ABSTRACT_FACE_DOWN 7
#define JOYSTICK_BUTTON_ABSTRACT_CENTER_LEFT 8
#define JOYSTICK_BUTTON_ABSTRACT_CENTER_RIGHT 10
#define JOYSTICK_BUTTON_ABSTRACT_BUMPER_LEFT 11
#define JOYSTICK_BUTTON_ABSTRACT_BUMPER_RIGHT 12
#define JOYSTICK_BUTTON_ABSTRACT_STICK_LEFT 15
#define JOYSTICK_BUTTON_ABSTRACT_STICK_RIGHT 16

// define dualshock 3 joystick buttons
#define JOYSTICK_BUTTON_DS3_DPAD_LEFT 7
#define JOYSTICK_BUTTON_DS3_DPAD_RIGHT 5
#define JOYSTICK_BUTTON_DS3_DPAD_UP 4
#define JOYSTICK_BUTTON_DS3_DPAD_DOWN 6
#define JOYSTICK_BUTTON_DS3_SQUARE 15
#define JOYSTICK_BUTTON_DS3_CIRCLE 13
#define JOYSTICK_BUTTON_DS3_TRIANGLE 12
#define JOYSTICK_BUTTON_DS3_CROSS 14
#define JOYSTICK_BUTTON_DS3_START 3
#define JOYSTICK_BUTTON_DS3_PS 16
#define JOYSTICK_BUTTON_DS3_SELECT 0
#define JOYSTICK_BUTTON_DS3_L1 10
#define JOYSTICK_BUTTON_DS3_R1 11
#define JOYSTICK_BUTTON_DS3_L2 8
#define JOYSTICK_BUTTON_DS3_R2 9
#define JOYSTICK_BUTTON_DS3_L3 1
#define JOYSTICK_BUTTON_DS3_R3 2

// define xbox 360 joystick buttons
#define JOYSTICK_BUTTON_XBOX360_DPAD_LEFT 16
#define JOYSTICK_BUTTON_XBOX360_DPAD_RIGHT 12
#define JOYSTICK_BUTTON_XBOX360_DPAD_UP 10
#define JOYSTICK_BUTTON_XBOX360_DPAD_DOWN 14
#define JOYSTICK_BUTTON_XBOX360_X 2
#define JOYSTICK_BUTTON_XBOX360_B 1
#define JOYSTICK_BUTTON_XBOX360_Y 3
#define JOYSTICK_BUTTON_XBOX360_A 0
#define JOYSTICK_BUTTON_XBOX360_START 7
#if !IBM
#define JOYSTICK_BUTTON_XBOX360_GUIDE 17
#endif
#define JOYSTICK_BUTTON_XBOX360_BACK 6
#define JOYSTICK_BUTTON_XBOX360_LEFT_BUMPER 4
#define JOYSTICK_BUTTON_XBOX360_RIGHT_BUMPER 5
#define JOYSTICK_BUTTON_XBOX360_LEFT_STICK 8
#define JOYSTICK_BUTTON_XBOX360_RIGHT_STICK 9

// define view types
#define VIEW_TYPE_FORWARDS_WITH_PANEL 1000
#define VIEW_TYPE_CHASE 1017
#define VIEW_TYPE_FORWARDS_WITH_HUD 1023
#define VIEW_TYPE_3D_COCKPIT_COMMAND_LOOK 1026

// define axis assignments
#define AXIS_ASSIGNMENT_NONE 0
#define AXIS_ASSIGNMENT_PITCH 1
#define AXIS_ASSIGNMENT_ROLL 2
#define AXIS_ASSIGNMENT_YAW 3
#define AXIS_ASSIGNMENT_VIEW_LEFT_RIGHT 41
#define AXIS_ASSIGNMENT_VIEW_UP_DOWN 42

// define default nullzone
#define DEFAULT_NULLZONE 0.15f

// define '.acf' file 'show cockpit object in: 2-d forward panel views' string
#define ACF_STRING_SHOW_COCKPIT_OBJECT_IN_2D_FORWARD_PANEL_VIEWS "P acf/_new_plot_XP3D_cock/0 1"

// define plugin signatures
#define BLU_FX_PLUGIN_SIGNATURE "de.bwravencl.blu_fx"
#define DREAMFOIL_AS350_PLUGIN_SIGNATURE "DreamFoil.AS350"
#define DREAMFOIL_B407_PLUGIN_SIGNATURE "DreamFoil.B407"
#define QPAC_A320_PLUGIN_SIGNATURE "QPAC.airbus.fbw"
#define ROTORSIM_EC135_PLUGIN_SIGNATURE "rotorsim.ec135.management"
#define X737_PLUGIN_SIGNATURE "bs.x737.plugin"
#define X_IVAP_PLUGIN_SIGNATURE "ivao.xivap"
#define X_XSQUAWKBOX_PLUGIN_SIGNATURE "vatsim.protodev.clients.xsquawkbox"

// define custom command names
#define CYCLE_RESET_VIEW_COMMAND NAME_LOWERCASE "/cycle_reset_view"
#define TOGGLE_ARM_SPEED_BRAKE_OR_TOGGLE_CARB_HEAT_COMMAND NAME_LOWERCASE "/toggle_arm_speed_brake_or_toggle_carb_heat"
#define TOGGLE_AUTOPILOT_OR_DISABLE_FLIGHT_DIRECTOR_COMMAND NAME_LOWERCASE "/toggle_autopilot_or_disable_flight_director"
#define VIEW_MODIFIER_COMMAND NAME_LOWERCASE "/view_modifier"
#define PROP_PITCH_THROTTLE_MODIFIER_COMMAND NAME_LOWERCASE "/prop_pitch_throttle_modifier"
#define MIXTURE_CONTROL_MODIFIER_COMMAND NAME_LOWERCASE "/mixture_control_modifier"
#define COWL_FLAP_MODIFIER_COMMAND NAME_LOWERCASE "/cowl_flap_modifier"
#define TRIM_MODIFIER_COMMAND NAME_LOWERCASE "/trim_modifier"
#define TRIM_RESET_COMMAND_NAME_LOWERCASE "/trim_reset"
#define TOGGLE_BETA_OR_TOGGLE_REVERSE_COMMAND NAME_LOWERCASE "/toggle_beta_or_toggle_reverse"
#define TOGGLE_MOUSE_POINTER_CONTROL_COMMAND NAME_LOWERCASE "/toggle_mouse_pointer_control"
#define PUSH_TO_TALK_COMMAND NAME_LOWERCASE "/push_to_talk"
#define TOGGLE_BRAKES_COMMAND NAME_LOWERCASE "/toggle_brakes"

// define auto-center view limit
#define AUTO_CENTER_VIEW_DISTANCE_LIMIT 0.05f
#define AUTO_CENTER_VIEW_ANGLE_LIMIT 6.0f

// define long press time
#define BUTTON_LONG_PRESS_TIME 1.0f

// define disable view heading time
#define DISABLE_VIEW_HEADING_TIME 3.0f

// define relative control multiplier
#define JOYSTICK_RELATIVE_CONTROL_MULTIPLIER 1.0f

// define collective control multiplier
#define COLLECTIVE_CONTROL_MULTIPLIER 0.2f

// define joystick sensitivity values
#define JOYSTICK_VIEW_SENSITIVITY 3.5f
#define JOYSTICK_MOUSE_POINTER_SENSITIVITY 30.0f

// define mouse buttons
#define MOUSE_BUTTON_LEFT 0
#define MOUSE_BUTTON_RIGHT 1

// hardcoded '.acf' files that have no 2-d panel
static const char* ACF_WITHOUT2D_PANEL[] = {"727-100.acf", "727-200Adv.acf", "727-200F.acf", "ATR72.acf"};

// global internal variables
static int controllerType = CONTROLLER_TYPE_XBOX360, axisOffset = 0, buttonOffset = 0, viewModifierDown = 0, propPitchThrottleModifierDown = 0, mixtureControlModifierDown = 0, cowlFlapModifierDown = 0, trimModifierDown = 0, mousePointerControlEnabled = 0, switchTo3DCommandLook = 0, lastMouseX = 0, lastMouseY = 0, bringFakeWindowToFront = 0, overrideControlCinemaVeriteFailed = 0, lastCinemaVerite = 0;
static float lastMouseUsageTime = 0.0f, lastViewModifierUsageTime = 0.0f, defaultHeadPositionX = FLT_MAX, defaultHeadPositionY = FLT_MAX, defaultHeadPositionZ = FLT_MAX;
static XPLMWindowID fakeWindow = NULL;
static std::stack <int*> buttonAssignmentsStack;
#if IBM
static HINSTANCE hGetProcIDDLL = NULL;
typedef int(__stdcall * pICFUNC) (int, ControllerStruct&);
pICFUNC getControllerData = NULL;
#elif LIN
static Display *display = NULL;
#endif

// global commandref variables
static XPLMCommandRef cycleResetViewCommand = NULL, toggleArmSpeedBrakeOrToggleCarbHeatCommand = NULL, toggleAutopilotOrDisableFlightDirectorCommand = NULL, viewModifierCommand = NULL, propPitchOrThrottleModifierCommand = NULL, mixtureControlModifierCommand = NULL, cowlFlapModifierCommand = NULL, trimModifierCommand = NULL, trimResetCommand = NULL, toggleBetaOrToggleReverseCommand = NULL, toggleMousePointerControlCommand = NULL, pushToTalkCommand = NULL, toggleBrakesCommand = NULL;

// global dataref variables
static XPLMDataRef acfCockpitTypeDataRef = NULL, acfPeXDataRef = NULL, acfPeYDataRef = NULL, acfPeZDataRef = NULL, acfRSCMingovPrpDataRef = NULL, acfRSCRedlinePrpDataRef = NULL, acfNumEnginesDataRef = NULL, acfHasBetaDataRef = NULL, acfSbrkEQDataRef = NULL, acfMinPitchDataRef = NULL, acfMaxPitchDataRef = NULL, acfVertCantDataRef = NULL, ongroundAnyDataRef = NULL, groundspeedDataRef = NULL, cinemaVeriteDataRef = NULL, pilotsHeadPsiDataRef = NULL, pilotsHeadTheDataRef = NULL, viewTypeDataRef = NULL, hasJostickDataRef = NULL, joystickPitchNullzoneDataRef = NULL, joystickRollNullzoneDataRef = NULL, joystickHeadingNullzoneDataRef = NULL, joystickAxisAssignmentsDataRef = NULL, joystickAxisReverseDataRef = NULL, joystickAxisValuesDataRef = NULL, joystickButtonAssignmentsDataRef = NULL, joystickButtonValuesDataRef = NULL, leftBrakeRatioDataRef = NULL, rightBrakeRatioDataRef = NULL, speedbrakeRatioDataRef = NULL, aileronTrimDataRef = NULL, elevatorTrimDataRef = NULL, rudderTrimDataRef = NULL, throttleRatioAllDataRef = NULL, propPitchDegDataRef = NULL, propRotationSpeedRadSecAllDataRef = NULL, mixtureRatioAllDataRef = NULL, carbHeatRatioDataRef = NULL, cowlFlapRatioDataRef = NULL, thrustReverserDeployRatioDataRef = NULL;

// global widget variables
static XPWidgetID settingsWidget = NULL, dualShock3ControllerRadioButton = NULL, xbox360ControllerRadioButton = NULL, axisOffsetCaption = NULL, buttonOffsetCaption = NULL, axisOffsetSlider = NULL, buttonOffsetSlider = NULL, setDefaultAssignmentsButton = NULL;

// flightloop-callback that resizes and brings the fake window back to the front if needed
static float UpdateFakeWindowCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void *inRefcon)
{
    if (fakeWindow != NULL)
    {
        int x = 0, y = 0;
        XPLMGetScreenSize(&x, &y);
        XPLMSetWindowGeometry(fakeWindow, 0, y, x, 0);

        if (bringFakeWindowToFront == 0)
        {
            XPLMBringWindowToFront(fakeWindow);
            bringFakeWindowToFront = 1;
        }
    }

    return -1.0f;
}

// push the current button assignments to the stack
static void PushButtonAssignments(void)
{
    int *joystickButtonAssignments = (int*) malloc(sizeof(int) * 1600);

    XPLMGetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);
    buttonAssignmentsStack.push(joystickButtonAssignments);
}

// pop the topmost button assignments from the stack
static void PopButtonAssignments(void)
{
    if (!buttonAssignmentsStack.empty())
    {
        int *joystickButtonAssignments = buttonAssignmentsStack.top();

        if (joystickButtonAssignments != NULL)
        {
            XPLMSetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);
            free(joystickButtonAssignments);
        }
        buttonAssignmentsStack.pop();
    }
}

// returns 1 if the current aircraft does have a 2D panel, otherwise 0 is returned - for non-parseable sub 1004 version '.acf' files 1 is returned
static int Has2DPanel(void)
{
    char fileName[256], path[512];
    XPLMGetNthAircraftModel(0, fileName, path);

    // check if the path to the '.acf' file matches one of the hardcoded aircraft that have no 2D panel and for which the check below fails
    for (int i = 0; i < sizeof(ACF_WITHOUT2D_PANEL) / sizeof(char*); i++)
    {
        if (strstr(path, ACF_WITHOUT2D_PANEL[i]) != NULL)
            return 0;
    }

    int has2DPanel = 1;

    // search the '.acf' file for a special string which indicates that the aircraft shows the 3D cockpit object in the 2D forward panel view
    FILE *file = fopen(path, "r");
    if(file != NULL)
    {
        char temp[512];
        while(fgets(temp, 512, file) != NULL)
        {
            if((strstr(temp, ACF_STRING_SHOW_COCKPIT_OBJECT_IN_2D_FORWARD_PANEL_VIEWS)) != NULL)
                has2DPanel = 0;
        }

        fclose(file);
    }

    return has2DPanel;
}

// command-handler that handles the switch / reset view command
static int CycleResetViewCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    static float beginTime = 0.0f;

    if (inPhase == xplm_CommandBegin)
        beginTime = XPLMGetElapsedTime();
    else if (inPhase == xplm_CommandContinue)
    {
        // reset view
        if (XPLMGetElapsedTime() - beginTime >= BUTTON_LONG_PRESS_TIME)
        {
            switch (XPLMGetDatai(viewTypeDataRef))
            {
            case VIEW_TYPE_FORWARDS_WITH_PANEL:
                XPLMCommandOnce(XPLMFindCommand("sim/view/3d_cockpit_cmnd_look"));
                XPLMCommandOnce(XPLMFindCommand("sim/view/forward_with_panel"));
                break;

            case VIEW_TYPE_3D_COCKPIT_COMMAND_LOOK:
                XPLMCommandOnce(XPLMFindCommand("sim/view/forward_with_panel"));
                XPLMCommandOnce(XPLMFindCommand("sim/view/3d_cockpit_cmnd_look"));
                break;

            case VIEW_TYPE_CHASE:
                XPLMCommandOnce(XPLMFindCommand("sim/view/circle"));
                XPLMCommandOnce(XPLMFindCommand("sim/view/chase"));
                break;
            }

            beginTime = FLT_MAX;
        }
    }
    else if (inPhase == xplm_CommandEnd)
    {
        // cycle view
        if (XPLMGetElapsedTime() - beginTime < BUTTON_LONG_PRESS_TIME && beginTime != FLT_MAX)
        {
            switch (XPLMGetDatai(viewTypeDataRef))
            {
            case VIEW_TYPE_FORWARDS_WITH_PANEL:
                XPLMCommandOnce(XPLMFindCommand("sim/view/3d_cockpit_cmnd_look"));
                break;

            case VIEW_TYPE_3D_COCKPIT_COMMAND_LOOK:
                XPLMCommandOnce(XPLMFindCommand("sim/view/forward_with_hud"));
                break;

            case VIEW_TYPE_FORWARDS_WITH_HUD:
                XPLMCommandOnce(XPLMFindCommand("sim/view/chase"));
                break;

            case VIEW_TYPE_CHASE:
            default:
                if (Has2DPanel())
                    XPLMCommandOnce(XPLMFindCommand("sim/view/forward_with_panel"));
                else
                    XPLMCommandOnce(XPLMFindCommand("sim/view/3d_cockpit_cmnd_look"));
                break;
            }
        }

        beginTime = 0.0f;
    }

    return 0;
}

// command-handler that handles the speedbrake toggle / arm command or the carb heat, if the plane has no speedbrake
static int ToggleArmSpeedBrakeOrToggleCarbHeatCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    // if a speedbrake exists this command controls it
    if (XPLMGetDatai(acfSbrkEQDataRef) != 0)
    {
        static float beginTime = 0.0f;

        float oldSpeedbrakeRatio = XPLMGetDataf(speedbrakeRatioDataRef);

        if (inPhase == xplm_CommandBegin)
            beginTime = XPLMGetElapsedTime();
        else if (inPhase == xplm_CommandContinue)
        {
            // arm / unarm speedbrake
            if (XPLMGetElapsedTime() - beginTime >= BUTTON_LONG_PRESS_TIME)
            {
                float newSpeedbrakeRatio = oldSpeedbrakeRatio == -0.5f ? 0.0f : -0.5f;

                XPLMSetDataf(speedbrakeRatioDataRef, newSpeedbrakeRatio);

                beginTime = FLT_MAX;
            }
        }
        else if (inPhase == xplm_CommandEnd)
        {
            // toggle speedbrake
            if (XPLMGetElapsedTime() - beginTime < BUTTON_LONG_PRESS_TIME && beginTime != FLT_MAX)
            {
                float newSpeedbrakeRatio = oldSpeedbrakeRatio <= 0.5f ? 1.0f : 0.0f;

                XPLMSetDataf(speedbrakeRatioDataRef, newSpeedbrakeRatio);
            }

            beginTime = 0.0f;
        }
    }
    // if the aircraft is not equipped with a speedbrake this command toggles the carb heat
    else
    {
        if (inPhase == xplm_CommandBegin)
        {
            int acfNumEngines = XPLMGetDatai(acfNumEnginesDataRef);

            if (acfNumEngines > 0)
            {
                float* carbHeatRatio = (float*) malloc(acfNumEngines * sizeof(float));
                XPLMGetDatavf(carbHeatRatioDataRef, carbHeatRatio, 0, acfNumEngines);

                float newCarbHeatRatio = carbHeatRatio[0] <= 0.5f ? 1.0f : 0.0f;

                for (int i = 0; i < acfNumEngines; i++)
                    carbHeatRatio[i] = newCarbHeatRatio;

                XPLMSetDatavf(carbHeatRatioDataRef, carbHeatRatio, 0, acfNumEngines);
                free(carbHeatRatio);
            }
        }
    }

    return 0;
}

// check if a plugin with a given signature is enabled
static int IsPluginEnabled(const char* pluginSignature)
{
    XPLMPluginID pluginId = XPLMFindPluginBySignature(pluginSignature);

    return XPLMIsPluginEnabled(pluginId);
}

// command-handler that handles the toggle autopilot / disable flight director command - depending on the aircraft custom commands are invoked instead of the default ones
static int ToggleAutopilotOrDisableFlightDirectorCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    static float beginTime = 0.0f;

    if (inPhase == xplm_CommandBegin)
        beginTime = XPLMGetElapsedTime();
    else if (inPhase == xplm_CommandContinue)
    {
        // disable flight director
        if (XPLMGetElapsedTime() - beginTime >= BUTTON_LONG_PRESS_TIME)
        {
            // custom handling for QPAC A320
            if (IsPluginEnabled(QPAC_A320_PLUGIN_SIGNATURE) != 0)
                XPLMCommandOnce(XPLMFindCommand("sim/autopilot/servos_and_flight_dir_off"));
            // custom handling for x737
            else if (IsPluginEnabled(X737_PLUGIN_SIGNATURE) != 0)
            {
                XPLMCommandOnce(XPLMFindCommand("x737/mcp/FD_A_OFF"));
                XPLMCommandOnce(XPLMFindCommand("x737/mcp/FD_B_OFF"));
            }
            // custom handling for RotorSim EC135
            else if (IsPluginEnabled(ROTORSIM_EC135_PLUGIN_SIGNATURE) != 0)
                XPLMCommandOnce(XPLMFindCommand("ec135/autopilot/apmd_dcpl"));
            // default handling
            else
                XPLMCommandOnce(XPLMFindCommand("sim/autopilot/servos_fdir_off"));

            beginTime = FLT_MAX;
        }
    }
    else if (inPhase == xplm_CommandEnd)
    {
        // toggle autopilot
        if (XPLMGetElapsedTime() - beginTime < BUTTON_LONG_PRESS_TIME && beginTime != FLT_MAX)
        {
            // custom handling for QPAC A320
            if (IsPluginEnabled(QPAC_A320_PLUGIN_SIGNATURE) != 0)
            {
                XPLMDataRef ap1EngageDataRef = XPLMFindDataRef("AirbusFBW/AP1Engage");
                XPLMDataRef ap2EngageDataRef = XPLMFindDataRef("AirbusFBW/AP2Engage");

                if (ap1EngageDataRef != NULL && ap2EngageDataRef != NULL)
                {
                    if (XPLMGetDatai(ap1EngageDataRef) == 0 && XPLMGetDatai(ap2EngageDataRef) == 0)
                        XPLMCommandOnce(XPLMFindCommand("airbus_qpac/ap1_push"));
                    else
                        XPLMCommandOnce(XPLMFindCommand("sim/autopilot/servos_and_flight_dir_off"));
                }
            }
            // custom handling for x737
            else if (IsPluginEnabled(X737_PLUGIN_SIGNATURE) != 0)
            {
                XPLMDataRef cmdADataRef = XPLMFindDataRef("x737/systems/afds/CMD_A");
                XPLMDataRef cmdBDataRef = XPLMFindDataRef("x737/systems/afds/CMD_B");

                if (cmdADataRef != NULL && cmdBDataRef != NULL)
                {
                    if (XPLMGetDatai(cmdADataRef) == 0 && XPLMGetDatai(cmdBDataRef) == 0)
                        XPLMCommandOnce(XPLMFindCommand("x737/mcp/CMDA_ON"));
                    else
                    {
                        XPLMCommandOnce(XPLMFindCommand("x737/mcp/CMDA_OFF"));
                        XPLMCommandOnce(XPLMFindCommand("x737/mcp/CMDB_OFF"));
                    }
                }
            }
            // custom handling for RotorSim EC135
            else if (IsPluginEnabled(ROTORSIM_EC135_PLUGIN_SIGNATURE) != 0)
                XPLMCommandOnce(XPLMFindCommand("ec135/autopilot/ap_on"));
            // default handling
            else
                XPLMCommandOnce(XPLMFindCommand("sim/autopilot/servos_toggle"));
        }

        beginTime = 0.0f;
    }

    return 0;
}

// override control cinema verite function of BLU-fx - returns 1 if it fails
static int SetOverrideControlCinemaVeriteDataRefCallback(int overrideEnabled)
{
    if (IsPluginEnabled(BLU_FX_PLUGIN_SIGNATURE) != 0)
    {
        XPLMSetDatai(XPLMFindDataRef("blu_fx/override_control_cinema_verite"), overrideEnabled == 0 ? 0 : 1);

        return 0;
    }

    return 1;
}

// disables cinema verite and also overrides the control cinema verite function of BLU-fx
static void DisableCinemaVerite(void)
{
    // enable the control cinema verite override of BLU-fx
    overrideControlCinemaVeriteFailed = SetOverrideControlCinemaVeriteDataRefCallback(1);

    // disable cinema verite if it is enabled and store its status
    lastCinemaVerite = XPLMGetDatai(cinemaVeriteDataRef);
    if (lastCinemaVerite != 0)
        XPLMSetDatai(cinemaVeriteDataRef, 0);
}

// restores cinema verite and removes the override of the control cinema verite function of BLU-fx
static void RestoreCinemaVerite(void)
{
    // disable the control cinema verite override of BLU-fx if we enabled it before
    if (overrideControlCinemaVeriteFailed == 0)
        SetOverrideControlCinemaVeriteDataRefCallback(0);

    // restore cinema verite to its old status
    if (lastCinemaVerite != 0)
        XPLMSetDatai(cinemaVeriteDataRef, 1);
}

// converts an abstract axis index to an actual index of the selected controller while respecting the selected axis offset
static int AxisIndex(int abstractAxisIndex)
{
    if (controllerType == CONTROLLER_TYPE_DS3)
    {
        switch (abstractAxisIndex)
        {
        case JOYSTICK_AXIS_ABSTRACT_LEFT_X:
            return JOYSTICK_AXIS_DS3_LEFT_X + axisOffset;
        case JOYSTICK_AXIS_ABSTRACT_LEFT_Y:
            return JOYSTICK_AXIS_DS3_LEFT_Y + axisOffset;
        case JOYSTICK_AXIS_ABSTRACT_RIGHT_X:
            return JOYSTICK_AXIS_DS3_RIGHT_X + axisOffset;
        case JOYSTICK_AXIS_ABSTRACT_RIGHT_Y:
            return JOYSTICK_AXIS_DS3_RIGHT_Y + axisOffset;
        default:
            return -1;
        }
    }
    else if (controllerType == CONTROLLER_TYPE_XBOX360)
    {
        switch (abstractAxisIndex)
        {
        case JOYSTICK_AXIS_ABSTRACT_LEFT_X:
            return JOYSTICK_AXIS_XBOX360_LEFT_X + axisOffset;
        case JOYSTICK_AXIS_ABSTRACT_LEFT_Y:
            return JOYSTICK_AXIS_XBOX360_LEFT_Y + axisOffset;
        case JOYSTICK_AXIS_ABSTRACT_RIGHT_X:
            return JOYSTICK_AXIS_XBOX360_RIGHT_X + axisOffset;
        case JOYSTICK_AXIS_ABSTRACT_RIGHT_Y:
            return JOYSTICK_AXIS_XBOX360_RIGHT_Y + axisOffset;
        default:
            return -1;
        }
    }
    else
        return -1;
}

// converts an abstract button index to an actual index of the selected controller while respecting the selected button offset
static int ButtonIndex(int abstractButtonIndex)
{
    if (controllerType == CONTROLLER_TYPE_DS3)
    {
        switch (abstractButtonIndex)
        {
        case JOYSTICK_BUTTON_ABSTRACT_DPAD_LEFT:
            return JOYSTICK_BUTTON_DS3_DPAD_LEFT + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_DPAD_RIGHT:
            return JOYSTICK_BUTTON_DS3_DPAD_RIGHT + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_DPAD_UP:
            return JOYSTICK_BUTTON_DS3_DPAD_UP + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_DPAD_DOWN:
            return JOYSTICK_BUTTON_DS3_DPAD_DOWN + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_FACE_LEFT:
            return JOYSTICK_BUTTON_DS3_SQUARE + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_FACE_RIGHT:
            return JOYSTICK_BUTTON_DS3_CIRCLE + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_FACE_UP:
            return JOYSTICK_BUTTON_DS3_TRIANGLE + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_FACE_DOWN:
            return JOYSTICK_BUTTON_DS3_CROSS + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_CENTER_LEFT:
            return JOYSTICK_BUTTON_DS3_SELECT + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_CENTER_RIGHT:
            return JOYSTICK_BUTTON_DS3_START + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_BUMPER_LEFT:
            return JOYSTICK_BUTTON_DS3_L1 + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_BUMPER_RIGHT:
            return JOYSTICK_BUTTON_DS3_R1 + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_STICK_LEFT:
            return JOYSTICK_BUTTON_DS3_L3 + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_STICK_RIGHT:
            return JOYSTICK_BUTTON_DS3_R3 + buttonOffset;
        default:
            return -1;
        }
    }
    else if (controllerType == CONTROLLER_TYPE_XBOX360)
    {
        switch (abstractButtonIndex)
        {
        case JOYSTICK_BUTTON_ABSTRACT_DPAD_LEFT:
            return JOYSTICK_BUTTON_XBOX360_DPAD_LEFT + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_DPAD_RIGHT:
            return JOYSTICK_BUTTON_XBOX360_DPAD_RIGHT + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_DPAD_UP:
            return JOYSTICK_BUTTON_XBOX360_DPAD_UP + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_DPAD_DOWN:
            return JOYSTICK_BUTTON_XBOX360_DPAD_DOWN + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_FACE_LEFT:
            return JOYSTICK_BUTTON_XBOX360_X + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_FACE_RIGHT:
            return JOYSTICK_BUTTON_XBOX360_B + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_FACE_UP:
            return JOYSTICK_BUTTON_XBOX360_Y + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_FACE_DOWN:
            return JOYSTICK_BUTTON_XBOX360_A + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_CENTER_LEFT:
            return JOYSTICK_BUTTON_XBOX360_BACK + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_CENTER_RIGHT:
            return JOYSTICK_BUTTON_XBOX360_START + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_BUMPER_LEFT:
            return JOYSTICK_BUTTON_XBOX360_LEFT_BUMPER + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_BUMPER_RIGHT:
            return JOYSTICK_BUTTON_XBOX360_RIGHT_BUMPER + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_STICK_LEFT:
            return JOYSTICK_BUTTON_XBOX360_LEFT_STICK + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_STICK_RIGHT:
            return JOYSTICK_BUTTON_XBOX360_RIGHT_STICK + buttonOffset;
        default:
            return -1;
        }
    }
    else
        return -1;
}

// command-handler that handles the view modifier command
static int ViewModifierCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    if (inPhase != xplm_CommandContinue && trimModifierDown == 0  && mousePointerControlEnabled == 0)
    {
        int joystickAxisAssignments[100];
        XPLMGetDatavi(joystickAxisAssignmentsDataRef, joystickAxisAssignments, 0, 100);

        // only apply the modifier if no other modifier is down which can alter any assignments
        if (inPhase == xplm_CommandBegin)
        {
            viewModifierDown = 1;

            // unassign the left joystick's axis
            joystickAxisAssignments[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] = AXIS_ASSIGNMENT_NONE;
            joystickAxisAssignments[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] = AXIS_ASSIGNMENT_NONE;

            // store the default button assignments
            PushButtonAssignments();

            // assign panel scrolling controls to the dpad
            int joystickButtonAssignments[1600];
            XPLMGetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);

            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_LEFT)] = (std::size_t) XPLMFindCommand("sim/general/left");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_RIGHT)] = (std::size_t) XPLMFindCommand("sim/general/right");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_UP)] = (std::size_t) XPLMFindCommand("sim/general/up");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_DOWN)] = (std::size_t) XPLMFindCommand("sim/general/down");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_LEFT)] = (std::size_t) XPLMFindCommand("sim/general/rot_left");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_RIGHT)] = (std::size_t) XPLMFindCommand("sim/general/rot_right");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_UP)] = (std::size_t) XPLMFindCommand("sim/general/forward");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_DOWN)] = (std::size_t) XPLMFindCommand("sim/general/backward");

            XPLMSetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);

            // temporarily disable cinema verite
            DisableCinemaVerite();
        }
        else if (inPhase == xplm_CommandEnd)
        {
            viewModifierDown = 0;

            // auto-center 3D cockpit view if it is only the defined distance or angle off from the center anyways
            if (XPLMGetDatai(viewTypeDataRef) == VIEW_TYPE_3D_COCKPIT_COMMAND_LOOK && fabs(defaultHeadPositionX - XPLMGetDataf(acfPeXDataRef)) <= AUTO_CENTER_VIEW_DISTANCE_LIMIT && fabs(defaultHeadPositionY - XPLMGetDataf(acfPeYDataRef)) <= AUTO_CENTER_VIEW_DISTANCE_LIMIT && fabs(defaultHeadPositionZ - XPLMGetDataf(acfPeZDataRef)) <= AUTO_CENTER_VIEW_DISTANCE_LIMIT)
            {
                XPLMSetDataf(acfPeXDataRef, defaultHeadPositionX);
                XPLMSetDataf(acfPeYDataRef, defaultHeadPositionY);
                XPLMSetDataf(acfPeZDataRef, defaultHeadPositionZ);

                float pilotsHeadPsi = XPLMGetDataf(pilotsHeadPsiDataRef);

                if ((pilotsHeadPsi >= 360.0f - AUTO_CENTER_VIEW_ANGLE_LIMIT || pilotsHeadPsi <= AUTO_CENTER_VIEW_ANGLE_LIMIT) && fabs(XPLMGetDataf(pilotsHeadTheDataRef)) <= AUTO_CENTER_VIEW_ANGLE_LIMIT)
                {
                    XPLMSetDataf(pilotsHeadPsiDataRef, 0.0f);
                    XPLMSetDataf(pilotsHeadTheDataRef, 0.0f);
                }
            }

            // assign the default controls to the left joystick's axis
            joystickAxisAssignments[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] = AXIS_ASSIGNMENT_YAW;
            joystickAxisAssignments[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] = AXIS_ASSIGNMENT_NONE;

            // restore the default button assignments
            PopButtonAssignments();

            // restore cinema verite
            RestoreCinemaVerite();
        }

        XPLMSetDatavi(joystickAxisAssignmentsDataRef, joystickAxisAssignments, 0, 100);
    }

    lastViewModifierUsageTime = XPLMGetElapsedTime();

    return 0;
}

// command-handler that handles the prop pitch modifier command for fixed-wing airplanes or the throttle modifier command for helicopters
static int PropPitchOrThrottleModifierCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    if (inPhase == xplm_CommandBegin)
        propPitchThrottleModifierDown = 1;
    else if (inPhase == xplm_CommandEnd)
        propPitchThrottleModifierDown = 0;

    return 0;
}

// command-handler that handles the mixture control modifier command
static int MixtureControlModifierCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    if (inPhase == xplm_CommandBegin)
        mixtureControlModifierDown = 1;
    else if (inPhase == xplm_CommandEnd)
        mixtureControlModifierDown = 0;

    return 0;
}

// command-handler that handles the cowl flap modifier command
static int CowlFlapModifierCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    if (inPhase == xplm_CommandBegin)
        cowlFlapModifierDown = 1;
    else if (inPhase == xplm_CommandEnd)
        cowlFlapModifierDown = 0;

    return 0;
}

// command-handler that handles the trim modifier command
static int TrimModifierCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    // only apply the modifier if no other modifier is down which can alter any assignments
    if (inPhase == xplm_CommandBegin && viewModifierDown == 0 && mousePointerControlEnabled == 0)
    {
        trimModifierDown = 1;

        // store the default button assignments
        PushButtonAssignments();

        // assign trim controls to the buttons and dpad
        int joystickButtonAssignments[1600];
        XPLMGetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);

        // custom handling for DreamFoil AS350
        if (IsPluginEnabled(DREAMFOIL_AS350_PLUGIN_SIGNATURE) != 0)
        {
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_LEFT)] = (std::size_t) XPLMFindCommand("sim/flight_controls/rudder_trim_left");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_RIGHT)] = (std::size_t) XPLMFindCommand("sim/flight_controls/rudder_trim_right");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_CENTER_LEFT)] = (std::size_t) XPLMFindCommand(TRIM_RESET_COMMAND_NAME_LOWERCASE);

            XPLMCommandBegin(XPLMFindCommand("AS350/Trim/Force_Trim"));
        }
        // custom handling for DreamFoil B407
        else if (IsPluginEnabled(DREAMFOIL_B407_PLUGIN_SIGNATURE) != 0)
        {
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_LEFT)] = (std::size_t) XPLMFindCommand("sim/flight_controls/rudder_trim_left");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_RIGHT)] = (std::size_t) XPLMFindCommand("sim/flight_controls/rudder_trim_right");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_CENTER_LEFT)] = (std::size_t) XPLMFindCommand(TRIM_RESET_COMMAND_NAME_LOWERCASE);

            XPLMCommandBegin(XPLMFindCommand("B407/flight_controls/force_trim"));
        }
        // custom handling for RotorSim EC135
        else if (IsPluginEnabled(ROTORSIM_EC135_PLUGIN_SIGNATURE) != 0)
        {
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_LEFT)] = (std::size_t) XPLMFindCommand("ec135/autopilot/beep_left");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_RIGHT)] = (std::size_t) XPLMFindCommand("ec135/autopilot/beep_right");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_UP)] = (std::size_t) XPLMFindCommand("ec135/autopilot/beep_fwd");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_DOWN)] = (std::size_t) XPLMFindCommand("ec135/autopilot/beep_aft");
        }
        // default handling
        else
        {
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_LEFT)] = (std::size_t) XPLMFindCommand("sim/flight_controls/aileron_trim_left");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_RIGHT)] = (std::size_t) XPLMFindCommand("sim/flight_controls/aileron_trim_right");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_UP)] = (std::size_t) XPLMFindCommand("sim/flight_controls/pitch_trim_down");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_DOWN)] = (std::size_t) XPLMFindCommand("sim/flight_controls/pitch_trim_up");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_LEFT)] = (std::size_t) XPLMFindCommand("sim/flight_controls/rudder_trim_left");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_RIGHT)] = (std::size_t) XPLMFindCommand("sim/flight_controls/rudder_trim_right");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_CENTER_LEFT)] = (std::size_t) XPLMFindCommand(TRIM_RESET_COMMAND_NAME_LOWERCASE);
        }

        XPLMSetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);
    }
    else if (inPhase == xplm_CommandEnd && viewModifierDown == 0 && mousePointerControlEnabled == 0)
    {
        trimModifierDown = 0;

        // restore the default button assignments
        PopButtonAssignments();

        // custom handling for DreamFoil AS350
        if (IsPluginEnabled(DREAMFOIL_AS350_PLUGIN_SIGNATURE) != 0)
            XPLMCommandEnd(XPLMFindCommand("AS350/Trim/Force_Trim"));
        // custom handling for DreamFoil B407
        else if (IsPluginEnabled(DREAMFOIL_B407_PLUGIN_SIGNATURE) != 0)
            XPLMCommandEnd(XPLMFindCommand("B407/flight_controls/force_trim"));
    }

    return 0;
}

// command-handler that handles the trim reset command
static int TrimResetCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    if (inPhase == xplm_CommandBegin)
    {
        // custom handling for DreamFoil AS350
        if (IsPluginEnabled(DREAMFOIL_AS350_PLUGIN_SIGNATURE) != 0)
        {
            XPLMCommandEnd(XPLMFindCommand("AS350/Trim/Force_Trim"));
            XPLMCommandOnce(XPLMFindCommand("AS350/Trim/Trim_Release"));
        }
        // custom handling for DreamFoil B407
        else if (IsPluginEnabled(DREAMFOIL_B407_PLUGIN_SIGNATURE) != 0)
        {
            XPLMCommandEnd(XPLMFindCommand("B407/flight_controls/force_trim"));
            XPLMCommandOnce(XPLMFindCommand("B407/flight_controls/trim_release"));
        }
        else
        {
            XPLMSetDataf(aileronTrimDataRef, 0.0f);
            XPLMSetDataf(elevatorTrimDataRef, 0.0f);
        }

        XPLMSetDataf(rudderTrimDataRef, 0.0f);
    }

    return 0;
}

// command-handler that handles the toggle beta or toggle reverse command
static int ToggleBetaOrToggleReverseCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    static float beginTime = 0.0f;

    if (inPhase == xplm_CommandBegin)
        beginTime = XPLMGetElapsedTime();
    else if (inPhase == xplm_CommandContinue)
    {
        // toggle beta
        if (XPLMGetElapsedTime() - beginTime >= BUTTON_LONG_PRESS_TIME)
        {
            if (XPLMGetDatai(acfHasBetaDataRef) != 0)
                XPLMCommandOnce(XPLMFindCommand("sim/engines/beta_toggle"));

            beginTime = FLT_MAX;
        }
    }
    else if (inPhase == xplm_CommandEnd)
    {
        // toggle reverse
        if (XPLMGetElapsedTime() - beginTime < BUTTON_LONG_PRESS_TIME && beginTime != FLT_MAX)
            XPLMCommandOnce(XPLMFindCommand("sim/engines/thrust_reverse_toggle"));

        beginTime = 0.0f;
    }

    return 0;
}

// toggle mouse button state
static void ToggleMouseButton(int button, int down)
{
#if IBM
    static int lastLeftDown = 0, lastRightDown = 0;
    if ((button == MOUSE_BUTTON_LEFT && lastLeftDown != down) || (button == MOUSE_BUTTON_RIGHT && lastRightDown != down))
    {
        INPUT input[1];
        input[0].type = INPUT_MOUSE;
        if (button == MOUSE_BUTTON_LEFT)
            input[0].mi.dwFlags = down != 0 ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
        else
            input[0].mi.dwFlags = down != 0 ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;

        SendInput((UINT)1, input, sizeof(INPUT));

        if (button == MOUSE_BUTTON_LEFT)
            lastLeftDown = down;
        else
            lastRightDown = down;
    }
#elif APL
    CGEventType mouseType = 0;
    CGMouseButton mouseButton = 0;
    if (button == MOUSE_BUTTON_LEFT)
    {
        mouseType = (down == 0 ? kCGEventLeftMouseUp : kCGEventLeftMouseDown);
        mouseButton = kCGMouseButtonLeft;
    }
    else
    {
        mouseType = (down == 0 ? kCGEventRightMouseUp : kCGEventRightMouseDown);
        mouseButton = kCGMouseButtonRight;
    }

    int state = CGEventSourceButtonState(kCGEventSourceStateCombinedSessionState, mouseButton);

    if ((down == 0 && state) || (down != 0 && !state))
    {
        CGEventRef getLocationEvent = CGEventCreate(NULL);
        CGPoint location = CGEventGetLocation(getLocationEvent);
        if (getLocationEvent != NULL)
            CFRelease(getLocationEvent);

        CGEventRef event = CGEventCreateMouseEvent(NULL, mouseType, location, mouseButton);
        CGEventPost(kCGHIDEventTap, event);
        if (event != NULL)
            CFRelease(event);
    }
#elif LIN
    if (display != NULL)
    {
        XTestFakeButtonEvent(display, (button == MOUSE_BUTTON_LEFT ? 1 : 3), (down == 0 ? False : True), CurrentTime);
        XFlush(display);
    }
#endif
}

// command-handler that handles the toggle mouse pointer control command
static int ToggleMousePointerControlCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    if (inPhase == xplm_CommandBegin && viewModifierDown == 0 && trimModifierDown == 0)
    {
        int joystickAxisAssignments[100];
        XPLMGetDatavi(joystickAxisAssignmentsDataRef, joystickAxisAssignments, 0, 100);

        if (mousePointerControlEnabled == 0)
        {
            mousePointerControlEnabled = 1;

            // assign no controls to the left joystick's axis since it will control the mouse pointer
            joystickAxisAssignments[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] = AXIS_ASSIGNMENT_NONE;
            joystickAxisAssignments[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] = AXIS_ASSIGNMENT_NONE;

            // store the default button assignments
            PushButtonAssignments();

            // assign no commands to the l2 and r2 buttons since they will be used as left and right mouse buttons
            int joystickButtonAssignments[1600];
            XPLMGetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);

            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_DOWN)] = (std::size_t) XPLMFindCommand("sim/none/none");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_RIGHT)] = (std::size_t) XPLMFindCommand("sim/none/none");

            XPLMSetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);

            // temporarily disable cinema verite
            DisableCinemaVerite();
        }
        else
        {
            mousePointerControlEnabled = 0;

            // release both mouse buttons if they were still pressed while the mouse pointer control mode was turned off
            ToggleMouseButton(MOUSE_BUTTON_LEFT, 0);
            ToggleMouseButton(MOUSE_BUTTON_RIGHT, 0);

            // assign the default controls to the left joystick's axis
            joystickAxisAssignments[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] = AXIS_ASSIGNMENT_YAW;
            joystickAxisAssignments[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] = AXIS_ASSIGNMENT_NONE;

            // restore the default button assignments
            PopButtonAssignments();

            // restore cinema verite
            RestoreCinemaVerite();
        }

        XPLMSetDatavi(joystickAxisAssignmentsDataRef, joystickAxisAssignments, 0, 100);
    }

    return 0;
}

// command-handler that handles the push-to-talk command
static int PushToTalkCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    // only do push-to-talk if X-IvAp is enabled
    if (IsPluginEnabled(X_IVAP_PLUGIN_SIGNATURE) != 0 || IsPluginEnabled(X_XSQUAWKBOX_PLUGIN_SIGNATURE) != 0)
    {
        if (inPhase != xplm_CommandContinue)
        {
#if IBM
            INPUT input[1];
            input[0].type = INPUT_KEYBOARD;
            input[0].ki.wScan = (WORD) 24;
            DWORD flags;
            if (inPhase == xplm_CommandBegin)
                flags = KEYEVENTF_SCANCODE;
            else
                flags = KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE;
            input[0].ki.dwFlags = flags;

            SendInput((UINT) 1, input, sizeof(INPUT));
#elif APL
            static CGEventSourceRef eventSource = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
            CGEventRef event = CGEventCreateKeyboardEvent(eventSource, (CGKeyCode) kVK_ANSI_O, inPhase == xplm_CommandBegin);
            CGEventPost(kCGHIDEventTap, event);
            if (event != NULL)
                CFRelease(event);
#elif LIN
            if (display != NULL)
            {
                KeyCode keycode = XKeysymToKeycode(display, XK_O);
                XTestFakeKeyEvent(display, keycode, inPhase == xplm_CommandBegin, CurrentTime);
                XFlush(display);
            }
#endif
        }
    }

    return 0;
}

// command-handler that handles the toggling of brakes
static int ToggleBrakesCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    if (inPhase == xplm_CommandBegin)
    {
        // custom handling for QPAC A320
        if (IsPluginEnabled(QPAC_A320_PLUGIN_SIGNATURE))
            XPLMCommandOnce(XPLMFindCommand("airbus_qpac/park_brake_toggle"));
        // default handling
        else
            XPLMCommandOnce(XPLMFindCommand("sim/flight_controls/brakes_toggle_regular"));
    }

    return 0;
}

// check if the player's aircraft is a helicopter
static int IsHelicopter()
{
    if (XPLMGetDatai(acfCockpitTypeDataRef) == 5)
        return 1;

    float acfVertCant[8];
    XPLMGetDatavf(acfVertCantDataRef, acfVertCant, 0, 8);

    for (int i = 0; i < 8; i++)
    {
        if (acfVertCant[i] > 45.0f)
            return 1;
    }

    return 0;
}

// normalizes a value of a range [inMin, inMax] to a value of the range [outMin, outMax]
static float Normalize(float value, float inMin, float inMax, float outMin, float outMax)
{
    return (outMax - outMin) / (inMax - inMin) * (value - inMax) + (outMax - outMin);
}

// flightloop-callback that mainly handles the joystick axis among other minor stuff
static float FlightLoopCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void *inRefcon)
{
    // update the default head position when required
    if (defaultHeadPositionX == FLT_MAX || defaultHeadPositionY == FLT_MAX || defaultHeadPositionZ == FLT_MAX)
    {
        defaultHeadPositionX = XPLMGetDataf(acfPeXDataRef);
        defaultHeadPositionY = XPLMGetDataf(acfPeYDataRef);
        defaultHeadPositionZ = XPLMGetDataf(acfPeZDataRef);
    }

    int isHelicopter = IsHelicopter();

    int currentMouseX, currentMouseY;
    XPLMGetMouseLocation(&currentMouseX, &currentMouseY);

    // handle switch to 3D command look
    if (switchTo3DCommandLook != 0)
    {
        XPLMCommandOnce(XPLMFindCommand("sim/view/3d_cockpit_cmnd_look"));
        switchTo3DCommandLook = 0;
    }

    if (XPLMGetDatai(hasJostickDataRef) != 0)
    {
#if IBM
        static int guideButtonDown = 0;

        if (controllerType == CONTROLLER_TYPE_XBOX360 && getControllerData != NULL)
        {
            ControllerStruct buttons;
            getControllerData(0, buttons);

            if (guideButtonDown == 0 && buttons.guideButton != 0)
            {
                guideButtonDown = 1;
                XPLMCommandBegin(toggleMousePointerControlCommand);
            }
            else if (guideButtonDown == 1 && buttons.guideButton == 0)
            {
                guideButtonDown = 0;
                XPLMCommandEnd(toggleMousePointerControlCommand);
            }
        }
#endif

        float sensitivityMultiplier = JOYSTICK_RELATIVE_CONTROL_MULTIPLIER * inElapsedSinceLastCall;

        float joystickPitchNullzone = XPLMGetDataf(joystickPitchNullzoneDataRef);

        float joystickAxisValues[100];
        XPLMGetDatavf(joystickAxisValuesDataRef, joystickAxisValues, 0, 100);

        static int joystickAxisLeftXCalibrated = 0;
        if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] > 0.0f)
            joystickAxisLeftXCalibrated = 1;

        // keep the value of the left joystick's y axis at 0.5 until a value higher/lower than 0.0/1.0 is read because axis can get initialized with a value of 0.0 or 1.0 instead of 0.5 if they haven't been moved yet - this can result in unexpected behaviour especially if the axis is used in relative mode
        static float leftJoystickMinYValue = 1.0f, leftJoystickMaxYValue = 0.0f;

        if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] < leftJoystickMinYValue)
            leftJoystickMinYValue = joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)];

        if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] > leftJoystickMaxYValue)
            leftJoystickMaxYValue = joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)];

        if (leftJoystickMinYValue == 1.0f || leftJoystickMaxYValue == 0.0f)
            joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] = 0.5f;

        int acfNumEngines = XPLMGetDatai(acfNumEnginesDataRef);

        static int leftTriggerDown = 0, rightTriggerDown = 0;
        if (controllerType == CONTROLLER_TYPE_XBOX360 && joystickAxisLeftXCalibrated)
        {
            if (leftTriggerDown == 0 && rightTriggerDown == 0 && joystickAxisValues[JOYSTICK_AXIS_XBOX360_TRIGGERS + axisOffset] >= 0.85f)
            {
                leftTriggerDown = 1;
                XPLMCommandBegin(pushToTalkCommand);
            }
            else if (leftTriggerDown == 0 && rightTriggerDown == 0 && joystickAxisValues[JOYSTICK_AXIS_XBOX360_TRIGGERS + axisOffset] <= 0.15f)
            {
                rightTriggerDown = 1;
                XPLMCommandBegin(toggleBrakesCommand);
            }
            else if (leftTriggerDown == 1 && rightTriggerDown == 0 && joystickAxisValues[JOYSTICK_AXIS_XBOX360_TRIGGERS + axisOffset] < 0.85f)
            {
                leftTriggerDown = 0;
                XPLMCommandEnd(pushToTalkCommand);
            }
            else if (leftTriggerDown == 0 && rightTriggerDown == 1 && joystickAxisValues[JOYSTICK_AXIS_XBOX360_TRIGGERS + axisOffset] > 0.15f)
            {
                rightTriggerDown = 0;
                XPLMCommandEnd(toggleBrakesCommand);
            }
        }

        if (viewModifierDown != 0)
        {
            int viewType = XPLMGetDatai(viewTypeDataRef);

            if (viewType == VIEW_TYPE_3D_COCKPIT_COMMAND_LOOK)
            {
                float deltaPsi = 0.0f, deltaThe = 0.0f;

                // turn head to the left
                if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] < 0.5f - joystickPitchNullzone)
                {
                    // normalize range [0.5, 0.0] to [0.0, 1.0]
                    float d = Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)], 0.5f, 0.0f, 0.0f, 1.0f);

                    // apply acceleration function (y = x^4)
                    deltaPsi -= powf(d * JOYSTICK_VIEW_SENSITIVITY, 4.0f) * inElapsedSinceLastCall;
                }
                // turn head to the right
                else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] > 0.5f + joystickPitchNullzone)
                {
                    // normalize range [0.5, 1.0] to [0.0, 1.0]
                    float d = Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)], 0.5f, 1.0f, 0.0f, 1.0f);

                    // apply acceleration function (y = x^4)
                    deltaPsi += powf(d * JOYSTICK_VIEW_SENSITIVITY, 4.0f) * inElapsedSinceLastCall;
                }

                // turn head upwards
                if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] < 0.5f - joystickPitchNullzone)
                {
                    // normalize range [0.5, 0.0] to [0.0, 1.0]
                    float d = Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 0.0f, 0.0f, 1.0f);

                    // apply acceleration function (y = x^4)
                    deltaThe += powf(d * JOYSTICK_VIEW_SENSITIVITY, 4.0f) * inElapsedSinceLastCall;
                }
                // turn head downwards
                else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] > 0.5f + joystickPitchNullzone)
                {
                    // normalize range [0.5, 1.0] to [0.0, 1.0]
                    float d = Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 1.0f, 0.0f, 1.0f);

                    // apply acceleration function (y = x^4)
                    deltaThe -= powf(d * JOYSTICK_VIEW_SENSITIVITY, 4.0f) * inElapsedSinceLastCall;
                }

                float pilotsHeadPsi = XPLMGetDataf(pilotsHeadPsiDataRef);
                float pilotsHeadThe = XPLMGetDataf(pilotsHeadTheDataRef);

                float newPilotsHeadPsi = pilotsHeadPsi + deltaPsi;
                float newPilotsHeadThe = pilotsHeadThe + deltaThe;

                if (pilotsHeadPsi < 180.0f && newPilotsHeadPsi > 179.9f)
                    newPilotsHeadPsi = 179.9f;
                if (pilotsHeadPsi > 180.0f && newPilotsHeadPsi < 180.1f)
                    newPilotsHeadPsi = 180.1f;

                if (newPilotsHeadThe < -89.9f)
                    newPilotsHeadThe = -89.9f;
                if (newPilotsHeadThe > 89.9f)
                    newPilotsHeadThe = 89.9f;

                XPLMSetDataf(pilotsHeadPsiDataRef, newPilotsHeadPsi);
                XPLMSetDataf(pilotsHeadTheDataRef, newPilotsHeadThe);
            }
            else if (viewType == VIEW_TYPE_FORWARDS_WITH_PANEL || viewType == VIEW_TYPE_CHASE)
            {
                // move camera to the left
                if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] < 0.5f - joystickPitchNullzone)
                {
                    // normalize range [0.5, 0.0] to [0.0, 1.0]
                    float d = Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)], 0.5f, 0.0f, 0.0f, 1.0f);

                    // apply acceleration function (y = x^2) and round to integer
                    int n = (int) (powf(2.0f * d, 2.0f) + 0.5f);

                    // apply the command
                    for (int i = 0; i < n; i++)
                        XPLMCommandOnce(XPLMFindCommand("sim/general/left"));
                }
                // move camera to the right
                else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] > 0.5f + joystickPitchNullzone)
                {
                    // normalize range [0.5, 1.0] to [0.0, 1.0]
                    float d = Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)], 0.5f, 1.0f, 0.0f, 1.0f);

                    // apply acceleration function (y = x^2) and round to integer
                    int n = (int) (powf(2.0f * d, 2.0f) + 0.5f);

                    // apply the command
                    for (int i = 0; i < n; i++)
                        XPLMCommandOnce(XPLMFindCommand("sim/general/right"));
                }

                // move camera up
                if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] < 0.5f - joystickPitchNullzone)
                {
                    // normalize range [0.5, 0.0] to [0.0, 1.0]
                    float d = Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 0.0f, 0.0f, 1.0f);

                    // apply acceleration function (y = x^2) and round to integer
                    int n = (int) (powf(2.0f * d, 2.0f) + 0.5f);

                    // apply the command
                    for (int i = 0; i < n; i++)
                        XPLMCommandOnce(XPLMFindCommand("sim/general/up"));
                }
                // move camera down
                else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] > 0.5f + joystickPitchNullzone)
                {
                    // normalize range [0.5, 1.0] to [0.0, 1.0]
                    float d = Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 1.0f, 0.0f, 1.0f);

                    // apply acceleration function (y = x^2) and round to integer
                    int n = (int) (powf(2.0f * d, 2.0f) + 0.5f);

                    // apply the command
                    for (int i = 0; i < n; i++)
                        XPLMCommandOnce(XPLMFindCommand("sim/general/down"));
                }
            }
        }
        else
        {
            if (isHelicopter == 0 && propPitchThrottleModifierDown != 0)
            {
                float acfRSCMingovPrp = XPLMGetDataf(acfRSCMingovPrpDataRef);
                float acfRSCRedlinePrp = XPLMGetDataf(acfRSCRedlinePrpDataRef);
                float propRotationSpeedRadSecAll = XPLMGetDataf(propRotationSpeedRadSecAllDataRef);

                // increase prop pitch
                if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] < 0.5f - joystickPitchNullzone)
                {
                    // normalize range [0.5, 0.0] to [acfRSCMingovPrp, acfRSCRedlinePrp]
                    float d = sensitivityMultiplier * Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 0.0f, acfRSCMingovPrp, acfRSCRedlinePrp);

                    float newPropRotationSpeedRadSecAll = propRotationSpeedRadSecAll + d;

                    // ensure we don't set values larger than 1.0
                    XPLMSetDataf(propRotationSpeedRadSecAllDataRef, newPropRotationSpeedRadSecAll < acfRSCRedlinePrp ? newPropRotationSpeedRadSecAll : acfRSCRedlinePrp);
                }
                // decrease prop pitch
                else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] > 0.5f + joystickPitchNullzone)
                {
                    // normalize range [0.5, 1.0] to [acfRSCMingovPrp, acfRSCRedlinePrp]
                    float d = sensitivityMultiplier * Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 1.0f, acfRSCMingovPrp, acfRSCRedlinePrp);

                    float newPropRotationSpeedRadSecAll = propRotationSpeedRadSecAll - d;

                    // ensure we don't set values smaller than 0.0
                    XPLMSetDataf(propRotationSpeedRadSecAllDataRef, newPropRotationSpeedRadSecAll > acfRSCMingovPrp ? newPropRotationSpeedRadSecAll : acfRSCMingovPrp);
                }
            }
            else if (mixtureControlModifierDown != 0)
            {
                float mixtureRatioAll = XPLMGetDataf(mixtureRatioAllDataRef);

                // increase mixture setting
                if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] < 0.5f - joystickPitchNullzone)
                {
                    // normalize range [0.5, 0.0] to [0.0, 1.0]
                    float d = sensitivityMultiplier * Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 0.0f, 0.0f, 1.0f);

                    float newMixtureRatioAll = mixtureRatioAll + d;

                    // ensure we don't set values larger than 1.0
                    XPLMSetDataf(mixtureRatioAllDataRef, newMixtureRatioAll < 1.0f ? newMixtureRatioAll : 1.0f);
                }
                // decrease mixture setting
                else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] > 0.5f + joystickPitchNullzone)
                {
                    // normalize range [0.5, 1.0] to [0.0, 1.0]
                    float d = sensitivityMultiplier * Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 1.0f, 0.0f, 1.0f);

                    float newMixtureRatioAll = mixtureRatioAll - d;

                    // ensure we don't set values smaller than 0.0
                    XPLMSetDataf(mixtureRatioAllDataRef, newMixtureRatioAll > 0.0f ? newMixtureRatioAll : 0.0f);
                }
            }
            else if (cowlFlapModifierDown != 0)
            {
                float* cowlFlapRatio = (float*) malloc(acfNumEngines * sizeof(float));
                XPLMGetDatavf(cowlFlapRatioDataRef, cowlFlapRatio, 0, acfNumEngines);

                // decrease cowl flap setting
                if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] < 0.5f - joystickPitchNullzone)
                {
                    // normalize range [0.5, 0.0] to [0.0, 1.0]
                    float d = sensitivityMultiplier * Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 0.0f, 0.0f, 1.0f);

                    // ensure we don't set values smaller than 0.0
                    for (int i = 0; i < acfNumEngines; i++)
                    {
                        float newCowlFlapRatio = cowlFlapRatio[i] - d;
                        cowlFlapRatio[i] = newCowlFlapRatio > 0.0f ? newCowlFlapRatio : 0.0f;
                    }
                }
                // increase cowl flap setting
                else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] > 0.5f + joystickPitchNullzone)
                {
                    // normalize range [0.5, 1.0] to [0.0, 1.0]
                    float d = sensitivityMultiplier * Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 1.0f, 0.0f, 1.0f);

                    // ensure we don't set values larger than 1.0
                    for (int i = 0; i < acfNumEngines; i++)
                    {
                        float newCowlFlapRatio = cowlFlapRatio[i] + d;
                        cowlFlapRatio[i] = newCowlFlapRatio < 1.0f ? newCowlFlapRatio : 1.0f;
                    }
                }

                XPLMSetDatavf(cowlFlapRatioDataRef, cowlFlapRatio, 0, acfNumEngines);
                free(cowlFlapRatio);
            }
            else if (mousePointerControlEnabled != 0)
            {
                int distX = 0, distY = 0;

                // move mouse pointer left
                if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] < 0.5f - joystickPitchNullzone)
                {
                    // normalize range [0.5, 0.0] to [0.0, 1.0]
                    float d = Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)], 0.5f, 0.0f, 0.0f, 1.0f);

                    // apply acceleration function (y = x^2)
                    distX -= (int) (powf(d * JOYSTICK_MOUSE_POINTER_SENSITIVITY, 2.0f) * inElapsedSinceLastCall);
                }
                // move mouse pointer right
                else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] > 0.5f + joystickPitchNullzone)
                {
                    // normalize range [0.5, 1.0] to [0.0, 1.0]
                    float d = Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)], 0.5f, 1.0f, 0.0f, 1.0f);

                    // apply acceleration function (y = x^2)
                    distX += (int) (powf(d * JOYSTICK_MOUSE_POINTER_SENSITIVITY, 2.0f) * inElapsedSinceLastCall);
                }

                // move mouse pointer up
                if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] < 0.5f - joystickPitchNullzone)
                {
                    // normalize range [0.5, 0.0] to [0.0, 1.0]
                    float d = Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 0.0f, 0.0f, 1.0f);

                    // apply acceleration function (y = x^2)
                    distY -= (int) (powf(d * JOYSTICK_MOUSE_POINTER_SENSITIVITY, 2.0f) * inElapsedSinceLastCall);
                }
                // move mouse pointer down
                else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] > 0.5f + joystickPitchNullzone)
                {
                    // normalize range [0.5, 1.0] to [0.0, 1.0]
                    float d = Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 1.0f, 0.0f, 1.0f);

                    // apply acceleration function (y = x^2)
                    distY += (int) (powf(d * JOYSTICK_MOUSE_POINTER_SENSITIVITY, 2.0f) * inElapsedSinceLastCall);
                }

#if IBM
                INPUT input[1];
                input[0].type = INPUT_MOUSE;
                input[0].mi.dx = (long) distX;
                input[0].mi.dy = (long) distY;
                input[0].mi.dwFlags = MOUSEEVENTF_MOVE;

                SendInput((UINT) 1, input, sizeof(INPUT));
#elif APL
                // get current mouse pointer location
                CGEventRef getLocationEvent = CGEventCreate(NULL);
                CGPoint oldLocation = CGEventGetLocation(getLocationEvent);
                if (getLocationEvent)
                    CFRelease(getLocationEvent);

                CGPoint newLocation;
                newLocation.x = oldLocation.x + distX;
                newLocation.y = oldLocation.y + distY;

                // get active displays
                CGDirectDisplayID activeDisplays[8];
                uint32_t displayCount = 0;
                CGGetActiveDisplayList(8, activeDisplays, &displayCount);

                // get display ids of the display on which the mouse pointer was contained before and will be once moved - values of -1 indicate that the pointer is outside of all displays
                int oldContainingDisplay = -1;
                int newContainingDisplay = -1;
                for (int i = 0; i < displayCount; i++)
                {
                    CGRect screenBounds = CGDisplayBounds(activeDisplays[i]);

                    if (CGRectContainsPoint(screenBounds, oldLocation))
                        oldContainingDisplay = i;

                    if (CGRectContainsPoint(screenBounds, newLocation))
                        newContainingDisplay = i;
                }

                // ensure the pointer is not moved beyond the bounds of the display it was contained in before
                if (newContainingDisplay == -1 && oldContainingDisplay > -1)
                {
                    CGRect screenBounds = CGDisplayBounds(activeDisplays[oldContainingDisplay]);
                    int minX = (int) screenBounds.origin.x;
                    int minY = (int) screenBounds.origin.y;
                    int maxX = (int) minX + screenBounds.size.width - 1;
                    int maxY = (int) minY + screenBounds.size.height - 1;

                    newLocation.x = newLocation.x >= minX ? newLocation.x : minX;
                    newLocation.x = newLocation.x <= maxX ? newLocation.x : maxX;
                    newLocation.y = newLocation.y >= minY ? newLocation.y : minY;
                    newLocation.y = newLocation.y < maxY ? newLocation.y : maxY;
                }

                // move mouse pointer by distX and distY pixels
                CGEventRef moveMouseEvent = CGEventCreateMouseEvent(NULL, kCGEventMouseMoved, newLocation, kCGMouseButtonLeft);
                CGEventPost(kCGHIDEventTap, moveMouseEvent);
                if (moveMouseEvent != NULL)
                    CFRelease(moveMouseEvent);
#elif LIN
                if (display != NULL)
                {
                    XWarpPointer(display, None, None, 0, 0, 0, 0, distX, distY);
                    XFlush(display);
                }
#endif

                int joystickButtonValues[1600];
                XPLMGetDatavi(joystickButtonValuesDataRef, joystickButtonValues, 0, 1600);

                // handle left and right mouse button presses
                ToggleMouseButton(MOUSE_BUTTON_LEFT, joystickButtonValues[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_DOWN)] != 0 ? 1 : 0);
                ToggleMouseButton(MOUSE_BUTTON_RIGHT, joystickButtonValues[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_RIGHT)] != 0 ? 1 : 0);
            }
            else
            {
                if (isHelicopter == 0)
                {
                    static float normalizedViewHeading = 0.0f;

                    float elapsedTime = XPLMGetElapsedTime() - (lastMouseUsageTime > lastViewModifierUsageTime ? lastMouseUsageTime : lastViewModifierUsageTime);

                    // handle view heading when the joystick has been calibrated, the aircraft is on ground and the current view is 3D cockpit command look
                    if (joystickAxisLeftXCalibrated != 0 && XPLMGetDatai(ongroundAnyDataRef) != 0 && XPLMGetDatai(viewTypeDataRef) == VIEW_TYPE_3D_COCKPIT_COMMAND_LOOK && elapsedTime >= DISABLE_VIEW_HEADING_TIME)
                    {
                        float joystickHeadingNullzone = XPLMGetDataf(joystickHeadingNullzoneDataRef);

                        // handle nullzone
                        if(fabs(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] - 0.5f) > joystickHeadingNullzone)
                        {
                            // center head position except view heading
                            XPLMSetDataf(acfPeXDataRef, defaultHeadPositionX);
                            XPLMSetDataf(acfPeYDataRef, defaultHeadPositionY);
                            XPLMSetDataf(acfPeZDataRef, defaultHeadPositionZ);
                            XPLMSetDataf(pilotsHeadTheDataRef, 0.0f);

                            // normalize axis deflection
                            if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] > 0.5f)
                                normalizedViewHeading = Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)], 0.5f + joystickHeadingNullzone, 1.0f, 0.0f, 1.0f);
                            else
                                normalizedViewHeading = Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)], 0.5f - joystickHeadingNullzone, 0.0f, 0.0f, -1.0f);

                            float groundspeed = XPLMGetDataf(groundspeedDataRef);

                            // clamp absolute groundspeed to 15 m/s
                            groundspeed = fabs(groundspeed < 15.0f ?  groundspeed : 15.0f);

                            // apply acceleration function (y = x^2) and fade-out with increasing groundspeed
                            float newPilotsHeadPsi = normalizedViewHeading * powf(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] - 0.5f, 2.0f) * 2.0f * Normalize(groundspeed, 15.0f, 0.0f, -1.0f, 0.0f) * 90.0f;

                            XPLMSetDataf(pilotsHeadPsiDataRef, newPilotsHeadPsi);
                        }
                        // reset the view if the axis deflection has jumped from a large value right into the nullzone
                        else if (normalizedViewHeading != 0.0f)
                        {
                            XPLMSetDataf(pilotsHeadPsiDataRef, 0.0f);
                            normalizedViewHeading = 0.0f;
                        }
                    }
                }

                if (isHelicopter != 0 && propPitchThrottleModifierDown == 0)
                {
                    float acfMinPitch[8];
                    XPLMGetDatavf(acfMinPitchDataRef, acfMinPitch, 0, 8);
                    float acfMaxPitch[8];
                    XPLMGetDatavf(acfMaxPitchDataRef, acfMaxPitch, 0, 8);
                    float* propPitchDeg = (float*) malloc(acfNumEngines * sizeof(float));
                    XPLMGetDatavf(propPitchDegDataRef, propPitchDeg, 0, acfNumEngines);

                    for (int i = 0; i < acfNumEngines; i++)
                    {
                        // increase prop pitch
                        if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] < 0.5f - joystickPitchNullzone)
                        {
                            // normalize range [0.5, 0.0] to [acfMinPitch, acfMaxPitch]
                            float d = COLLECTIVE_CONTROL_MULTIPLIER * sensitivityMultiplier * Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 0.0f, acfMinPitch[i], acfMaxPitch[i]);

                            float newPropPitchDeg = propPitchDeg[i] + d;

                            // ensure we don't set values larger than 1.0
                            propPitchDeg[i] = newPropPitchDeg < acfMaxPitch[i] ? newPropPitchDeg : acfMaxPitch[i];
                        }
                        // decrease prop pitch
                        else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] > 0.5f + joystickPitchNullzone)
                        {
                            // normalize range [0.5, 1.0] to [acfMinPitch, acfMaxPitch]
                            float d = COLLECTIVE_CONTROL_MULTIPLIER * sensitivityMultiplier * Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 1.0f, acfMinPitch[i], acfMaxPitch[i]);

                            float newPropPitchDeg = propPitchDeg[i] - d;

                            // ensure we don't set values smaller than 0.0
                            propPitchDeg[i] = newPropPitchDeg > acfMinPitch[i] ? newPropPitchDeg : acfMinPitch[i];
                        }
                    }

                    XPLMSetDatavf(propPitchDegDataRef, propPitchDeg, 0, acfNumEngines);
                    free(propPitchDeg);
                }
                else
                {
                    if (acfNumEngines == 0 && XPLMGetDatai(acfSbrkEQDataRef) != 0)
                    {
                        float speedbrakeRatio = XPLMGetDataf(speedbrakeRatioDataRef);

                        // decrease speedbrake ratio
                        if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] < 0.5f - joystickPitchNullzone)
                        {
                            // de-arm speedbrake if armed
                            if (speedbrakeRatio == -0.5f)
                                speedbrakeRatio = 0.0f;

                            // normalize range [0.5, 0.0] to [0.0, 1.0]
                            float d = sensitivityMultiplier * Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 0.0f, 0.0f, 1.0f);

                            float newSpeedbrakeRatio = speedbrakeRatio - d;

                            // ensure we don't set values smaller than 0.0
                            XPLMSetDataf(speedbrakeRatioDataRef, newSpeedbrakeRatio > 0.0f ? newSpeedbrakeRatio : 0.0f);
                        }
                        // increase speedbrake ratio
                        else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] > 0.5f + joystickPitchNullzone)
                        {
                            // de-arm speedbrake if armed
                            if (speedbrakeRatio == -0.5f)
                                speedbrakeRatio = 0.0f;

                            // normalize range [0.5, 1.0] to [0.0, 1.0]
                            float d = sensitivityMultiplier * Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 1.0f, 0.0f, 1.0f);

                            float newSpeedbrakeRatio = speedbrakeRatio + d;

                            // ensure we don't set values larger than 1.0
                            XPLMSetDataf(speedbrakeRatioDataRef, newSpeedbrakeRatio < 1.0f ? newSpeedbrakeRatio : 1.0f);
                        }
                    }
                    else
                    {
                        float throttleRatioAll = XPLMGetDataf(throttleRatioAllDataRef);

                        float* thrustReverserDeployRatio = (float*) malloc(acfNumEngines * sizeof(float));

                        XPLMGetDatavf(thrustReverserDeployRatioDataRef, thrustReverserDeployRatio, 0, acfNumEngines);

                        float averageThrustReverserDeployRatio = 0.0f;
                        for (int i = 0; i < acfNumEngines; i++)
                            averageThrustReverserDeployRatio += thrustReverserDeployRatio[i];
                        averageThrustReverserDeployRatio /= acfNumEngines;

                        // increase throttle setting
                        if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] < 0.5f - joystickPitchNullzone)
                        {
                            // normalize range [0.5, 0.0] to [0.0, 1.0]
                            float d = sensitivityMultiplier * Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 0.0f, 0.0f, 1.0f);

                            // invert d if thrust reversers are engaged
                            if (averageThrustReverserDeployRatio > 0.5f)
                            {
                                float newThrottleRatioAll = throttleRatioAll - d;

                                // ensure we don't set values smaller than 0.0
                                XPLMSetDataf(throttleRatioAllDataRef, newThrottleRatioAll > 0.0f ? newThrottleRatioAll : 0.0f);
                            }
                            else
                            {
                                float newThrottleRatioAll = throttleRatioAll + d;

                                // ensure we don't set values larger than 1.0
                                XPLMSetDataf(throttleRatioAllDataRef, newThrottleRatioAll < 1.0f ? newThrottleRatioAll : 1.0f);
                            }
                        }
                        // decrease throttle setting
                        else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] > 0.5f + joystickPitchNullzone)
                        {
                            // normalize range [0.5, 1.0] to [0.0, 1.0]
                            float d = sensitivityMultiplier * Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 1.0f, 0.0f, 1.0f);

                            // invert d if thrust reversers are engaged
                            if (averageThrustReverserDeployRatio > 0.5f)
                            {
                                float newThrottleRatioAll = throttleRatioAll + d;

                                // ensure we don't set values larger than 1.0
                                XPLMSetDataf(throttleRatioAllDataRef, newThrottleRatioAll < 1.0f ? newThrottleRatioAll : 1.0f);
                            }
                            else
                            {
                                float newThrottleRatioAll = throttleRatioAll - d;

                                // ensure we don't set values smaller than 0.0
                                XPLMSetDataf(throttleRatioAllDataRef, newThrottleRatioAll > 0.0f ? newThrottleRatioAll : 0.0f);
                            }
                        }

                        free(thrustReverserDeployRatio);
                    }
                }

                if (isHelicopter == 0 || (isHelicopter != 0 && propPitchThrottleModifierDown == 0))
                {
                    // handle left brake
                    if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] >= 0.8f && joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] <= 0.7f)
                        XPLMSetDataf(leftBrakeRatioDataRef, 1.0f);
                    else
                        XPLMSetDataf(leftBrakeRatioDataRef, 0.0f);

                    // handle right brake
                    if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] >= 0.8f && joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] >= 0.3f)
                        XPLMSetDataf(rightBrakeRatioDataRef, 1.0f);
                    else
                        XPLMSetDataf(rightBrakeRatioDataRef, 0.0f);
                }
            }
        }
    }

    return -1.0f;
}

// set the default axis and button assignments
static void SetDefaultAssignments(void)
{
    // only set default assignments if a joystick is found and if no modifier is down which can alter any assignments
    if (XPLMGetDatai(hasJostickDataRef) && viewModifierDown == 0 && trimModifierDown == 0  && mousePointerControlEnabled == 0)
    {
        // set default axis assignments
        int joystickAxisAssignments[100];
        XPLMGetDatavi(joystickAxisAssignmentsDataRef, joystickAxisAssignments, 0, 100);

        joystickAxisAssignments[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] = AXIS_ASSIGNMENT_YAW;
        joystickAxisAssignments[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] = AXIS_ASSIGNMENT_NONE;
        joystickAxisAssignments[AxisIndex(JOYSTICK_AXIS_ABSTRACT_RIGHT_X)] = AXIS_ASSIGNMENT_ROLL;
        joystickAxisAssignments[AxisIndex(JOYSTICK_AXIS_ABSTRACT_RIGHT_Y)] = AXIS_ASSIGNMENT_PITCH;

        XPLMSetDatavi(joystickAxisAssignmentsDataRef, joystickAxisAssignments, 0, 100);

        // set default button assignments
        int joystickButtonAssignments[1600];
        XPLMGetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);

        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_LEFT)] = (std::size_t) XPLMFindCommand("sim/flight_controls/flaps_up");
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_RIGHT)] = (std::size_t) XPLMFindCommand("sim/flight_controls/flaps_down");
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_UP)] = (std::size_t) XPLMFindCommand(TOGGLE_ARM_SPEED_BRAKE_OR_TOGGLE_CARB_HEAT_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_DOWN)] = (std::size_t) XPLMFindCommand("sim/flight_controls/landing_gear_toggle");
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_LEFT)] = (std::size_t) XPLMFindCommand(CYCLE_RESET_VIEW_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_RIGHT)] = (std::size_t) XPLMFindCommand(MIXTURE_CONTROL_MODIFIER_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_UP)] = (std::size_t) XPLMFindCommand(PROP_PITCH_THROTTLE_MODIFIER_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_DOWN)] = (std::size_t) XPLMFindCommand(COWL_FLAP_MODIFIER_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_CENTER_LEFT)] = (std::size_t) XPLMFindCommand(TOGGLE_BETA_OR_TOGGLE_REVERSE_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_CENTER_RIGHT)] = (std::size_t) XPLMFindCommand(TOGGLE_AUTOPILOT_OR_DISABLE_FLIGHT_DIRECTOR_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_BUMPER_LEFT)] = (std::size_t) XPLMFindCommand(TRIM_MODIFIER_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_BUMPER_RIGHT)] = (std::size_t) XPLMFindCommand(VIEW_MODIFIER_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_STICK_LEFT)] = (std::size_t) XPLMFindCommand("sim/general/zoom_out");
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_STICK_RIGHT)] = (std::size_t) XPLMFindCommand("sim/general/zoom_in");
        if (controllerType == CONTROLLER_TYPE_DS3)
        {
            joystickButtonAssignments[JOYSTICK_BUTTON_DS3_PS + buttonOffset] = (std::size_t) XPLMFindCommand(TOGGLE_MOUSE_POINTER_CONTROL_COMMAND);
            joystickButtonAssignments[JOYSTICK_BUTTON_DS3_L2 + buttonOffset] = (std::size_t) XPLMFindCommand(PUSH_TO_TALK_COMMAND);
            joystickButtonAssignments[JOYSTICK_BUTTON_DS3_R2 + buttonOffset] = (std::size_t) XPLMFindCommand(TOGGLE_BRAKES_COMMAND);
        }
#if !IBM
        else if (controllerType == CONTROLLER_TYPE_XBOX360)
            joystickButtonAssignments[JOYSTICK_BUTTON_XBOX360_GUIDE + buttonOffset] = (std::size_t) XPLMFindCommand(TOGGLE_MOUSE_POINTER_CONTROL_COMMAND);
#endif

        XPLMSetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);

        // set default nullzone
        XPLMSetDataf(joystickPitchNullzoneDataRef, DEFAULT_NULLZONE);
        XPLMSetDataf(joystickRollNullzoneDataRef, DEFAULT_NULLZONE);
        XPLMSetDataf(joystickHeadingNullzoneDataRef, DEFAULT_NULLZONE);
    }
}

// updates all caption widgets and slider positions associated with settings variables
static void UpdateSettingsWidgets(void)
{
    XPSetWidgetProperty(dualShock3ControllerRadioButton, xpProperty_ButtonState, (int) (controllerType == CONTROLLER_TYPE_DS3));
    XPSetWidgetProperty(xbox360ControllerRadioButton, xpProperty_ButtonState, (int) (controllerType == CONTROLLER_TYPE_XBOX360));

    char stringAxisOffset[32];
    sprintf(stringAxisOffset, "Axis Offset: %d", axisOffset);
    XPSetWidgetDescriptor(axisOffsetCaption, stringAxisOffset);

    char stringButtonOffset[32];
    sprintf(stringButtonOffset, "Button Offset: %d", buttonOffset);
    XPSetWidgetDescriptor(buttonOffsetCaption, stringButtonOffset);

    XPSetWidgetProperty(axisOffsetSlider, xpProperty_ScrollBarSliderPosition, (intptr_t) axisOffset);
    XPSetWidgetProperty(buttonOffsetSlider, xpProperty_ScrollBarSliderPosition, (intptr_t) buttonOffset);
}

// saves current settings to the config file
static void SaveSettings(void)
{
    std::fstream file;
    file.open(CONFIG_PATH, std::ios_base::out | std::ios_base::trunc);

    if (file.is_open())
    {
        file << "controllerType=" << controllerType << std::endl;
        file << "axisOffset=" << axisOffset << std::endl;
        file << "buttonOffset=" << buttonOffset << std::endl;

        file.close();
    }
}

// handles the settings widget
static int SettingsWidgetHandler(XPWidgetMessage inMessage, XPWidgetID inWidget, long inParam1, long inParam2)
{
    if (inMessage == xpMessage_CloseButtonPushed)
    {
        if (XPIsWidgetVisible(settingsWidget))
        {
            SaveSettings();
            XPHideWidget(settingsWidget);
        }
    }
    else if (inMessage == xpMsg_ButtonStateChanged)
    {
        if (inParam1 == (long) dualShock3ControllerRadioButton)
        {
            if ((int) XPGetWidgetProperty(dualShock3ControllerRadioButton, xpProperty_ButtonState, 0) != 0)
            {
                controllerType = CONTROLLER_TYPE_DS3;
                UpdateSettingsWidgets();
            }
        }
        else if (inParam1 == (long) xbox360ControllerRadioButton)
        {
            if ((int) XPGetWidgetProperty(xbox360ControllerRadioButton, xpProperty_ButtonState, 0) != 0)
            {
                controllerType = CONTROLLER_TYPE_XBOX360;
                UpdateSettingsWidgets();
            }
        }
    }
    else if (inMessage == xpMsg_ScrollBarSliderPositionChanged)
    {
        if (inParam1 == (long) axisOffsetSlider)
            axisOffset = XPGetWidgetProperty(axisOffsetSlider, xpProperty_ScrollBarSliderPosition, 0);
        else if (inParam1 == (long) buttonOffsetSlider)
            buttonOffset = XPGetWidgetProperty(buttonOffsetSlider, xpProperty_ScrollBarSliderPosition, 0);

        UpdateSettingsWidgets();
    }
    else if (inMessage == xpMsg_PushButtonPressed)
    {
        if (inParam1 == (long) setDefaultAssignmentsButton)
            SetDefaultAssignments();
    }

    return 0;
}

// handles the menu-entries
static void MenuHandlerCallback(void *inMenuRef, void *inItemRef)
{
    // settings menu entry
    if ((long)inItemRef == 0)
    {
        if (settingsWidget == NULL)
        {
            // create settings widget
            int x = 10, y = 0, w = 350, h = 355;
            XPLMGetScreenSize(NULL, &y);
            y -= 100;

            int x2 = x + w;
            int y2 = y - h;

            // widget window
            settingsWidget = XPCreateWidget(x, y, x2, y2, 1, NAME" Settings", 1, 0, xpWidgetClass_MainWindow);

            // add close box
            XPSetWidgetProperty(settingsWidget, xpProperty_MainWindowHasCloseBoxes, 1);

            // add controller type sub window
            XPCreateWidget(x + 10, y - 30, x2 - 10, y - 100 - 10, 1, "Controller Type:", 0, settingsWidget, xpWidgetClass_SubWindow);

            // add controller type caption
            XPCreateWidget(x + 10, y - 30, x2 - 20, y - 45, 1, "Controller Type:", 0, settingsWidget, xpWidgetClass_Caption);

            // add dualshock 3 controller radio button
            dualShock3ControllerRadioButton = XPCreateWidget(x + 20, y - 60, x2 - 20, y - 75, 1, "DualShock 3 Controller", 0, settingsWidget, xpWidgetClass_Button);
            XPSetWidgetProperty(dualShock3ControllerRadioButton, xpProperty_ButtonType, xpRadioButton);
            XPSetWidgetProperty(dualShock3ControllerRadioButton, xpProperty_ButtonBehavior, xpButtonBehaviorRadioButton);

            // add xbox 360 controller radio button
            xbox360ControllerRadioButton = XPCreateWidget(x + 20, y - 85, x2 - 20, y - 100, 1, "Xbox 360 Controller", 0, settingsWidget, xpWidgetClass_Button);
            XPSetWidgetProperty(xbox360ControllerRadioButton, xpProperty_ButtonType, xpRadioButton);
            XPSetWidgetProperty(xbox360ControllerRadioButton, xpProperty_ButtonBehavior, xpButtonBehaviorRadioButton);

            // add offsets type sub window
            XPCreateWidget(x + 10, y - 125, x2 - 10, y - 195 - 10, 1, "Offsets:", 0, settingsWidget, xpWidgetClass_SubWindow);

            // add offsets type caption
            XPCreateWidget(x + 10, y - 125, x2 - 20, y - 140, 1, "Offsets:", 0, settingsWidget, xpWidgetClass_Caption);

            // add axis offset caption
            char stringBrightness[32];
            sprintf(stringBrightness, "Axis Offset: %d", axisOffset);
            axisOffsetCaption = XPCreateWidget(x + 30, y - 155, x2 - 50, y - 170, 1, stringBrightness, 0, settingsWidget, xpWidgetClass_Caption);

            // add axis offset slider
            axisOffsetSlider = XPCreateWidget(x + 195, y - 155, x2 - 15, y - 170, 1, "Axis Offset", 0, settingsWidget, xpWidgetClass_ScrollBar);
            XPSetWidgetProperty(axisOffsetSlider, xpProperty_ScrollBarMin, 0);
            XPSetWidgetProperty(axisOffsetSlider, xpProperty_ScrollBarMax, 95);

            // add button offset caption
            char stringContrast[32];
            sprintf(stringContrast, "Button Offset: %d", buttonOffset);
            buttonOffsetCaption = XPCreateWidget(x + 30, y - 180, x2 - 50, y - 195, 1, stringContrast, 0, settingsWidget, xpWidgetClass_Caption);

            // add button offset slider
            buttonOffsetSlider = XPCreateWidget(x + 195, y - 180, x2 - 15, y - 195, 1, "Button Offset", 0, settingsWidget, xpWidgetClass_ScrollBar);
            XPSetWidgetProperty(buttonOffsetSlider, xpProperty_ScrollBarMin, 0);
            XPSetWidgetProperty(buttonOffsetSlider, xpProperty_ScrollBarMax, 1583);

            // add assignments sub window
            XPCreateWidget(x + 10, y - 220, x2 - 10, y - 265 - 10, 1, "Assignments:", 0, settingsWidget, xpWidgetClass_SubWindow);

            // add assignments caption
            XPCreateWidget(x + 10, y - 220, x2 - 20, y - 235, 1, "Assignments:", 0, settingsWidget, xpWidgetClass_Caption);

            // add set default assignments preset button
            setDefaultAssignmentsButton = XPCreateWidget(x + 30, y - 250, x2 - 120, y - 265, 1, "Set Default Assignments", 0, settingsWidget, xpWidgetClass_Button);
            XPSetWidgetProperty(setDefaultAssignmentsButton, xpProperty_ButtonType, xpPushButton);

            // add about sub window
            XPCreateWidget(x + 10, y - 290, x2 - 10, y - 335 - 10, 1, "About:", 0, settingsWidget, xpWidgetClass_SubWindow);

            // add about caption
            XPCreateWidget(x + 10, y - 290, x2 - 20, y - 305, 1, NAME " " VERSION, 0, settingsWidget, xpWidgetClass_Caption);
            XPCreateWidget(x + 10, y - 305, x2 - 20, y - 320, 1, "Thank you for using " NAME " by Matteo Hausner", 0, settingsWidget, xpWidgetClass_Caption);
            XPCreateWidget(x + 10, y - 320, x2 - 20, y - 335, 1, "Contact: matteo.hausner@gmail.com or www.bwravencl.de", 0, settingsWidget, xpWidgetClass_Caption);

            // init checkbox and slider positions
            UpdateSettingsWidgets();

            // register widget handler
            XPAddWidgetCallback(settingsWidget, (XPWidgetFunc_t) SettingsWidgetHandler);
        }
        else
        {
            // settings widget already created
            if (!XPIsWidgetVisible(settingsWidget))
                XPShowWidget(settingsWidget);
        }
    }
}

static void DrawWindow(XPLMWindowID inWindowID, void *inRefcon)
{
}

static void HandleKey(XPLMWindowID inWindowID, char inKey, XPLMKeyFlags inFlags, char inVirtualKey, void *inRefcon, int losingFocus)
{
}

static int HandleMouseClick(XPLMWindowID inWindowID, int x, int y, XPLMMouseStatus inMouse, void *inRefcon)
{
    lastMouseUsageTime = XPLMGetElapsedTime();

    return 0;
}

static XPLMCursorStatus HandleCursor(XPLMWindowID inWindowID, int x, int y, void *inRefcon)
{
    static int lastX = x, lastY = y;

    if (x != lastX || y != lastY)
    {
        lastMouseUsageTime = XPLMGetElapsedTime();
        lastX = x;
        lastY = y;
    }

    return xplm_CursorDefault;
}

static int HandleMouseWheel(XPLMWindowID inWindowID, int x, int y, int wheel, int clicks, void *inRefcon)
{
    lastMouseUsageTime = XPLMGetElapsedTime();

    return 0;
}

// loads settings from the config file
static void LoadSettings(void)
{
    std::ifstream file;
    file.open(CONFIG_PATH);

    if (file.is_open())
    {
        std::string line;

        while (getline(file, line))
        {
            std::string val = line.substr(line.find("=") + 1);
            std::istringstream iss(val);

            if (line.find("controllerType") != -1)
                iss >> controllerType;
            else if (line.find("axisOffset") != -1)
                iss >> axisOffset;
            else if (line.find("buttonOffset") != -1)
                iss >> buttonOffset;
        }

        file.close();
    }
}

PLUGIN_API int XPluginStart(char *outName, char *outSig, char *outDesc)
{
#if IBM
    hGetProcIDDLL = LoadLibrary(L"XInput1_3.dll");
    FARPROC lpfnGetProcessID = GetProcAddress(HMODULE(hGetProcIDDLL), (LPCSTR) 100);
    getControllerData = pICFUNC(lpfnGetProcessID);
#endif

    // set plugin info
    strcpy(outName, NAME);
    strcpy(outSig, "de.bwravencl." NAME_LOWERCASE);
    strcpy(outDesc, NAME" allows flying X-Plane by gamepad!");

    // get paths in POSIX format
    XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);

    // obtain datarefs
    acfCockpitTypeDataRef = XPLMFindDataRef("sim/aircraft/view/acf_cockpit_type");
    acfPeXDataRef = XPLMFindDataRef("sim/aircraft/view/acf_peX");
    acfPeYDataRef = XPLMFindDataRef("sim/aircraft/view/acf_peY");
    acfPeZDataRef = XPLMFindDataRef("sim/aircraft/view/acf_peZ");
    acfRSCMingovPrpDataRef = XPLMFindDataRef("sim/aircraft/controls/acf_RSC_mingov_prp");
    acfRSCRedlinePrpDataRef = XPLMFindDataRef("sim/aircraft/controls/acf_RSC_redline_prp");
    acfNumEnginesDataRef = XPLMFindDataRef("sim/aircraft/engine/acf_num_engines");
    acfHasBetaDataRef = XPLMFindDataRef("sim/aircraft/overflow/acf_has_beta");
    acfSbrkEQDataRef = XPLMFindDataRef("sim/aircraft/parts/acf_sbrkEQ");
    acfMinPitchDataRef = XPLMFindDataRef("sim/aircraft/prop/acf_min_pitch");
    acfMaxPitchDataRef = XPLMFindDataRef("sim/aircraft/prop/acf_max_pitch");
    acfVertCantDataRef = XPLMFindDataRef("sim/aircraft/prop/acf_vertcant");
    ongroundAnyDataRef = XPLMFindDataRef("sim/flightmodel/failures/onground_any");
    groundspeedDataRef = XPLMFindDataRef("sim/flightmodel/position/groundspeed");
    cinemaVeriteDataRef = XPLMFindDataRef("sim/graphics/view/cinema_verite");
    pilotsHeadPsiDataRef = XPLMFindDataRef("sim/graphics/view/pilots_head_psi");
    pilotsHeadTheDataRef = XPLMFindDataRef("sim/graphics/view/pilots_head_the");
    viewTypeDataRef = XPLMFindDataRef("sim/graphics/view/view_type");
    hasJostickDataRef = XPLMFindDataRef("sim/joystick/has_joystick");
    joystickPitchNullzoneDataRef = XPLMFindDataRef("sim/joystick/joystick_pitch_nullzone");
    joystickRollNullzoneDataRef = XPLMFindDataRef("sim/joystick/joystick_roll_nullzone");
    joystickHeadingNullzoneDataRef = XPLMFindDataRef("sim/joystick/joystick_heading_nullzone");
    joystickAxisAssignmentsDataRef = XPLMFindDataRef("sim/joystick/joystick_axis_assignments");
    joystickButtonAssignmentsDataRef = XPLMFindDataRef("sim/joystick/joystick_button_assignments");
    joystickAxisValuesDataRef = XPLMFindDataRef("sim/joystick/joystick_axis_values");
    joystickAxisReverseDataRef = XPLMFindDataRef("sim/joystick/joystick_axis_reverse");
    joystickButtonValuesDataRef = XPLMFindDataRef("sim/joystick/joystick_button_values");
    leftBrakeRatioDataRef = XPLMFindDataRef("sim/cockpit2/controls/left_brake_ratio");
    rightBrakeRatioDataRef = XPLMFindDataRef("sim/cockpit2/controls/right_brake_ratio");
    speedbrakeRatioDataRef = XPLMFindDataRef("sim/cockpit2/controls/speedbrake_ratio");
    aileronTrimDataRef = XPLMFindDataRef("sim/cockpit2/controls/aileron_trim");
    elevatorTrimDataRef = XPLMFindDataRef("sim/cockpit2/controls/elevator_trim");
    rudderTrimDataRef = XPLMFindDataRef("sim/cockpit2/controls/rudder_trim");
    throttleRatioAllDataRef = XPLMFindDataRef("sim/cockpit2/engine/actuators/throttle_ratio_all");
    propPitchDegDataRef = XPLMFindDataRef("sim/cockpit2/engine/actuators/prop_pitch_deg");
    propRotationSpeedRadSecAllDataRef = XPLMFindDataRef("sim/cockpit2/engine/actuators/prop_rotation_speed_rad_sec_all");
    mixtureRatioAllDataRef = XPLMFindDataRef("sim/cockpit2/engine/actuators/mixture_ratio_all");
    carbHeatRatioDataRef = XPLMFindDataRef("sim/cockpit2/engine/actuators/carb_heat_ratio");
    cowlFlapRatioDataRef = XPLMFindDataRef("sim/cockpit2/engine/actuators/cowl_flap_ratio");
    thrustReverserDeployRatioDataRef = XPLMFindDataRef("sim/flightmodel2/engines/thrust_reverser_deploy_ratio");

    // create custom commands
    cycleResetViewCommand = XPLMCreateCommand(CYCLE_RESET_VIEW_COMMAND, "Cycle / Reset View");
    toggleArmSpeedBrakeOrToggleCarbHeatCommand = XPLMCreateCommand(TOGGLE_ARM_SPEED_BRAKE_OR_TOGGLE_CARB_HEAT_COMMAND, "Toggle / Arm Speedbrake / Toggle Carb Heat");
    toggleAutopilotOrDisableFlightDirectorCommand = XPLMCreateCommand(TOGGLE_AUTOPILOT_OR_DISABLE_FLIGHT_DIRECTOR_COMMAND, "Toggle Autopilot / Disable Flight Director");
    viewModifierCommand = XPLMCreateCommand(VIEW_MODIFIER_COMMAND, "View Modifier");
    propPitchOrThrottleModifierCommand = XPLMCreateCommand(PROP_PITCH_THROTTLE_MODIFIER_COMMAND, "Prop Pitch / Throttle Modifier");
    mixtureControlModifierCommand = XPLMCreateCommand(MIXTURE_CONTROL_MODIFIER_COMMAND, "Mixture Control Modifier");
    cowlFlapModifierCommand = XPLMCreateCommand(COWL_FLAP_MODIFIER_COMMAND, "Cowl Flap Modifier");
    trimModifierCommand = XPLMCreateCommand(TRIM_MODIFIER_COMMAND, "Trim Modifier");
    trimResetCommand = XPLMCreateCommand(TRIM_RESET_COMMAND_NAME_LOWERCASE, "Trim Reset");
    toggleBetaOrToggleReverseCommand = XPLMCreateCommand(TOGGLE_BETA_OR_TOGGLE_REVERSE_COMMAND, "Toggle Beta / Toggle Reverse");
    toggleMousePointerControlCommand = XPLMCreateCommand(TOGGLE_MOUSE_POINTER_CONTROL_COMMAND, "Toggle Mouse Pointer Control");
    pushToTalkCommand = XPLMCreateCommand(PUSH_TO_TALK_COMMAND, "Push-To-Talk");
    toggleBrakesCommand = XPLMCreateCommand(TOGGLE_BRAKES_COMMAND, "Toggle Brakes");

    // register custom commands
    XPLMRegisterCommandHandler(cycleResetViewCommand, CycleResetViewCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(toggleArmSpeedBrakeOrToggleCarbHeatCommand, ToggleArmSpeedBrakeOrToggleCarbHeatCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(toggleAutopilotOrDisableFlightDirectorCommand, ToggleAutopilotOrDisableFlightDirectorCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(viewModifierCommand, ViewModifierCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(propPitchOrThrottleModifierCommand, PropPitchOrThrottleModifierCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(mixtureControlModifierCommand, MixtureControlModifierCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(cowlFlapModifierCommand, CowlFlapModifierCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(trimModifierCommand, TrimModifierCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(trimResetCommand, TrimResetCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(toggleBetaOrToggleReverseCommand, ToggleBetaOrToggleReverseCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(toggleMousePointerControlCommand, ToggleMousePointerControlCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(pushToTalkCommand, PushToTalkCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(toggleBrakesCommand, ToggleBrakesCommandHandler, 1, NULL);

    // read and apply config file
    LoadSettings();

    // create fake window
    XPLMCreateWindow_t fakeWindowParameters;
    memset(&fakeWindowParameters, 0, sizeof(fakeWindowParameters));
    fakeWindowParameters.structSize = sizeof(fakeWindowParameters);
    fakeWindowParameters.left = 0;
    int x = 0, y = 0;
    XPLMGetScreenSize(&x, &y);
    fakeWindowParameters.top = y;
    fakeWindowParameters.right = x;
    fakeWindowParameters.bottom = 0;
    fakeWindowParameters.visible = 1;
    fakeWindowParameters.drawWindowFunc = DrawWindow;
    fakeWindowParameters.handleKeyFunc = HandleKey;
    fakeWindowParameters.handleMouseClickFunc = HandleMouseClick;
    fakeWindowParameters.handleCursorFunc = HandleCursor;
    fakeWindowParameters.handleMouseWheelFunc = HandleMouseWheel;
    fakeWindow = XPLMCreateWindowEx(&fakeWindowParameters);

    // register flight loop callbacks
    XPLMRegisterFlightLoopCallback(UpdateFakeWindowCallback, -1, NULL);
    XPLMRegisterFlightLoopCallback(FlightLoopCallback, -1, NULL);

    // create menu-entries
    int subMenuItem = XPLMAppendMenuItem(XPLMFindPluginsMenu(), NAME, 0, 1);
    XPLMMenuID menu = XPLMCreateMenu(NAME, XPLMFindPluginsMenu(), subMenuItem, MenuHandlerCallback, 0);
    XPLMAppendMenuItem(menu, "Settings", (void*) 0, 1);

#if LIN
    display = XOpenDisplay(NULL);
    if(display != NULL)
    {
        int screenNumber = DefaultScreen(display);
        XEvent event;
        XQueryPointer(display, RootWindow(display, DefaultScreen(display)), &event.xbutton.root, &event.xbutton.window, &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
    }
#endif

    return 1;
}

PLUGIN_API void	XPluginStop(void)
{
    // revert any remaining button assignments
    while (!buttonAssignmentsStack.empty())
        PopButtonAssignments();

    // unregister custom commands
    XPLMUnregisterCommandHandler(cycleResetViewCommand, CycleResetViewCommandHandler, 1, NULL);
    XPLMUnregisterCommandHandler(toggleArmSpeedBrakeOrToggleCarbHeatCommand, ToggleArmSpeedBrakeOrToggleCarbHeatCommandHandler, 1, NULL);
    XPLMUnregisterCommandHandler(toggleAutopilotOrDisableFlightDirectorCommand, ToggleAutopilotOrDisableFlightDirectorCommandHandler, 1, NULL);
    XPLMUnregisterCommandHandler(viewModifierCommand, ViewModifierCommandHandler, 1, NULL);
    XPLMUnregisterCommandHandler(propPitchOrThrottleModifierCommand, PropPitchOrThrottleModifierCommandHandler, 1, NULL);
    XPLMUnregisterCommandHandler(mixtureControlModifierCommand, MixtureControlModifierCommandHandler, 1, NULL);
    XPLMUnregisterCommandHandler(cowlFlapModifierCommand, CowlFlapModifierCommandHandler, 1, NULL);
    XPLMUnregisterCommandHandler(trimModifierCommand, TrimModifierCommandHandler, 1, NULL);
    XPLMUnregisterCommandHandler(toggleBetaOrToggleReverseCommand, ToggleBetaOrToggleReverseCommandHandler, 1, NULL);
    XPLMUnregisterCommandHandler(toggleMousePointerControlCommand, ToggleMousePointerControlCommandHandler, 1, NULL);
    XPLMUnregisterCommandHandler(pushToTalkCommand, PushToTalkCommandHandler, 1, NULL);
    XPLMUnregisterCommandHandler(toggleBrakesCommand, ToggleBrakesCommandHandler, 1, NULL);

    // register flight loop callbacks
    XPLMUnregisterFlightLoopCallback(UpdateFakeWindowCallback, NULL);
    XPLMUnregisterFlightLoopCallback(FlightLoopCallback, NULL);

#if IBM
    if (hGetProcIDDLL != NULL)
        FreeLibrary(hGetProcIDDLL);
#elif LIN
    if(display != NULL)
        XCloseDisplay(display);
#endif
}

PLUGIN_API void XPluginDisable(void)
{
}

PLUGIN_API int XPluginEnable(void)
{
    return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFromWho, long inMessage, void *inParam)
{
    switch (inMessage)
    {
    case XPLM_MSG_PLANE_LOADED:
        bringFakeWindowToFront = 0;
        // reset the default head position if a new plane is loaded so that it is updated during the next flight loop
        defaultHeadPositionX = FLT_MAX;
        defaultHeadPositionY = FLT_MAX;
        defaultHeadPositionZ = FLT_MAX;
    case XPLM_MSG_AIRPORT_LOADED:
        // schedule a switch to the 3D cockpit view during the next flight loop
        switchTo3DCommandLook = 0;
        if (Has2DPanel() == 0)
            switchTo3DCommandLook = 1;
        break;
    }
}
