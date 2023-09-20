/****************************************************************************
  PackageName  [ qcir/optimizer ]
  Synopsis     [ Define optimizer package commands ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include <spdlog/spdlog.h>

#include <cstddef>
#include <string>

#include "../qcir.hpp"
#include "../qcir_cmd.hpp"
#include "../qcir_mgr.hpp"
#include "./optimizer.hpp"
#include "cli/cli.hpp"
#include "util/util.hpp"

using namespace dvlab::argparse;
using dvlab::CmdExecResult;
using dvlab::Command;

extern bool stop_requested();

namespace qsyn::qcir {

//----------------------------------------------------------------------
//    Optimize
//----------------------------------------------------------------------
Command qcir_optimize_cmd(QCirMgr& qcir_mgr) {
    return {"qccoptimize",
            [](ArgumentParser& parser) {
                parser.description("optimize QCir");

                parser.add_argument<bool>("-physical")
                    .default_value(false)
                    .action(store_true)
                    .help("optimize physical circuit, i.e preserve the swap path");
                parser.add_argument<bool>("-copy")
                    .default_value(false)
                    .action(store_true)
                    .help("copy a circuit to perform optimization");
                parser.add_argument<bool>("-statistics")
                    .default_value(false)
                    .action(store_true)
                    .help("count the number of rules operated in optimizer.");
                parser.add_argument<bool>("-trivial")
                    .default_value(false)
                    .action(store_true)
                    .help("Use the trivial optimization.");
            },
            [&](ArgumentParser const& parser) {
                if (!qcir_mgr_not_empty(qcir_mgr)) return CmdExecResult::error;
                Optimizer optimizer;
                std::optional<QCir> result;
                std::string procedure_str{};
                if (parser.get<bool>("-trivial")) {
                    result        = optimizer.trivial_optimization(*qcir_mgr.get());
                    procedure_str = "Trivial Optimize";
                } else {
                    result        = optimizer.basic_optimization(*qcir_mgr.get(), {.doSwap             = !parser.get<bool>("-physical"),
                                                                                   .separateCorrection = false,
                                                                                   .maxIter            = 1000,
                                                                                   .printStatistics    = parser.get<bool>("-statistics")});
                    procedure_str = "Optimize";
                }
                if (result == std::nullopt) {
                    spdlog::error("Fail to optimize circuit.");
                    return CmdExecResult::error;
                }

                if (parser.get<bool>("-copy")) {
                    qcir_mgr.add(qcir_mgr.get_next_id(), std::make_unique<QCir>(std::move(*result)));
                } else {
                    qcir_mgr.set(std::make_unique<QCir>(std::move(*result)));
                }

                if (stop_requested()) {
                    procedure_str += "[INT]";
                }

                qcir_mgr.get()->add_procedure(procedure_str);

                return CmdExecResult::done;
            }};
}

bool add_qcir_optimize_cmds(dvlab::CommandLineInterface& cli, QCirMgr& qcir_mgr) {
    if (!(cli.add_command(qcir_optimize_cmd(qcir_mgr)))) {
        spdlog::critical("Registering \"optimize\" commands fails... exiting");
        return false;
    }
    return true;
}

}  // namespace qsyn::qcir
