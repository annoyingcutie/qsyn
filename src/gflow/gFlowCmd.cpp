/****************************************************************************
  FileName     [ gFlowCmd.cpp ]
  PackageName  [ gflow ]
  Synopsis     [ Define gflow package commands ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cstddef>
#include <iomanip>
#include <iostream>
#include <string>

#include "cmdParser.h"
#include "gFlow.h"
#include "zxCmd.h"
#include "zxGraphMgr.h"

using namespace std;
using namespace ArgParse;

extern ZXGraphMgr zxGraphMgr;
extern size_t verbose;

unique_ptr<ArgParseCmdType> ZXGGFlowCmd();

bool initGFlowCmd() {
    if (!cmdMgr->regCmd("ZXGGFlow", 5, ZXGGFlowCmd())) {
        cerr << "Registering \"gflow\" commands fails... exiting" << endl;
        return false;
    }
    return true;
}

unique_ptr<ArgParseCmdType> ZXGGFlowCmd() {
    auto cmd = make_unique<ArgParseCmdType>("ZXGGFlow");

    cmd->precondition = []() { return zxGraphMgrNotEmpty("ZXGGFlow"); };

    cmd->parserDefinition = [](ArgumentParser& parser) {
        parser.help("calculate and print the generalized flow of a ZXGraph");

        auto mutex = parser.addMutuallyExclusiveGroup().required(false);

        mutex.addArgument<bool>("-all")
            .action(storeTrue)
            .help("print both GFlow levels and correction sets");
        mutex.addArgument<bool>("-levels")
            .action(storeTrue)
            .help("print GFlow levels");
        mutex.addArgument<bool>("-corrections")
            .action(storeTrue)
            .help("print the correction set to each ZXVertex");
        mutex.addArgument<bool>("-summary")
            .action(storeTrue)
            .help("print basic information on the ZXGraph's GFlow");

        parser.addArgument<bool>("-extended")
            .action(storeTrue)
            .help("calculate the extended GFlow, i.e., allowing XY, YZ, XZ plane measurements");

        parser.addArgument<bool>("-independent-set")
            .action(storeTrue)
            .help("force each GFlow level to be an independent set");
    };

    cmd->onParseSuccess = [](ArgumentParser const& parser) {
        GFlow gflow(zxGraphMgr.get());

        gflow.doExtendedGFlow(parser["-extended"]);
        gflow.doIndependentLayers(parser["-independent-set"]);

        gflow.calculate();

        if (parser["-all"].isParsed()) {
            gflow.print();
        } else if (parser["-levels"].isParsed()) {
            gflow.printLevels();
        } else if (parser["-corrections"].isParsed()) {
            gflow.printXCorrectionSets();
        }

        gflow.printSummary();
        if (!gflow.isValid()) gflow.printFailedVertices();

        return CMD_EXEC_DONE;
    };

    return cmd;
}