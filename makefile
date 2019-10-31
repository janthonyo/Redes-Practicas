#main: domino.cc mainPruebas.cc
#	g++ domino.cc mainPruebas.cc -o prueba

main: server-v2.cc clienteOriginal.cc
	g++ server-v2.cc message.cc -o servidor
	g++ cliente.cc -o cliente
