# Mailing_App

- Setting up the server:

To set up the database, we need to first create a SQL server instance. You can use any framework you wish, but in this file we will use MySQL for exemplification.

1. If you don't have any SQL framework in your computer, you can download mySQL for free in :
	https://dev.mysql.com/downloads/installer/
2. Open mySQL workbench and create a local instance. To do so, press the "add" button and set 'localhost' as host and '3306' as port.
3. Once the instance has been properly initialized, click on the newly appeared tab with the corresponding name.
4. Once inside the server instance, click "Instance- Start /Shutdown" in the left panel.
5. Wait, and you should see a message telling you the server is running.
6. Now, we have to load our database. Click on "Data import/Restore" on the left tab. Then, select "Import from self-contained file" in import options and browse to the file included in the release called 'mailing_app_database.sql'. Then, set the default schema to 'mailing_app' and click "Start import" on the lower right corner.
7. Check if the database has been imported by looking at the "schemas" section of the left panel. If it is, make sure the server is currently running before executing the program.

- Menus and controls

Once executed, the app will first try to connect to the server, so you should see the message "Connecting...". If the app is unable to connect to the server, make sure you have followed the previous section steps correctly.

Once connected, you will reach the login menu. Here you must enter a username and password to enter the chat room. If you have not yet registered in the app, click 'Sign in' instead of 'Login'. In this menu you can also access the connection configuration, which contains the connected IP and port and an option to simulate the database. Do not change the default IP or port unless you using this code for your own purposes.

Once logged, you will reach the chat room menu. At the upper left corner of the screen you can see online and offline users, and below the chat tab, which will be empty until a user has been selected by clicking on it.

Once a user has been selected, the chat tab will load all the whole chat between you and the selected user. Use the input field below the chat tab to write messages, and press the button "Send" or press enter to send them.

- Notes

The first client to connect to the app will be the server host, which is displayed as the text 'hosting server' on the upper left screen and the server log tab which will appear under the client log tab. 

If you enable the "simulate database" option, the app will work locally so no information will be sent to the database. This means that the changes you made will not be saved and will be lost upon closing.

In both real and simulated databases, the users "client1" to "client5" are created by default. You can login as one of them or sign in with your own user. The password for all default created users is '12345'.

- Features:

	- Almost instant messaging service
	- Displays if an user is online or if it's writing
	- Messages contain timestamps
	- Label to mark read messages.
	- Connection and Writing ping to provide accurate info 	  even with connection problems.
	- Structured and editable UI for clear understanding. 