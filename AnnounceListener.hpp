#ifndef IMC_STRAY_ANNOUNCELISTENER_HPP
#define IMC_STRAY_ANNOUNCELISTENER_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

class SystemListener
{
  int sockfd; /* socket */
  int portno; /* port to listen on */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  std::array<char, 4096> buf; /* message buf */
  char* hostaddrp; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */

  constexpr static void
  error(const char* msg)
  {
    printf("%s", msg);
  }

public:
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

  std::unique_ptr<IMC::Announce>
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
      std::cout << "null message" << std::endl;
      delete msg;
      return nullptr;
    }

    if (msg->getId() != IMC::Announce::getIdStatic())
    {
      std::cout <<  "discarding spurious message " << msg->getName();
      delete msg;
      return nullptr;
    }

    std::unique_ptr<IMC::Announce> ptr;
    ptr.reset((IMC::Announce*) msg);
    return ptr;
  }
};


#endif
