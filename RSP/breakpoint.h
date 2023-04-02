#include "Rsp.h"

#define MaxBPoints			0x30

typedef struct {
   unsigned int Location;
} BPOINT;

extern BPOINT BPoint[MaxBPoints];
extern  int	NoOfBpoints;

void Add_BPoint ( void );
void CreateBPPanel ( HWND hDlg, rectangle rcBox );
void HideBPPanel ( void );
void PaintBPPanel ( window_paint ps );
void ShowBPPanel ( void );
void RefreshBpoints (HWND hList );
void RemoveBpoint (HWND hList, int index );
void RemoveAllBpoint ( void );

int  AddRSP_BPoint ( DWORD Location, int Confirm );
int  CheckForRSPBPoint ( DWORD Location );
void RemoveRSPBreakPoint (DWORD Location);
