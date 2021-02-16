// sfera-mondo/apps/hello/server/main.cpp
//

#include <chrono>
#include <csignal>
#include <thread>

#include <fmt/printf.h>
#include <tclap/CmdLine.h>

#include <util/LogUtil.h>
#include <util/TimeUtil.h>

constexpr uint32_t MAJOR_VERSION = 0;
constexpr uint32_t MINOR_VERSION = 1;
constexpr uint32_t PATCH_VERSION = 0;
constexpr uint32_t DEFAULT_GRPC_PORT = 50051;
const std::string DEFAULT_GREETING = "hello";
constexpr uint32_t DEFAULT_NUM_GREETS = 1;


int32_t g_num_exit_signals = 0;
int32_t g_exit_value = 0;

// exit_handler() is used to interrupt the mainloop
//
void exit_handler(int32_t signum ) {
    ++g_num_exit_signals;
    LOG1("received interrupt signal={} count={}\n", signum, g_num_exit_signals);
    if (signum == SIGTERM) {
        // SIGTERM indicates clean intentional shutdown
        g_exit_value = 0;
    } else {
        g_exit_value = 1;
    }
    if (g_num_exit_signals > 2) {
        // hint: send multiple signals to force exit on deadlock
        exit(1);
    }
}

// HelloConfig is an example ConfigInterface that can be updated
// at runtime, via editing a config file, or by network packet.
//
class HelloConfig : public ConfigUtil::ConfigInterface {
public:
    HelloConfig() { }

    ~HelloConfig() { }

    // required overrides
    nlohmann::json getJson() const override {
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        json obj;
        obj["greeting"] = _word;
        obj["number"] = _number;
        return obj;
    }

    void updateJson(const nlohmann::json& obj) override {
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        bool something_changed = false;
        if (obj.contains("greeting") && obj["greeting"].is_string()) {
            _word = obj["greeting"];
            something_changed = true;
        }
        if (obj.contains("number") && obj["number"].is_number()) {
            _number = obj["number"];
            something_changed = true;
        }
        if (something_changed) {
            bumpVersion();
        }
    }

    std::string getGreeting() const {
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        return _greeting;
    }

    uint32_t getNumber() const {
        // no lock necessary since this is effectively atomic
        return _number;
    }

private:
    std::string _greeting("hello");
    uint32_t _number { 1 };
};

int32_t main(int32_t argc, char** argv) {
    // prepare to catch INTERRUPT signal
    signal(SIGINT, exit_handler);
    signal(SIGTERM, exit_handler);

    std::string version_string = fmt::format("{}.{}.{}",
            MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION);
    TCLAP::CmdLine cmd("Example sferamondo app", '=', version_string);

    bool required = true;
    TCLAP::ValueArg<uint32_t> verbose_arg("v", "verbose", "verbosity level (0-3)", !required, 0, "level");
    TCLAP::ValueArg<uint32_t> port_arg("p", "port", "gRPC port", !required, DEFAULT_GRPC_PORT, "number");
    TCLAP::ValueArg<std::string> greeting_arg("G", "greeting", "Greeting to use in reply", !required, DEFAULT_GREETING, "greeting");
    TCLAP::ValueArg<uint32_t> number_arg("N", "number", "Number of greetings per reply", !required, DEFAULT_NUM_GREETS, "num_greets");
    //TCLAP::SwitchArg reverseSwitch("r", "reverse", "Print name backwards", !required);

    // Note: 'help' will list options in reverse order of how they were added
    cmd.add(verbose_arg);
    cmd.add(number_arg);
    cmd.add(greeting_arg);
    cmd.add(port_arg);
    //cmd.add(reverseSwitch);

    // 'try' because tclap will 'throw' exceptions
    try {
        cmd.parse(argc, argv);
    } catch (TCLAP::ArgException &e) {
        fmt::printf("error: '{}' for arg {}\n", e.error(), e.argId());
        return 1;
    }

    uint32_t verbosity = verbose_arg.getValue();
    LogUtil::set_verbosity(verbosity);
    LOG1("verbosity={}\n", verbosity);


    // config is initialized with settings via CLI arguments
    HelloConfig config;
    config.setGreeting(greeting_arg.getValue());
    config.setNumber(number_arg.getValue();

    // create the server which starts its own thread immediately
    mondo::Server server(&config);

    // mainloop
    constexpr uint32_t MAIN_LOOP_NAP = 5; // msec
    while (g_num_exit_signals == 0 && server.isRunning()) {
        //uint64_t now = TimeUtil::getNowMsec();
        std::this_thread::sleep_for(std::chrono::milliseconds(MAIN_LOOP_NAP));
    }

    // always shutdown the server
    // this will block until queues are cleared
    server.shutdown();

    return g_exit_value;
}
