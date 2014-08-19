#include "PCHIncludes.h"
#pragma hdrstop

#include <math.h>

#include "Brain2BrainUI.h"
#include "BCIException.h"
#include "Shapes.h"

// Color of targets when idle
#define TARGET_FILL_COLOR RGBColor::Blue

Brain2BrainUI::Brain2BrainUI(GUI::DisplayWindow& display) 
    : window(display), dwellTime(0) {}

Brain2BrainUI::~Brain2BrainUI() {
    delete cursor;
    delete yesTarget;
    delete yesTargetText;
    delete noTarget;
    delete noTargetText;
    delete titleBox;
    delete questionBox;
    delete answerBox;
}

void Brain2BrainUI::Initialize() {
    // Initialize the cursor to be a circle
    float cursorWidth = Parameter("CursorWidth") / 100.0f;
	GUI::Rect cursorRect = {0, 0, cursorWidth, cursorWidth * window.Width() / window.Height()};
    cursor = new EllipticShape(window, 1);
    cursor->SetColor(RGBColor::White)
           .SetFillColor(RGBColor::White)
           .SetObjectRect(cursorRect)
           .Hide();

    // On average, we need to cross half the workspace during a trial
    float feedbackDuration = Parameter("FeedbackDuration").InSampleBlocks();
    cursorSpeed = 0.5f / feedbackDuration;

    // Initialize the two targets
    float targetHeight = Parameter("TargetHeight") / 100.0f;
    RGBColor targetBorderColor = RGBColor::Gray;
    RGBColor targetTextColor = RGBColor::Black;
    float targetTextHeight = 0.5f;
    
        // Initialize the YES target
        GUI::Rect yesTargetRect = {0, 0, 1.0f, targetHeight};
        yesTarget = new RectangularShape(window);
        yesTarget->SetColor(targetBorderColor)
                  .SetObjectRect(yesTargetRect)
                  .Hide();
        
        yesTargetText = new TextField(window);
        yesTargetText->SetText("Yes")
			          .SetTextColor(targetTextColor)
                      .SetTextHeight(targetTextHeight)
                      .SetColor(RGBColor::NullColor)
                      .SetObjectRect(yesTargetRect)
                      .Hide();
        
        // Initialize the NO target
        GUI::Rect noTargetRect = {0, 1.0f - targetHeight, 1.0f, 1.0f};
        noTarget = new RectangularShape(window);
        noTarget->SetColor(targetBorderColor)
                 .SetObjectRect(noTargetRect)
                 .Hide();
        
        noTargetText = new TextField(window);
        noTargetText->SetText("No")
					 .SetTextColor(targetTextColor)
                     .SetTextHeight(targetTextHeight)
                     .SetColor(RGBColor::NullColor)
                     .SetObjectRect(noTargetRect)
                     .Hide();

    // Initialize the title box message
    GUI::Rect titleBoxRect = {0.1f, 0.4f, 0.9f, 0.6f};
    titleBox = new TextField(window);
    titleBox->SetText("Timeout")
		     .SetTextColor(RGBColor::Lime)
             .SetTextHeight(0.8f)
             .SetColor(RGBColor::Gray)
             .SetObjectRect(titleBoxRect);
             
    // Initialize the optional text fields
    RGBColor textColor = RGBColor::White;
    GUI::Rect questionBoxRect = {0.1f, 0.1f, 0.9f, 0.3f};
    questionBox = new TextField(window);
    questionBox->SetText("")
                .SetColor(RGBColor::NullColor)
                .SetTextColor(textColor)
                .SetAspectRatioMode(GUI::AspectRatioModes::AdjustHeight)
                .SetObjectRect(questionBoxRect);
                
    GUI::Rect answerBoxRect = {0.1f, 0.7f, 0.9f, 0.9f};
    answerBox = new TextField(window);
    answerBox->SetText("")
              .SetColor(RGBColor::NullColor)
              .SetTextColor(textColor)
              .SetAspectRatioMode(GUI::AspectRatioModes::AdjustHeight)
              .SetObjectRect(answerBoxRect);
}

void Brain2BrainUI::OnStartRun() {
    titleBox->SetText(">> Get Ready! <<");
}

void Brain2BrainUI::OnTrialBegin() {
    titleBox->Hide();

    yesTarget->SetFillColor(TARGET_FILL_COLOR)
		      .Show();
    yesTargetText->Show();
    noTarget->SetFillColor(TARGET_FILL_COLOR)
		     .Show();
    noTargetText->Show();
    
    questionBox->Show();
    answerBox->Show();
}

void Brain2BrainUI::OnFeedbackBegin() {
    GUI::Point center = {0.5f, 0.5f};
    cursor->SetCenter(center)
           .Show();
}

Brain2BrainUI::TargetHitType Brain2BrainUI::DoFeedback(const GenericSignal& ControlSignal) {
    GUI::Point cursorPosition = cursor->Center();
    GUI::Rect cursorRect = cursor->ObjectRect();

    // Use the control signal to move up and down
    if (ControlSignal.Channels() > 0) {
        cursorPosition.y += cursorSpeed * ControlSignal(0, 0);
    }

    // Restrict cursor movement to the screen itself
    cursorPosition.y = std::max(cursorRect.bottom - cursorRect.top, 
                           std::min(1.0f - cursorRect.bottom + cursorRect.top, cursorPosition.y));

    // Update cursor position
    cursor->SetCenter(cursorPosition);
    State("CursorCenter") = static_cast<int>(window.Height() * cursorPosition.y);
    
    // Determine if either of the targets were hit
    RGBColor hitColor = RGBColor::Red;
    TargetHitType hit = NOTHING_HIT;
    if (Shape::AreaIntersection(*cursor, *yesTarget)) {
        yesTarget->SetFillColor(hitColor);
        hit = YES_TARGET;
    } else if (Shape::AreaIntersection(*cursor, *noTarget)) {
        noTarget->SetFillColor(hitColor);
        hit = NO_TARGET;
    } else {
        yesTarget->SetFillColor(TARGET_FILL_COLOR);
        noTarget->SetFillColor(TARGET_FILL_COLOR);
    }

    // Delay reporting of a hit for a little bit of time
    if (dwellTime >= static_cast<int>(Parameter("DwellTime").InSampleBlocks())) {
        dwellTime = 0;
        return hit;
    }

    switch (hit) {
    case YES_TARGET:
    case NO_TARGET:
      dwellTime++;
      break;
    default:
      dwellTime = 0;
      break;
    }
    
    return NOTHING_HIT;
}

void Brain2BrainUI::OnFeedbackEnd() {
    // Reset the target colors
    yesTarget->SetFillColor(TARGET_FILL_COLOR);
    noTarget->SetFillColor(TARGET_FILL_COLOR);
    
    cursor->Hide();
}

void Brain2BrainUI::OnStopRun() {
    titleBox->SetText("Timeout")
             .Show();
    yesTarget->Hide();
    yesTargetText->Hide();
    noTarget->Hide();
    noTargetText->Hide();
    questionBox->Hide();
    answerBox->Hide();
}

void Brain2BrainUI::SetQuestion(std::string data) {
    questionBox->SetText(data);
}

void Brain2BrainUI::SetAnswer(std::string data) {
    answerBox->SetText(data);
}