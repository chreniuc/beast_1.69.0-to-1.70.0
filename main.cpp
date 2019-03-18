
class::class(
  ::boost::asio::io_context& ioc,
  ::boost::asio::io_context& main_context,
  ::boost::asio::ssl::context& ctx)
  : ctx(ctx), acceptor(main_context), socket(::boost::asio::make_strand(ioc)), rw_ioc(ioc)
{
 // main_context is used only for accepting connections
 // rw_ioc is used for write and read operations, save it as a refence to pass it to make_strand
 
 // load server certificates
 // prepare acceptor
 acceptor.async_accept(
  socket,
  std::bind(
    &class::on_socket_accept,
    this,
    std::placeholders::_1));
}
//----------------------------------------------------------------------------//
void class::on_socket_accept(boost::system::error_code ec)
{
//... handle error
// Create the session and run it
std::shared_ptr<connection> connection =
  std::make_shared<connection>();

connection->ws = std::make_shared< websocket::stream<beast::ssl_stream<
   beast::tcp_stream>>>(std::move(socket), ctx);

do_handshake(connection);
socket = boost::asio::ip::tcp::socket(boost::asio::make_strand(rw_ioc));

// Accept another connection
acceptor.async_accept(
  socket,
  std::bind(
    &class::on_socket_accept,
    this,
    std::placeholders::_1));
}
//----------------------------------------------------------------------------//
void class::do_handshake(const connection_ptr& connection)
{
  connection->ws.set_option(
    websocket::stream_base::timeout::suggested(beast::role_type::server));
  // do handshake
  conn_ptr->ws->next_layer().async_handshake(
    boost::asio::ssl::stream_base::server,
      std::bind(
        &class::on_handshake,
        this,
        connection,
        std::placeholders::_1));
}
//----------------------------------------------------------------------------//
void class::on_handshake(const connection_ptr& connection,
  boost::system::error_code ec)
{
  // ... handle error
  // Accept the websocket handshake
  conn_ptr->ws->async_accept(
      std::bind(
        &class::on_accept,
        this,
        connection,
        std::placeholders::_1));
}
//----------------------------------------------------------------------------//
void class::on_accept(const connection_ptr& connection,
  boost::system::error_code ec)
{
  //... handle error
  // Read a message
  do_read(connection);
}
//----------------------------------------------------------------------------//
void class::do_read(const connection_ptr& connection)
{
  std::shared_ptr<boost::beast::flat_buffer> message_ptr =
    std::make_shared<boost::beast::flat_buffer>();
  connection->ws->async_read(
    *message_ptr,
      std::bind(
        &class::on_read,
        this,
        connection,
        message_ptr,
        std::placeholders::_1,
        std::placeholders::_2));
}
//----------------------------------------------------------------------------//
void class::on_read(const connection_ptr& connection,
  std::shared_ptr<boost::beast::flat_buffer> &message_ptr,
  boost::system::error_code ec, size_t bytes_transferred)
{
  //.. do stuff
  do_read(conn_ptr);
}
//----------------------------------------------------------------------------//
void class::do_write(const connection_ptr& connection,
  const std::shared_ptr<string>& message_ptr )
{
  connection->ws->async_write(
    boost::asio::buffer(*message_ptr),
      std::bind(
        &class::on_write,
        this,
        connection,
        message_ptr,
        std::placeholders::_1,
        std::placeholders::_2 ));
}
//----------------------------------------------------------------------------//
void class::on_write(const connection_ptr& connection,
  const std::shared_ptr<string> &message,
  boost::system::error_code ec, size_t /*bytes_transferred*/ )
{
    // do stuff
    do_write(conn_ptr, message);
}
//----------------------------------------------------------------------------//
void class::do_close(const connection_ptr& connection,
  const std::shared_ptr<string> &message_ptr)
{
  connection->ws->async_close(
    boost::beast::websocket::close_reason("reason"),
      std::bind(
        &class::on_close,
        this,
        connection,
        message_ptr,
        std::placeholders::_1));
}
//----------------------------------------------------------------------------//
void class::on_close(const connection_ptr& connection,
  const std::shared_ptr<string> &message, boost::system::error_code ec )
{
  // do stuff
}
