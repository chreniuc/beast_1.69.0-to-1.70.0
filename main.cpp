
class::class(
  ::boost::asio::io_context& ioc,
  ::boost::asio::io_context& main_context,
  ::boost::asio::ssl::context& ctx)
  : ctx(ctx), acceptor(main_context), socket(ioc)
{

 // main_context is used only for accepting connections
 // ioc is used for write and read operations
 
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

connection->ws = std::make_shared< boost::beast::websocket::stream<
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>>(
  std::move(socket), ctx);

connection->strand = std::make_shared<
  boost::asio::strand<boost::asio::io_context::executor_type>>(
  connection->ws->get_executor());

do_handshake(connection);

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
  // do handshake
  conn_ptr->ws->next_layer().async_handshake(
    boost::asio::ssl::stream_base::server,
    boost::asio::bind_executor(
      *connection->strand,
      std::bind(
        &class::on_handshake,
        this,
        connection,
        std::placeholders::_1)));
}
//----------------------------------------------------------------------------//
void class::on_handshake(const connection_ptr& connection,
  boost::system::error_code ec)
{
  // ... handle error
  // Accept the websocket handshake
  conn_ptr->ws->async_accept(
    boost::asio::bind_executor(
      *connection->strand,
      std::bind(
        &class::on_accept,
        this,
        connection,
        std::placeholders::_1)));
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
    boost::asio::bind_executor(
      *connection->strand,
      std::bind(
        &class::on_read,
        this,
        connection,
        message_ptr,
        std::placeholders::_1,
        std::placeholders::_2)));
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
    boost::asio::bind_executor(
      *connection->strand,
      std::bind(
        &class::on_write,
        this,
        connection,
        message_ptr,
        std::placeholders::_1,
        std::placeholders::_2 )));
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
    boost::asio::bind_executor(
      *connection->strand,
      std::bind(
        &class::on_close,
        this,
        connection,
        message_ptr,
        std::placeholders::_1)));
}
//----------------------------------------------------------------------------//
void class::on_close(const connection_ptr& connection,
  const std::shared_ptr<string> &message, boost::system::error_code ec )
{
  // do stuff
}
