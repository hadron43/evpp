#include <evpp/udp/udp_server.h>
#include <evpp/udp/udp_message.h>
#include <evpp/event_loop.h>
#include <evpp/event_loop_thread_pool.h>

#ifdef _WIN32
#include "../../winmain-inl.h"
#endif

#define bind_addr "0.0.0.0"
#define send_addr "0.0.0.0"
#define send_port 1055
#define iface "lo"

int main(int argc, char* argv[]) {
    std::vector<int> ports = { 1053, 5353 };
    int port = 29099;
    int thread_num = 2;
    int f_sockfd;
    struct sockaddr_in f_addr;

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

    if (argc == 3) {
        port = atoi(argv[1]);
        thread_num = atoi(argv[2]);
    }
    ports.push_back(port);

    // Creating socket file descriptor
    if ( (f_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("forward socket creation failed");
        exit(EXIT_FAILURE);
    }
    memset(&f_addr, 0, sizeof(f_addr));
    // Filling server information
    f_addr.sin_family = AF_INET;
    f_addr.sin_port = htons(send_port);
    f_addr.sin_addr.s_addr = inet_addr(send_addr);

    evpp::udp::Server server;
    server.setBindAddr(bind_addr);
    server.setInterface(iface);
    server.SetThreadDispatchPolicy(evpp::ThreadDispatchPolicy::kIPAddressHashing);
    server.SetMessageHandler([&](evpp::EventLoop* loop, evpp::udp::MessagePtr& msg) {
        std::stringstream oss;
        oss << "func=" << __FUNCTION__ << " OK"
            << " body=" << std::string(msg->data(), msg->size()) << "\n";

        sendto(f_sockfd, msg -> data(), strlen(msg->data()) + 1,
        MSG_CONFIRM, (const struct sockaddr *) &f_addr,
            sizeof(f_addr));
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
