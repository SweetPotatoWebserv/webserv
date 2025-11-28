#include "ClientHandlerManager.h"
#include <signal.h>
#include <sys/wait.h>
#include "../core/Fd.h"



void check_cgi_timeout(CgiSession& cgi_session) {

}

void check_request_timeout() {

}

void ClientHandlerManager::check_timeout_all() {
    for (std::vector<ClientHandler>::iterator client_handlers = client_handlers_.begin(); client_handlers != client_handlers_.end(); ++client_handlers) {
        check_cgi_timeout(client_handlers->getCgiSession());
        CgiSession cgi_session = ;
        if (cgi_session.pid == -1) {
            return ;
        }

        std::time_t now = std::time(NULL);
        if (std::difftime(now, cgi_session.startTime) >= CGI_TIMEOUT_SEC) {
            kill(cgi_session.pid, SIGKILL);
            waitpid(cgi_session.pid, NULL, 0);
            cgi_session.pid = -1;
            if (cgi_session.readFd != -1) {
                event_.del(cgi_session.readFd);
                Fd::SafeClose(cgi_session.readFd);
                cgi_session.readFd = -1;
            }
            if (cgi_session.writeFd != -1) {
                event_.del(cgi_session.writeFd);
                Fd::SafeClose(cgi_session.writeFd);
                cgi_session.writeFd = -1;
            }
            handle_cgi_error(HttpStatus::RequestTimeout);
        }
    }
}

