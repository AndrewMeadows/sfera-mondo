// sfera-mondo/apps/hello/server/main.cpp
//

#include <fmt/printf.h>
#include <tclap/CmdLine.h>

#include <util/LogUtil.h>

const uint32_t MAJOR_VERSION = 0;
const uint32_t MINOR_VERSION = 1;
const uint32_t PATCH_VERSION = 0;
const uint32_t DEFAULT_GRPC_PORT = 50051;
std::string DEFAULT_NAME = "homer";

int32_t main(int32_t argc, char** argv) {
    std::string version_string = fmt::format("{}.{}.{}",
            MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION);
    TCLAP::CmdLine cmd("Example sferamondo app", '=', version_string);

    bool required = true;
    TCLAP::ValueArg<uint32_t> verboseArg("v", "verbose", "verbosity level (0-3)", !required, 0, "level");
    TCLAP::ValueArg<uint32_t> portArg("p", "port", "gRPC port", !required, DEFAULT_GRPC_PORT, "number");
    TCLAP::ValueArg<std::string> nameArg("n", "name", "Name to print", !required, DEFAULT_NAME, "fubar");
    TCLAP::SwitchArg reverseSwitch("r", "reverse", "Print name backwards", !required);

    // Note: 'help' will list options in reverse order from how they were added
    cmd.add(verboseArg);
    cmd.add(portArg);
    cmd.add(nameArg);
    cmd.add(reverseSwitch);

    // 'try' because tclap will 'throw' exceptions
    try {
        cmd.parse(argc, argv);
    } catch (TCLAP::ArgException &e) {
        fmt::printf("error: '{}' for arg {}\n", e.error(), e.argId());
        return 1;
    }

    uint32_t verbosity = verboseArg.getValue();
    LogUtil::set_verbosity(verbosity);
    LOG1("verbosity={}\n", verbosity);

    std::string name = nameArg.getValue();
    bool reverseName = reverseSwitch.getValue();

    if (reverseName) {
        std::reverse(name.begin(),name.end());
        std::cout << "My name (spelled backwards) is: " << name << std::endl;
    } else {
        std::cout << "My name is: " << name << std::endl;
    }

    fmt::printf("hello world\n");
    return 0;
}
