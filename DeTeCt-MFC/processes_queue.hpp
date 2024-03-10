#pragma once
//		NB_INSTANCES											MIN		LOW		MEDIUM	HIGH	MAX
///#define MAX_INSTANCES									1		max		max		max		max
//#define MIN_FREE_SYSTEM_MEM_PC	00.0	// (in %)		0		50.0	40.0	30.0	20.0		
//#define MIN_AVAILABLE_CPU_PC		00.0	// (in %)		0		70.0	50.0	30.0	15.0
#define DETECT_CHILD_MEM_MB		415		//MB			1.33 * 310
#define DETECT_CHILD_PROC_FACTOR_PC	225.0	// (in %)

#include "stdafx.h"
#include "dtc.h"

enum class _Rating_type { Error, Null, Low, High };
typedef enum _Rating_type Rating_type;

enum class _Resources_usage { min, low, medium, high, max };
typedef enum _Resources_usage Resources_usage;

// ************** DeTeCt file and folder functions **********
char	*DeTeCtFileName(char *DeTeCtFileNameChar);
CString  DeTeCt_additional_filename_exe_fullpath(CString suffix);
CString  DeTeCt_additional_filename_from_folder(const CString foldername, const CString suffix);
CString  DeTeCt_exe_folder();

// ************** DeTeCt process queue management **********	
BOOL	PushItemToQueue			(const CString line, const CString tag, CString QueueFilename, HANDLE* pQueueFileHandle, const BOOL close_handle_at_end);			//API+internal	-W-
BOOL	GetItemFromQueue		(CString *objectname, const CString tag, const CString QueueFilename, HANDLE* pQueueFileHandle, const BOOL close_handle_at_end);	//API+internal	R--
int		NbItemFromQueue			(const CString tag, const CString QueueFilename, HANDLE* pQueueFileHandle, const BOOL close_handle_at_end);							//API+internal	R--
void	RemoveItemsFromQueue	(const CString objectname, const CString tag, CString QueueFilename, HANDLE* pQueueFileHandle, const BOOL close_handle_at_end);		//API+internal	--D
void	SetIntParamToQueue		(const int param, const CString tag, const CString QueueFilename);																	//API			-WD
int		GetIntParamFromQueue(const CString tag, const CString QueueFilename);																						//API			R--

BOOL	IsFileAlreadyQueued		(const CString objectname, const CString QueueFilename);																			//API			R--
void	PushFileToQueue			(const CString objectname, const CString QueueFilename);																			//API			-W-
void	RemoveFileFromQueue		(const CString objectname, const  CString QueueFilename, HANDLE* pQueueFileHandle, const BOOL close_handle_at_end);					//API+internal	--D

int		NbFilesFromQueue		(const CString QueueFilename);																										//API			R--
BOOL	GetFileFromQueue		(CString *objectname, const CString QueueFilename);																					//API			-WD
void	SetFileProcessingFromQueue(const CString objectname, const CString QueueFilename);																			//API			-WD
void	SetProcessingFileProcessedFromQueue(const CString objectname, const CString details, const CString tag, const CString QueueFilename);						//API			-WD
BOOL	GetProcessedFileFromQueue(CString *processed_filename, CString *processed_filename_acquisition, CString *processed_message, Rating_type *processed_rating, double *duration, int *nframe_child, int *fps_int_child, const CString QueueFilename);  //API	-WD

BOOL OpenRWQueueFile(const CString QueueFilename, HANDLE* pQueueFileHandle);

// ************** Process functions **********
int		ProcessRunningInstancesNumber(const char *ProcessFilename);
BOOL	IsProcessRunning(const DWORD pid);

DWORD	getParentPID(const DWORD pid);
int		getProcessName(const DWORD pid, wchar_t *fname, DWORD sz);
int		KillsChildrenProcesses();
int		ChildrenProcessesNumber();
int		ParentChildrenProcessesNumber(const DWORD parent_PID);

// ************** memory and CPU functions **********
void	Set_ressource_usage(const int resources_usage);
float	GetCPULoad();
int		NbPossibleChildInstances_fromMemoryUsage();
int		NbPossibleChildInstances_fromCPUUsage();
int		NbPossibleChildInstances_fromMemoryandCPUUsage();
