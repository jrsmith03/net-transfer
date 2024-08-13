# net-transfer
Exploration into network sockets, TCP/IP, efficient file transfer methods and POSIX multithreading.

# Motivation
After my Operating Systems course, I wanted to practically apply the networking concepts we learned. I decided to use the Linux Sockets API to create a basic client-server application. I am currently exploring various applications for this paradigm and have primarily focused on creating a command-line interface for simple and fast file transfers. 

# Interface
Run the *server* application to launch the service. The first required argument is the **port number**, which can virtually be any number. The second is the **server mode**; currently, option 2 allows for multithreaded file transfers, but I plan on extending that in the future. Then, on a system in the same local network, launch the *client* application. You must first specify the **IPv4 address** of the server, then the **port number** which *must be the same as the server*, and finally the **application mode** which must also correspond to the server. For file transfers, type the path of the file or directory that you wish to copy.

# Key features
- Multithreaded server application supports and processes multiple client sessions concurrently.
- Memory-mapped I/O is used in the client application to greatly speed up file caching for transfer.
- Multithreaded client allows multiple files to be transferred concurrently (I want to later investigate the impact of this on performance)

# Issues / Future plans
This was a short project, but I may return later to improve the interface, add more customization/options, and potentially create security features such as authentication. Moreover, I feel that some form of encryption (so that only encrypted data is being sent over the network) may make this a more viable real-world application.

## Disclaimer
This code is for experimental and educational purposes only. I have not designed it with security or safety in mind. It has not seen the testing that would make me endorse it for any production use. I am therefore not responsible for any losses incurred by using my codebase.
