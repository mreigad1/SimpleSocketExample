# SimpleSocketExample
This project demonstrates a simple *nix chat application driven via client-server socket connection. All code is written in C.

	Getting Started:
		Easy Execution:

			Server Steps:

				Execute 'make all', this will compile all relevant applications and will launch 3 separate gnome-terminal windows to execute each
				of the 3 relevant system processes. By default this will generate a randomized user name and will launch a client chat instance as
				as well as the server instance.

				To deploy only server application or if gnome-terminal is unsupported, execute instead with:

					(1) make server
					(2) ./server

			Client Steps:

				(1) Execute "./setServerIP.sh [SERVER_IPv4]" to set local configuration file to target server at address SERVER_IPv4

				(2) Execute "./setUser.sh [USER_NAME]" to set local configuration file with desired user name (Undefined behavior for user name with spaces)

				(3) Execute "make client" to build client applications

				(4) Execute "make clientDeployment" to deploy client applications with settings from configuration files

				To deploy only client applications or if gnome-terminal is unsupported, executed instead with:

					(i)   Execute "make client" to build applications

					(ii)  In shell window 'A' excute "./clientSocket [BUFFER_FILE] [SERVER_IPv4]" where BUFFER_FILE is a temporary file to be
					      created by the program for inotify and SERVER_IPv4 is the IPv4 of the target hosting server

					(iii) In shell window 'B' execute "./clientInput [BUFFER_FILE] [USER_NAME]" where BUFFER_FILE is the temporary file to be
					      shared with clientSocket via inotify and USER_NAME is the desired user name of this client instance
					      (Undefined behavior for user name with spaces)

					      NOTE: By default, the clientInput will prepend the user name of the system user to the
					            contents of the exported file used by inotify. HOWEVER, the clientSocket application
					            is completely agnostic of user name, and completely agnostic of existence of
					            clientInput's existence. The file specified to clientSocket need only be changed for
					            inotify to automatically buffer the new file contents and export them. This makes
					            execution of clientInput completely optional, as you can just echo into BUFFER_FILE
					            and changes will be sent. It is worth note, only (BUF_SIZE-1) characters at maximum
					            will be read to/from inotify and/or the socket connection. This value exists defined
					            in universe.h and may be reconfigured, but will rapidly exceed pthreads thread
					            stack-space if too large, as all read+write buffers in program space are kept on thread
					            stacks.

			Known Issues:

				(1) Broken pipe in server on first terminated client application causes server application to
				    terminate and close connection with all clients.

				(2) Rapid input in clientInput application can cause inotify updates to be dropped resulting in dropped messages
				    (issue in inotify driver code, non-issue in socket connection).

				(3) Have not yet confirmed client-server connections function across internet, have only confirmed local network
				    client-server functionality.

			Bonus Features:

				(1) Purely asynchronous architecture

				(2) Clients are each given read thread during reads, each given additional write thread during writes, primary thread drives accept loop.

				(3) No memory verified memory leaks in design

				(4) Modular design

				(5) Easily configurable

		This project comprises 3 applications defined as follows:

			clientInput:
				This application when launched from the shell will accept user input to be buffered and sent out to the server.
				
				DIRECTIONS:

					Direct Build with:
						make clientInput

					Execute with:
						./clientInput [BUFFER_FILE] [USER_NAME]

					Arguments:

						BUFFER_FILE: A file to be written to by the user input application.
									 This file will be watched for changes by the clientSocket application (which will write changes).

						USER_NAME: A user name which will be displayed for user's messages on server

					NOTE: By default, the clientInput will prepend the user name of the system user to the
					      contents of the exported file used by inotify. HOWEVER, the clientSocket application
					      is completely agnostic of user name, and completely agnostic of existence of
					      clientInput's existence. The file specified to clientSocket need only be changed for
					      inotify to automatically buffer the new file contents and export them. This makes
					      execution of clientInput completely optional, as you can just echo into BUFFER_FILE
					      and changes will be sent. It is worth note, only (BUF_SIZE-1) characters at maximum
					      will be read to/from inotify and/or the socket connection. This value exists defined
					      in universe.h and may be reconfigured, but will rapidly exceed pthreads thread
					      stack-space if too large, as all read+write buffers in program space are kept on thread
					      stacks.

			clientSocket:
				This application when launched will watch the file written to by the clientInput process [with inotify] and will
				push changes in the content of this file through a socket connection established to the server to display as message text.
				This process also receives input from server socket and displays for user output.

				DIRECTIONS:

					Direct Build with:
						make clientSocket

					Execute with:
						./clientSocket [BUFFER_FILE] [SERVER_IPv4]

					Arguments:

						BUFFER_FILE: A file watched for new messages to be shared and coordinated with the clientInput.
									 On close for writing, the contexts of this file are pushed through the socket to the server.

						SERVER_IPv4: IPv4 address of server hosting chat service application

			server:
				This application when launched will accept connections with clients. Data will be read from clients asynchronously
				and will be written to clients asynchronously. All threading is manage with POSIX compliant threading API's and will
				lock critical sections appropriately to ensure client connections are not leaked or improperly buffered. Will listen
				on port 1234 by default. To change this port, reconfigure macro SERVER_PORT_NO in universe.h before building
				client or server applications

				DIRECTIONS:

					Direct Build with:
						make server

					Execute with:
						./server

					Arguments:
						None.

