Project: Nurikabe puzzle solving using answer set programming with web based user interface.
Engineer: Omar Rodriguez Santiago	
Professor: Dr. Richard Watson

Install:

Os: Ubuntu 16, can work as well on mac OS.
Web server: Apache, latest version.

Unzip project folder “ASP” and copy folder in the apache /www/hmtl/ directory.

After copying the folder in the directory give the permission using chmod 777 /ASP for clingo and mkatoms command

Running:

Go for web browser and type in address bar: IP_Address/ASP.

Demo:

http://107.170.56.239/ASP/

Project description:

We solve Nurikabe puzzle using answer set programming. To make the answer set result understandable we made a user interface using HTM and Bootstrap css framework. 
The User interfacer allows the user to enter the input of the puzzle. In the front end of the application the system use Javascript and Jquery to collect the input from the user. 
Then the system use AJAX to send the user data input to the PHP services. In the backend the PHP services creates input file with received data from the client then run clingo and mkatoms for the answer set programing. 
Finally, php send answer set of puzzle solution back to the client using json format. Client receive solution and update the grid to show the results.
