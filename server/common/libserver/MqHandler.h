/**
 *  LibBoostAsio.h
 *
 *  Implementation for the AMQP::TcpHandler for boost::asio. You can use this class 
 *  instead of a AMQP::TcpHandler class, just pass the boost asio service to the 
 *  constructor and you're all set.  See tests/libboostasio.cpp for example.
 *
 *  Watch out: this class was not implemented or reviewed by the original author of 
 *  AMQP-CPP. However, we do get a lot of questions and issues from users of this class,
 *  so we cannot guarantee its quality. If you run into such issues too, it might be
 *  better to implement your own handler that interact with boost.
 *
 *
 *  @author Gavin Smith <gavin.smith@coralbay.tv>
 */


/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include <memory>

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/asio/buffer.hpp>
#include "amqpcpp.h"
#include "amqpcpp/linux_tcp.h"

// C++17 has 'weak_from_this()' support.
#if __cplusplus >= 201701L
#define PTR_FROM_THIS(T) weak_from_this()
#else
#define PTR_FROM_THIS(T) std::weak_ptr<T>(shared_from_this())
#endif

/**
 *  Set up namespace
 */

/**
 *  Class definition
 *  @note Because of a limitation on Windows, this will only work on POSIX based systems - see https://github.com/chriskohlhoff/asio/issues/70
 */
class MqHandler : public virtual AMQP::TcpHandler
{
protected:

    /**
     *  Helper class that wraps a boost io_context socket monitor.
     */
    class Watcher : public virtual std::enable_shared_from_this<Watcher>
    {
    private:

        /**
         *  The boost asio io_context which is responsible for detecting events.
         *  @var class boost::asio::io_context&
         */
        boost::asio::io_context & _ioservice;

        /**
         *  The boost asio io_context::strand managed pointer.
         *  @var class std::shared_ptr<boost::asio::io_context>
         */

        /**
         *  The boost tcp socket.
         *  @var class boost::asio::ip::tcp::socket
         *  @note https://stackoverflow.com/questions/38906711/destroying-boost-asio-socket-without-closing-native-handler
         */
#ifdef WIN32
		boost::asio::windows::stream_handle _socket;
#else
		boost::asio::posix::stream_descriptor _socket;
		
#endif 

        
		
        /**
         *  A boolean that indicates if the watcher is monitoring for read events.
         *  @var _read True if reads are being monitored else false.
         */
        bool _read{false};

        /**
         *  A boolean that indicates if the watcher has a pending read event.
         *  @var _read True if read is pending else false.
         */
        bool _read_pending{false};

        /**
         *  A boolean that indicates if the watcher is monitoring for write events.
         *  @var _read True if writes are being monitored else false.
         */
        bool _write{false};

        /**
         *  A boolean that indicates if the watcher has a pending write event.
         *  @var _read True if read is pending else false.
         */
        bool _write_pending{false};

        using handler_cb = boost::function<void(boost::system::error_code,std::size_t)>;
        using io_handler = boost::function<void(const boost::system::error_code&, const std::size_t)>;

        /**
         * Builds a io handler callback that executes the io callback in a strand.
         * @param  io_handler  The handler callback to dispatch
         * @return handler_cb  A function wrapping the execution of the handler function in a io_context::strand.
         */
      
        
        /**
         * Binds and returns a read handler for the io operation.
         * @param  connection   The connection being watched.
         * @param  fd           The file descripter being watched.
         * @return handler callback
         */
        handler_cb get_read_handler(AMQP::TcpConnection *const connection, const int fd)
        {
            auto fn = boost::bind(&Watcher::read_handler,
                                  this,
                                  _1,
                                  _2,
                                  PTR_FROM_THIS(Watcher),
                                  connection,
                                  fd);
            return (fn);
        }

        /**
         * Binds and returns a read handler for the io operation.
         * @param  connection   The connection being watched.
         * @param  fd           The file descripter being watched.
         * @return handler callback
         */
        handler_cb get_write_handler(AMQP::TcpConnection *const connection, const int fd)
        {
            auto fn = boost::bind(&Watcher::write_handler,
                                  this,
                                  _1,
                                  _2,
                                  PTR_FROM_THIS(Watcher),
                                  connection,
                                  fd);
            return (fn);
        }

        /**
         *  Handler method that is called by boost's io_context when the socket pumps a read event.
         *  @param  ec          The status of the callback.
         *  @param  bytes_transferred The number of bytes transferred.
         *  @param  awpWatcher  A weak pointer to this object.
         *  @param  connection  The connection being watched.
         *  @param  fd          The file descriptor being watched.
         *  @note   The handler will get called if a read is cancelled.
         */
        void read_handler(const boost::system::error_code &ec,
                          const std::size_t bytes_transferred,
                          const std::weak_ptr<Watcher> awpWatcher,
						  AMQP::TcpConnection *const connection,
                          const int fd)
        {
            // Resolve any potential problems with dangling pointers
            // (remember we are using async).
            const std::shared_ptr<Watcher> apWatcher = awpWatcher.lock();
            if (!apWatcher) { return; }

            _read_pending = false;

            if ((!ec || ec == boost::asio::error::would_block) && _read)
            {
                connection->process(fd, AMQP::readable);

                _read_pending = true;
                
                _socket.async_read_some(
                    boost::asio::null_buffers(),
                    get_read_handler(connection, fd));
            }
        }

        /**
         *  Handler method that is called by boost's io_context when the socket pumps a write event.
         *  @param  ec          The status of the callback.
         *  @param  bytes_transferred The number of bytes transferred.
         *  @param  awpWatcher  A weak pointer to this object.
         *  @param  connection  The connection being watched.
         *  @param  fd          The file descriptor being watched.
         *  @note   The handler will get called if a write is cancelled.
         */
        void write_handler(const boost::system::error_code ec,
                           const std::size_t bytes_transferred,
                           const std::weak_ptr<Watcher> awpWatcher,
						   AMQP::TcpConnection *const connection,
                           const int fd)
        {
            // Resolve any potential problems with dangling pointers
            // (remember we are using async).
            const std::shared_ptr<Watcher> apWatcher = awpWatcher.lock();
            if (!apWatcher) { return; }

            _write_pending = false;

            if ((!ec || ec == boost::asio::error::would_block) && _write)
            {
                connection->process(fd, AMQP::writable);

                _write_pending = true;

                _socket.async_write_some(
                    boost::asio::null_buffers(),
                    get_write_handler(connection, fd));
            }
        }

    public:
        /**
         *  Constructor- initialises the watcher and assigns the filedescriptor to 
         *  a boost socket for monitoring.
         *  @param  io_context      The boost io_context
         *  @param  wpstrand        A weak pointer to a io_context::strand instance.
         *  @param  fd              The filedescriptor being watched
         */
        Watcher(boost::asio::io_context &io_context,
                const int fd) :
            _ioservice(io_context),
            _socket(_ioservice)
        {
            _socket.assign(fd);

            _socket.non_blocking(true);
        }

        /**
         *  Watchers cannot be copied or moved
         *
         *  @param  that    The object to not move or copy
         */
        Watcher(Watcher &&that) = delete;
        Watcher(const Watcher &that) = delete;

        /**
         *  Destructor
         */
        ~Watcher()
        {
            _read = false;
            _write = false;
            _socket.release();
        }

        /**
         *  Change the events for which the filedescriptor is monitored
         *  @param  events
         */
        void events(AMQP::TcpConnection *connection, int fd, int events)
        {
            // 1. Handle reads?
            _read = ((events & AMQP::readable) != 0);

            // Read requsted but no read pending?
            if (_read && !_read_pending)
            {
                _read_pending = true;

                _socket.async_read_some(
                    boost::asio::null_buffers(),
                    get_read_handler(connection, fd));
            }

            // 2. Handle writes?
            _write = ((events & AMQP::writable) != 0);

            // Write requested but no write pending?
            if (_write && !_write_pending)
            {
                _write_pending = true;

                _socket.async_write_some(
                    boost::asio::null_buffers(),
                    get_write_handler(connection, fd));
            }
        }
    };

    /**
     *  Timer class to periodically fire a heartbeat
     */
    class Timer : public std::enable_shared_from_this<Timer>
    {
    private:

        /**
         *  The boost asio io_context which is responsible for detecting events.
         *  @var class boost::asio::io_context&
         */
        boost::asio::io_context & _ioservice;


        /**
         *  The boost asynchronous deadline timer.
         *  @var class boost::asio::deadline_timer
         */
        boost::asio::deadline_timer _timer;

        using handler_fn = boost::function<void(boost::system::error_code)>;

		/**
		* Binds and returns a lamba function handler for the io operation.
		* @param  connection   The connection being watched.
		* @param  timeout      The file descripter being watched.
		* @return handler callback
		*/
		handler_fn get_handler(AMQP::TcpConnection *const connection, const uint16_t timeout)
		{
			const auto fn = boost::bind(&Timer::timeout,
				this,
				_1,
				PTR_FROM_THIS(Timer),
				connection,
				timeout);

			return fn;
		}

        /**
         *  Callback method that is called by libev when the timer expires
         *  @param  ec          error code returned from loop
         *  @param  loop        The loop in which the event was triggered
         *  @param  connection
         *  @param  timeout
         */
        void timeout(const boost::system::error_code &ec,
                     std::weak_ptr<Timer> awpThis,
					 AMQP::TcpConnection *const connection,
                     const uint16_t timeout)
        {
            // Resolve any potential problems with dangling pointers
            // (remember we are using async).
            const std::shared_ptr<Timer> apTimer = awpThis.lock();
            if (!apTimer) { return; }

            if (!ec)
            {
                if (connection)
                {
                    // send the heartbeat
                    connection->heartbeat();
                }

                // Reschedule the timer for the future:
                _timer.expires_at(_timer.expires_at() + boost::posix_time::seconds(timeout));

                // Posts the timer event
                _timer.async_wait(get_handler(connection, timeout));
            }
        }

        /**
         *  Stop the timer
         */
        void stop()
        {
            // do nothing if it was never set
            _timer.cancel();
        }

    public:
        /**
         *  Constructor
         *  @param  io_context The boost asio io_context.
         *  @param  wpstrand   A weak pointer to a io_context::strand instance.
         */
        Timer(boost::asio::io_context &io_context) :
              _ioservice(io_context),
              _timer(_ioservice)
        {

        }

        /**
         *  Timers cannot be copied or moved
         *
         *  @param  that    The object to not move or copy
         */
        Timer(Timer &&that) = delete;
        Timer(const Timer &that) = delete;

        /**
         *  Destructor
         */
        ~Timer()
        {
            // stop the timer
            stop();
        }

        /**
         *  Change the expire time
         *  @param  connection
         *  @param  timeout
         */
        void set(AMQP::TcpConnection *connection, uint16_t timeout)
        {
            // stop timer in case it was already set
            stop();

            // Reschedule the timer for the future:
            _timer.expires_from_now(boost::posix_time::seconds(timeout));

            // Posts the timer event
            _timer.async_wait(get_handler(connection, timeout));
        }
    };

    /**
     *  The boost asio io_context.
     *  @var class boost::asio::io_context&
     */
    boost::asio::io_context & _ioservice;


	/*
	check connection is closed
	*/

	bool m_is_connected;

    /**
     *  All I/O watchers that are active, indexed by their filedescriptor
     *  @var std::map<int,Watcher>
     */
    std::map<int, std::shared_ptr<Watcher> > _watchers;

    /**
     * The boost asio io_context::deadline_timer managed pointer.
     * @var class std::shared_ptr<Timer>
     */
    std::shared_ptr<Timer> _timer;

    /**
     *  Method that is called by AMQP-CPP to register a filedescriptor for readability or writability
     *  @param  connection  The TCP connection object that is reporting
     *  @param  fd          The filedescriptor to be monitored
     *  @param  flags       Should the object be monitored for readability or writability?
     */
    void monitor(AMQP::TcpConnection *const connection,
                 const int fd,
                 const int flags) override
    {
        // do we already have this filedescriptor
        auto iter = _watchers.find(fd);

        // was it found?
        if (iter == _watchers.end())
        {
            // we did not yet have this watcher - but that is ok if no filedescriptor was registered
            if (flags == 0){ return; }

            // construct a new pair (watcher/timer), and put it in the map
            const std::shared_ptr<Watcher> apWatcher = 
                std::make_shared<Watcher>(_ioservice, fd);

            _watchers[fd] = apWatcher;

            // explicitly set the events to monitor
            apWatcher->events(connection, fd, flags);
        }
        else if (flags == 0)
        {
            // the watcher does already exist, but we no longer have to watch this watcher
            _watchers.erase(iter);
        }
        else
        {
            // Change the events on which to act.
            iter->second->events(connection,fd,flags);
        }
	
    }
	
protected:
    /**
     *  Method that is called when the heartbeat frequency is negotiated between the server and the client.
     *  @param  connection      The connection that suggested a heartbeat interval
     *  @param  interval        The suggested interval from the server
     *  @return uint16_t        The interval to use
     */
    virtual uint16_t onNegotiate(AMQP::TcpConnection *connection, uint16_t interval) override
    {
        // skip if no heartbeats are needed
        if (interval == 0) return 0;

        // set the timer
        _timer->set(connection, interval);

        // we agree with the interval
        return interval;
    }

	


public:

    /**
     *  Handler cannot be default constructed.
     *
     *  @param  that    The object to not move or copy
     */
    MqHandler() = delete;

    /**
     *  Constructor
     *  @param  io_context    The boost io_context to wrap
     */
    explicit MqHandler(boost::asio::io_context &io_context) :
        _ioservice(io_context),
		m_is_connected(false),
        _timer(std::make_shared<Timer>(_ioservice))
		
    {

    }

    /**
     *  Handler cannot be copied or moved
     *
     *  @param  that    The object to not move or copy
     */
    MqHandler(MqHandler &&that) = delete;
    MqHandler(const MqHandler &that) = delete;

    /**
     *  Returns a reference to the boost io_context object that is being used.
     *  @return The boost io_context object.
     */
    boost::asio::io_context &service()
    {
       return _ioservice;
    }

	bool is_connected() { return m_is_connected; }

	void close_mq()
	{
		_timer.reset();
		m_is_connected = false;
	}

    /**
     *  Destructor
     */
    ~MqHandler() override = default;

    /**
     * Make sure to stop the heartbeat timer after the connection is closed.
     * Otherwise it will keep the service running forever.
     */

    void onClosed(AMQP::TcpConnection* connection) override
    {
        (void)connection;
		close_mq();
    }
	void onError(AMQP::TcpConnection *connection, const char *message) override
	{
		(void)connection;
		close_mq();
	}

	void onConnected(AMQP::TcpConnection *connection) override
	{
		(void)connection;
		close_mq();
	}
};


/**
 *  End of namespace
 */
