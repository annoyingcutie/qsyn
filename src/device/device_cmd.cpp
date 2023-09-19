/****************************************************************************
  PackageName  [ device ]
  Synopsis     [ Define device package commands ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cstddef>
#include <memory>
#include <string>

#include "cli/cli.hpp"
#include "device/device.hpp"
#include "device/device_mgr.hpp"
#include "fmt/core.h"

using namespace dvlab::argparse;
using dvlab::CmdExecResult;
using dvlab::Command;

namespace qsyn::device {

bool device_mgr_not_empty(DeviceMgr const& device_mgr) {
    return dvlab::utils::expect(!device_mgr.empty(), "Device list is empty now. Please DTRead first.");
}

Command device_checkout_cmd(qsyn::device::DeviceMgr& device_mgr);
Command device_mgr_reset_cmd(qsyn::device::DeviceMgr& device_mgr);
Command device_delete_cmd(qsyn::device::DeviceMgr& device_mgr);
Command device_graph_read_cmd(qsyn::device::DeviceMgr& device_mgr);
Command device_graph_print_cmd(qsyn::device::DeviceMgr& device_mgr);
Command device_mgr_print_cmd(qsyn::device::DeviceMgr& device_mgr);

bool add_device_cmds(dvlab::CommandLineInterface& cli, qsyn::device::DeviceMgr& device_mgr) {
    if (!(cli.add_command(device_checkout_cmd(device_mgr)) &&
          cli.add_command(device_mgr_reset_cmd(device_mgr)) &&
          cli.add_command(device_delete_cmd(device_mgr)) &&
          cli.add_command(device_graph_read_cmd(device_mgr)) &&
          cli.add_command(device_graph_print_cmd(device_mgr)) &&
          cli.add_command(device_mgr_print_cmd(device_mgr)))) {
        LOGGER.fatal("Registering \"device topology\" commands fails... exiting");
        return false;
    }
    return true;
}

std::function<bool(size_t const&)> valid_device_id(qsyn::device::DeviceMgr const& device_mgr) {
    return [&device_mgr](size_t const& id) {
        if (device_mgr.is_id(id)) return true;
        LOGGER.error("Device {} does not exist!!", id);
        return false;
    };
};

dvlab::Command device_checkout_cmd(qsyn::device::DeviceMgr& device_mgr) {
    return {"dtcheckout",
            [&device_mgr](ArgumentParser& parser) {
                parser.description("checkout to Device <id> in DeviceMgr");

                parser.add_argument<size_t>("id")
                    .constraint(valid_device_id(device_mgr))
                    .help("the ID of the device");
            },
            [&device_mgr](ArgumentParser const& parser) {
                device_mgr.checkout(parser.get<size_t>("id"));
                return CmdExecResult::done;
            }};
}

dvlab::Command device_mgr_reset_cmd(qsyn::device::DeviceMgr& device_mgr) {
    return {"dtreset",
            [](ArgumentParser& parser) {
                parser.description("reset DeviceMgr");
            },
            [&device_mgr](ArgumentParser const& /*parser*/) {
                device_mgr.reset();
                return CmdExecResult::done;
            }};
    auto cmd = std::make_unique<dvlab::Command>("DTReset");
}

dvlab::Command device_delete_cmd(qsyn::device::DeviceMgr& device_mgr) {
    return {"dtdelete",
            [&device_mgr](ArgumentParser& parser) {
                parser.description("remove a Device from DeviceMgr");

                parser.add_argument<size_t>("id")
                    .constraint(valid_device_id(device_mgr))
                    .help("the ID of the device");
            },
            [&device_mgr](ArgumentParser const& parser) {
                device_mgr.remove(parser.get<size_t>("id"));
                return CmdExecResult::done;
            }};
}

dvlab::Command device_graph_read_cmd(qsyn::device::DeviceMgr& device_mgr) {
    return {"dtgread",
            [](ArgumentParser& parser) {
                parser.description("read a device topology");

                parser.add_argument<std::string>("filepath")
                    .help("the filepath to device file");

                parser.add_argument<bool>("-replace")
                    .action(store_true)
                    .help("if specified, replace the current device; otherwise store to a new one");
            },
            [&device_mgr](ArgumentParser const& parser) {
                qsyn::device::Device buffer_device;
                auto filepath = parser.get<std::string>("filepath");
                auto replace  = parser.get<bool>("-replace");

                if (!buffer_device.read_device(filepath)) {
                    LOGGER.error("the format in \"{}\" has something wrong!!", filepath);
                    return CmdExecResult::error;
                }

                if (device_mgr.empty() || !replace) {
                    device_mgr.add(device_mgr.get_next_id(), std::make_unique<qsyn::device::Device>(std::move(buffer_device)));
                } else {
                    device_mgr.set(std::make_unique<qsyn::device::Device>(std::move(buffer_device)));
                }

                return CmdExecResult::done;
            }};
}

dvlab::Command device_mgr_print_cmd(qsyn::device::DeviceMgr& device_mgr) {
    return {"dtprint",
            [](ArgumentParser& parser) {
                parser.description("print info about Devices");

                auto mutex = parser.add_mutually_exclusive_group();

                mutex.add_argument<bool>("-focus")
                    .action(store_true)
                    .help("print the info of device in focus");
                mutex.add_argument<bool>("-list")
                    .action(store_true)
                    .help("print a list of devices");
            },
            [&device_mgr](ArgumentParser const& parser) {
                if (parser.parsed("-focus"))
                    device_mgr.print_focus();
                else if (parser.parsed("-list"))
                    device_mgr.print_list();
                else
                    device_mgr.print_manager();

                return CmdExecResult::done;
            }};
}

//-----------------------------------------------------------------------------------------------------------
//    DTGPrint [-Summary | -Edges | -Path | -Qubit]
//-----------------------------------------------------------------------------------------------------------

dvlab::Command device_graph_print_cmd(qsyn::device::DeviceMgr& device_mgr) {
    return {"dtgprint",
            [](ArgumentParser& parser) {
                parser.description("print info of device topology");

                auto mutex = parser.add_mutually_exclusive_group().required(false);

                mutex.add_argument<bool>("-summary")
                    .action(store_true)
                    .help("print basic information of the topology");

                mutex.add_argument<size_t>("-edges")
                    .nargs(0, 2)
                    .help(
                        "print information of edges. "
                        "If no qubit ID is specified, print for all edges; "
                        "if one qubit ID specified, list the adjacent edges to the qubit; "
                        "if two qubit IDs are specified, list the edge between them");

                mutex.add_argument<size_t>("-qubits")
                    .nargs(NArgsOption::zero_or_more)
                    .help(
                        "print information of qubits. "
                        "If no qubit ID is specified, print for all qubits;"
                        "otherwise, print information of the specified qubit IDs");

                mutex.add_argument<size_t>("-path")
                    .nargs(2)
                    .metavar("(q1, q2)")
                    .help(
                        "print routing paths between q1 and q2");
            },
            [&device_mgr](ArgumentParser const& parser) {
                if (!qsyn::device::device_mgr_not_empty(device_mgr)) return CmdExecResult::error;

                if (parser.parsed("-edges")) {
                    device_mgr.get()->print_edges(parser.get<std::vector<size_t>>("-edges"));
                    return CmdExecResult::done;
                }
                if (parser.parsed("-qubits")) {
                    device_mgr.get()->print_qubits(parser.get<std::vector<size_t>>("-qubits"));
                    return CmdExecResult::done;
                }
                if (parser.parsed("-path")) {
                    auto qids = parser.get<std::vector<size_t>>("-path");
                    device_mgr.get()->print_path(qids[0], qids[1]);
                    return CmdExecResult::done;
                }

                device_mgr.get()->print_topology();
                return CmdExecResult::done;
            }};
}

}  // namespace qsyn::device
