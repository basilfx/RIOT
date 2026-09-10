/**
 * @defgroup pkg_picohttpparser  Tiny HTTP/1.x request and response parser
 * @ingroup  pkg
 * @ingroup  sys_net
 * @brief    Provides a minimal, dependency-free HTTP/1.x header parser to RIOT
 *
 * picohttpparser only parses HTTP/1.x request and response lines and
 * headers. It does not open sockets, buffer bodies, or generate
 * responses, that is left to the application, for example by using
 * @ref net_sock_tcp.
 *
 * @see      https://github.com/h2o/picohttpparser
 */
