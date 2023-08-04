/****************************************************************************
  FileName     [ cliCharDef.cpp ]
  PackageName  [ cli ]
  Synopsis     [ Process keyboard inputs ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/
#include "cliCharDef.h"

#include <termios.h>

#include <cctype>
#include <iostream>

#include "cli.h"

using namespace std;

//----------------------------------------------------------------------
//    keypress detection details
//----------------------------------------------------------------------

namespace detail {
static struct termios stored_settings;

static auto reset_keypress(void) {
    tcsetattr(0, TCSANOW, &stored_settings);
}

static auto set_keypress(void) {
    struct termios new_settings;
    tcgetattr(0, &stored_settings);
    new_settings = stored_settings;
    new_settings.c_lflag &= (~ICANON);
    new_settings.c_lflag &= (~ECHO);
    new_settings.c_cc[VTIME] = 0;
    tcgetattr(0, &stored_settings);
    new_settings.c_cc[VMIN] = 1;
    tcsetattr(0, TCSANOW, &new_settings);
}

static auto mygetc(istream& istr) -> char {
    char ch;
    set_keypress();
    istr.unsetf(ios_base::skipws);
    istr >> ch;
    istr.setf(ios_base::skipws);
    reset_keypress();
#ifdef TEST_ASC
    cout << left << setw(6) << int(ch);
#endif  // TEST_ASC
    return ch;
}

}  // namespace detail

int CommandLineInterface::getChar(istream& istr) const {
    using namespace detail;
    using namespace KeyCode;
    char ch = mygetc(istr);

    assert(ch != INTERRUPT_KEY);
    switch (ch) {
        // Simple keys: one code for one key press
        // -- The following should be platform-independent
        case LINE_BEGIN_KEY:     // Ctrl-a
        case LINE_END_KEY:       // Ctrl-e
        case INPUT_END_KEY:      // Ctrl-d
        case TAB_KEY:            // tab('\t') or Ctrl-i
        case NEWLINE_KEY:        // enter('\n') or ctrl-m
        case CLEAR_CONSOLE_KEY:  // Clear console (Ctrl-l)
            return ch;

        // -- The following simple/combo keys are platform-dependent
        //    You should test to check the returned codes of these key presses
        // -- You should either modify the "enum KeyCode" definitions in
        //    "cmdCharDef.h", or revise the control flow of the "case ESC" below
        case BACK_SPACE_KEY:
            return ch;
        case BACK_SPACE_CHAR:
            return BACK_SPACE_KEY;

        // Combo keys: multiple codes for one key press
        // -- Usually starts with ESC key, so we check the "case ESC"
        case ESC_KEY: {
            char combo = mygetc(istr);
            // Note: ARROW_KEY_INT == MOD_KEY_INT, so we only check MOD_KEY_INT
            if (combo == char(MOD_KEY_INT)) {
                char key = mygetc(istr);
                if ((key >= char(MOD_KEY_BEGIN)) && (key <= char(MOD_KEY_END))) {
                    if (mygetc(istr) == MOD_KEY_DUMMY)
                        return int(key) + MOD_KEY_FLAG;
                    else
                        return UNDEFINED_KEY;
                } else if ((key >= char(ARROW_KEY_BEGIN)) &&
                           (key <= char(ARROW_KEY_END)))
                    return int(key) + ARROW_KEY_FLAG;
                else
                    return UNDEFINED_KEY;
            } else {
                beep();
                return getChar(istr);
            }
        }
        // For the remaining printable and undefined keys
        default:
            return (isprint(ch)) ? ch : UNDEFINED_KEY;
    }
}
