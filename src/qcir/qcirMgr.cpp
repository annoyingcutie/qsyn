/****************************************************************************
  FileName     [ qcirMgr.cpp ]
  PackageName  [ qcir ]
  Synopsis     [ Define qcir manager functions ]
  Author       [ Chin-Yi Cheng ]
  Copyright    [ Copyleft(c) 2022-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include <vector>
#include <string>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "qcirMgr.h"

using namespace std;
QCirMgr *qCirMgr = 0;

void QCirMgr::printSummary() const
{
    cout << "Follow QASM file (Topological order)" << endl;
    for (size_t i = 0; i < _qgate.size(); i++)
    {
        _qgate[i]->printGate();
    }
}
void QCirMgr::printQubits() const
{
    for (size_t i = 0; i < _bitFirst.size(); i++)
    {
        QCirGate *current = _bitFirst[i];
        size_t last_time = 0;
        cout << "Q" << right << setfill(' ') << setw(2) << i << "  ";
        while (current != NULL)
        {
            cout << "-";
            while (last_time < current->getTime())
            {
                cout << "----";
                last_time++;
            }
            cout << setfill(' ') << setw(2) << current->getTypeStr().substr(0, 2);
            last_time = current->getTime() + 1;
            current = current->getQubit(i)._child;
            cout << "-";
        }
        cout << endl;
    }
}
void QCirMgr::addAncilla(size_t num)
{
    for (size_t i = 0; i < num; i++)
    {
        _bitLast.push_back(NULL);
        _bitFirst.push_back(NULL);
    }
}
void QCirMgr::appendGate(string type, vector<size_t> bits)
{
    QCirGate *temp = new QCirGate(_gateId, type, bits.size());
    size_t max_time = 0;
    for (size_t k = 0; k < bits.size(); k++)
    {
        size_t q = bits[k];
        temp->addQubit(q, k == bits.size() - 1);
        if (_bitLast[q] != NULL)
        {
            temp->addParent(q, _bitLast[q]);
            _bitLast[q]->addChild(q, temp);
            if ((_bitLast[q]->getTime() + 1) > max_time)
                max_time = _bitLast[q]->getTime() + 1;
        }
        else
            _bitFirst[q] = temp;

        _bitLast[q] = temp;
    }
    for (size_t k = 0; k < bits.size(); k++)
        temp->setTime(max_time);

    _qgate.push_back(temp);
    _gateId++;
}
bool QCirMgr::parseQASM(string filename)
{
    // read file and open
    fstream qasm_file;
    string tmp;
    vector<string> record;
    qasm_file.open(filename.c_str(), ios::in);
    if (!qasm_file.is_open())
    {
        cerr << "Cannot open QASM \"" << filename << "\"!!" << endl;
        return false;
    }
    string str;
    for (int i = 0; i < 6; i++)
    {
        // OPENQASM 2.0;
        // include "qelib1.inc";
        // qreg q[int];
        qasm_file >> str;
    }
    // For netlist
    int nqubit = stoi(str.substr(str.find("[") + 1, str.size() - str.find("[") - 3));
    addAncilla(nqubit);
    vector<string> single_list{"x", "sx", "s", "rz", "i", "h", "t", "tdg"};

    while (qasm_file >> str)
    {
        string space_delimiter = " ";
        string type = str.substr(0, str.find(" "));
        type = str.substr(0, str.find("("));
        string is_CX = type.substr(0, 2);
        string is_CRZ = type.substr(0, 3);
        if (is_CX != "cx" && is_CRZ != "crz")
        {
            if (find(begin(single_list), end(single_list), type) !=
                end(single_list))
            {
                qasm_file >> str;
                string singleQubitId = str.substr(2, str.size() - 4);
                size_t q = stoul(singleQubitId);
                vector<size_t> pin_id;
                pin_id.push_back(q); // targ
                appendGate(type, pin_id);
            }
            else
            {
                if (type != "creg" && type != "qreg")
                {
                    cerr << "Unseen Gate " << type << endl;
                    return false;
                }
                else
                    qasm_file >> str;
            }
        }
        else
        {
            // Parse
            qasm_file >> str;
            string delimiter = ",";
            string token = str.substr(0, str.find(delimiter));
            string qubit_id = token.substr(2, token.size() - 3);
            size_t q1 = stoul(qubit_id);
            token = str.substr(str.find(delimiter) + 1,
                               str.size() - str.find(delimiter) - 2);
            qubit_id = token.substr(2, token.size() - 3);
            size_t q2 = stoul(qubit_id);
            vector<size_t> pin_id;
            pin_id.push_back(q1); // ctrl
            pin_id.push_back(q2); // targ
            appendGate(type, pin_id);
        }
    }
    return true;
}