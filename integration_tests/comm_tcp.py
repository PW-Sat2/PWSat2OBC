import SocketServer
import threading
import time


class CommServerHandler(SocketServer.BaseRequestHandler):
    def setup(self):
        self.server.add_client(self)

    def handle(self):
        while True:
            length = ord(self.request.recv(1))

            data = self.request.recv(length)

            print "Req length: %d: %s" % (length, data)

    def finish(self):
        self.server.remove_client(self)

    def send(self, data):
        self.request.sendall(data)


class CommServer(SocketServer.ThreadingTCPServer):
    def __init__(self, server_address=None):
        SocketServer.ThreadingTCPServer.__init__(self, server_address, CommServerHandler)
        self.clients = set()

    def add_client(self, client):
        self.clients.add(client)

    def remove_client(self, client):
        self.clients.remove(client)

    def broadcast(self, data):
        for c in self.clients:
            c.send(data)


def comm_source(server):
    while True:
        print "a"
        server.broadcast("a")
        time.sleep(1)

server = CommServer(("127.0.0.1", 1234))

t = threading.Thread(target=comm_source, args=(server,))
t.start()

server.serve_forever()
