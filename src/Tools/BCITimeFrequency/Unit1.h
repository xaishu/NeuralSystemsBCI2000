//---------------------------------------------------------------------------

#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TOutputForm : public TForm
{
__published:	// IDE-managed Components
        TRadioGroup *OutputOrder;
private:	// User declarations
public:		// User declarations
        __fastcall TOutputForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TOutputForm *OutputForm;
//---------------------------------------------------------------------------
#endif
