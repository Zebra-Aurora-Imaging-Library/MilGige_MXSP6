﻿/********************************************************************************/
/*
* File name: milgige.cpp
*
* Synopsis:  This program demonstrates new MIL features for managing GigE Vision(tm)
*            devices using the Matrox Driver for GigE Vision(tm).
*
* Note:      The camera features inquired by this example are taken from the GenICam(tm)
*            Standard Feature Naming Convention (SFNC). It can be downloaded from the
*            European Machine Vision Association's web site: http://www.emva.org/
*            Only a subset of defined SNFC features are used by this example for
*            illustrative purposes.
*
*            Some of the features accessed by this example might not be implemented
*            by your camera, therefore MIL error prints are temporarily disabled while
*            the process of feature enumeration is done. SFNC features not supported
*            by your camera will be marked with N/A. Because of this the code for
*            enumerating features is made more complex to account for varying
*            implementations by camera manufacturers.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-YYYY.
* All Rights Reserved
*/

/* Headers. */
#include <mil.h>
#include <vector>
#if M_MIL_USE_WINDOWS
#include <windows.h>
#endif

using namespace std;

/* Set the PRINT_LOOKUP_TABLE define to 1 to print your camera's LUT       */
/* values (if present).                                                    */
#define PRINT_LOOKUP_TABLE       0

/* List of function prototypes used to enumerate and print camera features. */
void CameraPrintDeviceControls(MIL_ID MilDigitizer);
void CameraPrintTransportLayerControls(MIL_ID MilDigitizer);
void CameraPrintImageFormatControls(MIL_ID MilDigitizer);
void CameraPrintAcquisitionControls(MIL_ID MilDigitizer);
void CameraPrintIOControls(MIL_ID MilDigitizer);
void CameraPrintCounterAndTimerControls(MIL_ID MilDigitizer);
void CameraPrintEventControls(MIL_ID MilDigitizer);
void CameraPrintLUT(MIL_ID MilDigitizer);
void CameraPrintDeviceCapabilities(MIL_ID MilDigitizer);
void CameraPrintControlProtocolCapabilities(MIL_ID MilDigitizer);
void CameraPrintStreamProtocolCapabilities(MIL_ID MilDigitizer);
void CameraPrintMessageProtocolCapabilities(MIL_ID MilDigitizer);
void CameraPrintStreamChannelCapabilities(MIL_ID MilDigitizer);
void CameraPrintPhysicalLinkConfigurationCapabilities(MIL_ID MilDigitizer);
void CameraPrintNetworkInterfaceCapabilities(MIL_ID MilDigitizer);
void CameraPrintNetworkInterfaceConfiguration(MIL_ID MilDigitizer);

/* List of function prototypes used to perform triggered acquisition. */
typedef enum {eSingleFrame=1, eMultiFrame, eContinuous} eTriggerType;
void SetTriggerControls(MIL_ID MilDigitizer, eTriggerType& Type, MIL_INT64& NbFrames,
   MIL_STRING& oTriggerSelector, bool &SoftwareTriggerSelected);
void SelectTriggerSource(MIL_ID MilDigitizer, bool& SoftwareTriggerSelected);
void ResetTriggerControls(MIL_ID MilDigitizer);
void DoTriggeredAcquisition(MIL_ID MilSystem, MIL_ID MilDigitizer, MIL_ID MilImageDisp);

/* Global variables used to store camera capabilities. */
bool ContinuousAMSupport = false;
bool SingleFrameAMSupport = false;
bool MultiFrameAMSupport = false;
MIL_INT MultipleAcquisitionModeSupport = 0;
bool CanTriggerAcquisitionStart = false;
bool CanTriggerFrameStart = false;

/* Main function. */
int MosMain(void)
   {
   MIL_ID MilApplication,  /* Application identifier.  */
          MilSystem,       /* System identifier.       */
          MilDisplay,      /* Display identifier.      */
          MilDigitizer,    /* Digitizer identifier.    */
          MilImage;        /* Image buffer identifier. */
   MIL_INT SystemType;
   MIL_INT Selection;

   /* Allocate defaults. */
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay, M_NULL, M_NULL);

   /* Get information on the system we are using and print a welcome message to the console. */
   MsysInquire(MilSystem, M_SYSTEM_TYPE, &SystemType);

   if ((SystemType != M_SYSTEM_GIGE_VISION_TYPE) &&
       (SystemType != M_SYSTEM_GEVIQ_TYPE))
      {
      /* Print error message. */
      MosPrintf(MIL_TEXT("This example program can only be used with the Matrox Driver for ")
                MIL_TEXT("GigE Vision or the\nMatrox GevIQ Smart GigE Vision Adapter.\n"));
      MosPrintf(MIL_TEXT("Please ensure that the default system type is set accordingly in ")
                MIL_TEXT("MIL Config.\n"));
      MosPrintf(MIL_TEXT("-------------------------------------------------------------\n\n"));
      MosPrintf(MIL_TEXT("Press <enter> to quit.\n"));
      MosGetch();
      MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);
      return 1;
      }

   /* Allocate the digitizer controlling the camera. */
   MdigAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilDigitizer);

   /* In cases where the preferred method for device allocation requires allocating with     */
   /* a user-defined name the following code can be used. "MyCameraName" must be replaced    */
   /* with the actual camera name written in the camera.                                     */

   /* MdigAlloc(MilSystem, M_GC_CAMERA_ID(MIL_TEXT("MyCameraName")), MIL_TEXT("M_DEFAULT"),  */
   /*    M_GC_DEVICE_NAME, &MilDigitizer);                                              */


   /* Allocate grab and display buffer. */
   MbufAllocColor(MilSystem,
      MdigInquire(MilDigitizer, M_SIZE_BAND, M_NULL),
      MdigInquire(MilDigitizer, M_SIZE_X, M_NULL),
      MdigInquire(MilDigitizer, M_SIZE_Y, M_NULL),
      MdigInquire(MilDigitizer, M_TYPE, M_NULL),
      M_IMAGE + M_DISP + M_GRAB,
      &MilImage);
   MbufClear(MilImage, 0);

   /* Print a message. */
   MosPrintf(MIL_TEXT("This example showcases GigE Vision specific features.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to start.\n\n"));
   MosGetch();
#if M_MIL_USE_WINDOWS
   system("cls");
#endif
   MosPrintf(MIL_TEXT("------------------------------------------------------------\n\n"));
   MosPrintf(MIL_TEXT("                  Camera features summary.                  \n"));

   /* Disable error printing in case camera is not SFNC compliant with regard
      to some of the features it supports. */
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);

   /* Enumerate and print camera features. */
   CameraPrintDeviceControls(MilDigitizer);
   CameraPrintTransportLayerControls(MilDigitizer);
   CameraPrintImageFormatControls(MilDigitizer);
   CameraPrintAcquisitionControls(MilDigitizer);
   CameraPrintEventControls(MilDigitizer);
   CameraPrintIOControls(MilDigitizer);
   CameraPrintCounterAndTimerControls(MilDigitizer);
#if PRINT_LOOKUP_TABLE
   CameraPrintLUT(MilDigitizer);
#endif

   /* Re-enable error printing. */
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   /* Print a message. */
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n"));
   MosGetch();

   CameraPrintDeviceCapabilities(MilDigitizer);

   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n"));
   MosGetch();

   /* Clear the console text. */
#if M_MIL_USE_WINDOWS
   system("cls");
#endif
   /* Pop-up the camera feature browser GUI. */
   MdigControl(MilDigitizer, M_GC_FEATURE_BROWSER, M_OPEN+M_ASYNCHRONOUS);
   
   /* Print a message. */
   MosPrintf(MIL_TEXT("\nDisplaying the camera's feature browser.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetch();

   /* Start a continuous acquisition. */
   MdispSelect(MilDisplay, MilImage);
   MdigGrabContinuous(MilDigitizer, MilImage);

   /* Print a message. */
   MosPrintf(MIL_TEXT("\nContinuous image grab in progress.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to stop.\n"));
   MosGetch();
   
   /* Stop the continuous acquisition. */
   MdigHalt(MilDigitizer);

   /* If we can trigger AcquisitionStart or FrameStart events, ask if we should do
      triggered grabs. */
   if(CanTriggerAcquisitionStart || CanTriggerFrameStart)
      {
#if M_MIL_USE_WINDOWS
      system("cls");
#endif
      MosPrintf(MIL_TEXT("\nYour camera supports acquisition triggers.\n"));
      MosPrintf(MIL_TEXT("Do you want to test triggered acquisition (Y/N)? "));
      Selection = MosGetch();
      MosPrintf(MIL_TEXT("\n"));
      if ((Selection == 'Y') || (Selection == 'y'))
         DoTriggeredAcquisition(MilSystem, MilDigitizer, MilImage);
         }
   else
      {
      MosPrintf(MIL_TEXT("\nPress <Enter> to quit.\n"));
      MosGetch();
      }


   MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImage);

   return 0;
   }

/* Prints SFNC features */
void CameraPrintDeviceControls(MIL_ID MilDigitizer)
   {
   MIL_STRING CameraVendor;
   MIL_STRING CameraModel;
   MIL_STRING CameraSerialNumber;
   MIL_STRING CameraUserName;
   MIL_STRING CameraScanType;
   MIL_STRING InterfaceName;
   MIL_STRING IpAddress;

   MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("DeviceVendorName"), M_TYPE_STRING, CameraVendor);
   MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("DeviceModelName"), M_TYPE_STRING, CameraModel);
   MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("DeviceID"), M_TYPE_STRING, CameraSerialNumber);
   MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("DeviceUserID"), M_TYPE_STRING, CameraUserName);
   MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("DeviceScanType"), M_TYPE_STRING, CameraScanType);

   MosPrintf(MIL_TEXT("\n------------------ Camera Device Controls ------------------\n\n"));
   MosPrintf(MIL_TEXT("%30s %s %s\n"), MIL_TEXT("Camera:"), CameraVendor.empty() ? MIL_TEXT("N/A") : CameraVendor.c_str(), CameraModel.empty() ? MIL_TEXT("N/A") : CameraModel.c_str());
   MosPrintf(MIL_TEXT("%30s %s\n"), MIL_TEXT("Serial number:"), CameraSerialNumber.empty() ? MIL_TEXT("N/A") : CameraSerialNumber.c_str());
   MosPrintf(MIL_TEXT("%30s %s\n"), MIL_TEXT("User-defined name:"), CameraUserName.empty() ? MIL_TEXT("N/A") : CameraUserName.c_str());
   MosPrintf(MIL_TEXT("%30s %s\n"), MIL_TEXT("Device scan type:"), CameraScanType.empty() ? MIL_TEXT("N/A") : CameraScanType.c_str());

   MdigInquire(MilDigitizer, M_GC_LOCAL_IP_ADDRESS_STRING, IpAddress);
   MdigInquire(MilDigitizer, M_GC_INTERFACE_NAME, InterfaceName);
   MosPrintf(MIL_TEXT("%30s %s (%s)\n"), MIL_TEXT("Camera is connected to:"), InterfaceName.c_str(), IpAddress.c_str());
   }

/* Prints SFNC features */
void CameraPrintImageFormatControls(MIL_ID MilDigitizer)
   {
   MIL_INT64 SensorWidth = 0, SensorHeight = 0;
   MIL_INT64 Width = 0, Height = 0, WidthMax = 0, HeightMax = 0, WidthMin = 0, HeightMin = 0;
   MIL_INT PixFrmtCount = 0;
   MIL_BOOL ReverseX = M_FALSE, ReverseY = M_FALSE;
   vector<MIL_STRING> PixelFormats;

   MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE,  MIL_TEXT("SensorWidth"),   M_TYPE_INT64, &SensorWidth);
   MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE,  MIL_TEXT("SensorHeight"),  M_TYPE_INT64, &SensorHeight);
   MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE,  MIL_TEXT("Width"),         M_TYPE_INT64, &Width);
   MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE,  MIL_TEXT("Height"),        M_TYPE_INT64, &Height);
   MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE,  MIL_TEXT("ReverseX"),      M_TYPE_BOOLEAN, &ReverseX);
   MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE,  MIL_TEXT("ReverseY"),      M_TYPE_BOOLEAN, &ReverseY);
   MdigInquireFeature(MilDigitizer, M_FEATURE_MAX,    MIL_TEXT("Width"),         M_TYPE_INT64, &WidthMax);
   MdigInquireFeature(MilDigitizer, M_FEATURE_MAX,    MIL_TEXT("Height"),        M_TYPE_INT64, &HeightMax);
   MdigInquireFeature(MilDigitizer, M_FEATURE_MIN,    MIL_TEXT("Width"),         M_TYPE_INT64, &WidthMin);
   MdigInquireFeature(MilDigitizer, M_FEATURE_MIN,    MIL_TEXT("Height"),        M_TYPE_INT64, &HeightMin);

   MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_COUNT, MIL_TEXT("PixelFormat"), M_TYPE_MIL_INT, &PixFrmtCount);
   if(PixFrmtCount)
      {
      PixelFormats.assign(PixFrmtCount, MIL_TEXT(""));
      for (size_t i = 0; i<PixelFormats.size(); i++)
         {
         MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_NAME+i, MIL_TEXT("PixelFormat"), M_TYPE_STRING, PixelFormats[i]);
         }
      }

   MosPrintf(MIL_TEXT("\n------------------- Image Format Controls ------------------\n\n"));
   MosPrintf(MIL_TEXT("%30s %4lld x %-4lld\n"), MIL_TEXT("Sensor size:"), SensorWidth?SensorWidth:Width, SensorHeight?SensorHeight:Height);
   MosPrintf(MIL_TEXT("%30s %4lld x %-4lld\n"), MIL_TEXT("Current ROI:"), Width, Height);
   MosPrintf(MIL_TEXT("%30s %4lld x %-4lld;%4lld x %-4lld\n"), MIL_TEXT("Maximum and Minimum ROI:"), WidthMax, HeightMax, WidthMin, HeightMin);
   MosPrintf(MIL_TEXT("\n%30s %s\n"), MIL_TEXT("Image Reverse X:"), ReverseX ? MIL_TEXT("true") : MIL_TEXT("false"));
   MosPrintf(MIL_TEXT("%30s %s\n"), MIL_TEXT("Image Reverse Y:"), ReverseY ? MIL_TEXT("true") : MIL_TEXT("false"));
   MosPrintf(MIL_TEXT("\n%30s %s\n"), MIL_TEXT("Supported pixel formats:"), PixelFormats.size() ? PixelFormats[0].c_str() : MIL_TEXT("N/A"));
   for (size_t i = 1; i<PixelFormats.size(); i++)
      MosPrintf(MIL_TEXT("                               %s\n"), PixelFormats[i].c_str());
   }

/* Prints SFNC features */
void CameraPrintAcquisitionControls(MIL_ID MilDigitizer)
   {
   vector<MIL_STRING> AcquisitionModes;
   vector<MIL_STRING> TriggerSelectors;
   vector<MIL_STRING> ExposureModes;
   MIL_DOUBLE ExposureTime = 0.0;
   MIL_INT AcMdCount = 0;
   MIL_INT TgSelCount = 0;
   MIL_INT ExMdCount = 0;

   MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_COUNT, MIL_TEXT("AcquisitionMode"), M_TYPE_MIL_INT, &AcMdCount);
   if(AcMdCount)
      {
      AcquisitionModes.assign(AcMdCount, MIL_TEXT(""));
      if(AcMdCount > 1)
         MultipleAcquisitionModeSupport = 1;

   for (size_t i = 0; i < AcquisitionModes.size(); i++)
      {
         MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_NAME + i, MIL_TEXT("AcquisitionMode"), M_TYPE_STRING, AcquisitionModes[i]);

      if(MIL_TEXT("Continuous") == AcquisitionModes[i])
         ContinuousAMSupport = true;
      else if(MIL_TEXT("SingleFrame") == AcquisitionModes[i])
         SingleFrameAMSupport = true;
      else if(MIL_TEXT("MultiFrame") == AcquisitionModes[i])
         MultiFrameAMSupport = true;
      }
      }

   MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_COUNT, MIL_TEXT("TriggerSelector"), M_TYPE_MIL_INT, &TgSelCount);
   if(TgSelCount)
      {
      TriggerSelectors.assign(TgSelCount, MIL_TEXT(""));
   for (size_t i = 0; i < TriggerSelectors.size(); i++)
      {
         MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_NAME + i, MIL_TEXT("TriggerSelector"), M_TYPE_STRING, TriggerSelectors[i]);

      if(MIL_TEXT("AcquisitionStart") == TriggerSelectors[i])
         CanTriggerAcquisitionStart = true;
      else if(MIL_TEXT("FrameStart") == TriggerSelectors[i])
         CanTriggerFrameStart = true;
      }
      }

   MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_COUNT, MIL_TEXT("ExposureMode"), M_TYPE_MIL_INT, &ExMdCount);
   if(ExMdCount)
      {
      ExposureModes.assign(ExMdCount, MIL_TEXT(""));
      for (size_t i = 0; i < ExposureModes.size(); i++)
         {
         MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_NAME+i, MIL_TEXT("ExposureMode"), M_TYPE_STRING, ExposureModes[i]);
         }
      }

   MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("ExposureTime"), M_TYPE_MIL_DOUBLE, &ExposureTime);

   MosPrintf(MIL_TEXT("\n------------------- Acquisition Controls -------------------\n\n"));
   
   MosPrintf(MIL_TEXT("%30s %s\n"), MIL_TEXT("Supported acquisition modes:"), AcquisitionModes.size() ? AcquisitionModes[0].c_str() : MIL_TEXT("N/A"));
   for (size_t i = 1; i < AcquisitionModes.size(); i++)
      MosPrintf(MIL_TEXT("%30s %s\n"), MIL_TEXT(""), AcquisitionModes[i].c_str());

   MosPrintf(MIL_TEXT("\n%30s %s\n"), MIL_TEXT("Supported trigger selectors:"), TriggerSelectors.size() ? TriggerSelectors[0].c_str() : MIL_TEXT("N/A"));
   for (size_t i = 1; i < TriggerSelectors.size(); i++)
      MosPrintf(MIL_TEXT("%30s %s\n"), MIL_TEXT(""), TriggerSelectors[i].c_str());

   MosPrintf(MIL_TEXT("\n%30s %s\n"), MIL_TEXT("Supported exposure modes:"), ExposureModes.size() ? ExposureModes[0].c_str() : MIL_TEXT("N/A"));
   for (size_t i = 1; i < ExposureModes.size(); i++)
      MosPrintf(MIL_TEXT("%30s %s\n"), MIL_TEXT(""), ExposureModes[i].c_str());

   if(ExposureTime == 0.0)
      MosPrintf(MIL_TEXT("\n%30s %s\n"), MIL_TEXT("Exposure time:"), MIL_TEXT("N/A"));
   else
      MosPrintf(MIL_TEXT("\n%30s %.1f us\n"), MIL_TEXT("Exposure time:"), ExposureTime);
   }

/* Prints SFNC features */
void CameraPrintIOControls(MIL_ID MilDigitizer)
   {
   vector<MIL_STRING> Lines;
   vector<MIL_STRING> LineFormats;
   vector<MIL_STRING> LineModes;
   MIL_INT LineCnt = 0;

   MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_COUNT, MIL_TEXT("LineSelector"), M_TYPE_MIL_INT, &LineCnt);
   if(LineCnt)
      {
      Lines.assign(LineCnt, MIL_TEXT(""));
      LineFormats.assign(LineCnt, MIL_TEXT(""));
      LineModes.assign(LineCnt, MIL_TEXT(""));
   for (size_t i = 0; i < Lines.size(); i++)
      {
         MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_NAME+i, MIL_TEXT("LineSelector"), M_TYPE_STRING, Lines[i]);
         }

      for (size_t i = 0; i < Lines.size(); i++)
         {
      MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("LineSelector"), M_TYPE_STRING, Lines[i]);
         MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("LineMode"), M_TYPE_STRING, LineModes[i]);
         MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("LineFormat"), M_TYPE_STRING, LineFormats[i]);
      }
      }
   
   MosPrintf(MIL_TEXT("\n------------------- Digital I/O Controls -------------------\n\n"));

   MosPrintf(MIL_TEXT("%7s%-18s%-18s%-18s%7s\n\n"), MIL_TEXT(""), MIL_TEXT("Name"), MIL_TEXT("Mode"), MIL_TEXT("Format"), MIL_TEXT(""));

   if(LineCnt == 0)
      MosPrintf(MIL_TEXT("%7s%-18s%-18s%-18s%7s\n"), MIL_TEXT(""), MIL_TEXT("N/A"), MIL_TEXT("N/A"), MIL_TEXT("N/A"), MIL_TEXT(""));
   else
      {
      for (size_t i = 0; i < Lines.size(); i++)
         MosPrintf(MIL_TEXT("%7s%-18s%-18s%-18s%7s\n"), MIL_TEXT(""), Lines[i].c_str(), LineModes[i].c_str(), LineFormats[i].c_str(), MIL_TEXT(""));
      }
   }

/* Prints SFNC features */
void CameraPrintTransportLayerControls(MIL_ID MilDigitizer)
   {
   MIL_INT64 GigEMajorVersion = 0;
   MIL_INT64 GigEMinorVersion = 0;
   MIL_INT64 StreamChannelPacketSize = 0;
   MIL_INT64 CurrentIp = -1;
   MIL_INT64 MAC = -1;
   MIL_UINT8* Ip = (MIL_UINT8*)&CurrentIp;
   MIL_UINT8* pMAC = (MIL_UINT8*)&MAC;

   MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("GevVersionMajor"),      M_TYPE_INT64, &GigEMajorVersion);
   MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("GevVersionMinor"),      M_TYPE_INT64, &GigEMinorVersion);
   MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("GevSCPSPacketSize"),    M_TYPE_INT64, &StreamChannelPacketSize);
   MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("GevMACAddress"),        M_TYPE_INT64, &MAC);
   MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("GevCurrentIPAddress"),  M_TYPE_INT64, &CurrentIp);

   MosPrintf(MIL_TEXT("\n-------------- Camera Transport Layer Controls -------------\n\n"));
   if(GigEMajorVersion == 0)
      MosPrintf(MIL_TEXT("%30s N/A\n"), MIL_TEXT("GigE Vision Version:"));
   else 
      MosPrintf(MIL_TEXT("%30s %d.%d\n"), MIL_TEXT("GigE Vision Version:"), (int)GigEMajorVersion, (int)GigEMinorVersion);

   if(MAC == -1)
      MosPrintf(MIL_TEXT("%30s N/A\n"), MIL_TEXT("MAC Address:"));
   else
      MosPrintf(MIL_TEXT("%30s %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n"), MIL_TEXT("MAC Address:"), pMAC[5], pMAC[4], pMAC[3], pMAC[2], pMAC[1], pMAC[0]);

   if(CurrentIp == -1)
      MosPrintf(MIL_TEXT("%30s N/A\n"), MIL_TEXT("Current IP Address:"));
   else
      MosPrintf(MIL_TEXT("%30s %d.%d.%d.%d\n"), MIL_TEXT("Current IP Address:"), (int)Ip[3], (int)Ip[2], (int)Ip[1], (int)Ip[0]);
   
   MosPrintf(MIL_TEXT("%30s %lld\n"), MIL_TEXT("Packet size:"), (long long)StreamChannelPacketSize);
   }

/* Prints SFNC features */
void CameraPrintCounterAndTimerControls(MIL_ID MilDigitizer)
   {
   vector<MIL_STRING> Counters;
   vector<MIL_STRING> CountersStatus;
   vector<MIL_STRING> Timers;
   vector<MIL_STRING> TimersStatus;
   MIL_INT CountersCnt = 0;
   MIL_INT TimersCnt = 0;

   MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_COUNT, MIL_TEXT("CounterSelector"), M_TYPE_MIL_INT, &CountersCnt);
   if(CountersCnt)
      {
      Counters.assign(CountersCnt, MIL_TEXT(""));
      CountersStatus.assign(CountersCnt, MIL_TEXT(""));
   for (size_t i = 0; i < Counters.size(); i++)
      {
         MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_NAME + i, MIL_TEXT("CounterSelector"), M_TYPE_STRING, Counters[i]);
      }
      }

   MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_COUNT, MIL_TEXT("TimerSelector"), M_TYPE_MIL_INT, &TimersCnt);
   if(TimersCnt)
      {
      Timers.assign(TimersCnt, MIL_TEXT(""));
      TimersStatus.assign(TimersCnt, MIL_TEXT(""));
   for (size_t i = 0; i < Timers.size(); i++)
      {
         MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_NAME + i, MIL_TEXT("TimerSelector"), M_TYPE_STRING, Timers[i]);
         }
      }

   for (size_t i = 0; i < CountersStatus.size(); i++)
      {
      MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("CounterSelector"), M_TYPE_STRING, Counters[i]);
      MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("CounterStatus"), M_TYPE_STRING, CountersStatus[i]);
      }

   for (size_t i = 0; i < TimersStatus.size(); i++)
      {
      MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("TimerSelector"), M_TYPE_STRING, Timers[i]);
      MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("TimerStatus"), M_TYPE_STRING, TimersStatus[i]);
      }

   MosPrintf(MIL_TEXT("\n---------------- Counter and Timer Controls ----------------\n\n"));

   MosPrintf(MIL_TEXT("%20s%-15s%-15s%20s\n\n"), MIL_TEXT(""), MIL_TEXT("Name"), MIL_TEXT("Status"), MIL_TEXT(""));

   if (Counters.size() == 0)
      MosPrintf(MIL_TEXT("%20s%-15s%-15s%20s\n"), MIL_TEXT(""), MIL_TEXT("N/A"), MIL_TEXT("N/A"), MIL_TEXT(""));
   else
      {
      for (size_t i = 0; i < Counters.size(); i++)
         MosPrintf(MIL_TEXT("%20s%-15s%-15s%20s\n"), MIL_TEXT(""), Counters[i].c_str(), CountersStatus[i].c_str(), MIL_TEXT(""));
      }

   if (Timers.size() == 0)
      MosPrintf(MIL_TEXT("%20s%-15s%-15s%20s\n"), MIL_TEXT(""), MIL_TEXT("N/A"), MIL_TEXT("N/A"), MIL_TEXT(""));
   else
      {
      for (size_t i = 0; i < Timers.size(); i++)
         MosPrintf(MIL_TEXT("%20s%-15s%-15s%20s\n"), MIL_TEXT(""), Timers[i].c_str(), TimersStatus[i].c_str(), MIL_TEXT(""));
      }
   }

/* Prints SFNC features */
void CameraPrintEventControls(MIL_ID MilDigitizer)
   {
   vector<MIL_STRING> Events;
   MIL_INT EventCnt = 0;

   MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_COUNT, MIL_TEXT("EventSelector"), M_TYPE_MIL_INT, &EventCnt);
   if(EventCnt)
      {
      Events.assign(EventCnt, MIL_TEXT(""));
      for (size_t i = 0; i < Events.size(); i++)
         {
         MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_NAME + i, MIL_TEXT("EventSelector"), M_TYPE_STRING, Events[i]);
         }
      }
   
   MosPrintf(MIL_TEXT("\n---------------------- Event Controls ----------------------\n\n"));
   MosPrintf(MIL_TEXT("%30s "), MIL_TEXT("Supported events:"));

   if(Events.size() == 0)
      MosPrintf(MIL_TEXT("N/A\n"));
   else
      {
      MosPrintf(MIL_TEXT("%s\n"), Events[0].c_str());
      for(size_t i = 1; i < Events.size(); i++)
         MosPrintf(MIL_TEXT("                               %s\n"), Events[i].c_str());
      }
   }

void CameraPrintLUT(MIL_ID MilDigitizer)
   {
   MIL_INT64 MinIndex = 0, MaxIndex = 0, LutValue = 0;
   MIL_INT LutSelCount = 0;
   vector<MIL_STRING> LutSelectors;
   MIL_STRING Str(16, '\0');

   MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_COUNT, MIL_TEXT("LUTSelector"), M_TYPE_MIL_INT, &LutSelCount);
   if(LutSelCount)
      {
      LutSelectors.assign(LutSelCount, MIL_TEXT(""));
   for (size_t i = 0; i < LutSelectors.size(); i++)
      {
         MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_NAME + i, MIL_TEXT("LUTSelector"), M_TYPE_STRING, LutSelectors[i]);

      MosPrintf(MIL_TEXT("\nPress <Enter> to print %s Lookup table.\n"), LutSelectors[i].c_str());
      MosGetch();
#if M_MIL_USE_WINDOWS
      system("cls");
#endif

      MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("LUTSelector"), M_TYPE_STRING, LutSelectors[i]);

      MosPrintf(MIL_TEXT("\n------- Printing (%s) lookup table contents -----\n"), LutSelectors[i].c_str());

      MdigInquireFeature(MilDigitizer, M_FEATURE_MIN, MIL_TEXT("LUTIndex"), M_TYPE_INT64, &MinIndex);
      MdigInquireFeature(MilDigitizer, M_FEATURE_MAX, MIL_TEXT("LUTIndex"), M_TYPE_INT64, &MaxIndex);

      for (MIL_INT64 j = MinIndex; j <= MaxIndex; j++)
         {
         MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("LUTIndex"), M_TYPE_INT64, &j);
         MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("LUTValue"), M_TYPE_INT64, &LutValue);

         if ((j % 5) == 0)
            MosPrintf(MIL_TEXT("\n"));

         MosSprintf(&Str[0], Str.size(), MIL_TEXT("[%lld]"), (long long)j);
         MosPrintf(MIL_TEXT("%7s : %-6lld"), Str.c_str(), (long long)LutValue);
         }
      }
   }
   }

void CameraPrintDeviceCapabilities(MIL_ID MilDigitizer)
   {
#if M_MIL_USE_WINDOWS
   system("cls");
#endif
   MosPrintf(MIL_TEXT("------------------------------------------------------------\n\n"));
   MosPrintf(MIL_TEXT("                      Camera capabilities.                  \n\n"));
   CameraPrintControlProtocolCapabilities(MilDigitizer);
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue\n"));
   MosGetch();
#if M_MIL_USE_WINDOWS
   system("cls");
#endif
   MosPrintf(MIL_TEXT("------------------------------------------------------------\n\n"));
   MosPrintf(MIL_TEXT("                      Camera capabilities.                  \n\n"));
   CameraPrintStreamProtocolCapabilities(MilDigitizer);
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue\n"));
   MosGetch();
#if M_MIL_USE_WINDOWS
   system("cls");
#endif
   MosPrintf(MIL_TEXT("------------------------------------------------------------\n\n"));
   MosPrintf(MIL_TEXT("                      Camera capabilities.                  \n\n"));
   CameraPrintMessageProtocolCapabilities(MilDigitizer);
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue\n"));
   MosGetch();
#if M_MIL_USE_WINDOWS
   system("cls");
#endif
   MosPrintf(MIL_TEXT("------------------------------------------------------------\n\n"));
   MosPrintf(MIL_TEXT("                      Camera capabilities.                  \n\n"));
   CameraPrintStreamChannelCapabilities(MilDigitizer);
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue\n"));
   MosGetch();
#if M_MIL_USE_WINDOWS
   system("cls");
#endif
   MosPrintf(MIL_TEXT("------------------------------------------------------------\n\n"));
   MosPrintf(MIL_TEXT("                      Camera capabilities.                  \n\n"));
   CameraPrintPhysicalLinkConfigurationCapabilities(MilDigitizer);
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue\n"));
   MosGetch();
#if M_MIL_USE_WINDOWS
   system("cls");
#endif
   MosPrintf(MIL_TEXT("------------------------------------------------------------\n\n"));
   MosPrintf(MIL_TEXT("                      Camera capabilities.                  \n\n"));
   CameraPrintNetworkInterfaceCapabilities(MilDigitizer);
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue\n"));
   MosGetch();
#if M_MIL_USE_WINDOWS
   system("cls");
#endif
   MosPrintf(MIL_TEXT("------------------------------------------------------------\n\n"));
   MosPrintf(MIL_TEXT("                      Camera capabilities.                  \n\n"));
   CameraPrintNetworkInterfaceConfiguration(MilDigitizer);
   }

void CameraPrintControlProtocolCapabilities(MIL_ID MilDigitizer)
   {
   MIL_INT Capability = 0;
   MdigInquire(MilDigitizer, M_GC_CONTROL_PROTOCOL_CAPABILITY, &Capability);
   
   MosPrintf(MIL_TEXT("Control Protocol Capabilities\n\n"));

   if (Capability == 0)
      {
      MosPrintf(MIL_TEXT("None\n"));
      return;
      }

   if (Capability & M_GC_USER_DEFINED_NAME_SUPPORT)
      MosPrintf(MIL_TEXT("User defined name\n"));
   if (Capability & M_GC_SERIAL_NUMBER_SUPPORT)
      MosPrintf(MIL_TEXT("Serial number\n"));
   if (Capability & M_GC_HEARTBEAT_DISABLE_SUPPORT)
      MosPrintf(MIL_TEXT("Heartbeat disable\n"));
   if (Capability & M_GC_LINK_SPEED_REGISTER_SUPPORT)
      MosPrintf(MIL_TEXT("Link speed register\n"));
   if (Capability & M_GC_PORT_AND_IP_REGISTER_SUPPORT)
      MosPrintf(MIL_TEXT("Port and IP register\n"));
   if (Capability & M_GC_MANIFEST_TABLE_SUPPORT)
      MosPrintf(MIL_TEXT("Manifest table\n"));
   if (Capability & M_GC_TEST_DATA_SUPPORT)
      MosPrintf(MIL_TEXT("Test data\n"));
   if (Capability & M_GC_DISCOVERY_ACK_DELAY_SUPPORT)
      MosPrintf(MIL_TEXT("Discovery ack delay\n"));
   if (Capability & M_GC_WRITABLE_DISCOVERY_ACK_DELAY_SUPPORT)
      MosPrintf(MIL_TEXT("Writable discovery ack_delay\n"));
   if (Capability & M_GC_EXTENDED_STATUS_CODES_1_SUPPORT)
      MosPrintf(MIL_TEXT("Extended status codes 1.1\n"));
   if (Capability & M_GC_PRIMARY_APP_SWITCHOVER_SUPPORT)
      MosPrintf(MIL_TEXT("Primary app switchover\n"));
   if (Capability & M_GC_UNCONDITIONAL_ACTION_SUPPORT)
      MosPrintf(MIL_TEXT("Unconditional action\n"));
   if (Capability & M_GC_IEEE_1588_SUPPORT)
      MosPrintf(MIL_TEXT("IEEE 1588\n"));
   if (Capability & M_GC_EXTENDED_STATUS_CODES_2_SUPPORT)
      MosPrintf(MIL_TEXT("Extended status codes 2.0\n"));
   if (Capability & M_GC_SCHEDULED_ACTION_SUPPORT)
      MosPrintf(MIL_TEXT("Scheduled action\n"));
   if (Capability & M_GC_ACTION_SUPPORT)
      MosPrintf(MIL_TEXT("Action\n"));
   if (Capability & M_GC_PENDING_ACK_SUPPORT)
      MosPrintf(MIL_TEXT("Pending ack\n"));
   if (Capability & M_GC_EVENT_DATA_SUPPORT)
      MosPrintf(MIL_TEXT("Event data\n"));
   if (Capability & M_GC_EVENT_SUPPORT)
      MosPrintf(MIL_TEXT("Event\n"));
   if (Capability & M_GC_PACKET_RESEND_SUPPORT)
      MosPrintf(MIL_TEXT("Packet resend\n"));
   if (Capability & M_GC_WRITE_MEM_SUPPORT)
      MosPrintf(MIL_TEXT("Write mem\n"));
   if (Capability & M_GC_CONCATENATION_SUPPORT)
      MosPrintf(MIL_TEXT("Concatenation\n"));
   }

void CameraPrintStreamProtocolCapabilities(MIL_ID MilDigitizer)
   {
   MIL_INT Capability = 0;
   MdigInquire(MilDigitizer, M_GC_STREAM_PROTOCOL_CAPABILITY, &Capability);

   MosPrintf(MIL_TEXT("Stream Protocol Capabilities\n\n"));
   
   if (Capability == 0)
      {
      MosPrintf(MIL_TEXT("None\n"));
      return;
      }

   if (Capability & M_GC_FIREWALL_TRAVERSAL_SUPPORT)
      MosPrintf(MIL_TEXT("Firewall traversal\n"));
   if (Capability & M_GC_LEGACY_16BIT_BLOCK_SUPPORT)
      MosPrintf(MIL_TEXT("Legacy 16bit block\n"));
   }

void CameraPrintMessageProtocolCapabilities(MIL_ID MilDigitizer)
   {
   MIL_INT Capability = 0;
   MdigInquire(MilDigitizer, M_GC_MESSAGE_PROTOCOL_CAPABILITY, &Capability);

   MosPrintf(MIL_TEXT("Message Protocol Capabilities\n\n"));

   if (Capability == 0)
      {
      MosPrintf(MIL_TEXT("None\n"));
      return;
      }

   if (Capability & M_GC_FIREWALL_TRAVERSAL_SUPPORT)
      MosPrintf(MIL_TEXT("Firewall traversal\n"));
   }

void CameraPrintStreamChannelCapabilities(MIL_ID MilDigitizer)
   {
   MIL_INT Capability = 0;
   MdigInquire(MilDigitizer, M_GC_STREAM_CHANNEL_CAPABILITY, &Capability);

   MosPrintf(MIL_TEXT("Stream Channel Capabilities\n\n"));

   if (Capability == 0)
      {
      MosPrintf(MIL_TEXT("None\n"));
      return;
      }

   if (Capability & M_GC_BIG_AND_LITTLE_ENDIAN_SUPPORT)
      MosPrintf(MIL_TEXT("Big and little_endian\n"));
   if (Capability & M_GC_IP_REASSEMBLY_SUPPORT)
      MosPrintf(MIL_TEXT("IP reassembly\n"));
   if (Capability & M_GC_MULTI_ZONE_SUPPORT)
      MosPrintf(MIL_TEXT("Multi zone\n"));
   if (Capability & M_GC_PACKET_RESEND_OPTION_SUPPORT)
      MosPrintf(MIL_TEXT("Packet resend option\n"));
   if (Capability & M_GC_ALL_IN_SUPPORT)
      MosPrintf(MIL_TEXT("All in\n"));
   if (Capability & M_GC_UNCONDITIONAL_STREAMING_SUPPORT)
      MosPrintf(MIL_TEXT("Unconditional streaming\n"));
   if (Capability & M_GC_EXTENDED_CHUNK_DATA_SUPPORT)
      MosPrintf(MIL_TEXT("Extended chunk data\n"));
   }

void CameraPrintPhysicalLinkConfigurationCapabilities(MIL_ID MilDigitizer)
   {
   MIL_INT Capability = 0;
   MdigInquire(MilDigitizer, M_GC_PHYSICAL_LINK_CONFIGURATION_CAPABILITY, &Capability);

   MosPrintf(MIL_TEXT("Physical Link Configuration Capabilities\n\n"));

   if (Capability == 0)
      {
      MosPrintf(MIL_TEXT("None\n"));
      return;
      }

   if (Capability & M_GC_SINGLE_LINK_SUPPORT)
      MosPrintf(MIL_TEXT("Single link\n"));
   if (Capability & M_GC_MULTIPLE_LINK_SUPPORT)
      MosPrintf(MIL_TEXT("Multiple link\n"));
   if (Capability & M_GC_STATIC_LINK_AGGREGATION_SUPPORT)
      MosPrintf(MIL_TEXT("Static link aggregation\n"));
   if (Capability & M_GC_DYNAMIC_LINK_AGGREGATION_SUPPORT)
      MosPrintf(MIL_TEXT("Dynamic link aggregation\n"));
   }

void CameraPrintNetworkInterfaceCapabilities(MIL_ID MilDigitizer)
   {
   MIL_INT Capability = 0;
   MdigInquire(MilDigitizer, M_GC_NETWORK_INTERFACE_CAPABILITY, &Capability);

   MosPrintf(MIL_TEXT("Network Interface Capabilities\n\n"));

   if (Capability == 0)
      {
      MosPrintf(MIL_TEXT("None\n"));
      return;
      }

   if (Capability & M_GC_PAUSE_RECEPTION_SUPPORT)
      MosPrintf(MIL_TEXT("Pause reception\n"));
   if (Capability & M_GC_PAUSE_GENERATION_SUPPORT)
      MosPrintf(MIL_TEXT("Pause generation\n"));
   if (Capability & M_GC_LINK_LOCAL_ADDRESS_SUPPORT)
      MosPrintf(MIL_TEXT("Link local address\n"));
   if (Capability & M_GC_DHCP_SUPPORT)
      MosPrintf(MIL_TEXT("DHCP\n"));
   if (Capability & M_GC_PERSISTENT_IP_SUPPORT)
      MosPrintf(MIL_TEXT("Persistent IP\n"));
   }

void CameraPrintNetworkInterfaceConfiguration(MIL_ID MilDigitizer)
   {
   MIL_INT Configuration = 0;
   MdigInquire(MilDigitizer, M_GC_NETWORK_INTERFACE_CONFIGURATION, &Configuration);

   MosPrintf(MIL_TEXT("Network Interface Configuration\n\n"));

   if (Configuration == 0)
      {
      MosPrintf(MIL_TEXT("None\n"));
      return;
      }

   if (Configuration & M_GC_PAUSE_RECEPTION_SUPPORT)
      MosPrintf(MIL_TEXT("Pause reception Enabled\n"));
   if (Configuration & M_GC_PAUSE_GENERATION_SUPPORT)
      MosPrintf(MIL_TEXT("Pause generation Enabled\n"));
   if (Configuration & M_GC_LINK_LOCAL_ADDRESS_SUPPORT)
      MosPrintf(MIL_TEXT("Link local address Enabled\n"));
   if (Configuration & M_GC_DHCP_SUPPORT)
      MosPrintf(MIL_TEXT("DHCP Enabled\n"));
   if (Configuration & M_GC_PERSISTENT_IP_SUPPORT)
      MosPrintf(MIL_TEXT("Persistent IP Enabled\n"));
   }

/* Set the camera in triggered mode according to the user's input. */
void SetTriggerControls(MIL_ID MilDigitizer, eTriggerType& Type, MIL_INT64& NbFrames, MIL_STRING& oTriggerSelector, bool &SoftwareTriggerSelected)
   {
   MIL_INT Done = 0;

   if(CanTriggerAcquisitionStart && MultipleAcquisitionModeSupport)
      {
      MIL_INT Selection;

      do
         {
         MosPrintf(MIL_TEXT("\n\n%-35s"), MIL_TEXT("Do you want to trigger a:")); 

         if(ContinuousAMSupport)
            MosPrintf(MIL_TEXT("(C) %-30s\n%35s"), MIL_TEXT("Continuous acquisition"), MIL_TEXT(""));
         if(MultiFrameAMSupport)
            MosPrintf(MIL_TEXT("(M) %-30s\n%35s"), MIL_TEXT("MultiFrame acquisition"), MIL_TEXT(""));
         if(SingleFrameAMSupport)
            MosPrintf(MIL_TEXT("(S) %-30s\n%35s"), MIL_TEXT("SingleFrame acquisition"), MIL_TEXT(""));

         MosPrintf(MIL_TEXT("\n"));
         Selection = MosGetch();
         Done = 1;
         switch(Selection)
            {
            case 'c':
            case 'C':
               oTriggerSelector = MIL_TEXT("AcquisitionStart");
               MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("AcquisitionMode"), M_TYPE_STRING, MIL_TEXT("Continuous"));
               MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("TriggerSelector"), M_TYPE_STRING, MIL_TEXT("AcquisitionStart"));
               MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("TriggerMode"), M_TYPE_STRING, MIL_TEXT("On"));
               MosPrintf(MIL_TEXT("Continuous acquisition trigger selected.\n"));
               SelectTriggerSource(MilDigitizer, SoftwareTriggerSelected);
               Type = eContinuous;
               break;
            case 'm':
            case 'M':
               oTriggerSelector = MIL_TEXT("AcquisitionStart");
               MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("AcquisitionMode"), M_TYPE_STRING, MIL_TEXT("MultiFrame"));
               MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("TriggerSelector"), M_TYPE_STRING, MIL_TEXT("AcquisitionStart"));
               MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("TriggerMode"), M_TYPE_STRING, MIL_TEXT("On"));
               MosPrintf(MIL_TEXT("Multi Frame acquisition trigger selected.\n"));
               SelectTriggerSource(MilDigitizer, SoftwareTriggerSelected);

               MosPrintf(MIL_TEXT("\n%-30s"), MIL_TEXT("\nHow many frames per trigger?"), MIL_TEXT(""));
#if M_MIL_USE_WINDOWS
               scanf_s("%lld", &(long long)NbFrames);
#else
               scanf("%lld", (long long *)&NbFrames);
#endif
               MosPrintf(MIL_TEXT("%lld Frames will be acquired per trigger.\n"), (long long)NbFrames);
               MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("AcquisitionFrameCount"), M_TYPE_INT64, &NbFrames);
               Type = eMultiFrame;
               break;
            case 's':
            case 'S':
               if(CanTriggerFrameStart)
                  {
                  oTriggerSelector = MIL_TEXT("FrameStart");
                  MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("AcquisitionMode"), M_TYPE_STRING, MIL_TEXT("Continuous"));
                  MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("TriggerSelector"), M_TYPE_STRING, MIL_TEXT("FrameStart"));
                  MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("TriggerMode"), M_TYPE_STRING, MIL_TEXT("On"));
                  }
               else
                  {
                  oTriggerSelector = MIL_TEXT("AcquisitionStart");
                  MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("AcquisitionMode"), M_TYPE_STRING, MIL_TEXT("SingleFrame"));
                  MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("TriggerSelector"), M_TYPE_STRING, MIL_TEXT("AcquisitionStart"));
                  MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("TriggerMode"), M_TYPE_STRING, MIL_TEXT("On"));
                  }

               MosPrintf(MIL_TEXT("Single Frame acquisition trigger selected.\n"));
               SelectTriggerSource(MilDigitizer, SoftwareTriggerSelected);
               Type = eSingleFrame;
               break;
            default:
               MosPrintf(MIL_TEXT("Invalid selection."));
               Done = 0;
               break;
            }
         }
      while(!Done);
      }
   else if(CanTriggerFrameStart)
      {
      oTriggerSelector = MIL_TEXT("FrameStart");
      MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("TriggerSelector"), M_TYPE_STRING, MIL_TEXT("FrameStart"));
      MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("TriggerMode"), M_TYPE_STRING, MIL_TEXT("On"));
      MosPrintf(MIL_TEXT("\n\nFrame start trigger will be performed.\n"));
      SelectTriggerSource(MilDigitizer, SoftwareTriggerSelected);
      Type = eSingleFrame;
      }
   }

/* Set the source of the trigger (software, input pin, ... according to the user's input */
void SelectTriggerSource(MIL_ID MilDigitizer, bool& SoftwareTriggerSelected)
   {
   vector<MIL_STRING> TriggerSource;
   MIL_INT Cnt = 0;
   MIL_INT Done = 0;
   int Selection = 0;

   SoftwareTriggerSelected = false;
   MosPrintf(MIL_TEXT("%-35s"), MIL_TEXT("Please select the trigger source:"));

   MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_COUNT, MIL_TEXT("TriggerSource"), M_TYPE_MIL_INT, &Cnt);
   if(Cnt)
      {
      TriggerSource.assign(Cnt, MIL_TEXT(""));
      for (size_t i = 0; i < TriggerSource.size(); i++)
         {
         MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_NAME + i, MIL_TEXT("TriggerSource"), M_TYPE_STRING, TriggerSource[i]);
         }

   MosPrintf(MIL_TEXT("(%d) %-30s\n"), 0, TriggerSource[0].c_str());
   for (size_t i = 1; i < TriggerSource.size(); i++)
      {
      MosPrintf(MIL_TEXT("%-35s(%d) %-20s\n"), MIL_TEXT(""), (int)i, TriggerSource[i].c_str());
      }

   do
      {
#if M_MIL_USE_WINDOWS
      scanf_s("%d", &Selection);
#else
      scanf("%d", &Selection);
#endif
         if(Selection >= 0 && Selection < Cnt)
         {
         MosPrintf(MIL_TEXT("%s selected\n"), TriggerSource[Selection].c_str());
         Done = 1;
         }
      else
         MosPrintf(MIL_TEXT("Invalid selection.\n"));
         }
      while(!Done);

      MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("TriggerSource"), M_TYPE_STRING, TriggerSource[Selection]);
      if (TriggerSource[Selection] == MIL_TEXT("Software"))
         SoftwareTriggerSelected = true;
   }
   }

/* Puts the camera back in non-triggered mode. */
void ResetTriggerControls(MIL_ID MilDigitizer)
   {
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("TriggerSelector"), M_TYPE_STRING, MIL_TEXT("FrameStart"));
   MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("TriggerMode"), M_TYPE_STRING, MIL_TEXT("Off"));

   MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("TriggerSelector"), M_TYPE_STRING, MIL_TEXT("AcquisitionStart"));
   MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("TriggerMode"), M_TYPE_STRING, MIL_TEXT("Off"));
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);
   }

/* User's processing function hook data structure. */
typedef struct
   {
   MIL_ID  MilDigitizer;
   MIL_ID  MilImageDisp;
   MIL_INT ProcessedImageCount;
   } HookDataStruct;

/* User's processing function prototype. */
MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType,
                                  MIL_ID HookId,
                                  void* HookDataPtr);

void DoTriggeredAcquisition(MIL_ID MilSystem, MIL_ID MilDigitizer, MIL_ID MilImageDisp)
   {
   eTriggerType TriggerType = (eTriggerType)0;
   bool SoftwareTriggerSelected = false;
   MIL_INT64 NbFrames = 10;
   MIL_INT MilGrabBufferListSize;
   MIL_INT Done = 0;
   MIL_INT Ch = 0;
   MIL_ID* MilGrabBufferList = NULL;
   MIL_STRING TriggerSelector;
   HookDataStruct UserHookData;
   MIL_INT StartOp = M_START;

   /*Set-up the camera in triggered mode according to the user's input. */
   SetTriggerControls(MilDigitizer, TriggerType, NbFrames, TriggerSelector, SoftwareTriggerSelected);

   MilGrabBufferList = new MIL_INT[(NbFrames == M_INFINITE) ? 10 : (size_t)NbFrames];

   /* Allocate the grab buffers and clear them. */
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   for(MilGrabBufferListSize = 0; 
            MilGrabBufferListSize<NbFrames; MilGrabBufferListSize++)
      {
      MbufAlloc2d(MilSystem,
                  MdigInquire(MilDigitizer, M_SIZE_X, M_NULL),
                  MdigInquire(MilDigitizer, M_SIZE_Y, M_NULL),
                  8+M_UNSIGNED,
                  M_IMAGE+M_GRAB+M_PROC,
                  &MilGrabBufferList[MilGrabBufferListSize]);

      if (MilGrabBufferList[MilGrabBufferListSize])
         {
         MbufClear(MilGrabBufferList[MilGrabBufferListSize], 0xFF);
         }
      else
         break;
      }
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   /* Initialize the User's processing function data structure. */
   UserHookData.MilDigitizer        = MilDigitizer;
   UserHookData.MilImageDisp        = MilImageDisp;
   UserHookData.ProcessedImageCount = 0;

   /* Set the grab timeout to infinite for triggered grab. */
   MdigControl(MilDigitizer, M_GRAB_TIMEOUT, M_INFINITE);

   /* Print a message and wait for a key press after a minimum number of frames. */
   if(SoftwareTriggerSelected)
      MosPrintf(MIL_TEXT("\n\nPress <t> to do a software trigger.\n"));
   else
      MosPrintf(MIL_TEXT("\n\nWaiting for a input trigger signal.\n"));
   MosPrintf(MIL_TEXT("Press any other key to quit.\n\n"));

   if (TriggerType == eMultiFrame)
      StartOp = M_SEQUENCE + M_COUNT(NbFrames);

   do
      {
      /* Start the processing. The processing function is called for every frame grabbed. */
      MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize,
                     StartOp, M_ASYNCHRONOUS, ProcessingFunction, &UserHookData);

      /* If trigger mode is software, send a software trigger when the user presses the <T> key. */
      if(SoftwareTriggerSelected)
         {
         do
            {
            Ch =  MosGetch();
            if(Ch == 'T' || Ch == 't')
               {
               MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("TriggerSelector"), M_TYPE_STRING, TriggerSelector);
               MdigControlFeature(MilDigitizer, M_FEATURE_EXECUTE, MIL_TEXT("TriggerSoftware"), M_DEFAULT, M_NULL);
               if(TriggerType == eMultiFrame)
                  break;
               }
            else
               Done = 1;
            }
         while(!Done);
         }
      else if(TriggerType != eMultiFrame)
         Done = MosGetch();
      else if(MosKbhit())
         Done = 1;

      /* Stop the processing. */
      MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize,
                                 Done ? M_STOP : M_STOP+M_WAIT, M_DEFAULT, ProcessingFunction, &UserHookData);
      }
   while(!Done);

   /* Reset the camera to non-triggered mode. */
   ResetTriggerControls(MilDigitizer);
   
   /* Free the grab buffers. */
   while(MilGrabBufferListSize > 0)
      MbufFree(MilGrabBufferList[--MilGrabBufferListSize]);
   
   delete [] MilGrabBufferList;
   }

/* User's processing function called every time a grab buffer is modified. */
/* -----------------------------------------------------------------------*/

/* Local defines. */
#define STRING_LENGTH_MAX  20
#define STRING_POS_X       20
#define STRING_POS_Y       20

MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType,
                                  MIL_ID HookId,
                                  void* HookDataPtr)
   {
   HookDataStruct *UserHookDataPtr = (HookDataStruct *)HookDataPtr;
   MIL_ID ModifiedBufferId;
   MIL_TEXT_CHAR Text[STRING_LENGTH_MAX]= {MIL_TEXT('\0'),};

   /* Retrieve the MIL_ID of the grabbed buffer. */
   MdigGetHookInfo(HookId, M_MODIFIED_BUFFER+M_BUFFER_ID, &ModifiedBufferId);

   /* Print and draw the frame count. */
   UserHookDataPtr->ProcessedImageCount++;
   MosPrintf(MIL_TEXT("Processing frame #%lld.\r"), (long long)UserHookDataPtr->ProcessedImageCount);
   MosSprintf(Text, STRING_LENGTH_MAX, MIL_TEXT("%lld"), 
                                       (long long)UserHookDataPtr->ProcessedImageCount);
   MgraText(M_DEFAULT, ModifiedBufferId, STRING_POS_X, STRING_POS_Y, Text);

   /* Perform the processing and update the display. */
   MbufCopy(ModifiedBufferId, UserHookDataPtr->MilImageDisp);
   
   return 0;
   }
