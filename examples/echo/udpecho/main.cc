#include <evpp/udp/udp_server.h>
#include <evpp/udp/udp_message.h>
#include <evpp/event_loop.h>
#include <evpp/event_loop_thread_pool.h>

#ifdef _WIN32
#include "../../winmain-inl.h"
#endif

#define bind_addr "0.0.0.0"
#define send_addr "0.0.0.0"
#define iface "lo"

int main(int argc, char* argv[]) {
    std::vector<int> ports = { 1053, 5353 };
    int port = 29099;
    int thread_num = 2;

    if (argc > 1) {
        if (std::string("-h") == argv[1] ||
            std::string("--h") == argv[1] ||
            std::string("-help") == argv[1] ||
            std::string("--help") == argv[1]) {
            std::cout << "usage : " << argv[0] << " <listen_port> <thread_num>\n";
            std::cout << " e.g. : " << argv[0] << " 8080 24\n";
            return 0;
        }
    }

    if (argc == 2) {
        port = atoi(argv[1]);
    }
    ports.push_back(port);

    if (argc == 3) {
        port = atoi(argv[1]);
        thread_num = atoi(argv[2]);
    }

    evpp::udp::Server server;
    server.setBindAddr(bind_addr);
    server.setInterface(iface);
    server.SetThreadDispatchPolicy(evpp::ThreadDispatchPolicy::kIPAddressHashing);
    server.SetMessageHandler([](evpp::EventLoop* loop, evpp::udp::MessagePtr& msg) {
        std::stringstream oss;
        oss << "func=" << __FUNCTION__ << " OK"
            << " body=" << std::string(msg->data(), msg->size()) << "\n";
        auto tt = (sockaddr_in *) msg -> remote_addr_edit();
        tt->sin_addr.s_addr = inet_addr(send_addr);
        std::cout << msg -> remote_ip() << "\n";
        evpp::udp::SendMessage(msg);
    });
    server.Init(ports);
    server.Start();

    evpp::EventLoop loop;
    std::shared_ptr<evpp::EventLoopThreadPool> tpool(new evpp::EventLoopThreadPool(&loop, thread_num));
    tpool->Start(true);
    server.SetEventLoopThreadPool(tpool);
    loop.Run();
    server.Stop(true);
    tpool->Stop(true);
    return 0;
}
