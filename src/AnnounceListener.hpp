#ifndef IMC_STRAY_ANNOUNCELISTENER_HPP
#define IMC_STRAY_ANNOUNCELISTENER_HPP

#include <IMC/Base/Packet.hpp>
#include <IMC/Spec/Announce.hpp>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <QObject>

class SystemListener : public QObject
{
Q_OBJECT
  static constexpr std::array<int, 5> c_ports = {30100, 30101, 30102, 30103, 30104};
  int sockfd; /* socket */
  int portno; /* port to listen on */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  std::array<char, 4096> buf; /* message buf */
  char* hostaddrp; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */

  static void
  error(const char* msg)
  {
    printf("%s\n", msg);
  }

  bool bind(int port)
  {
    portno = port;

    /*
     * socket: create the parent socket
     */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
      error("ERROR opening socket");
      return false;
    }

    /* setsockopt: Handy debugging trick that lets
     * us rerun the server immediately after we kill it;
     * otherwise we have to wait about 20 secs.
     * Eliminates "ERROR on binding: Address already in use" error.
     */
    optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
               (const void *)&optval , sizeof(int));

    /*
     * build the server's Internet address
     */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)portno);

    /*
     * bind: associate the parent socket with a port
     */
    if (::bind(sockfd, (struct sockaddr *) &serveraddr,
               sizeof(serveraddr)) < 0)
    {
      error("ERROR on binding");
      return false;
    }

    return true;
  }

public:
  //! Timeout after which a node is considered stale
  static constexpr float c_stale_node_timeout = 20.0;

  SystemListener() :
      sockfd(-1),
      portno(-1),
      serveraddr(),
      clientaddr(),
      hostp(nullptr),
      buf(),
      hostaddrp(nullptr),
      optval(0)
  {  }

  int
  getPort()
  {
    return portno;
  }

  bool
  bind()
  {
    return std::any_of(c_ports.begin(), c_ports.end(), [this](int port) {return bind(port);});
  }

  bool
  poll(double timeout)
  {
    fd_set rfd;
    FD_ZERO(&rfd);
    FD_SET(sockfd, &rfd);

    int rv = 0;
    if (timeout < 0.0)
    {
      rv = select(sockfd + 1, &rfd, nullptr, nullptr, nullptr);
    }
    else
    {
      timeval tv = {(long)timeout, (long)((timeout - (long)timeout) * 1000000u)};
      rv = select(sockfd + 1, &rfd, nullptr, nullptr, &tv);
    }

    if (rv == -1)
    {
      if (errno == EINTR)
        return false;
      else
        throw std::runtime_error("polling error");
    }

    return rv > 0;
  }

  std::pair<QString, IMC::Announce*>
  read()
  {
    int clientlen = sizeof(clientaddr);
    size_t n = recvfrom(sockfd, &buf.data()[0], 4096, 0,
                        (struct sockaddr *) &clientaddr, (::socklen_t*)&clientlen);

    if (n == 0)
      throw std::runtime_error("failed to read");

    IMC::Message* msg;
    msg = IMC::Packet::deserialize((uint8_t *) buf.data(), n);

    if (msg == nullptr)
    {
      std::cerr << "null message" << std::endl;
      delete msg;
      return {};
    }

    if (msg->getId() != IMC::Announce::getIdStatic())
    {
      std::cerr <<  "discarding spurious message " << msg->getName();
      delete msg;
      return {};
    }

    char str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(clientaddr.sin_addr.s_addr), str, INET_ADDRSTRLEN);

    return std::make_pair(str, (IMC::Announce*) msg);
  }

signals:
  void announceEvent(IMC::Announce* announce, QString addr);
  void checkPurgeEvent();
};


#endif
