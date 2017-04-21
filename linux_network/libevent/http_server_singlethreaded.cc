//
// 2010/11/06 Kazuki Ohta <kazuki.ohta@gmail.com>
// Single-Threaded HTTPServer using evhttp
//

#include <event.h>
#include <evhttp.h>
#include <unistd.h>

namespace servers {
namespace util {

class HTTPServer {
public:
  HTTPServer() {}
  ~HTTPServer() {}
  int serv(int port);
protected:
  static void GenericHandler(struct evhttp_request *req, void *arg);
  void ProcessRequest(struct evhttp_request *request);
};

int HTTPServer::serv(int port) {
  struct event_base *base = NULL;
  struct evhttp *httpd = NULL;
  base = event_init();
  if (base == NULL) return -1;
  httpd = evhttp_new(base);
  if (httpd == NULL) return -1;
  if (evhttp_bind_socket(httpd, "0.0.0.0", port) != 0) return -1;
  evhttp_set_gencb(httpd, HTTPServer::GenericHandler, (void*)this);
  event_base_dispatch(base);
  return 0;
}

void HTTPServer::GenericHandler(struct evhttp_request *req, void *arg) {
  ((HTTPServer*)arg)->ProcessRequest(req);
}

void HTTPServer::ProcessRequest(evhttp_request *req) {
  sleep(1);
  struct evbuffer *buf = evbuffer_new();
  if (buf == NULL) return;
  evbuffer_add_printf(buf, "Requested: %s\n", evhttp_request_uri(req));
  evhttp_send_reply(req, HTTP_OK, "OK", buf);
}

}
}

int main() {
  servers::util::HTTPServer s;
  s.serv(19850);
}
