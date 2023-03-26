/****************************************************************************
  FileName     [ duostraCmd.h ]
  PackageName  [ duostra ]
  Synopsis     [ Define Duostra package commands ]
  Author       [ Chin-Yi Cheng, Chien-Yi Yang, Ren-Chu Wang, Yi-Hsiang Kuo ]
  Paper        [ https://arxiv.org/abs/2210.01306 ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef DUOSTRA_CMD_H
#define DUOSTRA_CMD_H

#include "cmdParser.h"

CmdClass(DuostraCmd);
CmdClass(DuostraSetCmd);
#endif  // DUOSTRA_CMD_H