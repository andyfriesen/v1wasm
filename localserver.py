#!/usr/bin/env python
try:
    import http.server as SimpleHTTPServer
except:
    import SimpleHTTPServer

try:
    import socketserver as SocketServer
except:
    import SocketServer

PORT = 8000

class Handler(SimpleHTTPServer.SimpleHTTPRequestHandler):
    pass

Handler.extensions_map['.wasm'] = 'application/wasm'

httpd = SocketServer.TCPServer(("", PORT), Handler)

print("serving at port {0}".format(PORT))
httpd.serve_forever()