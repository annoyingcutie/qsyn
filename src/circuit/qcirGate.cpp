/****************************************************************************
  FileName     [ qcirGate.cpp ]
  PackageName  [ src/circuit ]
  Synopsis     [ Define qcir gate functions ]
  Author       [ Chin-Yi Cheng ]
  Copyright    [ Copyleft(c) 2022-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "qcirGate.h"

using namespace std;

void QCirGate::printGate() const
{
    cout << "Gate " << _id << ": " << _type << " \t"
         << " Qubit: ";
    for (size_t i = 0; i < _pins.size(); i++)
    {
        cout << _pins[i] << " ";
    }
    cout << endl;
}