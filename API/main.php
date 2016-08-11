<?php
	include_once "Cmd.php";
	$data = null;
	switch ($_POST["task"])
	{
		case "solve":
			
			//recieved data from client
			$txt = $_POST["data"];
			
			//generate random number for file name for each client
			$num = mt_rand().".sm";
	
			//create the file
			$myfile = fopen($num, "w");
			
			//wright data recieved in he file created
			for($i=0; $i<sizeof($txt); $i++){
				fwrite($myfile, $txt[$i]."\n");
			}
			
			//close the file
			fclose($myfile);
			
			//run clingo command from class Cmd and get result set in a variable for send to client
			$data = Cmd::solve($num);
			
			//delete file
			unlink($num);
			break;

		default:
			break;
	}

	//return result data to client in json format
	echo json_encode($data); 
?>