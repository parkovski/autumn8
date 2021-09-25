# Architecture

## A8Gfx

This describes a basic uWebSockets/WebView app.

The program starts in `a8main`, unless it is a DLL such as the FL plugin.
This thread is called Main. While not required, this thread probably will
become the main event loop thread by calling `runEventLoop`.

All GUI activity should happen on this thread. `Window` contains a function
to post a callback to this thread if needed.

Messages are received on a separate thread(s?). This is aptly called the
Message thread. It probably should be made into a thread pool tbh, then you
can do whatever processing you want on it. Also should add SystemEvent
filtering and some UserEvent and CallbackEvent (not subscribable - just runs
a callback on this thread).

The web server also runs on its own thread. It can't be posted to, you'd have
to make a web request.

I guess what this would look like ideally is:
```c++
auto messageThreadPool = ...
us_listen_socket_t *listenSocket;
auto serverThread = ...
auto window = new Window;
window->connect<SystemEvent::Close>([](...){
  window->quitOnClose(); // Quits the event loop on the main thread.
  window->close();
});

int exitCode = runEventLoop();
us_close_socket(listenSocket);
serverThread.join();
// At this point everything generating messages should be gone.
messageThreadPool.shutdown();
messageThreadPool.joinAll();
return exitCode;
```

Maybe it'd be better to track any open top level windows and provide a
`quitOnLastWindowClosed` function, or even a `SystemEvent::AllWindowsClosed`.

## A8FLP

FL Studio runs some of its own threads and for performance reasons some things
will run on its threads.

It also manages the Window event loop, so we don't want to post quit messages
there. Our shutdown stuff will also have to happen at a different point.