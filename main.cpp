#include <iostream>
#include <string>
#include <unordered_map>
#include <functional>
#include <cctype>
#include <cstring>
#include <chrono>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <ctime>

#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>

struct Options {
    int camIndex = -1;           
    std::string source;          
    int width = 1280;            
    int height = 720;            
    int backend = cv::CAP_ANY;   
    bool showHelp = false;       
};

static void printUsage() {
    std::cout <<
        R"(Camera-Control (v0.1.x)

Usage:
  Camera-Control.exe [--cam <index> | --source <path_or_url>] [--width <W>] [--height <H>] [--backend <msmf|dshow|ffmpeg|any>]

Examples:
  Camera-Control.exe --cam 0 --width 1920 --height 1080
  Camera-Control.exe --source "C:\videos\test.mp4"
  Camera-Control.exe --source "rtsp://user:pass@host/stream" --backend ffmpeg

Keys:
  q / ESC  quit
  s        save current frame as frame-YYYYMMDD_HHMMSS.png
)";
}

static std::string toLower(std::string s) {
    for (auto& c : s) c = char(std::tolower(unsigned char(c)));
    return s;
}

static const char* backendName(int b) {
    switch (b) {
    case cv::CAP_MSMF:  return "MSMF";
    case cv::CAP_DSHOW: return "DSHOW";
    case cv::CAP_FFMPEG:return "FFMPEG";
    case cv::CAP_ANY:   return "ANY";
    default:            return "UNKNOWN";
    }
}

static std::string timestamp() {
    using clock = std::chrono::system_clock;
    std::time_t t = clock::to_time_t(clock::now());
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    tm = *std::localtime(&t);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d_%H%M%S");
    return oss.str();
}

static Options parseArgs(int argc, char** argv) {
    Options opt;
    bool error = false;

    auto require = [&](int i, const char* name) -> bool {
        if (i + 1 >= argc) {
            std::cerr << "ERROR: Missing value for " << name << "\n";
            opt.showHelp = true;
            return false;
        }
        return true;
        };
    auto parseInt = [&](const char* s, const char* name, int& out) -> bool {
        try {
            size_t pos = 0;
            long v = std::stol(s, &pos);
            if (pos == std::strlen(s)) { out = int(v); return true; }
        }
        catch (...) {}
        std::cerr << "ERROR: " << name << " expects an integer, got '" << s << "'\n";
        opt.showHelp = true;
        return false;
        };

    using Handler = std::function<void(int& i)>;
    std::unordered_map<std::string, Handler> H;

    H["--help"] = [&](int&) { opt.showHelp = true; };
    H["-h"] = H["--help"];

    H["--cam"] = [&](int& i) {
        if (!require(i, "--cam")) { error = true; return; }
        int v;
        if (!parseInt(argv[++i], "--cam", v)) { error = true; return; }
        opt.camIndex = v;
        };

    H["--source"] = [&](int& i) {
        if (!require(i, "--source")) { error = true; return; }
        opt.source = argv[++i];
        };

    H["--width"] = [&](int& i) {
        if (!require(i, "--width")) { error = true; return; }
        int v;
        if (!parseInt(argv[++i], "--width", v) || v <= 0) {
            std::cerr << "ERROR: --width must be > 0\n";
            opt.showHelp = true; error = true; return;
        }
        opt.width = v;
        };

    H["--height"] = [&](int& i) {
        if (!require(i, "--height")) { error = true; return; }
        int v;
        if (!parseInt(argv[++i], "--height", v) || v <= 0) {
            std::cerr << "ERROR: --height must be > 0\n";
            opt.showHelp = true; error = true; return;
        }
        opt.height = v;
        };

    H["--backend"] = [&](int& i) {
        if (!require(i, "--backend")) { error = true; return; }
        std::string b = toLower(argv[++i]);
        if (b == "msmf")   opt.backend = cv::CAP_MSMF;
        else if (b == "dshow")  opt.backend = cv::CAP_DSHOW;
        else if (b == "ffmpeg") opt.backend = cv::CAP_FFMPEG;
        else if (b == "any")    opt.backend = cv::CAP_ANY;
        else {
            std::cerr << "WARN: Unknown backend '" << b << "', using ANY.\n";
            opt.backend = cv::CAP_ANY;
        }
        };

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];

        if (a.rfind("--", 0) != 0) {
            if (opt.source.empty()) opt.source = a;
            else std::cerr << "WARN: Unknown argument: " << a << "\n";
            continue;
        }

        if (auto it = H.find(a); it != H.end()) {
            it->second(i);           
            if (error) break;
        }
        else {
            std::cerr << "WARN: Unknown flag: " << a << "\n";
        }
    }

    if (!opt.source.empty() && opt.camIndex >= 0) {
        std::cerr << "INFO: --source given; ignoring --cam " << opt.camIndex << "\n";
        opt.camIndex = -1;
    }
    return opt;
}

static void dumpOptions(const Options& o) {
    std::cout << "[opts] camIndex=" << o.camIndex
        << " source=" << (o.source.empty() ? "(none)" : o.source)
        << " size=" << o.width << "x" << o.height
        << " backend=" << backendName(o.backend)
        << " showHelp=" << std::boolalpha << o.showHelp << "\n";
}


int main(int argc, char** argv) {
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_WARNING);

    Options opt = parseArgs(argc, argv);
    if (opt.showHelp) { printUsage(); return 0; }

    dumpOptions(opt); 

    cv::VideoCapture cap;

    if (opt.camIndex >= 0) {
        cap.open(opt.camIndex, opt.backend);
    }
    else if (!opt.source.empty()) {
        cap.open(opt.source, opt.backend);
    }
    else {
        cap.open(0, opt.backend); 
    }

    if (!cap.isOpened() && opt.backend != cv::CAP_ANY) {
        std::cerr << "WARN: Open failed with backend " << backendName(opt.backend)
            << ", retrying with CAP_ANY...\n";
        if (opt.camIndex >= 0) cap.open(opt.camIndex, cv::CAP_ANY);
        else if (!opt.source.empty()) cap.open(opt.source, cv::CAP_ANY);
    }

    if (!cap.isOpened()) {
        std::cerr << "ERROR: Could not open camera/source.\n";
        return 1;
    }
   
    cap.set(cv::CAP_PROP_FRAME_WIDTH, opt.width);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, opt.height);

    const std::string win = "Preview";
    cv::namedWindow(win, cv::WINDOW_AUTOSIZE);
    std::cout << "Preview running. Press 'q' or ESC to quit, 's' to save a frame.\n";

    using clock = std::chrono::steady_clock;
    auto t0 = clock::now();
    int frames = 0;
    double fps = 0.0;
    int saved = 0;

    cv::Mat frame;
    for (;;) {
        if (!cap.read(frame)) {
            std::cerr << "WARN: Failed to grab frame.\n";
            break;
        }

        frames++;
        auto t1 = clock::now();
        double sec = std::chrono::duration<double>(t1 - t0).count();
        if (sec >= 1.0) {
            fps = frames / sec;
            frames = 0;
            t0 = t1;
        }

        
        int aw = (int)cap.get(cv::CAP_PROP_FRAME_WIDTH);
        int ah = (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT);

        
        std::ostringstream oss;
        oss << "FPS: " << std::fixed << std::setprecision(1) << fps
            << "  |  " << aw << "x" << ah
            << "  |  saved: " << saved;
        cv::putText(frame, oss.str(), { 10, 30 }, cv::FONT_HERSHEY_SIMPLEX, 0.8,
            cv::Scalar(0, 255, 0), 2, cv::LINE_AA);

        cv::imshow(win, frame);

        
        if (cv::getWindowProperty(win, cv::WND_PROP_VISIBLE) < 1) break;

        int key = cv::waitKey(1) & 0xFF;
        if (key == 27 || key == 'q') break;
        if (key == 's') {
            std::string name = "frame-" + timestamp() + ".png";
            bool ok = cv::imwrite(name, frame);
            if (ok) { ++saved; std::cout << "Saved: " << name << "\n"; }
            else     std::cout << "Failed to save frame.\n";
        }
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}