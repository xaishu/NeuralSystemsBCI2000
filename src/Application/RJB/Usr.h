//---------------------------------------------------------------------------

#ifndef UsrH
#define UsrH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "UGenericFilter.h"
#include <ExtCtrls.hpp>

#include "UCoreComm.h"

#define TARGET_OFF      0
#define TARGET_ON       1
#define TARGET_RESULT   2

#define CURSOR_OFF      0
#define CURSOR_ON       1
#define CURSOR_RESULT   2

#define NTARGS  8

//---------------------------------------------------------------------------
class TUser : public TForm
{
__published:	// IDE-managed Components
        TShape *Cursor;
        TShape *Target;
        TLabel *tT;
        TLabel *tO;
        TLabel *TargetText1;
        TLabel *TargetText2;
        TLabel *ResultText;
        TLabel *tPreRunIntervalText;
        TShape *Target2;
private:	// User declarations
        int Wx;                 // task window x location
        int Wy;                 // task window y location
        int Wxl;                // task window x size
        int Wyl;                // task window y size
        int CursorSize;         // cursor size
        int HalfCursorSize;     // 1/2 of cursor size
        int TargetType;         // Targets or YES/NO
        int YesNoCorrect;
        int YesNoOnTime;
        int YesNoOffTime;
        float cursor_y_start;   // y starting position of cursor
        float cursor_x_start;   // x starting position of cursor
        float limit_top;
        float limit_bottom;
        float limit_left;
        float limit_right;
        float xscalef;
        float yscalef;
        AnsiString      TargetWord, NoTargetWord;
public:		// User declarations
        float targx[NTARGS+1];
        float targy[NTARGS+1];
        float targsizex[NTARGS+1];
        float targsizey[NTARGS+1];
        float targy_btm[NTARGS+1];

        float scale_x;
        float scale_y;
        void Scale( float , float );
        float ran1( long *idem );       // from Press et al
        void GetLimits( float *, float *, float *, float * );
       // void TestCursorLocation( float x, float y );
        void PutCursor( float y, float x, int );      // self explanitory
        void PutTarget( int targno, int );
        void PutT(bool);
        void PutO(bool);
        void Clear( void );
        void PreRunInterval(int time);
        void Outcome(int time, int result);
        void __fastcall SetUsr( PARAMLIST *plist, STATELIST *slist);
        void __fastcall Initialize(PARAMLIST *plist, STATELIST *slist, CORECOMM *corecomm);
        __fastcall TUser(TComponent* Owner);
        __fastcall ~TUser();
};
//---------------------------------------------------------------------------
extern PACKAGE TUser *User;
//---------------------------------------------------------------------------
#endif
