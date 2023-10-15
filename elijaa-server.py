from xmlrpc.server import SimpleXMLRPCServer

def sapaElijaa():
    return 'halo elijaa'

server = SimpleXMLRPCServer(("10.0.2.15", 8000))
print("Listening on port 8000")
server.register_function(sapaElijaa)
server.serve_forever()