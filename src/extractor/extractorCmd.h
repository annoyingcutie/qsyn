/****************************************************************************
  FileName     [ extractorCmd.h ]
  PackageName  [ extractor ]
  Synopsis     [ Define extractor package commands ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef EXTRACTOR_CMD_H
#define EXTRACTOR_CMD_H

#include "cmdParser.h"

CmdClass(ExtractCmd);
CmdClass(ExtractSetCmd);
CmdClass(ExtractStepCmd);
CmdClass(ExtractPrintCmd);
#endif  // EXTRACTOR_CMD_H