import xmlrpc.client

proxy = xmlrpc.client.ServerProxy("http://10.0.2.15:8000/")

sapaElijaa = proxy.sapaElijaa()

print(sapaElijaa)
