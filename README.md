# Img Server

I have created this server just for fun. This is not an HTTP server : only the GET HTTP request is supported and the HTML pages are generated automatically.

This program is free software: you can rededistribute and/or modify it under the terms of the GNU General Public License.

### How does it work ?

If you send a request to this server (from a browser), it will scan the `images` directory, create an HTML page and send it to the client. So, if you put some images in this directory, you make them accessible everywhere in the network. You can also put plain text, etc.  
For security, you can't access to the hidden directorys (if you try, you'll get an "Unauthorized" page).

By default, the server works with the port 776. For example, if the server is running on a machine with IP "192.168.1.100", you'll need to type in your browser `http://192.168.1.100:776` to access to your images. If you don't want to need to write `:776` at the end of the adress, use the port 80 (you can change in the begin of the main.c file, then compile again the server).

### How to compile it ?

Write these commands in a console :  
`git clone https://github.com/paly2/Img-Server`  
`gcc *.c -o 'img-server'`  
`sudo ./img-server`
