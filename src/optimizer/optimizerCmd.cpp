/****************************************************************************
  FileName     [ optimizerCmd.cpp ]
  PackageName  [ optimizer ]
  Synopsis     [ Define optimizer package commands ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cstddef>
#include <iostream>
#include <string>

#include "./optimizer.hpp"
#include "cli/cli.hpp"
#include "qcir/qcir.hpp"
#include "qcir/qcirCmd.hpp"
#include "qcir/qcirMgr.hpp"
#include "util/util.hpp"

using namespace std;
using namespace ArgParse;
extern size_t verbose;
extern int effLimit;
extern QCirMgr qcirMgr;
extern bool stop_requested();

unique_ptr<ArgParseCmdType> optimizeCmd();

bool initOptimizeCmd() {
    if (!(cli.regCmd("OPTimize", 3, optimizeCmd()))) {
        cerr << "Registering \"optimize\" commands fails... exiting" << endl;
        return false;
    }
    return true;
}

//----------------------------------------------------------------------
//    Optimize
//----------------------------------------------------------------------
unique_ptr<ArgParseCmdType> optimizeCmd() {
    auto cmd = make_unique<ArgParseCmdType>("OPTimize");

    cmd->precondition = []() { return qcirMgrNotEmpty("OPTimize"); };

    cmd->parserDefinition = [](ArgumentParser &parser) {
        parser.help("optimize QCir");
        parser.addArgument<bool>("-physical")
            .defaultValue(false)
            .action(storeTrue)
            .help("optimize physical circuit, i.e preserve the swap path");
        parser.addArgument<bool>("-copy")
            .defaultValue(false)
            .action(storeTrue)
            .help("copy a circuit to perform optimization");
        parser.addArgument<bool>("-statistics")
            .defaultValue(false)
            .action(storeTrue)
            .help("count the number of rules operated in optimizer.");
        parser.addArgument<bool>("-trivial")
            .defaultValue(false)
            .action(storeTrue)
            .help("Use the trivial optimization.");
    };

    cmd->onParseSuccess = [](ArgumentParser const &parser) {
        Optimizer optimizer(qcirMgr.get());
        QCir *result;
        std::string procedure_str{};
        if (parser.get<bool>("-trivial")) {
            result = optimizer.trivial_optimization();
            procedure_str = "Trivial Optimize";
        } else {
            result = optimizer.basic_optimization(!parser.get<bool>("-physical"), false, 1000, parser.get<bool>("-statistics"));
            procedure_str = "Optimize";
        }
        if (result == nullptr) {
            cout << "Error: fail to optimize circuit." << endl;
            return CmdExecResult::ERROR;
        }
        auto name = qcirMgr.get()->getFileName();
        auto procedures = qcirMgr.get()->getProcedures();

        if (parser.get<bool>("-copy")) {
            qcirMgr.add(qcirMgr.getNextID());
        }

        if (stop_requested()) {
            procedure_str += "[INT]";
        }

        qcirMgr.set(std::make_unique<QCir>(std::move(*result)));
        qcirMgr.get()->printCirInfo();

        qcirMgr.get()->addProcedures(procedures);
        qcirMgr.get()->addProcedure(procedure_str);

        return CmdExecResult::DONE;
    };
    return cmd;
}
